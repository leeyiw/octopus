#ifndef OCT_LOG_H_
#define OCT_LOG_H_

#include <errno.h>
#include <stdint.h>
#include <string.h>

#define OCT_LOG_MAX_LEN		2048

#define OCT_LOG_DEBUG		0
#define OCT_LOG_INFO		1
#define OCT_LOG_WARN		2
#define OCT_LOG_ERROR		3
#define OCT_LOG_FATAL		4

#define oct_log_debug(...) \
	oct_log(OCT_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define oct_log_info(...) \
	oct_log(OCT_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define oct_log_warn(...) \
	oct_log(OCT_LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define oct_log_error(...) \
	oct_log(OCT_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define oct_log_fatal(fmt, ...) \
	oct_log(OCT_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define ERRMSG strerror(errno)

void oct_log(uint8_t level, const char *file, int line, const char *fmt, ...);

#endif  // OCT_LOG_H_
