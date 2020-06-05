#include <signal.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include "./lib/server.h"

static const int BUSY_WORK_SECONDS = 2;
static int EXIT = 0;

static void handle_connection(int connection_fd) {
    pid_t pid = getpid();
    printf("pid %d | processing a new client\n", pid);

    char buffer[50];
    int bytes_read;

    while(1) {
        bytes_read = read_line(connection_fd, buffer, 50);

        if (bytes_read < 0) {
            printf("pid %d | error receiving data | %s\n", pid, buffer);
            break;
        }

        if (strncmp(buffer, "goodbye", 7) == 0) {
            printf("pid %d | received goodbye | %s\n", pid, buffer);
            break;
        }
        
        printf("pid %d | finished reading line | %s\n", pid, buffer);
        printf("pid %d | simulating some busy work for %d seconds \n", pid, BUSY_WORK_SECONDS);
        sleep(BUSY_WORK_SECONDS);
        write(connection_fd, "ack", 3);
        printf("pid %d | sent ack \n", pid);
    }
}

static void exit_signal() {
    printf("Exit signal trapped...\n");

    EXIT = 1;
}

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
