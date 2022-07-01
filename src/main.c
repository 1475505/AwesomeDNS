/*
设计一个DNS服务器程序，读入“域名-IP地址”对照表（一个文件）
当客户端查询域名对应的IP地址时，用域名检索该对照表，得到三种检索结果：
- 检索结果为ip地址0.0.0.0，则向客户端返回“域名不存在”的报错（不良网站拦截）
- 检索结果为普通IP地址，则向客户返回这个地址（服务器功能）
- 表中未检到该域名，则向因特网DNS服务器发出查询，并将结果返给客户端（中继功能）
     考虑多个计算机上的客户端会同时查询，需要进行消息ID的转换
*/
#define DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "Socket.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include "config.h"
#include "DNS.h"
#include "utils.h"
#include <errno.h>

#define MAXLINE 512
#define SERV_PORT 53


extern int debug_info;
extern char serverName[16];
extern char configFile[64];

extern void initTrie();
extern void insertTrie(char * domain, int32_t ip, int32_t ttl);

void DNS_process(char* buf, int len);
void DNS_process_test(char* buf, int len);

int main(int argc, char* argv[]) {
    config(argc, argv);
#ifdef DEBUG
    debug_info = 2;
#endif
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int sockfd;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    int i, n;

    initTrie();
    FILE * fp;
    if(fp = fopen("dnsrelay.txt", "r"))
    {
        char ipstr[15], name[256];
        while (fscanf(fp, "%s %s", ipstr, name) != EOF)
        {
            uint32_t ip = inet_addr(ipstr);
            insertTrie(name, ntohl(ip), 255);
            /* code */
            memset(ipstr, 0, 15);
            memset(name, 0, 256);
        }
    }
    else
    {
        printf("open fail errno = %d reason = %s \n", errno, strerror(errno));
    }
    fclose(fp);

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    printf("Accepting connections ...\n");
    while (1) {
        cliaddr_len = sizeof(cliaddr);
        n = recvfrom(sockfd, buf, MAXLINE, 0, (struct sockaddr*)&cliaddr,
                     &cliaddr_len);
        if (n == -1)
            perr_exit("recvfrom error");
        inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str));
        log(0, "[serving]%s:%d...\n", &str, ntohs(cliaddr.sin_port));

        log(2, "%s\n", buf);

        
        DNS_process(buf, n);  // You can use _test to test connection.

        n = sendto(sockfd, buf, n, 0, (struct sockaddr*)&cliaddr,
                   sizeof(cliaddr));

        // LOG(2, strcat("Send:", buf));
        if (n == -1)
            perr_exit("sendto error");
    }

    return 0;
}

void DNS_process(char* buf, int len) {
// #ifdef DEBUG
//     assert(sizeof(dnsHeader) == 12);
// #endif
    // Qsection q[dnsHeader.QDcount];
    // RRformat rr_q[dnsHeader.ANcount];
    // RRformat rr_auth[dnsHeader.NScount];
    // RRformat rr_add[dnsHeader.ARcount];
    DNS dns;
    size_t bias;
    dns.header = (DNSHeader *)buf;
    log(2, "get DNS header: QDcount %d", dns.header->QDcount);
    dns.header->QDcount = ntohs(dns.header->QDcount);
    dns.header->ANcount = ntohs(dns.header->ANcount);
    dns.header->ARcount = ntohs(dns.header->ARcount);
    dns.header->NScount = ntohs(dns.header->NScount);
    log(2, "get DNS header: QDcount %d", dns.header->QDcount);
    dns.question = (Qsection*)malloc(dns.header->QDcount * sizeof(Qsection));
    bias = readQuestions(buf, dns.question, dns.header->QDcount);
    dns.answer = (RRformat*)malloc(dns.header->ANcount * sizeof(RRformat));
    bias = readRRs(buf, dns.answer, dns.header->ANcount, bias);
    dns.authority = (RRformat*)malloc(dns.header->NScount * sizeof(RRformat));
    bias = readRRs(buf, dns.authority, dns.header->NScount, bias);
    dns.additional = (RRformat*)malloc(dns.header->ARcount * sizeof(RRformat));
    bias = readRRs(buf, dns.additional, dns.header->ARcount, bias);
#ifdef DEBUG
    assert(sizeof(dns) >= 12);
#endif

    if(dns.header->qr == 0)//if it receives from client
    {
        for (int i = 0; i < dns.header->QDcount; i++) {
            char url[128];
            size_t offset;
            getURL(dns.question[i].Qname, url, &offset);  //(BUG)
            switch (dns.question[i].Qtype) {     // todo
                case 0:
                    //todo
                    break;
                case 2:
                    // fallthrough
                case 1:
                    dns.answer[i].RDlength = 4;
                    uint16_t data[2];
                    uint8_t found = 0;
                    uint32_t ip = findIP(url, &found);
                    if (ip == 0) dns.header->rcode = 3;
                    //if (!found) connectCloudDNS();
                    memcpy(data, &ip, sizeof data);
                    dns.answer[i].Rdata = data;
                    break;
                case 5:
                    dns.answer[i].Rdata = url;  // todo: should return 别名
                    break;
                default:
                    break;
            }
            // TODO: should wrap, not epoll.
        }
    }
    else//it receives from server
    {
        //todo
    }
}

void DNS_process_test(char* buf, int len) {
    for (int i = 0; i < len; i++)
        buf[i] = toupper(buf[i]);
}