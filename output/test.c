#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint32_t ip2hex(char* ip) {
    uint32_t ans = 0;
    uint8_t bias = 24;
    char* token = strtok(ip, ".");
    while (token != NULL) {
        int tmp = atoi(token);
        printf("%s ", token);
        printf("%d\n", tmp);
        token = strtok(NULL, ".");
        ans += tmp << bias;
        bias -= 8;
    }
    printf("%u\n", ans);
    return ans;
}

int main(){
    char ip[16]="202.106.0.20";
    uint32_t ans = ip2hex(ip);
    printf("%u\n", ans);
    return 0;
}