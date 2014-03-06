#include <sys/socket.h>

#include "oct_socket.h"

int
oct_set_so_reuseaddr(int fd)
{
	int flag = 1;
	int len = sizeof(flag);
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, len);
}
