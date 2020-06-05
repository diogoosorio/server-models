#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "./lib/queue.h"

int main() {
    struct Queue* queue = createQueue(5);
    for (int i = 0; i < 5; i++) {
        printf("Enqueued %d: %d\n", i, enqueue(queue, i));
    }

    for (int i = 0; i < 5; i++) {
        printf("Dequeued: %d\n", dequeue(queue));
    }
}
