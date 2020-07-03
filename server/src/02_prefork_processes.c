#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "./lib/server.h"
#include "./lib/signals.h"

static const int BUSY_WORK_SECONDS = 1;
static const int NUMBER_WORKERS = 5;
static int WORKER_PIDS[5];

static void handle_connection(int connection_fd) {
    pid_t pid = getpid();
    printf("pid %d | processing a new client\n", pid);

    char buffer[50];
    int bytes_read;

    while(1) {
        memset(&buffer, ' ', sizeof(buffer));
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

static int start_worker(int server_fd, int *exit) {
    while (*exit == 0) {
        int connection_fd = accept_connection(server_fd);
        handle_connection(connection_fd);
        close(connection_fd);
        printf("pid %d | closed client connection\n", getpid());
    }

    return 0;
}

static void wait_workers() {
    // this is extremely optimistic (e.g. what if the worker fails
    // to respect the signal?)...
    printf("pid: %d | Waiting for workers to kill themselves...\n", getpid());
    int waitpid;
    do { waitpid = wait(NULL); } while (waitpid > 0);
    printf("pid: %d | Done! All workers commited suicied!\n", getpid());
}

int main() {
    int exit = 0;
    int server_fd = create_server();
    if (server_fd == -1) {
        return 1;
    }

    for (int i = 0; i < NUMBER_WORKERS; i++) {
        int forked = fork();

        if (forked == 0) {
            return start_worker(server_fd, &exit);
        }
        
        if (forked == -1) {
            printf("pid %d | Error forking process #%d\n", getpid(), i);
        }

        if (forked > 0) {
            printf("pid %d | Forked a new worker with pid: %d\n", getpid(), forked);
            WORKER_PIDS[i] = forked;
        }
    }

    trap_exit(&exit);

    while(exit == 0) {
        // do nothing... this is where a decent web-server would allocate
        // some time doing some recurrent sanity checks, for example check
        // if any of the works is unhealthy (using a shared memory address 
        // space to exchange data, for example) and kill them.
    }

    wait_workers();
    close(server_fd);

    return 0;
}
