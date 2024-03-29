/*
设计一个DNS服务器程序，读入“域名-IP地址”对照表（一个文件）
当客户端查询域名对应的IP地址时，用域名检索该对照表，得到三种检索结果：
- 检索结果为ip地址0.0.0.0，则向客户端返回“域名不存在”的报错（不良网站拦截）
- 检索结果为普通IP地址，则向客户返回这个地址（服务器功能）
- 表中未检到该域名，则向因特网DNS服务器发出查询，并将结果返给客户端（中继功能）
     考虑多个计算机上的客户端会同时查询，需要进行消息ID的转换
*/
// #define DEBUG
#include "DNS.h"
#include "Socket.h"
#include "config.h"
#include "connect.h"
#include "utils.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <WinSock2.h>

WSADATA wsaData;
extern SOCKET Sock;
#elif __linux__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

extern int sockfd;

static int Sock;
#endif

#define MAXLINE 512
#define SERV_PORT 53

extern int debug_info;
extern char serverName[16];
extern char configFile[64];
extern Request requests[REQ_SZLIMIT];

struct sockaddr_in servaddr, cliaddr;
socklen_t cliaddr_len;

int DNS_process(char *buf, ssize_t len);
void DNS_process_test(char *buf, int len);

int main(int argc, char *argv[]) {
  config(argc, argv);
#ifdef DEBUG
  debug_info = 3;
//   freopen("log", "w", stdout);
#endif
  char buf[MAXLINE];
  char str[INET_ADDRSTRLEN];
  int i, n;
  printf("Current configFile: %s\n", configFile);
  initTrie();
  initCache();
  FILE *fp;
  if (fp = fopen(configFile, "r")) {
    char ipstr[15], name[256];
    while (fscanf(fp, "%s %s", ipstr, name) != EOF) {
      uint32_t ip = inet_addr(ipstr);
      insertTrie(name, ntohl(ip), 86400);
      memset(ipstr, 0, 15);
      memset(name, 0, 256);
    }
  } else {
    printf("open fail errno = %d reason = %s \n", errno, strerror(errno));
  }
  fclose(fp);

#ifdef _WIN32
  int status = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (status != 0) {
    log(0, "Windows Socket DLL Error\n");
    exit(-1);
  }
#endif

  sockfd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(serverName);
  servaddr.sin_port = htons(SERV_PORT);

  memset(&cliaddr, 0, sizeof(struct sockaddr_in));
  cliaddr.sin_family = AF_INET;
  cliaddr.sin_addr.s_addr = INADDR_ANY;
  cliaddr.sin_port = htons(SERV_PORT);

  const int REUSE = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&REUSE,
             sizeof(REUSE));

  Bind(sockfd, (struct sockaddr *)&cliaddr, sizeof(servaddr));

  printf("Accepting connections ...\n");
  while (1) {
    cliaddr_len = sizeof(cliaddr);
    n = recvfrom(sockfd, buf, MAXLINE, 0, (struct sockaddr *)&cliaddr,
                 &cliaddr_len);
    if (n == -1)
      perr_exit("recvfrom error");
    inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str));
    log(0, "[serving]%s:%d...\n", &str, ntohs(cliaddr.sin_port));

    // log(3, "%s\n", buf);

    int respn = DNS_process(buf, n); // You can use _test to test connection.
    log(3, "returning to client of %d B\n", respn);

    if (respn)
      n = sendto(sockfd, buf, respn, 0, (struct sockaddr *)&cliaddr,
                 sizeof(cliaddr));
    if (n == -1)
      perr_exit("sendto error");
  }

#ifdef _WIN32
  WSACleanup();
#endif

  return 0;
}

int DNS_process(char *buf, ssize_t len) {
  // #ifdef DEBUG
  //     assert(sizeof(dnsHeader) == 12);
  // #endif
  DNS dns;
  size_t bias;
  dns.header = (DNSHeader *)buf;
  log(2, "get DNS header: QDcount %d, ANcount %d, NScount %d, ARcount %d\n",
      ntohs(dns.header->QDcount), ntohs(dns.header->ANcount),
      ntohs(dns.header->NScount), ntohs(dns.header->ARcount));
  dns.question =
      (Qsection *)malloc(ntohs(dns.header->QDcount) * sizeof(Qsection));
  bias = readQuestions(buf, dns.question, ntohs(dns.header->QDcount));
#ifdef DEBUG
  assert(sizeof(dns) >= 12);
#endif
  if (dns.header->qr == 0) // if it receives from client
  {
    log(2, "start processing client-message of %d B\n", len);
    if (dns.header->opcode == 0 && ntohs(dns.header->QDcount) == 1) {

      char url[128];
      uint16_t data[2];
      uint8_t found = 1;
      uint32_t ip;
      switch (dns.question->Qtype) { // todo
      case 2:
        // fallthrough
      case 1: // ipv4
        ip = findIP(dns.question->Qname, &found, &dns);
        if (found) {
          if (ip == 0) {
            dns.header->ra = 1;
            dns.header->rcode = 3;
            dns.header->qr = 1;
          } else {
            dns.header->ra = 1;
            dns.answer->name = dns.question->Qname;
            dns.header->rcode = 0;
            dns.header->qr = 1;
            dns.header->ANcount = htons(1);
            dns.answer->type = 1;
            dns.answer->clas = 1;
            dns.answer->RDlength = 4;
            writeAN(buf + bias, dns);
            len += sizeof(RRformat) - sizeof(char *) +
                   strlen(dns.question->Qname) + 2;
          }
        } else {
          dns.header->ra = 1;
          dns.header->ID = connectCloudDNS(dns);
          assert(dns.header->z == 0);
          ssize_t n = sendto(sockfd, buf, len, 0, (struct sockaddr *)&servaddr,
                             sizeof(servaddr));
          if (n == -1) {
            perr_exit("sendto error");
            requests[dns.header->ID].used = 0;
          }
          free(dns.question->Qname);
          free(dns.question);
          return 0;
        }
        return len;
        break;
      case 28: // ipv6, todo
        break;
      default:
        break;
      }
      // TODO: should wrap, not epoll.
    }
  } else if (ntohs(dns.header->ANcount) == 1) // it receives from server
  {
    dns.answer =
        (RRformat *)malloc(ntohs(dns.header->ANcount) * sizeof(RRformat));
    bias = readRRs(buf, dns.answer, ntohs(dns.header->ANcount), bias);
    log(2, "begin processing server-message of %d B\n", len);
    if (dns.header->opcode == 0 && ntohs(dns.header->QDcount) == 1 &&
        ntohs(dns.answer->type) == 1) {
      addCache(dns.question->Qname, dns.answer->Rdata, dns.answer->TTL);
    }
    uint16_t idServer = dns.header->ID;
    struct sockaddr_in cliAddr;
    cliAddr.sin_family = AF_INET;
    uint16_t clientId = requests[idServer].id;
    cliAddr.sin_addr.s_addr = htonl(requests[idServer].ip);
    cliAddr.sin_port = htonl(requests[idServer].port);
    requests[idServer].used = 0;
    log(2, "receive the response %d -> %d", idServer, clientId);
    free(dns.answer->name);
    free(dns.question->Qname);
    free(dns.question);
    free(dns.answer);
    return len; // need to send back
  } else
    return 0;
  // free(dns.answer);
  free(dns.question->Qname);
  free(dns.question);
  return 0;
}

void DNS_process_test(char *buf, int len) { // just for developers' testing
  for (int i = 0; i < len; i++)
    buf[i] = toupper(buf[i]);
}