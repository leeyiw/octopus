#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

#include "oct_network.h"

int
oct_connect_nonb(int sockfd, const struct sockaddr *addr, socklen_t addrlen,
	int nsec)
{
	int flags, n, error;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;

	/* 设置套接字为非阻塞 */
	if (-1 == (flags = fcntl(sockfd, F_GETFL, 0))) {
		return -1;
	}
	if (-1 == (flags = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK))) {
		return -1;
	}

	error = 0;
	if ((n = connect(sockfd, addr, addrlen)) < 0) {
		if (errno != EINPROGRESS) {
			return -1;
		}
	}
	/* connect如果直接成功则马上返回 */
	if (n == 0) {
		goto done;
	}
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;
	if ((n = select(sockfd + 1, &rset, &wset, NULL,
		nsec ? &tval : NULL)) == 0) {
		close(sockfd);
		errno = ETIMEDOUT;
		return -1;
	}
	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
		len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
			return -1;
		}
	} else {
		return -1;
	}

done:
	if (-1 == fcntl(sockfd, F_SETFL, flags)) {
		return -1;
	}
	if (error) {
		close(sockfd);
		errno = error;
		return -1;
	}
	return 0;
}
