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

void LOG(char* str){
	fputs(str, stdout);
	return;
}


uint32_t getIP(char* name, char* res) {
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

int main(){
    char ip[16] = "60700773xyz000";
    char str[16];
    getIP(ip, str);
    printf("%s\n", str);
    return 0;
}
