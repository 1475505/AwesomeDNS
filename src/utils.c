#include "utils.h"
#include <stdint.h>
#include <string.h>

extern uint8_t debug_info;

uint32_t ip2hex(char* ip) {
    uint32_t ans = 0;
    uint8_t bias = 24;
    char* token = strtok(ip, ".");
    while (token != NULL) {
        int tmp = atoi(token);
        token = strtok(NULL, ".");
        ans += tmp << bias;
        bias -= 8;
    }
    return ans;
}

void LOG(int legal, char* info){
    if (legal >= debug_info){
        printf("%s\n",info);
    }
    return;
}