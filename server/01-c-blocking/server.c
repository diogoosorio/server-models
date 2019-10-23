#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

static const char* SERVER_ADDR = "127.0.0.1";
static const int SERVER_PORT = 3000;
static const int BACKLOG_SIZE = 2;
static const int REQUEST_LINE_CHARS = 50;

int create_server() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("Unable to create the socket\n");
        return -1;
    }

    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    struct sockaddr_in server_addr;
    memset(&server_addr, '\0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server_addr.sin_port = htons(SERVER_PORT);

    int bind_result = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_result == -1) {
        printf("Unable to bind the server to %s:%d\n", SERVER_ADDR, SERVER_PORT);
        return -1;
    }

    printf("Server listening on: %s:%d\n", SERVER_ADDR, SERVER_PORT);

    return socket_fd;
}

int client_connection(int server_fd) {
    int listen_result = listen(server_fd, BACKLOG_SIZE);
    if (listen_result == -1) {
        printf("Failed to transition the socket into passive mode\n");
        return -1;
    }

    struct sockaddr_in client_addr;
    int client_length = sizeof(client_addr);
    memset(&client_addr, '\0', sizeof(client_length));

    int connection_fd = accept(
            server_fd,
            (struct sockaddr*)&client_addr,
            (socklen_t*)&client_length);

    if (connection_fd == -1) {
        printf("Failed accepting the client connection\n");
        return -1;
    }

    return connection_fd;
}

void handle_connection(int connection_fd) {
    long thread_id = (long)pthread_self();
    printf("thread %lu | processing a new client\n", thread_id);

    char buffer[REQUEST_LINE_CHARS + 1];
    int bytes_read;

    while(1) {
        memset(&buffer, '\0', sizeof(buffer));
        bytes_read = read(connection_fd, &buffer, sizeof(char) * REQUEST_LINE_CHARS);
        
        if (bytes_read <= 0) {
            printf("thread %lu | no bytes read, closing connection\n", thread_id);
            break;
        }

        buffer[REQUEST_LINE_CHARS] = '\0';
        printf("thread %lu | finished reading line | %s\n", thread_id, buffer);
    }
}

int main() {
    int server_fd = create_server();
    if (server_fd == -1) {
        return 1;
    }

    while(1) {
        int connection_fd = client_connection(server_fd);
        handle_connection(connection_fd);
        close(connection_fd);
    }
    close(server_fd);

    return 0;
}
