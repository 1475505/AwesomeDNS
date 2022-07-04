#ifndef CONFIG_H
#define CONFIG_H

#include "cache.h"
#include "trie.h"
#include <stdint.h>

void config(int argc, char *argv[]);
uint32_t findIP(char *name, uint8_t *found, DNS *dns);

#endif