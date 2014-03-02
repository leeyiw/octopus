#ifndef OCT_THREAD_H_
#define OCT_THREAD_H_

#include <pthread.h>

typedef struct _oct_thread_arg_t {
	int listen_fd;
} oct_thread_arg_t;

typedef struct _oct_thread_info_t {
	pthread_t tid;
} oct_thread_info_t;

extern void oct_thread_create(const oct_thread_arg_t *arg, int nthreads);

#endif  // OCT_THREAD_H_
