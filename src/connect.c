#include "connect.h"
#include "DNS.h"
#include "Socket.h"
#include "utils.h"
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>

Request requests[REQ_SZLIMIT];
extern char serverName[16];

uint16_t connectCloudDNS(DNS dns) { // connect to cloud DNS code outline. TODO
  struct sockaddr_in clientAddr;
  uint16_t oldID = dns.header->ID;
  log(2, "transfering id %d", oldID);
  uint16_t newID = oldID;
  while (requests[newID].used) {
    if (time(NULL) - requests[newID].startTime > REQ_TIMELIMIT)
      requests[newID].used = 0;
    newID = (newID + 1) % REQ_SZLIMIT;
    if (newID == oldID) {
      perr_exit("Connection Pool FULL");
    }
  }

  log(2, "-> %d\n", newID);
  requests[newID].used = 1;
  requests[newID].startTime = time(NULL);
  requests[newID].ip = ntohl(clientAddr.sin_addr.s_addr);
  requests[newID].port = ntohl(clientAddr.sin_port);
  requests[newID].id = oldID;

  dns.header->ID = newID;

  

  return newID;
}