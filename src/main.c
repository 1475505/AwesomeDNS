/*
设计一个DNS服务器程序，读入“域名-IP地址”对照表（一个文件）
当客户端查询域名对应的IP地址时，用域名检索该对照表，得到三种检索结果：
- 检索结果为ip地址0.0.0.0，则向客户端返回“域名不存在”的报错（不良网站拦截）
- 检索结果为普通IP地址，则向客户返回这个地址（服务器功能）
- 表中未检到该域名，则向因特网DNS服务器发出查询，并将结果返给客户端（中继功能）
     考虑多个计算机上的客户端会同时查询，需要进行消息ID的转换
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "config.h"
#include "DNS.h"
#include "utils.h"

#define MAXLINE 80
#define SERV_PORT 53

extern int debug_info;
extern char serverName[16];
extern char configFile[64];

int main(int argc, char* argv[]) {
    config(argc, argv);
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
        printf("received from %u at PORT %d\n",
               inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
               ntohs(cliaddr.sin_port));

        LOG(2, buf);
        
        for (i = 0; i < n; i++)
            buf[i] = toupper(buf[i]);
        n = sendto(sockfd, buf, n, 0, (struct sockaddr*)&cliaddr,
                   sizeof(cliaddr));
        if (n == -1)
            perr_exit("sendto error");
    }

    return 0;
}