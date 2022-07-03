#include "utils.h"
#include "DNS.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINE 512

extern uint8_t debug_info;
extern char *configFile;

uint32_t ip2hex(char *ip) {
  uint32_t ans = 0;
  uint8_t bias = 24;
  char *token = strtok(ip, ".");
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
uint32_t getURL(char *name, char *res, size_t * offset) {
  assert(name);
  assert(res);
  log(2, "getting URL :");
  int len = strlen(name);
  (*offset) += len + 1;
  int idx = 0;
  int bias = name[0];
  int i = 1;
  while (i < len) {
    for (int j = 0; j < bias; j++) {
      res[idx] = name[i];
      idx++;
      i++;
    }
    bias = name[i];
    if (bias == 0 || i >= len)
      break;
    i++;
    res[idx] = '.';
    idx++;
  }
  res[idx] = '\0';
  return idx;
}

/**
 * @brief input questions
 *
 * @param buf
 * @param questions
 * @param QDcount
 * @return size_t buf's bias
 */
size_t readQuestions(char *buf, Qsection *questions, uint16_t QDcount) {
  size_t i, bias = 12;
  for (i = 0; i < QDcount; i++) {
    questions[i].Qname = (char *)malloc((strlen(buf + bias)) * sizeof(char));
    getURL(buf + bias, questions[i].Qname, &bias);
    log(1, "%s \n", questions[i].Qname);
    questions[i].Qtype = (uint16_t)(buf[bias] << 8) + buf[bias + 1];
    questions[i].Qclass = (uint16_t)(buf[bias + 2] << 8) + buf[bias + 3];
    bias += 4;
  }
  return bias;
}

size_t readRRs(char *buf, RRformat *RRs, uint16_t RRcount, size_t bias) {
  size_t i;
  for (i = 0; i < RRcount; i++) {
    RRs[i].name = (char *)malloc((strlen(buf + bias)) * sizeof(char));
    getURL(buf + bias, RRs[i].name, &bias);
    RRs[i].type = (uint16_t)(buf[bias] << 8) + buf[bias + 1];
    RRs[i].clas = (uint16_t)(buf[bias + 2] << 8) + buf[bias + 3];
    bias += 4;
    RRs[i].TTL = (uint32_t)(buf[bias] << 24) + (uint32_t)(buf[bias + 1] << 16) +
                 (uint32_t)(buf[bias + 2] << 8) + (uint32_t)(buf[bias + 3]);
    bias += 4;
    RRs[i].RDlength = (uint16_t)(buf[bias] << 8) + buf[bias + 1];
    bias += 2;
    RRs[i].Rdata =
        (uint32_t)(buf[bias] << 24) + (uint32_t)(buf[bias + 1] << 16) +
        (uint32_t)(buf[bias + 2] << 8) + (uint32_t)(buf[bias + 3]);
    bias += 4;
  }
  return bias;
}

uint32_t mapIP(char *url) {
  // TODO: Read file and match URL, return IP.
  FILE *fd = fopen(configFile, "r");
  char *line;
  while (fgets(line, 128, fd)) { // 128 : maxlen of a line
    char* ip = strtok(NULL, ".");
    char *urlname = strtok(line, " ");
    char name[128];size_t * offset;
    getURL(urlname, name, offset);
    if (strcmp(url, name))  return ip2hex(ip);//
  }
  return 0;
}

void log(int x, char *fmt, ...) {
  va_list arg_ptr;
  if (debug_info >= x) {
    va_start(arg_ptr, fmt);
    vprintf(fmt, arg_ptr);
    va_end(arg_ptr);
  }
}

void writeAN(char * start, DNS dns)
{
  int i, j = 0, length = 0;
  for(i = 0; i < strlen(dns.question->Qname);i++)
  {
    if(dns.question->Qname[i] == '.')
    {
      start[j] = length;
      length = 0;
      j = i + 1;
    }
    else
    {
      length++;
      start[i + 1] = dns.question->Qname[i];
    }
  }//convert to standard format
  sizeof(RRformat);
  start[j] = length;
  start[strlen(dns.question->Qname) + 1] = '\0';
  start[strlen(dns.question->Qname) + 2] = dns.answer->type / (1 << 8);
  start[strlen(dns.question->Qname) + 3] = dns.answer->type % (1 << 8);
  start[strlen(dns.question->Qname) + 4] = dns.answer->clas / (1 << 8);
  start[strlen(dns.question->Qname) + 5] = dns.answer->clas % (1 << 8);
  start[strlen(dns.question->Qname) + 6] = dns.answer->TTL / (1 << 24);
  start[strlen(dns.question->Qname) + 7] = (dns.answer->TTL / (1 << 16)) % (1 << 8);
  start[strlen(dns.question->Qname) + 8] = (dns.answer->TTL / (1 << 8)) % (1 << 8);
  start[strlen(dns.question->Qname) + 9] = dns.answer->TTL % (1 << 8);
  start[strlen(dns.question->Qname) + 10] = dns.answer->RDlength / (1 << 8);
  start[strlen(dns.question->Qname) + 11] = dns.answer->RDlength % (1 << 8);
  start[strlen(dns.question->Qname) + 12] = dns.answer->Rdata / (1 << 24);
  start[strlen(dns.question->Qname) + 13] = (dns.answer->Rdata / (1 << 16)) % (1 << 8);
  start[strlen(dns.question->Qname) + 14] = (dns.answer->Rdata / (1 << 8)) % (1 << 8);
  start[strlen(dns.question->Qname) + 15] = dns.answer->Rdata % (1 << 8);
  start[strlen(dns.question->Qname) + 16] = '\0';
}