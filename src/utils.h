#ifndef UTILS_H
#define UTILS_H


#define LOG(x, y, z)       \
if (debug_info >= x) { \
        printf("[LOG%d]", x);   \
        printf(y, z);      \
    }

#include <stdint.h>

uint32_t ip2hex(char* ip);
uint32_t getURL(char* name, char*);
// void LOG(int legal, char* info);

#endif