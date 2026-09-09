#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include "Parser.h"


int parse_request(char *raw, int len, http_request *req) {
    char *buf = strndup(raw, len);
    if (!buf) return -1;

    memset(req, 0, sizeof(http_request));
    req->headers = malloc(sizeof(struct header) * MAX_HEADERS);
    if (!req->headers) { free(buf); return -1; }

    char *pos = buf;

    // request line
    char *line_end = strstr(pos, "\r\n");
    if (!line_end) { free(buf); return -1; }
    *line_end = '\0';

    char *sp1 = strchr(pos, ' ');
    if (!sp1) { free(buf); return -1; }
    *sp1 = '\0';
    req->Method = strdup(pos);

    char *sp2 = strchr(sp1 + 1, ' ');
    if (!sp2) { free(buf); return -1; }
    *sp2 = '\0';
    req->Path = strdup(sp1 + 1);
    req->Version = strdup(sp2 + 1);

    pos = line_end + 2;

    // headers
    req->num_headers = 0;
    while (1) {
        line_end = strstr(pos, "\r\n");
        if (!line_end) break;
        if (line_end == pos) {
            pos = line_end + 2;
            break;
        }
        *line_end = '\0';

        char *colon = strchr(pos, ':');
        if (colon && req->num_headers < MAX_HEADERS) {
            *colon = '\0';
            char *val = colon + 1;
            while (*val == ' ') val++;
            req->headers[req->num_headers].key = strdup(pos);
            req->headers[req->num_headers].value = strdup(val);
            req->num_headers++;
        }
        pos = line_end + 2;
    }

    // body
    char *body_start = strstr(raw, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        int remaining = len - (body_start - raw);
        if (remaining > 0) {
            req->Body = strndup(body_start, remaining);
            req->body_len = remaining;
        }
    }

    free(buf);
    return 0;
}

char *get_header(http_request *req, const char *key) {
    for (int i = 0; i < req->num_headers; i++) {
        if (strcasecmp(req->headers[i].key, key) == 0)
            return req->headers[i].value;
    }
    return NULL;
}

void free_request(http_request *req) {
    free(req->Method);
    free(req->Path);
    free(req->Version);
    free(req->Body);
    for (int i = 0; i < req->num_headers; i++) {
        free(req->headers[i].key);
        free(req->headers[i].value);
    }
    free(req->headers);
}
