#include <limits.h>

enum QueueError {
    E_SUCCESS = 0,
    E_QUEUE_FULL = -1,
    E_QUEUE_EMPTY = INT_MIN,
};

struct Queue {
    int head, tail, size, capacity;
    int* items;
};

void freeQueue(struct Queue* queue);

struct Queue* createQueue(int capacity);

enum QueueError enqueue(struct Queue* queue, int item);

int dequeue(struct Queue* queue);
