#ifndef UTILS_H
#define UTILS_H


#define LOG(x, y, z)       \
if (debug_info >= x) { \
        printf("[LOG%d]", x);   \
        printf(y, z);      \
    }

#include <stdint.h>
#include "DNS.h"

uint32_t ip2hex(char* ip);
uint32_t getURL(char* name, char*);
void handleBuf(char * buf);
void readHeader(char * buf, DNSHeader * header);
size_t readQuestions(char * buf, Qsection * questions, uint16_t QDcount);
// void LOG(int legal, char* info);

#endif