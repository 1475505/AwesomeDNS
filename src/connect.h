#ifndef CONNECT_H_
#define CONNECT_H

#include "config.h"
#include <bits/types/timer_t.h>
#include <stdint.h>
#include "utils.h"
#include "DNS.h"
uint16_t connectCloudDNS(DNS dns);

typedef struct Request{
    uint8_t used;
    time_t startTime;
    uint16_t id;
    uint32_t ip;
    uint16_t port;
}Request;


#endif