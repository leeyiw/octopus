#include <string.h>

#include "oct_http.h"

void
oct_http_parse_req_line(oct_conn_t *conn, char *line, int len)
{
}

void
oct_http_parse_req_hdr_fields(oct_conn_t *conn, char *field, int len)
{
	int i;
	char *key = field, *colon = NULL, *value = NULL;
	for (i = 0; i < len; i++) {
		if (field[i] == ':') {
			colon = &field[i];
			value = colon + 2;
			break;
		}
	}
	if (colon == NULL) {
		return;
	}
	if (conn->host != NULL && 0 == strncmp(key, "Host", colon - key)) {
		conn->host_len = len - (value - key);
		conn->host = value;
	}
}
