#include "trie.h"

//Initialize trie tree
void initTrie()
{
    trie = (Trie *)malloc(sizeof(Trie));
    memset(trie->children, 0, sizeof(trie->children));
}

//Insert a new relation of domain and ip
void insertTrie(char * domain, uint32_t ip, uint32_t ttl)
{
    int i;
    Trie * t = trie;
    for(i = 0; i < strlen(domain); i++)
    {
        int index = getIndex(domain[i]);
        if(t->children[index] == NULL)
        {
            t->children[index] = (Trie *)malloc(sizeof(Trie));
            memset(t->children, 0, sizeof(t->children));
        }
        t = t->children[index];
    }
    if(t->children[0] == NULL)
    {
        t->children[0] = (Trie *)malloc(sizeof(Trie));
        memset(t->children, 0, sizeof(t->children));
    }
    t = t->children[0];
    if(t->leaf == NULL)
    {
        t->leaf = (Detail *)malloc(sizeof(Detail));
    }
    t->leaf->ip = ip;
    t->leaf->expireTime = time(NULL) + ttl;
}

//Know the index by giving characters
int getIndex(char c)
{
    if(c >= 'a' && c <= 'z') return c - 'a' + 1;
    else if(c >= '0' && c <= '9') return c - '0' + 27;
    return 37;
}

/**
 * @brief Search trie tree to get ip and ttl
 * 
 * @param domain 
 * @param ip 
 * @param ttl 
 * @return true 
 * @return false 
 */
bool searchTrie(char * domain, uint32_t * ip, uint32_t * ttl)
{
    int i;
    Trie * t = trie;
    for(i = 0; i < strlen(domain); i++)
    {
        int index = getIndex(domain[i]);
        if(t == NULL) return 0;
        t = t->children[index];
    }
    t = t->children[0];
    if(t == NULL) return 0;
    *ip = t->leaf->ip;
    *ttl = t->leaf->expireTime - time(NULL);
}

//Clear the tree
void freeTrie()
{
    Trie * t = trie;
    if(t == NULL) return;
    if(t->children[0]) free(t->children[0]);
    int i;
    for(i = 1; i < 38; i++)
    {
        freeTrie(t->children[i]);
        free(t);
    }
}