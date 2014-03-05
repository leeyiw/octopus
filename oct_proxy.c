#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "oct_http.h"
#include "oct_log.h"
#include "oct_proxy.h"

static int
oct_proxy_req_hdr(oct_conn_t *conn)
{
	ssize_t i, n;
	int line_start = 0;
	n = recv(conn->client_fd, conn->req_hdr, conn->req_hdr_max_len, 0);
	if (n == 0) {
		oct_log_info("client orderly shutdown");
		return OCT_PROXY_STOP;
	} else if(n == -1) {
		oct_log_info("recv data from client error: %s", ERRMSG);
		return OCT_PROXY_STOP;
	}
	for (i = 0; i < n; i++) {
		if (conn->req_hdr[i] == '\r' && conn->req_hdr[i + 1] == '\n') {
			if (line_start == 0) {
				oct_http_parse_req_line(conn, &conn->req_hdr[line_start],
					i- line_start);
			} else {
				oct_http_parse_req_hdr_fields(conn, &conn->req_hdr[line_start],
					i - line_start);
			}
			line_start = i + 2;
		}
	}
	return OCT_PROXY_CONTINUE;
}

static int
oct_proxy_req_body(oct_conn_t *conn)
{
	return OCT_PROXY_CONTINUE;
}

static int
oct_proxy_request(oct_conn_t *conn)
{
	if (conn->req_hdr_len == 0) {
		return oct_proxy_req_hdr(conn);
	} else {
		return oct_proxy_req_body(conn);
	}
}

oct_conn_t *
oct_proxy_init()
{
	oct_conn_t *c = (oct_conn_t *)malloc(sizeof(oct_conn_t));
	if (NULL == c) {
		oct_log_error("malloc for oct_conn_t error: %s", ERRMSG);
		return NULL;
	}
	memset(c, 0, sizeof(oct_conn_t));

	c->req_hdr_len = 0;
	c->req_hdr_max_len = REQUEST_HEADER_MAX_SIZE;
	c->req_hdr = (char *)malloc(REQUEST_HEADER_MAX_SIZE);
}

void
oct_proxy_process(oct_conn_t *conn)
{
	struct epoll_event event;
	struct epoll_event events[EPOLL_MAX_EVENTS];
	int epoll_fd;
	int epoll_timeout = 10;
	int loop = 1;

	if (-1 == (epoll_fd = epoll_create(2))) {
		oct_log_error("create epoll file descriptor error: %s", ERRMSG);
		return;
	}
	/* 先把客户端的套接字加入到epoll监听事件里面去 */
	memset(&event, 0, sizeof(&event));
	event.events = EPOLLIN;
	event.data.fd = conn->client_fd;
	if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn->client_fd, &event)) {
		oct_log_error("add client fd to epoll fd error: %s", ERRMSG);
		return;
	}
	while (loop) {
		int i, nevents;
		nevents = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, epoll_timeout);
		if (-1 == nevents) {
			oct_log_error("epoll_wait error: %s", ERRMSG);
		} else if (0 == nevents) {
			/* TODO 进行超时处理 */
		}
		for (i = 0; i < nevents; i++) {
			struct epoll_event *ev = &events[i];
			if (ev->events & EPOLLIN) {
				if (ev->data.fd == conn->client_fd) {
					if (OCT_PROXY_STOP == oct_proxy_request(conn)) {
						loop = 0;
						break;
					}
				}
			}
		}
	}
	/* 释放epoll资源 */
	close(epoll_fd);
}
