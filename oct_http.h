#ifndef OCT_HTTP_H_
#define OCT_HTTP_H_

#include "oct_proxy.h"

extern void oct_http_parse_req_line(oct_conn_t *conn, char *line, int len);
extern void oct_http_parse_req_hdr_fields(oct_conn_t *conn, char *field,
	int len);

#endif  // OCT_HTTP_H_
