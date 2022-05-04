#ifndef DNS_H
#define DNS_H

#include <unistd.h>

typedef struct DNSHeader {
    uint16_t ID;  // transaction ID

    /*

    *-----------------------------------------*
    |0 |1      4|5 |6 |7 |8 |9     12|13    15|
    |QR| Opcode |AA|TC|RD|RA|   Z    | RCODE  |
    *-----------------------------------------*

    QR：0表示查询报，1表示响应报。
    OPCODE:通常值为0（标准查询），其他值为1（反向查询）和2（服务器状态请求）。
    AA: 权威答案(Authoritative answer)
    TC: 截断的(Truncated). 应答的总长度超512字节时，只返回前512个字节
    RD: 期望递归(Recursion desired).
        - 告诉服务器处理递归查询。查询报中设置，响应报中返回
        - 该位为0且被请求的服务器无权威回答，返回能解答该查询的其他服务器列表
        - 这称为迭代查询
    RA：递归可用(Recursion Available).
        - 如果名字服务器支持递归查询，则在响应中该比特置为1 .
    Z：必须为0，保留字段
    RCODE: 响应码(Response coded)，仅用于响应报.值为0表没有差错
           - 值为3表示名字差错。从权威名字服务器返回，表示在查询中指定域名不存在
    */
    uint16_t info;

    uint16_t QDcount;  // Number of entries in the question section
    uint16_t ANcount;  // Number of RRs in the answer section
    uint16_t NScount;  // Number of name server RRs in authority records section
    uint16_t ARcount;  // Number of RRs in additional records section
} DNSHeader;

typedef struct Qsection {
    uint8_t** Qname;  //长度+字符串+。。。+长度+字符串+0，点是不出现的。

    /*
    A(1)        IP地址,存储32bit的二进制数
    AAAA(28)    IPv6地址
    PTR(12)     指针记录用于指针查询。
    CNAME(5)    规范名字(canonical name) 别名
    HINFO(13)   主机信息 主机CPU和操作系统
    MX(15)      邮件交换 16bit整数优先值，以及域名
                如果一个目的主机有多个MX项，按优先值由小到大顺序使用
    NS(2)       说明域的权威名字服务器
    */
    uint16_t Qtype;

    uint16_t Qclass;
} Qsection;

// Resource Record Format
typedef struct RRformat {
    uint16_t* name;  //例如一个域名，RDATA是一个IP地址
    uint16_t type;   //数据类型
    uint16_t class;
    uint32_t TTL;
    uint16_t RDlength;  //数据长度
    uint16_t* Rdata;    //实际数据
} RRformat;

// DNS 报文.后三段格式相同，每段都是由0~n个资源记录(Resource Record)构成
typedef struct DNS {
    DNSHeader header;
    uint16_t* question;    // the question for the name server
    uint16_t* answer;      // RRs answering the question
    uint16_t* authority;   // RRs pointing toward an authority
    uint16_t* additional;  // RRs holding additional information
} DNS;

#endif