#include <stdint.h>
#include <stdio.h>

void oct_log(uint8_t level, const char *file, int line, const char *fmt, ...)
{
	char log_data[OCT_LOG_MAX_LEN];
	snprintf(log_data, sizeof(log_data), "%s:%c %s", file, line, fmt);
	printf("%s", log_data);
}
