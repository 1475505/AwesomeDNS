#ifndef CACHE_H_
#define CACHE_H

#include "list.h"
#include "utils.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

struct list_head list;
static int list_size;
static int CACHE_SIZE = 256;

void initCache();
void addCache(char *name, uint32_t ip, uint32_t ttl);
bool searchCache(char *name, uint32_t *ip, uint32_t *ttl);
void printCache();

#endif