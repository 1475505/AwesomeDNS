#ifndef DNS_H
#define DNS_H

#include <unistd.h>

typedef struct DNSHeader{
    uint16_t ID;//transaction ID
    uint16_t info;
    uint16_t QDcount;
    uint16_t ANcount;
    uint16_t NScount;
    uint16_t ARcount;
} DNSHeader;

typedef struct Qsection{
    uint16_t* Qname;//长度+字符串+。。。+长度+字符串+0，点是不出现的。
    uint16_t Qtype;//A(1)CNAME(15),MX(5),PTR(12),.
    uint16_t Qclass;
} Qsection;

//Resource Record Format 
typedef struct RRformat{
    uint16_t* name;//例如一个域名，RDATA是一个IP地址
    uint16_t type;//数据类型
    uint16_t class;
    uint32_t TTL;
    uint16_t RDlength;//数据长度
    uint16_t* Rdata;//实际数据
} RRformat;


#endif