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
#include <assert.h>
#include "config.h"
#include "DNS.h"
#include "utils.h"

#define MAXLINE 512
#define SERV_PORT 53


extern int debug_info;
extern char serverName[16];
extern char configFile[64];

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
        printf("[serving]%s:%d...\n", &str, ntohs(cliaddr.sin_port));

        LOG(2, "%s\n", buf);

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
    DNSHeader dnsHeader;
    memcpy(&dnsHeader, buf, sizeof dnsHeader);
    // dnsHeader->info;
#ifdef DEBUG
    assert(sizeof(dnsHeader) == 12);
#endif
    // Qsection q[dnsHeader.QDcount];
    // RRformat rr_q[dnsHeader.ANcount];
    // RRformat rr_auth[dnsHeader.NScount];
    // RRformat rr_add[dnsHeader.ARcount];
    DNS dns;
    dns.header = dnsHeader;
    dns.question = (Qsection*)malloc(dnsHeader.QDcount * sizeof(Qsection));
    dns.answer = (RRformat*)malloc(dnsHeader.ANcount * sizeof(RRformat));
    dns.authority = (RRformat*)malloc(dnsHeader.NScount * sizeof(RRformat));
    dns.additional = (RRformat*)malloc(dnsHeader.ARcount * sizeof(RRformat));
    memcpy(&dns, buf, sizeof dns);
#ifdef DEBUG
    assert(sizeof(dns) >= 12);
#endif
    DNS DNSresp;
    DNSHeader dnsrespHeader = dnsHeader;
    dnsrespHeader.info |= (0x8000);
    DNSresp.header = dnsrespHeader;
    DNSresp.question = (Qsection*)malloc(dnsHeader.QDcount * sizeof(Qsection));
    DNSresp.answer = (RRformat*)malloc(dnsHeader.ANcount * sizeof(RRformat));
    DNSresp.authority = (RRformat*)malloc(dnsHeader.NScount * sizeof(RRformat));
    DNSresp.additional =
        (RRformat*)malloc(dnsHeader.ARcount * sizeof(RRformat));

    for (int i = 0; i < dnsHeader.QDcount; i++) {
        char url[128];
        getURL(dns.question[i].Qname, url);  //(BUG)
        switch (dns.question[i].Qtype) {     // todo
            case 2:
                // fallthrough
            case 1:
                dns.answer[i].RDlength = 4;
                uint16_t data[2];
                uint8_t found = 0;
                uint32_t ip = findIP(url, &found);
                if (ip == 0) dnsrespHeader.info |= 3;
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
        DNSresp.answer[i].name = dns.question[i].Qname;
        DNSresp.answer[i].type = dns.question[i].Qtype;
        DNSresp.answer[i].class = 1;  // for Internet. Fixed.
        DNSresp.answer[i].TTL = 2;    // I guess
    }
    memcpy(buf, &DNSresp, sizeof DNSresp);
}

void DNS_process_test(char* buf, int len) {
    for (int i = 0; i < len; i++)
        buf[i] = toupper(buf[i]);
}