#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "./lib/server.h"
#include "./lib/queue.h"
#include "./lib/signals.h"

static const int BUSY_WORK_SECONDS = 1;
static const int MAX_QUEUE_SIZE = 20;
static const int NUMBER_WORKERS = 5;

static pthread_t worker_threads[5];
static struct Queue *queue;
static int kill_server = 0;

static void handle_connection(int connection_fd) {
    int tid = (int)pthread_self();
    printf("pthread %X | processing a new client\n", tid);

    char buffer[50];
    int bytes_read;

    while(1) {
        memset(&buffer, ' ', sizeof(buffer));
        bytes_read = read_line(connection_fd, buffer, 50);

        if (bytes_read < 0) {
            printf("pthread %X | error reciving data | %s\n", tid, buffer);
            break;
        }

        if (strncmp(buffer, "goodbye", 7) == 0) {
            printf("pthread %X | received goodbye | %s\n", tid, buffer);
            break;
        }
        
        printf("pthread %X | finished reading line | %s\n", tid, buffer);
        printf("pthread %X | simulating some busy work for %d seconds \n", tid, 2);
        sleep(BUSY_WORK_SECONDS);
        write(connection_fd, "ack", 3);
        printf("pthread %X | sent ack \n", tid);
    }
}

static void *start_worker() {
    int tid = (int)pthread_self();
    printf("pthread %X | starting worker\n", tid);

    while (kill_server == 0) {
        int connection = dequeue(queue);

        if (connection != E_QUEUE_EMPTY) {
            handle_connection(connection);
        }
    }
    printf("pthread %X | stopping worker\n", tid);

    return NULL;
}

static void start_listening() {
    int tid = (int)pthread_self();
    printf("pthread %X | starting server\n", tid);

    int server_fd = create_server();

    while(kill_server == 0) {
       int connection = accept_connection(server_fd);
       enum QueueError result = enqueue(queue, connection);

       if (result == E_SUCCESS) {
           printf("pthread %X | Enqueue a new connection for processing: %d\n", tid, connection);
       } else {
           printf("pthread %X | Error enqueueing a new connection: %d\n", tid, connection);
           close(connection);
       }
    }

    printf("pthread %X | stopping server\n", tid);
    close(server_fd);
}

int main() {
    queue = createQueue(MAX_QUEUE_SIZE);
    trap_exit(&kill_server);

    // launch worker threads
    for (int i = 0; i < NUMBER_WORKERS; i++) {
        if (pthread_create(&worker_threads[i], NULL, start_worker, NULL) != 0) {
            printf("Failed to start a worker thread!\n");
            return 1;
        }
    }

    start_listening();

    for (int i = 0; i < NUMBER_WORKERS; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    printf("pthread %X | all workers stopped, exiting...\n", (int)pthread_self());

    return 0;
}
