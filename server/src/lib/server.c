#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

static const char* SERVER_ADDR = "127.0.0.1";
static const int SERVER_PORT = 3000;
static const int BACKLOG_SIZE = 20;
static const char DELIMITER = '\3';

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

    int listen_result = listen(socket_fd, BACKLOG_SIZE);
    if (listen_result == -1) {
        printf("Failed to transition the socket into passive mode\n");
        return -1;
    }


    return socket_fd;
}

int accept_connection(int server_fd) {
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

int read_line(int connection, char *buffer, int buffer_size) {
    int bytes_read = 0;

    do {
        bytes_read = read(connection, buffer, buffer_size - 1);
    } while (bytes_read >= 0 && buffer[bytes_read] == DELIMITER);

    if (bytes_read > 0) {
        buffer[buffer_size - 1] = '\0';
    }

    return bytes_read;
}

