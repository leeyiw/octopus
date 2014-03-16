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
	const oct_thread_arg_t *arg = (const oct_thread_arg_t *)argument;

	oct_proxy_loop(arg->listen_fd);

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
