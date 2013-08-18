#ifndef INFOCAST_NETWORK_H_
#define INFOCAST_NETWORK_H_

#include <sys/socket.h>

#include <list>

std::list<int> open_send_multicast_socket(struct sockaddr_in& saddr,
                                          const char* ip, short port);

#endif /* INFOCAST_NETWORK_H_ */
