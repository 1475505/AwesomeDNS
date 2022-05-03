#include "utils.h"
#include <stdint.h>
#include <string.h>

uint32_t ip2hex(char* ip) {
    uint32_t ans;
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