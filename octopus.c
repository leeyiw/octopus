#include <sys/socket.h>

#include "oct_log.h"

int
main(int argc, const char *argv[])
{
	int listen_fd = 0;

	oct_log_info("actopus start/running");
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == listen(listen_fd, 64)) {
		oct_log_fatal("listen to socket error: %s", ERRMSG);
	}
	return 0;
}
