#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <pthread.h>

#define THREAD_POOL_SIZE 4

typedef struct Task {
    void (*taskFunction)(void *); 
    void *arg1;
} Task;


// Variáveis globais
extern pthread_mutex_t mutexQueue;
extern pthread_cond_t condQueue;
extern Task taskQueue[50];
extern int taskCount;

// Funções
void initializeThreadPool();
void* startThread(void* args);
void submitTask(Task task);
void cleanupResources();
void executeTask(Task* task);

#endif // TASK_QUEUE_H
