#include "socketutil.h"

struct accepted_socket* acceptIncomingConnection(int server_socket_fd) {
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_address, &client_address_size);

    struct accepted_socket* acceptedSocket = malloc(sizeof(struct accepted_socket));
    acceptedSocket->accepted_socket_fd = client_socket_fd;
    acceptedSocket->address = client_address;
    acceptedSocket->acceptedSuccessfully = client_socket_fd >= 0;

    if (!acceptedSocket->acceptedSuccessfully) {
        acceptedSocket->error = errno;
    }
    return acceptedSocket;
}

void receiveAndPrintIncomingData(int socket_fd) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            printf("%s\n", buffer);

            sendReceiveMessageToTheOtherClients(buffer, socket_fd);
        }

        if (bytesReceived == 0) {
            break;
        }
    }

    close(socket_fd);
}

void receiveAndPrintIncomingDataOnSeparateThread(struct accepted_socket *pSocket) {
    pthread_t id;
    pthread_create(&id, NULL, (void *)receiveAndPrintIncomingData, (void *)pSocket->accepted_socket_fd);
}

struct accepted_socket accepted_sockets[10];
int accepted_sockets_count = 0;

void startAcceptingIncomingConnections(int server_socket_fd) {
    while (true) {
        struct accepted_socket* client_socket = acceptIncomingConnection(server_socket_fd);
        accepted_sockets[accepted_sockets_count++] = *client_socket;
        receiveAndPrintIncomingDataOnSeparateThread(client_socket);
    }
}

void sendReceiveMessageToTheOtherClients(char *buffer, int socket_fd) {

    for (int i = 0; i < accepted_sockets_count; i++) {
        if (accepted_sockets[i].accepted_socket_fd != socket_fd) {
            send(accepted_sockets[i].accepted_socket_fd, buffer, strlen(buffer), 0);
        }
    }
}

int main() {
    int server_socket_fd = createTCPIpv4Socket();

    struct sockaddr_in *server_address = createIPv4Address("",8080);

    int result = bind(server_socket_fd, (struct sockaddr*)server_address, sizeof(*server_address));

    if (result == 0) {
        printf("socket bound successfully\n");
    }

    int listen_result = listen(server_socket_fd, 10);
    if (listen_result == 0) {
        printf("socket listen successfully\n");
    }

    printf("Waiting for client...\n");

    // printf("Client connected!\n");
    startAcceptingIncomingConnections(server_socket_fd);

    shutdown(server_socket_fd, SHUT_RDWR);

    return 0;
}
