#include "config.h"
#include "Socket.h"
#include "utils.h"
#include <assert.h>
#include <bits/getopt_core.h>
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
// char serverName[16] = "192.168.32.2";//wxl
char configFile[64] = "dnsrelay.txt";//default

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
    /* May be this is the unix way to do this */
    int opt;
    while ((opt = getopt(argc, argv, "des:f:")) != -1){
        switch (opt) {
            case 'd':
                debug_info = 1;
                break;
            case 'e':
                debug_info = 2;
                //use `optarg` to get the following settings.
                break;
            case 's':
                strcpy(serverName, optarg);
                break;
            case 'f':
                strcpy(configFile, optarg);
                break;
            default:
                printf("usage: [-d | -e] [-s <cloud DNS server>] [-f "
                "<config filename>]\n");
                break;
        }
    }
    printf("===WELCOME TO Awesome-DNS-relay by \033[40;31mwxl\033[0m, pyl and ll!===\n");
    printf("如果对我们的程序感到满意，欢迎报考北京邮电大学～\n");
    printf("\n\                                                  
,------.  ,--.  ,--. ,---.                ,--.                  \n\
|  .-.  \\ |  ,'.|  |'   .-' ,--.--. ,---. |  | ,--,--.,--. ,--. \n\
|  |  \\  :|  |' '  |`.  `-. |  .--'| .-. :|  |' ,-.  | \\  '  /  \n\
|  '--'  /|  | `   |.-'    ||  |   \\   --.|  |\\ '-'  |  \\   '   \n\
`-------' `--'  `--'`-----' `--'    `----'`--' `--`--'.-'  /    \n\
                                                      `---'     \n\
");
    sleep(1);
    printf("current config:\n");
    printf("log level: %d\n", debug_info);
    printf("cloud DNS server: %s\n", serverName);
    /* Above comment done */
    // if (argc == 1) return;
    // for (int i = 1; i < argc; i++){
    //     if (strcmp(argv[i], "-d")) debug_info = 1;
    //     else if (strcmp(argv[i], "-dd")) debug_info = 2;
    //     else if (dotCount(argv[i]) == 1) strcpy(serverName, argv[i]);
    //     else if (i + 1 == argc) strcpy(configFile, argv[i]);
    // }
    return;
}

/* url has been dotted by called `getURL`. now Search the file to get ip */
uint32_t findIP(char* name, uint8_t* found, DNS * dns){
    dns->answer = (RRformat *)malloc(sizeof(RRformat));
    uint32_t ip, ttl;
    bool res = searchTrie(name, &ip, &ttl);
    if(res) 
    {
        log(1, "\n %s hit the trie tree in configFile!\n", name);
        dns->answer->TTL = ttl;
        dns->answer->Rdata = ip;
    }
    else res = searchCache(name, &ip, &ttl);
    if(res) {
        log(1, "\n %s hit the cache!\n", name);
        dns->answer->TTL = ttl;
        dns->answer->Rdata = ip;
    }
    //todo: what if not found?
    else
    {
        log(1, "\n%s not found in configFile, connecting to %s\n", name, serverName);
        *found = 0;
    }
    return ip;
}

