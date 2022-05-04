#include "config.h"
#include "Socket.h"
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
/* 
0 - 无调试信息输出
1[-d] - 仅输出时间坐标，序号，客户端IP地址，查询的域名
2[-dd] - 调试信息级别2(输出冗长的调试信息)
*/

uint8_t debug_info = 0;
char serverName[16] = "202.106.0.20";//default
char configFile[64] = "../dnsrelay.txt";//default

int dotCount(char* ip){
    int len = strlen(ip);
    int cnt = 0;
    for (int i = 0; i < len; i++){
        if (ip[i] == '.')   cnt++;
        if (isalpha(ip[i])) return -1;
    }
    if (cnt == 3) return 1;
    else return -1;
}

void config(int argc, char* argv[]){
    if (argc == 1) return;
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-d")) debug_info = 1;
        else if (strcmp(argv[i], "-dd")) debug_info = 2;
        else if (dotCount(argv[i]) == 0) strcpy(serverName, argv[i]);
        else if (i + 1 == argc) strcpy(configFile, argv[i]);
    }
    return;
}

/* url has been dotted by called `getURL`. now Search the file to get ip */
uint32_t findIP(char* name, uint8_t* found){
    FILE* fp = fopen(configFile, "r");
    char ip[16];
    char url[64];
    while (fscanf(fp, "%s %s", ip, url) != EOF){
        if (strcmp(name, url) == 0){
            found = 1;
            return inet_pton(ip);
        }
    }
    //todo: what if not found?
    return 0;
}

void connectCloudDNS(){// connect to cloud DNS code outline. TODO
    struct sockaddr_in servaddr;
	int sockfd, n;
	char buf[512];
	char str[INET_ADDRSTRLEN];
	socklen_t servaddr_len;
    
	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, serverName, &servaddr.sin_addr);
	servaddr.sin_port = htons(53);
    
	while (fgets(buf, 512, stdin) != NULL) {
		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if (n == -1)
			perr_exit("sendto error");

		n = recvfrom(sockfd, buf, 512, 0, NULL, 0);
		if (n == -1)
			perr_exit("recvfrom error");
	  
		Write(STDOUT_FILENO, buf, n);
	}
	Close(sockfd);
}