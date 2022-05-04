#include "config.h"
#include <stdint.h>
#include <ctype.h>
// extern uint8_t debug_info;
// extern char* serverName;
// extern char* configFile;

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

uint32_t findIP(char* name){
    fopen(configFile, "r");
    //todo
    return 0;
}