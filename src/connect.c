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
extern char *serverName;

uint16_t connectCloudDNS(DNS dns) { // connect to cloud DNS code outline. TODO
  struct sockaddr_in clientAddr;
  uint16_t oldID = dns.header->ID;
  log(2, "transfering id %d", oldID);
  uint16_t newID = oldID;
  while (!requests[newID].used) {
    if (time(NULL) - requests[newID].startTime > REQ_TIMELIMIT)
      requests[newID].used = 0;
    if (!requests[newID].used) {
      log(2, "-> %d\n", newID);
      break;
    }
    newID = (newID + 1) % REQ_SZLIMIT;
    if (newID == oldID) {
      perr_exit("Connection Pool FULL");
    }
  }

  requests[newID].used = 1;
  requests[newID].startTime = time(NULL);
  requests[newID].ip = ntohl(clientAddr.sin_addr.s_addr);
  requests[newID].port = ntohl(clientAddr.sin_port);
  requests[newID].id = oldID;

  dns.header->ID = newID;

  struct sockaddr_in servaddr;
  int sockfd, n;
  char buf[512];
  char str[INET_ADDRSTRLEN];
  socklen_t servaddr_len;

  sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  //bug: cannot access serverName - SegFault. same as puts(serverName)
  inet_pton(AF_INET, "8.8.8.8", &servaddr.sin_addr);  
  servaddr.sin_port = htons(53);

  n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr,
             sizeof(servaddr));
  if (n == -1)
    perr_exit("sendto error");

//   n = recvfrom(sockfd, buf, 512, 0, NULL, 0);
//   if (n == -1)
//     perr_exit("recvfrom error");
//   Write(STDOUT_FILENO, buf, n);

  Close(sockfd);

  return newID;
}