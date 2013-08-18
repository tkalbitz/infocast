/*
 * infocast - Make robots visible
 * Copyright 2012-2013 Tobias Kalbitz.
 * Subject to the AGPL, version 3.
 */

#include "infocast_network.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ifaddrs.h>


std::list<int> open_send_multicast_socket(struct sockaddr_in& saddr,
                                          const char* ip, short port)
{
    int status;
    static const int on = 1;

    std::list<int> sockets;

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);

    struct ifaddrs *addrs = NULL;
    int result = getifaddrs(&addrs);
    if (result < 0) {
        perror("Couldn't get network interfaces!");
        fprintf(stderr, "Error: getting iface list in file %s line %d. Exiting.\n", __FILE__, __LINE__);
        exit(1);
    }

    const struct ifaddrs *cursor = addrs;
    while (cursor != NULL) {
        if (cursor->ifa_addr != NULL && cursor->ifa_addr->sa_family == AF_INET
                && !(cursor->ifa_flags & IFF_POINTOPOINT)
                &&  (cursor->ifa_flags & IFF_MULTICAST)
                &&  (cursor->ifa_flags & IFF_UP)) {

            int sock = socket(AF_INET, SOCK_DGRAM, 0);
            if(sock < 0) {
                perror("Error creating socket");
                fprintf(stderr, "Error: creating socket in file %s line %d. Exiting.\n", __FILE__, __LINE__);
                exit(EXIT_FAILURE);
            }

            status = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
            if(status) {
                perror("setsockopt reuseaddr:");
                fprintf(stderr, "Error: setsockopt failed in file %s line %d. Exiting.\n", __FILE__, __LINE__);
                exit(EXIT_FAILURE);
            }

            status = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF,
                                &((struct sockaddr_in *)cursor->ifa_addr)->sin_addr,
                                sizeof(struct in_addr));
            if (status) {
                perror("Couldn't set multicast interfaces");
                fprintf(stderr, "Error: setsockopt failed in file %s line %d. Exiting.\n", __FILE__, __LINE__);
                exit(EXIT_FAILURE);
            }

            sockets.push_front(sock);
        }
        cursor = cursor->ifa_next;
    }
    freeifaddrs(addrs);
    return sockets;
}
