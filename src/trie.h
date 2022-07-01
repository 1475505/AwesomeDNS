#ifndef TRIE_H
#define TRIE_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

char ALPHABET[37] = "abcdefghijklmnopqrstuvwxyz0123456789-";

typedef struct Detail
{
    uint32_t ip;
    time_t expireTime;
}Detail;

//Trie tree node
typedef struct Trie
{
    union 
    {
        struct Trie * children[38];//26 characters,0-9,'-',and a end signal
        Detail * leaf;
        /* data */
    };
}Trie;

Trie * trie;
void initTrie();
void insertTrie(char * domain, int32_t ip, int32_t ttl);
int getIndex(char c);
bool searchTrie(char * domain, u_int32_t * ip, u_int32_t * ttl);
void freeTrie();

#endif