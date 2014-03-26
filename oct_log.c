#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "oct_log.h"

static const char *oct_log_level_str[5] = {
	"DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

void oct_log(uint8_t level, const char *file, int line, const char *fmt, ...)
{
	char log_data[OCT_LOG_MAX_LEN];
	char *ptr = log_data;
	time_t current_time = time(NULL);
	struct tm t;
	va_list ap;

	/* 获取当前时间 */
	localtime_r(&current_time, &t);
	/* 格式化日志头部 */
	ptr += snprintf(log_data, sizeof(log_data),
		"[%4d/%02d/%02d %02d:%02d:%02d] [%u] [%s] [%s:%d] ",
		t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
		t.tm_sec, pthread_self(), oct_log_level_str[level], file, line);
	/* 添加日志信息 */
	va_start(ap, fmt);
	vsnprintf(ptr, sizeof(log_data) - (ptr - log_data), fmt, ap);
	va_end(ap);
	printf("%s\n", log_data);
}
