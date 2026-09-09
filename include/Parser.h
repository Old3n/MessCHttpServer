#ifndef PARSER_H
#define PARSER_H
#define MAX_HEADERS 64

typedef struct {
    char *Method; 
    char *Path;
    char *Version;
    struct header {
        char *key; 
        char *value;
    } *headers;
    int num_headers;
    char *Body;
    int body_len;
} http_request;

int parse_request(char *raw, int len, http_request *req);
char *get_header(http_request *req, const char *key);
void free_request(http_request *req);

#endif
