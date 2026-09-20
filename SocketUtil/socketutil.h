#ifndef SOCKETUTIL_SOCKETUTIL_H
#define SOCKETUTIL_SOCKETUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

struct sockaddr_in* createIPv4Address(char *ip, int port);

int createTCPIpv4Socket();

struct accepted_socket {
    int accepted_socket_fd;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

void receiveAndPrintIncomingDataOnSeparateThread(struct accepted_socket *pSocket);

void sendReceiveMessageToTheOtherClients(char * buffer, int socket_fd);



#endif //SOCKETUTIL_SOCKETUTIL_H