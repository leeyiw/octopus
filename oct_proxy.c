#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "oct_conn.h"
#include "oct_http.h"
#include "oct_log.h"
#include "oct_proxy.h"

static oct_conn_t *
oct_proxy_accept(int listen_fd)
{
	int client_fd = 0;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	char client_ip[16];
	oct_conn_t *conn = NULL;

	/* 接收新的连接 */
	client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
	if (-1 == client_fd) {
		if (errno != EAGAIN) {
			oct_log_error("accpet new connection error: %s", ERRMSG);
		}
		return NULL;
	}
	/* 获取客户端IP */
	if (NULL == inet_ntop(AF_INET, &client_addr.sin_addr, client_ip,
		sizeof(client_ip))) {
		oct_log_warn("get client IP error: %s", ERRMSG);
	} else {
		oct_log_info("accpet new connection from %s:%d", client_ip,
			ntohs(client_addr.sin_port));
	}
	/* 创建连接结构体 */
	conn = oct_conn_init();
	if (NULL == conn) {
		oct_log_error("init connection structure error");
		close(client_fd);
		return NULL;
	}
	conn->client_fd = client_fd;
	return conn;
}

static int
oct_proxy_connect_server(oct_conn_t *conn)
{
	size_t i, hostname_len;
	char *hostname = NULL, *colon = NULL;
	char ip[INET_ADDRSTRLEN];
	uint16_t port;
	int ret;
	struct addrinfo *p, *result, hints;
	int success = 0;

	/* 查找Host字段对应的域名和端口号 */
	colon = conn->host + conn->host_len;
	for (i = 0; i < conn->host_len; i++) {
		if (conn->host[i] == ':') {
			colon = &conn->host[i];
			break;
		}
	}
	hostname_len = colon - conn->host;
	if (NULL == (hostname = (char *)malloc(hostname_len + 1))) {
		oct_log_error("malloc for hostname error: %s", ERRMSG);
		return OCT_PROXY_FAIL;
	}
	memcpy(hostname, conn->host, hostname_len);
	hostname[hostname_len] = '\0';
	if (hostname_len == conn->host_len) {
		port = 80;
	} else {
		port = atoi(colon + 1);
	}
	oct_log_info("client request hostname: %s port: %d", hostname, port);
	/* 把域名转换为IP */
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = 0;
	if (0 != (ret = getaddrinfo(hostname, NULL, &hints, &result))) {
		oct_log_error("get host IP by hostname error: %s", gai_strerror(ret));
		return OCT_PROXY_FAIL;
	}
	/* 创建服务器端套接字 */
	conn->server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == conn->server_fd) {
		oct_log_error("create socket for server error: %s", ERRMSG);
		return OCT_PROXY_FAIL;
	}
	/* 循环尝试连接服务器 */
	for (p = result; p != NULL; p = p->ai_next) {
		struct sockaddr_in *server_addr = (struct sockaddr_in *)(p->ai_addr);
		if (NULL == inet_ntop(p->ai_family, &server_addr->sin_addr, ip,
			sizeof(ip))) {
			oct_log_warn("get address string error: %s", ERRMSG);
			continue;
		}
		oct_log_info("trying connect to server %s:%d", ip, port);
		/* 设置端口 */
		server_addr->sin_port = htons(port);
		if (-1 == connect(conn->server_fd, (struct sockaddr *)server_addr,
			p->ai_addrlen)) {
			oct_log_error("connect to server %s:%d error: %s", ip, port,
				ERRMSG);
		} else {
			oct_log_info("connect to server %s:%d success", ip, port);
			success = 1;
			break;
		}
	}
	free(hostname);
	freeaddrinfo(result);
	if (success) {
		/* 把连接服务器的套接字加入到epoll中 */
		struct epoll_event event;
		memset(&event, 0, sizeof(&event));
		event.events = EPOLLIN;
		event.data.ptr = &conn->server_callback;
		if (-1 == epoll_ctl(conn->epoll_fd, EPOLL_CTL_ADD, conn->server_fd,
			&event)) {
			oct_log_error("add server fd to epoll fd error: %s", ERRMSG);
			return OCT_PROXY_FAIL;
		}
		return OCT_PROXY_SUCCESS;
	} else {
		close(conn->server_fd);
		conn->server_fd = 0;
		return OCT_PROXY_FAIL;
	}
}

static int
oct_proxy_req_hdr(oct_conn_t *conn)
{
	ssize_t n;

	n = recv(conn->client_fd, conn->req_hdr, conn->req_hdr_max_len, 0);
	if (0 == n) {
		oct_log_info("client orderly shutdown");
		return OCT_PROXY_STOP;
	} else if (-1 == n) {
		oct_log_error("recv data from client error: %s", ERRMSG);
		return OCT_PROXY_STOP;
	}
	/* 解析HTTP请求头 */
	oct_http_parse_req_hdr(conn, n);
	/* HTTP协议头部必须存在Host字段，否则断开连接 */
	if (NULL == conn->host) {
		return OCT_PROXY_STOP;
	}
	/* 连接到服务器 */
	if (OCT_PROXY_FAIL == oct_proxy_connect_server(conn)) {
		return OCT_PROXY_STOP;
	}
	/* 把请求头发送给服务器 */
	n = send(conn->server_fd, conn->req_hdr, n, 0);
	if (-1 == n) {
		oct_log_error("send data to server error: %s", ERRMSG);
		return OCT_PROXY_STOP;
	}
	return OCT_PROXY_CONTINUE;
}

