#ifndef OCT_NETWORK_H_
#define OCT_NETWORK_H_

#include <sys/socket.h>

extern int oct_connect_nonb(int sockfd, const struct sockaddr *addr,
	socklen_t addrlen, int nsec);

#endif  // OCT_NETWORK_H_
