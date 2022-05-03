#include "config.h"
#include <stdint.h>

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
char* configFile = "../dnsrelay.txt";//default

void config(int argc, char* argv[]){
    //todo
    return;
}