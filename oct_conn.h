#ifndef OCT_CONN_H_
#define OCT_CONN_H_

#define REQUEST_HEADER_MAX_SIZE		8192
#define RESPONSE_HEADER_MAX_SIZE	8192

typedef struct _oct_conn_t {
	int client_fd;
	int server_fd;
	int epoll_fd;

	struct _oct_conn_t *client_callback;
	struct _oct_conn_t *server_callback;

	/* 请求相关字段 */
	size_t req_hdr_len;
	size_t req_hdr_max_len;
	char *req_hdr;

	size_t host_len;
	char *host;

	/* 响应相关字段 */
	size_t rsp_hdr_len;
	size_t rsp_hdr_max_len;
	char *rsp_hdr;
} oct_conn_t;

extern oct_conn_t *oct_conn_init();
extern void oct_conn_destroy(oct_conn_t *conn);

#endif  // OCT_CONN_H_
