#ifndef CONNECT_H_
#define CONNECT_H

#include "DNS.h"
#include "config.h"
#include "utils.h"
#include <bits/types/timer_t.h>
#include <stdint.h>
uint16_t connectCloudDNS(DNS dns);

typedef struct Request {
  uint8_t used;
  time_t startTime;
  uint16_t id;
  uint32_t ip;
  uint16_t port;
} Request;

#endif