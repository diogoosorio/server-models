#include <signal.h>
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
static const int BACKLOG_SIZE = 20;
static const int BUSY_WORK_SECONDS = 2;
static int EXIT = 0;

/**
 * Creates the socket and binds it to the correct address/port
 *
 * 1. socket() - creates the actual socket
 *  AF_INET -> IPv4
 *  SOCK_STREAM -> TCP
 *
 * 2. bind() - binds it to the address/port
 * 
 * 3. listen() - marks the socket as being passive (i.e. used to accept incoming connections)
 */
static int create_server() {
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

/**
 * Accepts a new connection over the socket
 */
static int accept_connection(int server_fd) {
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

/**
 * Reads the data sent over by the client over the socket
 *
 * Our protocol is simple:
 *
 *  * All messages are ASCII encoded (1 byte per char is assumed)
 *  * All messages sent to the server are terminated with a '\3' character
 *  * A 'goodbye\0' message signifies that the communcation is to end
 */
static void handle_connection(int connection_fd) {
    pid_t pid = getpid();
    printf("pid %d | processing a new client\n", pid);

    char delimiter = '\3';
    char buffer[50];
    int bytes_read;

    while(1) {
        memset(&buffer, ' ', sizeof(buffer));
        bytes_read = 0;

        do {
            bytes_read = read(connection_fd, &buffer, sizeof(buffer) - 1);
        } while (bytes_read >= 0 && strchr(buffer, delimiter) == NULL);

        if (bytes_read < 0) {
            printf("pid %d | error receiving data | %s\n", pid, buffer);
            break;
        }

        if (strncmp(buffer, "goodbye", 7) == 0) {
            printf("pid %d | received goodbye | %s\n", pid, buffer);
            break;
        } else {
            printf("pid %d | finished reading line | %s\n", pid, buffer);
            printf("pid %d | simulating some busy work for %d seconds \n", pid, BUSY_WORK_SECONDS);
            sleep(BUSY_WORK_SECONDS);
            write(connection_fd, "ack", 3);
            printf("pid %d | sent ack \n", pid);
        }
    }
}

/**
 * Signal trap function, toggles an exit flag to instruct the proccess
 * to stop executing
 */
static void exit_signal() {
    printf("Exit signal trapped...\n");

    EXIT = 1;
}

/**
 * Registers a basic trap function for SIGINT and SIGTERM
 */
static void trap_signals() {
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = exit_signal;

    sigaction(SIGINT, &action, 0);
    sigaction(SIGTERM, &action, 0);
}


int main() {
    trap_signals();

    int server_fd = create_server();
    if (server_fd == -1) {
        return 1;
    }

    while(EXIT == 0) {
        int connection_fd = accept_connection(server_fd);
        handle_connection(connection_fd);
        close(connection_fd);
    }
    close(server_fd);

    return 0;
}
