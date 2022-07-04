#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct Detail {
  uint32_t ip;
  time_t expireTime;
} Detail;

// Trie tree node
typedef struct Trie {
  union {
    struct Trie *children[38]; // 26 characters,0-9,'-',and a end signal
    Detail *leaf;
    /* data */
  };
} Trie;

Trie *trie;
void initTrie();
void insertTrie(char *domain, uint32_t ip, uint32_t ttl);
int getIndex(char c);
bool searchTrie(char *domain, u_int32_t *ip, u_int32_t *ttl);
void freeTrie();

#endif