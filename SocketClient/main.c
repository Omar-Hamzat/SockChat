#include "socketutil.h"

void listenAndPrint(int socket_fd) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(socket_fd, buffer, 1024, 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            printf("%s\n", buffer);
        }

        if (bytesReceived == 0) {
            break;
        }
    }

    close(socket_fd);
}

void startListeningAndPrintMessagesOnNewThread(int socket_fd) {
    pthread_t id;
    pthread_create(&id, NULL, (void *)listenAndPrint, (void *)socket_fd);
};

int main(){

    int SocketFD = createTCPIpv4Socket();
    struct sockaddr_in *serverAddress = createIPv4Address("127.0.0.1", 8080);


    int result = connect(SocketFD, (struct sockaddr*)serverAddress, sizeof(*serverAddress));

    if (result == 0){
        printf("Connection established successfully\n");
    }

    char *name = NULL;
    size_t name_size = 0;
    printf("what should we call you?: \n ");
    ssize_t name_count = getline(&name, &name_size, stdin);
    name[name_count-1]= '\0';

    char *line = NULL;
    size_t line_size = 0;
    printf("type a message (type exit to escape): \n ");


    startListeningAndPrintMessagesOnNewThread(SocketFD);

    char buffer[1024];

    while (true) {
        ssize_t char_count = getline(&line,&line_size,stdin);
        line[char_count-1]='\0';

        sprintf(buffer, "%s:%s", name, line);


        if (char_count>0) {
            if (strcmp(line, "exit") == 0) {
                break;
            }

            ssize_t amount_sent = send(SocketFD, buffer, strlen(buffer), 0);
        }
    }

    close(SocketFD);
    return 0;
}
