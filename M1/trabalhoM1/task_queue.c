#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "task_queue.h"
#include "utils.h"

Task taskQueue[50];
int taskCount = 0;
pthread_t th_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;
pthread_mutex_t stdout_mutex;

void initializeThreadPool() {
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        // printf("THREAD POOL SIZE: %d", THREAD_POOL_SIZE)        ; 
        if (pthread_create(&th_pool[i], NULL, &startThread, NULL) != 0) {
            logError("Falha ao criar a thread");
        }
    }
}

void executeTask(Task* task) {
    // usleep(500000);
    pthread_mutex_lock(&stdout_mutex);
    // printf("====\n");
    // printf("Thread %lu processando tarefa.\n", pthread_self());
    pthread_mutex_unlock(&stdout_mutex);

    task->taskFunction(task->arg1); 

    pthread_mutex_lock(&mutexQueue);
    if (taskCount == 0) {
        pthread_mutex_lock(&stdout_mutex);
        // printf("Fila vazia. Nenhuma conexão pendente.\n");
        // printf("====\n");
        pthread_mutex_unlock(&stdout_mutex);
    }
    pthread_mutex_unlock(&mutexQueue);
}

void* startThread(void* args) {
    while (1) {
        Task task;

        pthread_mutex_lock(&mutexQueue);
        while (taskCount == 0) {
            pthread_cond_wait(&condQueue, &mutexQueue);
        }

        task = taskQueue[0];
        for (int i = 0; i < taskCount - 1; i++) {
            taskQueue[i] = taskQueue[i + 1];
        }
        taskCount--;
        pthread_mutex_unlock(&mutexQueue);

        executeTask(&task);
    }
}

void submitTask(Task task) {
    pthread_mutex_lock(&mutexQueue);
    taskQueue[taskCount] = task;
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}

void cleanupResources() {
    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);
}
