#ifndef OCT_CONN_H_
#define OCT_CONN_H_

#define REQUEST_HEADER_MAX_SIZE	8192

typedef struct _oct_conn_t {
	int client_fd;
	int server_fd;
	int epoll_fd;

	struct _oct_conn_t *client_callback;
	struct _oct_conn_t *server_callback;

	size_t req_hdr_len;
	size_t req_hdr_max_len;
	char *req_hdr;

	size_t host_len;
	char *host;
} oct_conn_t;

extern oct_conn_t *oct_conn_init();
extern void oct_conn_destroy(oct_conn_t *conn);

#endif
