#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>


int main () {
    struct sockaddr_in serverInfo = {0};
    struct sockaddr_in clientInfo = {0};
    uint clientSize = 0;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = 0;
    serverInfo.sin_port = ntohs(80);
    int fd = socket(AF_INET, SOCK_STREAM, 0 );
    if(fd == -1){
       perror("socket");
       return -1;
    }
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
    int cfd = 0; 
    cfd = accept(fd , (struct sockaddr*)&clientInfo,&clientSize);
    if (cfd == -1 ) {
        perror("cfd");
        close(fd);
        return -1;
    } 
    read 
} 
