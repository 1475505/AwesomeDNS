#ifndef UTILS_H
#define UTILS_H


#define LOG(x, y, z)       \
if (debug_info >= x) { \
        printf("[LOG%d]", x);   \
        printf(y, z);      \
    }

#include <stdint.h>
#include <stdarg.h>
#include "DNS.h"

uint32_t ip2hex(char* ip);
uint32_t getURL(char* name, char* res, size_t * offset);
void handleBuf(char * buf);
void readHeader(char * buf, DNSHeader * header);
size_t readQuestions(char * buf, Qsection * questions, uint16_t QDcount);
uint32_t mapIP(char* url);
char * getName(char * name, char * buf, size_t * bias);
size_t readRRs(char * buf, RRformat * RRs, uint16_t RRcount, size_t bias);
// void LOG(int legal, char* info);
void log(int x, char* fmt, ...);

#endif