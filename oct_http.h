#ifndef OCT_HTTP_H_
#define OCT_HTTP_H_

#include <sys/types.h>

#include "oct_conn.h"

extern void oct_http_parse_req_hdr(oct_conn_t *conn, ssize_t n);
extern void oct_http_parse_rsp_hdr(oct_conn_t *conn, ssize_t n);

#endif  // OCT_HTTP_H_
