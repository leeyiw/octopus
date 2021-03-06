#include <string.h>

#include "oct_http.h"
#include "oct_log.h"

static void
oct_http_parse_req_line(oct_conn_t *conn, char *line, int len)
{
	oct_log_debug("request line: %.*s", len, line);
}

static void
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
	if (conn->host == NULL && 0 == strncmp(key, "Host", colon - key)) {
		conn->host_len = len - (value - key);
		conn->host = value;
	}
}

void
oct_http_parse_req_hdr(oct_conn_t *conn, ssize_t n)
{
	int line_start = 0;
	ssize_t i;
	for (i = 0; i < n; i++) {
		if (conn->req_hdr[i] == '\r' && conn->req_hdr[i + 1] == '\n') {
			/* 如果是第一行，则解析请求行，否则解析请求字段 */
			if (line_start == 0) {
				oct_http_parse_req_line(conn, &conn->req_hdr[line_start],
					i- line_start);
			} else {
				oct_http_parse_req_hdr_fields(conn, &conn->req_hdr[line_start],
					i - line_start);
			}
			/* 遇到两个\r\n表示到了头部的结尾 */
			if (conn->req_hdr[i + 2] == '\r' && conn->req_hdr[i + 3] == '\n') {
				conn->req_hdr_len = i + 2;
				break;
			}
			line_start = i + 2;
		}
	}
}

static void
oct_http_parse_status_line(oct_conn_t *conn, char *line, int len)
{
	oct_log_debug("status line: %.*s", len, line);
}

static void
oct_http_parse_rsp_hdr_fields(oct_conn_t *conn, char *field, int len)
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
	//if (conn->host == NULL && 0 == strncmp(key, "Host", colon - key)) {
	//	conn->host_len = len - (value - key);
	//	conn->host = value;
	//}
}

void
oct_http_parse_rsp_hdr(oct_conn_t *conn, ssize_t n)
{
	int line_start = 0;
	ssize_t i;
	for (i = 0; i < n; i++) {
		if (conn->rsp_hdr[i] == '\r' && conn->rsp_hdr[i + 1] == '\n') {
			/* 如果是第一行，则解析请求行，否则解析请求字段 */
			if (line_start == 0) {
				oct_http_parse_status_line(conn, &conn->rsp_hdr[line_start],
					i- line_start);
			} else {
				oct_http_parse_rsp_hdr_fields(conn, &conn->rsp_hdr[line_start],
					i - line_start);
			}
			/* 遇到两个\r\n表示到了头部的结尾 */
			if (conn->rsp_hdr[i + 2] == '\r' && conn->rsp_hdr[i + 3] == '\n') {
				conn->rsp_hdr_len = i + 2;
				break;
			}
			line_start = i + 2;
		}
	}
}
