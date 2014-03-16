#ifndef OCT_PROXY_H_
#define OCT_PROXY_H_

#define EPOLL_MAX_EVENTS		64
#define OCT_PROXY_BUF_LEN		2048

#define OCT_PROXY_SUCCESS		0
#define OCT_PROXY_FAIL			-1

#define OCT_PROXY_CONTINUE		0
#define OCT_PROXY_STOP			-1

extern void oct_proxy_loop(int listen_fd);

#endif  // OCT_PROXY_H_
