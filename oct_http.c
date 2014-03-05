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
	char *colon = NULL;
	for (i = 0; i < len; i++) {
		if (colon == NULL && field[i] == ':') {
		}
	}
}
