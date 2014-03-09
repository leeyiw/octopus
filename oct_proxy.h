#ifndef OCT_PROXY_H_
#define OCT_PROXY_H_

#include <sys/types.h>

#define EPOLL_MAX_EVENTS		64
#define REQUEST_HEADER_MAX_SIZE	8192

#define OCT_PROXY_SUCCESS		0
#define OCT_PROXY_FAIL			-1

#define OCT_PROXY_CONTINUE		0
#define OCT_PROXY_STOP			-1

typedef struct _oct_conn_t {
	int client_fd;
	int server_fd;
	int epoll_fd;

	size_t req_hdr_len;
	size_t req_hdr_max_len;
	char *req_hdr;

	size_t host_len;
	char *host;
} oct_conn_t;

extern oct_conn_t *oct_proxy_init();
extern void oct_proxy_process(oct_conn_t *conn);

#endif  // OCT_PROXY_H_
