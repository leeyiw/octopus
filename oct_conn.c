#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "oct_conn.h"
#include "oct_log.h"

oct_conn_t *
oct_conn_init()
{
	oct_conn_t *c = (oct_conn_t *)malloc(sizeof(oct_conn_t));
	if (NULL == c) {
		oct_log_error("malloc for oct_conn_t error: %s", ERRMSG);
		return NULL;
	}
	memset(c, 0, sizeof(oct_conn_t));

	c->client_callback = c;
	c->server_callback = c;

	c->req_hdr_len = 0;
	c->req_hdr_max_len = REQUEST_HEADER_MAX_SIZE;
	c->req_hdr = (char *)malloc(REQUEST_HEADER_MAX_SIZE);

	c->rsp_hdr_len = 0;
	c->rsp_hdr_max_len = RESPONSE_HEADER_MAX_SIZE;
	c->rsp_hdr = (char *)malloc(RESPONSE_HEADER_MAX_SIZE);

	return c;
}

void
oct_conn_destroy(oct_conn_t *conn)
{
	if (0 != conn->client_fd) {
		/* 把套接字从epoll里面取消注册 */
		if (-1 == epoll_ctl(conn->epoll_fd, EPOLL_CTL_DEL, conn->client_fd,
			NULL)) {
			oct_log_error("delete client fd from epoll error: %s", ERRMSG);
		}
		/* 关闭套接字 */
		close(conn->client_fd);
		conn->client_fd = 0;
	}
	if (0 != conn->server_fd) {
		/* 把套接字从epoll里面取消注册 */
		if (-1 == epoll_ctl(conn->epoll_fd, EPOLL_CTL_DEL, conn->server_fd,
			NULL)) {
			oct_log_error("delete server fd from epoll error: %s", ERRMSG);
		}
		/* 关闭套接字 */
		close(conn->server_fd);
		conn->server_fd = 0;
	}
	/* 释放缓冲区 */
	if (NULL != conn->req_hdr) {
		free(conn->req_hdr);
		conn->req_hdr = NULL;
	}
}
