#include "utils.h"
#include <stdint.h>
#include <string.h>

uint16_t ip2hex(char* ip) {
    uint8_t len = strlen(ip);
    uint32_t ans;
    char* token = strtok(ip, ".");
    while (token != NULL) {
        int tmp = atoi(token);
        //todo
        token = strtok(NULL, " ");
    }
    return ans;
}