#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void freeQueue(struct Queue* queue) {
    free(queue->items);
    queue->items = NULL;

    free(queue);
}

struct Queue* createQueue(int capacity) {
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));

    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    queue->items = (int*)malloc(queue->capacity * sizeof(int));

    return queue;
}

enum QueueError enqueue(struct Queue* queue, int item) {
    if (queue->size == queue->capacity) {
        return E_QUEUE_FULL;
    }

    if (queue->head == queue->capacity) {
        queue->head = 0;
    }

    queue->items[queue->head] = item;
    queue->head = queue->head + 1;
    queue->size = queue->size + 1;

    return E_SUCCESS;
}

int dequeue(struct Queue* queue) {
    if (queue->size == 0) {
        return E_QUEUE_EMPTY;
    }

    if (queue->tail == queue->capacity) {
        queue->tail = 0;
    }

    int item = queue->items[queue->tail];
    queue->tail = queue->tail + 1;
    queue->size = queue->size - 1;

    return item;
}
