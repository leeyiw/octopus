#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "oct_log.h"
#include "oct_socket.h"
#include "oct_thread.h"

int
main(int argc, const char *argv[])
{
	int listen_fd = 0;
	struct sockaddr_in proxy_addr;
	const char *ip = "0.0.0.0";
	uint16_t port = 8080;

	oct_log_info("actopus start/running");
	/* 初始化套接字 */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == listen_fd) {
		oct_log_fatal("create socket error: %s", ERRMSG);
		exit(EXIT_FAILURE);
	}
	/* 设置地址重用 */
	if (-1 == oct_set_so_reuseaddr(listen_fd)) {
		oct_log_warn("set socket reuse address error: %s", ERRMSG);
		exit(EXIT_FAILURE);
	}
	/* 设置监听地址 */
	memset(&proxy_addr, 0, sizeof(proxy_addr));
	proxy_addr.sin_family = AF_INET;
	proxy_addr.sin_port = htons(port);
	if (1 != inet_pton(AF_INET, ip, &proxy_addr.sin_addr)) {
		oct_log_fatal("set proxy ip address to %s error", ip);
		exit(EXIT_FAILURE);
	}
	/* 绑定监听地址到套接字上 */
	if (-1 == bind(listen_fd, (const struct sockaddr *)&proxy_addr,
		sizeof(proxy_addr))) {
		oct_log_fatal("bind socket to address %s:%d error: %s", ip, port,
			ERRMSG);
		exit(EXIT_FAILURE);
	}
	/* 开始监听套接字 */
	if (-1 == listen(listen_fd, 64)) {
		oct_log_fatal("listen to socket error: %s", ERRMSG);
		exit(EXIT_FAILURE);
	}
	/* 创建线程处理请求 */
	oct_thread_arg_t arg;
	arg.listen_fd = listen_fd;
	oct_thread_create(&arg, 25);
	/* TODO 循环侦测线程状态，判断是否需要添加线程 */
	while (1) {}
	return 0;
}
