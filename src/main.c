/*
设计一个DNS服务器程序，读入“域名-IP地址”对照表（一个文件）
当客户端查询域名对应的IP地址时，用域名检索该对照表，得到三种检索结果：
- 检索结果为ip地址0.0.0.0，则向客户端返回“域名不存在”的报错（不良网站拦截）
- 检索结果为普通IP地址，则向客户返回这个地址（服务器功能）
- 表中未检到该域名，则向因特网DNS服务器发出查询，并将结果返给客户端（中继功能）
     考虑多个计算机上的客户端会同时查询，需要进行消息ID的转换
*/
#include "connect.h"
#include <sys/types.h>
#define DEBUG
#include "DNS.h"
#include "Socket.h"
#include "config.h"
#include "utils.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXLINE 512
#define SERV_PORT 53

extern int debug_info;
extern char serverName[16];
extern char configFile[64];
extern Request requests[REQ_SZLIMIT];

int DNS_process(char *buf, ssize_t len);
void DNS_process_test(char *buf, int len);

int main(int argc, char *argv[]) {
  config(argc, argv);
#ifdef DEBUG
  debug_info = 3;
//   freopen("log", "w", stdout);
#endif
  struct sockaddr_in servaddr, cliaddr;
  socklen_t cliaddr_len;
  int sockfd;
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
      /* code */
      memset(ipstr, 0, 15);
      memset(name, 0, 256);
    }
  } else {
    printf("open fail errno = %d reason = %s \n", errno, strerror(errno));
  }
  fclose(fp);

  sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);

  Bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

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

  return 0;
}

int DNS_process(char *buf, ssize_t len) {
  // #ifdef DEBUG
  //     assert(sizeof(dnsHeader) == 12);
  // #endif
  // Qsection q[dnsHeader.QDcount];
  // RRformat rr_q[dnsHeader.ANcount];
  // RRformat rr_auth[dnsHeader.NScount];
  // RRformat rr_add[dnsHeader.ARcount];
  DNS dns;
  size_t bias;
parse:
  dns.header = (DNSHeader *)buf;
  log(2, "get DNS header: QDcount %d, ANcount %d, NScount %d, ARcount %d\n",
      ntohs(dns.header->QDcount), ntohs(dns.header->ANcount),
      ntohs(dns.header->NScount), ntohs(dns.header->ARcount));
  dns.question =
      (Qsection *)malloc(ntohs(dns.header->QDcount) * sizeof(Qsection));
  bias = readQuestions(buf, dns.question, ntohs(dns.header->QDcount));
  // dns.answer = (RRformat*)malloc(ntohs(dns.header->ANcount) *
  // sizeof(RRformat)); bias = readRRs(buf, dns.answer,
  // ntohs(dns.header->ANcount), bias); dns.authority =
  // (RRformat*)malloc(ntohs(dns.header->NScount) * sizeof(RRformat)); bias =
  // readRRs(buf, dns.authority, ntohs(dns.header->NScount), bias);
  // dns.additional = (RRformat*)malloc(ntohs(dns.header->ARcount) *
  // sizeof(RRformat)); bias = readRRs(buf, dns.additional,
  // ntohs(dns.header->ARcount), bias);
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
        if (!found) {
          dns.header->ra = 1;
          dns.header->ID = connectCloudDNS(dns);
          struct sockaddr_in servaddr;
          int n;
          char str[INET_ADDRSTRLEN];
          socklen_t servaddr_len;

          int csockfd = Socket(AF_INET, SOCK_DGRAM, 0);
          bzero(&servaddr, sizeof(servaddr));
          servaddr.sin_family = AF_INET;
          inet_pton(AF_INET, serverName, &servaddr.sin_addr);
          servaddr.sin_port = htons(53);

          assert(dns.header->z == 0);
          n = sendto(csockfd, buf, len, 0, (struct sockaddr *)&servaddr,
                     sizeof(servaddr));
          if (n == -1) {
            perr_exit("sendto error");
            requests[dns.header->ID].used = 0;
          }
          n = recvfrom(csockfd, buf, 512, 0, NULL, 0);
          if (n == -1)
            perr_exit("recvfrom error");

          // log(2, "begin processing server-message of %d B\n", len);
          // if (dns.header->opcode == 0 && ntohs(dns.header->QDcount) == 1 &&
          //     ntohs(dns.answer->type) == 1) {
          //   addCache(dns.question->Qname, ntohl(dns.answer->Rdata),
          //            ntohl(dns.answer->TTL));
          // }
          // uint16_t idServer = dns.header->ID;
          // struct sockaddr_in cliAddr;
          // cliAddr.sin_family = AF_INET;
          // uint16_t clientId = requests[idServer].id;
          // cliAddr.sin_addr.s_addr = htonl(requests[idServer].ip);
          // cliAddr.sin_port = htonl(requests[idServer].port);
          // requests[idServer].used = 0;
          // log(2, "receive the response %d -> %d", idServer, clientId);
          // ip = findIP(dns.question->Qname, &found, &dns);
          // assert(found);
          Close(csockfd);
          goto parse;
        }
        if (ip == 0)
          dns.header->rcode = 3;
        else {
          dns.header->ra = 1;
          dns.header->rcode = 0;
          dns.header->qr = 1;
          dns.header->ANcount = htons(1);
          dns.answer->name = dns.question->Qname;
          dns.answer->type = 1;
          dns.answer->clas = 1;
          dns.answer->RDlength = 4;
          // memcpy(buf + bias, (char *)dns.answer, sizeof(RRformat));//bug
          writeAN(buf + bias, dns);
          memset(buf + bias + strlen(dns.question->Qname) + 16, 0,
                 MAXLINE - (bias + strlen(dns.question->Qname) + 16));
          len += sizeof(RRformat) - sizeof(char *) +
                 strlen(dns.question->Qname) + 2;
        }
        return len;
        break;
      case 28: // ipv6
        break;
      case 5:
        dns.answer->Rdata = url; // todo: should return 别名
        break;
      default:
        break;
      }
      // TODO: should wrap, not epoll.
    }
  } else // it receives from server
  {
    log(2, "begin processing server-message of %d B\n", len);
    if (dns.header->opcode == 0 && ntohs(dns.header->QDcount) == 1 &&
        ntohs(dns.answer->type) == 1) {
      addCache(dns.question->Qname, ntohl(dns.answer->Rdata),
               ntohl(dns.answer->TTL));
    }
    uint16_t idServer = dns.header->ID;
    struct sockaddr_in cliAddr;
    cliAddr.sin_family = AF_INET;
    uint16_t clientId = requests[idServer].id;
    cliAddr.sin_addr.s_addr = htonl(requests[idServer].ip);
    cliAddr.sin_port = htonl(requests[idServer].port);
    requests[idServer].used = 0;
    log(2, "receive the response %d -> %d", idServer, clientId);
    return 0; // need to send?
    // not finished yet
  }
  return len;
}

void DNS_process_test(char *buf, int len) {
  for (int i = 0; i < len; i++)
    buf[i] = toupper(buf[i]);
}