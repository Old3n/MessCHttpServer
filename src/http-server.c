#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Parser.h"

#define BUF_SIZE 65536
#define WWW_ROOT "./www"


const char *get_content_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".ico") == 0) return "image/x-icon";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    return "application/octet-stream";
}

void send_response(int cfd, const char *status, const char *ctype, const char *body, int body_len) {
    char header[1024];
    int hlen = snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, ctype, body_len);
    write(cfd, header, hlen);
    if (body && body_len > 0)
        write(cfd, body, body_len);
}

void handle_get(int cfd, http_request *req) {
    char filepath[512];
    if (strcmp(req->Path, "/") == 0)
        snprintf(filepath, sizeof(filepath), "%s/index.html", WWW_ROOT);
    else
        snprintf(filepath, sizeof(filepath), "%s%s", WWW_ROOT, req->Path);

    struct stat st;
    if (stat(filepath, &st) == -1) {
        const char *msg = "404 Not Found";
        send_response(cfd, "404 Not Found", "text/plain", msg, strlen(msg));
        return;
    }

    int file_fd = open(filepath, O_RDONLY);
    if (file_fd == -1) {
        const char *msg = "403 Forbidden";
        send_response(cfd, "403 Forbidden", "text/plain", msg, strlen(msg));
        return;
    }

    char *fbuf = malloc(st.st_size);
    read(file_fd, fbuf, st.st_size);
    close(file_fd);

    send_response(cfd, "200 OK", get_content_type(filepath), fbuf, st.st_size);
    free(fbuf);
}

void handle_post(int cfd, http_request *req) {
    printf("Body: %.*s\n", req->body_len, req->Body ? req->Body : "");

    const char *ctype = "text/plain";
    char *req_ct = get_header(req, "Content-Type");
    if (req_ct) ctype = req_ct;

    if (req->Body)
        send_response(cfd, "200 OK", ctype, req->Body, req->body_len);
    else
        send_response(cfd, "200 OK", "text/plain", "OK", 2);
}


int main () {
    struct sockaddr_in serverInfo = {0};
    struct sockaddr_in clientInfo = {0};
    uint clientSize = 0;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = 0;
    serverInfo.sin_port = ntohs(8080);
    int fd = socket(AF_INET, SOCK_STREAM, 0 );
    if(fd == -1){
       perror("socket");
       return -1;
    }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(fd , (struct sockaddr*)&serverInfo,sizeof(serverInfo))==-1){
        perror("bind");
        close(fd);
        return -1;
    }
    if(listen(fd , 0)== -1){
        perror("listen"); 
        close(fd); 
        return -1;
    }
    printf("listening on port 8080\n");

    while (1) {
        clientSize = sizeof(clientInfo);
        int cfd = accept(fd , (struct sockaddr*)&clientInfo,&clientSize);
        if (cfd == -1 ) {
            perror("accept");
            continue;
        }

        char buf[BUF_SIZE] = {0};
        int bytes = read(cfd, buf, sizeof(buf) - 1);
        if (bytes <= 0) {
            close(cfd);
            continue;
        }

        http_request req = {0};
        if (parse_request(buf, bytes, &req) == -1) {
            send_response(cfd, "400 Bad Request", "text/plain", "400 Bad Request", 15);
            close(cfd);
            continue;
        }

        printf("%s %s\n", req.Method, req.Path);

        if (strcmp(req.Method, "GET") == 0)
            handle_get(cfd, &req);
        else if (strcmp(req.Method, "POST") == 0)
            handle_post(cfd, &req);
        else {
            const char *msg = "405 Method Not Allowed";
            send_response(cfd, "405 Method Not Allowed", "text/plain", msg, strlen(msg));
        }

        free_request(&req);
        close(cfd);
    }

    close(fd);
    return 0;
} 
