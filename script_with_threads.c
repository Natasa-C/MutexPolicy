#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <limits.h>
#include <pthread.h>

pid_t getpid(void);
pid_t getppid(void);

// A structure to represent a queue
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    pid_t *array;
};

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1; // This is important, see the enqueue
    queue->array = (pid_t *)malloc(queue->capacity * sizeof(pid_t));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue *queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue *queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue *queue, pid_t item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    // printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue.
// It changes front and size
pid_t dequeue(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    pid_t item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
pid_t front(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}

// Function to get rear of queue
pid_t rear(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}

typedef struct
{
    int id;
    int locked;
    int used;
    struct Queue *queue;
    pid_t owner;
} Mutex;

int ind = 0;

void mutex_open(Mutex *mtx, pid_t pid)
{
    mtx->used = 1;
    mtx->id = ind;
    mtx->owner = pid;
    mtx->queue = createQueue(1000);
}

int mutex_close(Mutex *mtx)
{
    if (mtx->locked == 0)
    {
        mtx->used = 0;
        mtx->owner = 0;
        free(mtx->queue);
        return 0;
    }
    return -1;
}

void mutex_lock(Mutex *mtx, pid_t pid)
{
    if (mtx->locked)
    {
        enqueue(mtx->queue, pid);
        // printf("front %d\n", front(mtx->queue));
        while (mtx->locked || (!mtx->locked && front(mtx->queue) == pid))
            ;
    };
    mtx->locked = 1;
}

void mutex_unlock(Mutex *mtx)
{
    mtx->locked = 0;
    dequeue(mtx->queue);
}

Mutex mtx;


#define MAX_RESOURCES 5
int available_resources = MAX_RESOURCES;

int decrease_count(int count, int indi)
{
    if (count > MAX_RESOURCES)
    {
        printf("Resurse insuficiente.\n");
        return -1;
    }

    while (available_resources < count)
        ;
    mutex_lock(&mtx, indi);
    available_resources -= count;
    printf("Got %d resources %d remaining\n", count, available_resources);
    mutex_unlock(&mtx);
    return 0;
}

int increase_count(int count, int indi)
{
    mutex_lock(&mtx, indi);
    available_resources += count;
    printf("Released %d resources %d remaining\n", count, available_resources);
    mutex_unlock(&mtx);
    return 0;
}

void *
do_some_work(void *v)
{
    int *indi = (int *)v;
    int num = rand()%4 + 1;
    if (decrease_count(num, *indi) != -1)
    {
        increase_count(num, *indi);
    }
    free(v);
    return NULL;
}

int main()
{
    mutex_open(&mtx, getpid());
    int i;
    int num_of_threads = 5;
    int *arguments;
    pthread_t thr[num_of_threads];

    printf("MAX_RESOURCES: %d\n", MAX_RESOURCES);
    for (i = 0; i < num_of_threads; i++)
    {
        arguments = (int *)malloc(sizeof(int));
        *arguments = i;
        if (arguments == NULL)
        {
            printf("Memory allocation error.");
            return -1;
        }

        if (pthread_create(&thr[i], NULL, do_some_work, arguments))
        {
            perror(NULL);
            return errno;
        }
    }

    for (i = 0; i < num_of_threads; i++)
        if (pthread_join(thr[i], NULL))
        {
            perror(NULL);
            printf("%d", errno);
            return errno;
        }

    mutex_close(&mtx);
}