static int
oct_proxy_req_body(oct_conn_t *conn)
{
	ssize_t n;
	char buf[OCT_PROXY_BUF_LEN];

	n = recv(conn->client_fd, buf, sizeof(buf), 0);
	if (0 == n) {
		oct_log_info("client orderly shutdown");
		return OCT_PROXY_STOP;
	} else if (-1 == n) {
		oct_log_error("recv data from client error: %s", ERRMSG);
		return OCT_PROXY_STOP;
	}
	/* 把请求发送给服务器 */
	n = send(conn->server_fd, buf, n, 0);
	if (-1 == n) {
		oct_log_error("send data to server error: %s", ERRMSG);
		return OCT_PROXY_STOP;
	}
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

static int
oct_proxy_response(oct_conn_t *conn)
{
	ssize_t n;
	char buf[OCT_PROXY_BUF_LEN];

	n = recv(conn->server_fd, buf, sizeof(buf), 0);
	if (0 == n) {
		oct_log_info("server orderly shutdown");
		return OCT_PROXY_STOP;
	} else if (-1 == n) {
		oct_log_error("recv data from server error: %s", ERRMSG);
		return OCT_PROXY_STOP;
	}
	/* 把响应发送给客户端 */
	n = send(conn->client_fd, buf, n, 0);
	if (-1 == n) {
		oct_log_error("send data to client error: %s", ERRMSG);
		return OCT_PROXY_STOP;
	}
	return OCT_PROXY_CONTINUE;
}

void
oct_proxy_loop(int listen_fd)
{
	int epoll_fd = 0;
	struct epoll_event event;
	struct epoll_event events[EPOLL_MAX_EVENTS];
	int epoll_timeout = 10 * 1000;
	int loop = 1;

	/* 为了防止客户端断开，程序销毁连接后，服务器端的套接字仍有事件待处理，
	 * 导致在处理服务器发回数据时访问到已经销毁的连接内存块，维护一个已经销毁的
	 * data.ptr队列，每次销毁连接时将这个连接相关的data.ptr取值入队列 */
	oct_conn_t **destroyed_queue[EPOLL_MAX_EVENTS * 2];
	int destroyed_queue_len = 0;

	/* 创建epoll描述符 */
	if (-1 == (epoll_fd = epoll_create(2))) {
		oct_log_error("create epoll file descriptor error: %s", ERRMSG);
		return;
	}
	/* 先把监听套接字加入到epoll监听事件里面去 */
	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN;
	event.data.fd = listen_fd;
	if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event)) {
		oct_log_error("add listen fd to epoll error: %s", ERRMSG);
		return;
	}
	while (loop) {
		int i, j, nevents;
		nevents = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, epoll_timeout);
		if (-1 == nevents) {
			oct_log_error("epoll_wait error: %s", ERRMSG);
			continue;
		} else if (0 == nevents) {
			/* TODO 进行超时处理 */
		}
		destroyed_queue_len = 0;
		for (i = 0; i < nevents; i++) {
			struct epoll_event *ev = &events[i];
			oct_conn_t *conn = NULL;
			if (ev->data.fd == listen_fd) {
				/* 监听套接字可读，接收新连接 */
				oct_conn_t *new_conn = oct_proxy_accept(listen_fd);
				/* 如果accpet失败则继续处理其他事件 */
				if (NULL == new_conn) {
					continue;
				}
				/* accpet成功，把客户端套接字加入到epoll列表中 */
				new_conn->epoll_fd = epoll_fd;
				memset(&event, 0, sizeof(event));
				event.events = EPOLLIN;
				event.data.ptr = &new_conn->client_callback;
				if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD,
					new_conn->client_fd, &event)) {
					/* 如果添加到epoll列表失败则释放结构体，关闭连接 */
					oct_log_error("add client fd to epoll error: %s",
						ERRMSG);
					oct_conn_destroy(new_conn);
				}
				continue;
			}
			/* 判断epoll事件指针是否指向已经被销毁的连接结构，
			 * 如果是则不处理该事件 */
			for (j = 0; j < destroyed_queue_len; j++) {
				if (ev->data.ptr == destroyed_queue[j]) {
					break;
				}
			}
			if (j != destroyed_queue_len) {
				continue;
			}
			/* 处理epoll事件 */
			conn = *((oct_conn_t **)ev->data.ptr);
			if (ev->events & EPOLLIN) {
				if (ev->data.ptr == &conn->client_callback) {
					if (OCT_PROXY_STOP == oct_proxy_request(conn)) {
						/* 销毁连接前将可能出现在epoll事件中的指针放入
						 * 已销毁指针队列中 */
						destroyed_queue[destroyed_queue_len++] =
							&conn->client_callback;
						destroyed_queue[destroyed_queue_len++] =
							&conn->server_callback;
						oct_conn_destroy(conn);
					}
				} else if (ev->data.ptr == &conn->server_callback) {
					if (OCT_PROXY_STOP == oct_proxy_response(conn)) {
						/* 销毁连接前将可能出现在epoll事件中的指针放入
						 * 已销毁指针队列中 */
						destroyed_queue[destroyed_queue_len++] =
							&conn->client_callback;
						destroyed_queue[destroyed_queue_len++] =
							&conn->server_callback;
						oct_conn_destroy(conn);
					}
				}
			}
		}
	}
	/* 释放epoll资源 */
	close(epoll_fd);
}
