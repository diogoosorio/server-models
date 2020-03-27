#include <errno.h>
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
#include <sys/epoll.h>

static const char* SERVER_ADDR = "127.0.0.1";
static const int SERVER_PORT = 3000;
static const int REQUEST_LINE_CHARS = 50;
static const int BACKLOG_SIZE = 20;
static const int MAX_EVENTS = 10;

/**
 * Creates the socket and binds it to the correct address/port
 *
 * 1. socket() - creates the actual socket
 *  AF_INET -> IPv4
 *  SOCK_STREAM -> TCP
 *  SOCK_NONBLOCK -> Tell the kernel to treat the socket as non-blocking
 *
 * 2. bind() - binds it to the address/port
 * 
 * 3. listen() - marks the socket as being passive (i.e. used to accept incoming connections)
 */
int create_server() {
    int socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (socket_fd == -1) {
        printf("Unable to create the socket: %s\n", strerror(errno));
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
        printf("Unable to bind the server to %s:%d - %s\n", SERVER_ADDR, SERVER_PORT, strerror(errno));
        return -1;
    }

    int listen_result = listen(socket_fd, BACKLOG_SIZE);
    if (listen_result == -1) {
        printf("Failed to transition the socket into passive mode: %s\n", strerror(errno));
        return -1;
    }

    printf("Server listening on: %s:%d\n", SERVER_ADDR, SERVER_PORT);

    return socket_fd;
}

/**
 * Creates the epoll instance
 *
 * 1. epoll_create() - create the actual epoll instance
 * 2. epoll_ctl() - adds the server socket to the "interest list" of the epoll instance
 */
int setup_epoll(int server_fd) {
    int epoll_fd = epoll_create(MAX_EVENTS);
    if (epoll_fd == -1) {
        printf("Failed to create the epoll file descriptor: %s\n", strerror(errno));
        return -1;
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        printf("Add accept to the epoll fd failed: %s\n", strerror(errno));
        return -1;
    }

    return epoll_fd;
}

/**
 * Accepts a new connection over the socket
 *
 * Once the accept() is issued (which is also a non-blocking syscall, due to the 
 * fact that the server socket having been created with a EO_NONBLOCK flag), the 
 * new file descriptor is added to epoll's "list of interest".
 *
 * Once data is received from the client, an epoll event will be triggered and
 * we can have the process reading it (vs blocking on `read` until there's
 * actually something to read from the buffer).
 */
int accept_connection(int server_fd, int epoll_fd) {
    struct sockaddr_in client_addr;
    int client_length = sizeof(client_addr);
    memset(&client_addr, '\0', sizeof(client_length));

    int connection_fd = accept(
            server_fd,
            (struct sockaddr*)&client_addr,
            (socklen_t*)&client_length);

    if (connection_fd == -1) {
        printf("Failed accepting the client connection: %s\n", strerror(errno));
        return -1;
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = connection_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_fd, &event) == -1) {
        printf("Add read to the epoll fd failed: %s\n", strerror(errno));
        return -1;
    }

    return connection_fd;
}

/**
 * Reads the payload from an accept()'ed connection and
 * prints the received payload
 */
void handle_connection(int connection_fd) {
    char buffer[REQUEST_LINE_CHARS + 1];
    int bytes_read;

    memset(&buffer, '\0', sizeof(buffer));
    bytes_read = read(connection_fd, &buffer, sizeof(char) * REQUEST_LINE_CHARS);
    
    if (bytes_read <= 0) {
        return;
    }

    buffer[REQUEST_LINE_CHARS] = '\0';
    printf("%s\n", buffer);
}

/**
 * Loops waiting for epoll events (a.k.a. the "event loop")
 *
 * 1. epoll_wait() - blocks until new events are available
 *  * Returns the number of available events (number_ready)
 *  * The events pointer points to memory address with the actual event data
 *
 * 2. The same epoll instance is used to monitor two differnt type of events:
 *  * Incoming client connection over the server's TCP socket
 *  * accept()'ed connections ready to be processed
 *  * One can imagine a more robust architecture to handle other type of events :)
 */
int begin_polling(int server_fd, int epoll_fd) {
    int number_ready;
    struct epoll_event events[MAX_EVENTS];

    while(1) {
        number_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (number_ready == -1) {
            printf("Error performing the epoll_wait: %s\n", strerror(errno));
            return -1;
        }

        for (int i = 0; i < number_ready; i++) {
            if (events[i].data.fd == server_fd) {
                accept_connection(server_fd, epoll_fd);
            } else {
                handle_connection(events[i].data.fd);
            }
        }
    }
}

int main() {
    int server_fd = create_server();
    if (server_fd == -1) {
        return 1;
    }

    int epoll_fd = setup_epoll(server_fd);
    if (epoll_fd == -1) {
        return 1;
    }

    if (begin_polling(server_fd, epoll_fd) == -1) {
        return 1;
    }

    return 0;
}
