#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "oct_log.h"
#include "oct_proxy.h"
#include "oct_thread.h"

static void *
oct_thread_main(void *argument)
{
	int client_fd = 0;
	const oct_thread_arg_t *arg = (const oct_thread_arg_t *)argument;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	char client_ip[16];
	oct_conn_t *conn = NULL;

	while (1) {
		/* 接收新的连接 */
		client_fd = accept(arg->listen_fd, (struct sockaddr *)&client_addr,
			&len);
		if (-1 == client_fd) {
			oct_log_error("accpet new connection error: %s", ERRMSG);
			continue;
		}
		/* 获取客户端IP */
		if (NULL == inet_ntop(AF_INET, &client_addr.sin_addr, client_ip,
			sizeof(client_ip))) {
			oct_log_warn("get client IP error: %s", ERRMSG);
		} else {
			oct_log_info("accpet new connection from %s:%d", client_ip,
				ntohs(client_addr.sin_port));
		}
		/* 初始化代理模块 */
		conn = oct_proxy_init();
		if (NULL == conn) {
			oct_log_error("init proxy module error");
			close(client_fd);
			continue;
		}
		conn->client_fd = client_fd;
		/* 进行代理 */
		oct_proxy_process(conn);
		/* 销毁代理模块 */
		oct_proxy_destroy(conn);
	}
	return NULL;
}

void
oct_thread_create(const oct_thread_arg_t *arg, int nthreads)
{
	int errnum, i;
	pthread_t tid;
	for (i = 0; i < nthreads; i++) {
		errnum = pthread_create(&tid, NULL, oct_thread_main, (void *)arg);
		if (errnum != 0) {
			oct_log_error("create worker thread error: %s", ERRMSG);
		}
		oct_log_info("create worker thread id: %lu", tid);
	}
}
