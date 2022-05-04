#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

uint32_t ip2hex(char* ip);
uint32_t getURL(char* name, char*);
void LOG(int legal, char* info);

#endif