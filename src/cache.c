#include "cache.h"
#include "utils.h"

extern char* configFile;

struct listNode
{
    /* data */
    struct list_head list;
    char name[256];
    uint32_t ip;
    time_t expireTime;
};

//Initialize the cache
void initCache()
{
    memset(&list, 0, sizeof(list));
    INIT_LIST_HEAD(&list);
    list_size = 0;
}

//Add a record to cache
void addCache(char * name, uint32_t ip, uint32_t ttl)
{
    log(1, "%s->%s added to cache\n", name, ip);
    struct listNode * ln = (struct listNode *)malloc(sizeof(struct listNode));
    strcpy(ln->name, name);
    ln->ip = ip;
    ln->expireTime = time(NULL) + ttl;
    list_add(&ln->list, &list);
    list_size++;
    while(list_size > CACHE_SIZE)
    {
        list_del(list.prev);
        list_size--;
    }
}

bool searchCache(char * name, uint32_t * ip, uint32_t * ttl)
{
    struct list_head * pos = NULL, * n = NULL;
    list_for_each_safe(pos, n, &list)
    {
        struct listNode * ln = list_entry(pos, struct listNode, list);
        //Outdated
        if(ln->expireTime <= time(NULL))
        {
            list_del(pos);
            list_size--;
            free(ln);
        }
        else if(strcmp(ln->name, name) == 0)
        {
            *ip = ln->ip;
            *ttl = ln->expireTime - time(NULL);
            list_del(pos);
            free(ln);
            addCache(name, *ip, *ttl);
            return true;
        }
    }
    return false;
}

//Print the cache
void printCache()
{
    struct list_head * pos = NULL;
    list_for_each(pos, &list)
    {
        struct listNode * ln = list_entry(pos, struct listNode, list);
        log(2, "name:%s\nip:%x\n", ln->name, ln->ip);
    }
}
