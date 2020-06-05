#include <pthread.h>
#include <limits.h>

enum QueueError {
    E_SUCCESS = 0,
    E_QUEUE_FULL = -1,
    E_QUEUE_EMPTY = INT_MIN,
};

struct Queue {
    int head, tail, size, capacity;
    int* items;
    pthread_mutex_t m_queue;
};

void freeQueue(struct Queue* queue);

struct Queue* createQueue(int capacity);

enum QueueError enqueue(struct Queue* queue, int item);

int dequeue(struct Queue* queue);
