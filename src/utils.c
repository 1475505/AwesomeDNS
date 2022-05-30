#include "utils.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#define MAXLINE 512

extern uint8_t debug_info;
extern char* configFile;

uint32_t ip2hex(char* ip) {
    uint32_t ans = 0;
    uint8_t bias = 24;
    char* token = strtok(ip, ".");
    while (token != NULL) {
        int tmp = atoi(token);
        token = strtok(NULL, ".");
        ans += tmp << bias;
        bias -= 8;
    }
    return ans;
}

// void LOG(int legal, char* info) {
//     if (legal >= debug_info) {
//         printf("[LOG %d]", legal);
//         fputs(info, stdout);
//     }
//     return;
// }

/* trans `name` to dot url in res.
   eg. 60700773xyz0 -> 070077.xyz
   */
uint32_t getURL(char* name, char* res) {
    assert(name);
    int len = strlen(name);
    int idx = 0;
    int bias = name[0] - '0';
    int i = 1;
    while (i < len) {
        for (int j = 0; j < bias; j++) {
            res[idx] = name[i];
            idx++;
            i++;
        }
        bias = name[i] - '0';
        if (bias == 0 || i >= len) break;
        i++;
        res[idx] = '.';
        idx++;
    }
    res[idx] = '\0';
    return idx;
}

void readHeader(char * buf, DNSHeader * header)
{
    header->ID = (uint16_t)(buf[0] << 8) + buf[1];
    header->info = (uint16_t)(buf[2] << 8) + buf[3];
    header->QDcount = (uint16_t)(buf[4] << 8) + buf[5];
    header->ANcount = (uint16_t)(buf[6] << 8) + buf[7];
    header->NScount = (uint16_t)(buf[8] << 8) + buf[9];
    header->ARcount = (uint16_t)(buf[10] << 8) + buf[11];
}

/**
 * @brief input questions
 * 
 * @param buf 
 * @param questions 
 * @param QDcount 
 * @return size_t buf's bias
 */
size_t readQuestions(char * buf, Qsection * questions, uint16_t QDcount)
{
    size_t i, bias = 12;
    for(i = 0; i < QDcount; i++)
    {
        questions[i].Qname = getName(questions[i].Qname, buf, &bias);
        questions[i].Qtype = (uint16_t)(buf[bias] << 8) + buf[bias + 1];
        questions[i].Qtype = (uint16_t)(buf[bias + 2] << 8) + buf[bias + 3];
        bias += 4;
    }
    return bias;
}

size_t readRRs(char * buf, RRformat * RRs, uint16_t RRcount, size_t bias)
{
    size_t i;
    for(i = 0; i < RRcount; i++)
    {
        RRs[i].name = getName(RRs[i].name, buf, &bias);
        RRs[i].type = (uint16_t)(buf[bias] << 8) + buf[bias + 1];
        RRs[i].class = (uint16_t)(buf[bias + 2] << 8) + buf[bias + 3];
        bias += 4;
        RRs[i].TTL = (uint32_t)(buf[bias] << 24) + (uint32_t)(buf[bias + 1] << 16)
                   + (uint32_t)(buf[bias + 2] << 8) + (uint32_t)(buf[bias + 3]);
        bias += 4;
        RRs[i].RDlength = (uint16_t)(buf[bias] << 8) + buf[bias + 1];
        bias += 2;
        RRs[i].Rdata = (uint32_t *)malloc(sizeof(uint32_t) * RRs[i].RDlength);
        size_t j;
        for(j = 0; j < RRs[i].RDlength; j++)
        {
            RRs[i].Rdata[j] = (uint32_t)(buf[bias] << 24) + (uint32_t)(buf[bias + 1] << 16)
                            + (uint32_t)(buf[bias + 2] << 8) + (uint32_t)(buf[bias + 3]);
            bias += 4;
        }
    }
    return bias;
}

char * getName(char * name, char * buf, size_t * bias)
{
    size_t size = buf[(*bias)++], all = 0;
    name = NULL;

    while (size)
    {
        all += size;
        char * new = malloc(all * sizeof(char));
        memset(new, 0, all);
        if(name != NULL)
        {
            int j;
            for(j = 0; j < strlen(name); j++)
            {
                new[j] = name[j];
            }
            free(name);
        }
        memcpy(new + strlen(new), buf + (*bias), size);
        (*bias) += size;
        name = new;
        size = buf[(*bias)++];
        if(size)
        {
            all++;
            new = malloc((all) * sizeof(char));
            memset(new, 0, all);
            if(name != NULL)
            {
                int j;
                for(j = 0; j < strlen(name); j++)
                {
                    new[j] = name[j];
                }
                free(name);
            }
            new[strlen(new)] = '.';
            name = new;
        }
    }
    return name;
}

uint32_t getIP(char* url){
    //TODO: Read file and match URL, return IP.
    return 0;
}

void log(int x, char* fmt, ...)
{
    va_list arg_ptr;
    if(debug_info >= x)
    {
        va_start(arg_ptr, fmt);
        vprintf(fmt, arg_ptr);
        va_end(arg_ptr);
    }
}