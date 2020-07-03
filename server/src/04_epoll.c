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
#include "./lib/server.h"

static const int MAX_EVENTS = 10;

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

int watch_connection(int connection_fd, int epoll_fd) {
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = connection_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_fd, &event) == -1) {
        printf("Add read to the epoll fd failed: %s\n", strerror(errno));
        return -1;
    }

    return connection_fd;
}

void handle_connection(int connection_fd) {
    char buffer[50];
    int bytes_read;

    memset(&buffer, ' ', sizeof(buffer));
    bytes_read = read_line(connection_fd, buffer, 50);
    
    if (bytes_read <= 0) {
        return;
    }


    if (strncmp(buffer, "goodbye", 7) == 0) {
        printf("received goodbye | %s\n", buffer);
        return;
    }
    
    printf("finished reading line | %s\n", buffer);
    write(connection_fd, "ack", 3);
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
                int connection_fd = accept_connection(server_fd, epoll_fd);
                watch_connection(connection_fd, epoll_fd);
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
