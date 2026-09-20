#include "socketutil.h"


int createTCPIpv4Socket(){
    return socket(AF_INET, SOCK_STREAM, 0);
}


struct sockaddr_in* createIPv4Address(char *ip, int port) {
    struct sockaddr_in *serverAddress = malloc(sizeof(struct sockaddr_in));
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(port);

    if (strlen(ip) == 0) {
        serverAddress->sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, ip, &serverAddress->sin_addr.s_addr);
    }
    inet_pton(AF_INET, ip, &serverAddress->sin_addr.s_addr);
    return serverAddress;
}