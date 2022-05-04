#include "utils.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

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

void LOG(int legal, char* info) {
    if (legal >= debug_info) {
        printf("[LOG %d]", legal);
        fputs(info, stdout);
    }
    return;
}

/* trans `name` to dot url in res.
   eg. 60700773xyz0 -> 070077.xyz
   */
uint32_t getURL(char* name, char* res) {
    int len = strlen(name);
    int idx = 0;
    int bias = name[0] - '0';
    int i = 1;
    while (i < len) {
        for (int j = 0; j < bias; j++) {
            res[idx] = name[i];
            idx++;
            i++;
        }
        bias = name[i] - '0';
        if (bias == 0 || i >= len) break;
        i++;
        res[idx] = '.';
        idx++;
    }
    res[idx] = '\0';
    return idx;
}