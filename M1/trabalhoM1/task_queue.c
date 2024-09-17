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

/**
 * Função para inicializar o pool de threads.
 * Cria um número fixo de threads definidas por THREAD_POOL_SIZE, que ficarão aguardando tarefas.
 */
void initializeThreadPool() {
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&th_pool[i], NULL, &startThread, NULL) != 0) {
            logError("Falha ao criar a thread");
        }
    }
}

/**
 * Função para executar uma tarefa.
 * Cada thread chama essa função para processar a tarefa designada.
 * 
 * @param task Ponteiro para a tarefa a ser executada.
 */
void executeTask(Task* task) {
    pthread_mutex_lock(&stdout_mutex);
    printf("Thread %lu está processando a tarefa.\n", pthread_self());
    pthread_mutex_unlock(&stdout_mutex);

    task->taskFunction(task->arg1);

    pthread_mutex_lock(&stdout_mutex);
    printf("Thread %lu terminou a tarefa.\n", pthread_self());
    pthread_mutex_unlock(&stdout_mutex);
}

/**
 * Função executada por cada thread do pool.
 * As threads ficam em loop, aguardando tarefas na fila. 
 * 
 * @param args Argumentos da thread (não utilizados aqui).
 * @return Retorna NULL (padrão para threads).
 */
void* startThread(void* args) {
    while (1) {
        Task task;

        // Bloqueia a fila de tarefas para retirar uma tarefa
        pthread_mutex_lock(&mutexQueue);
        
        // Se a fila de tarefas estiver vazia, a thread entra em modo de espera
        while (taskCount == 0) {
            pthread_mutex_lock(&stdout_mutex);
            printf("Thread %lu entrou em modo de standby, aguardando novas tarefas...\n", pthread_self());
            pthread_mutex_unlock(&stdout_mutex);

            // A thread aguarda por novas tarefas
            pthread_cond_wait(&condQueue, &mutexQueue);
        }

        // Retira a tarefa da fila
        task = taskQueue[0];
        for (int i = 0; i < taskCount - 1; i++) {
            taskQueue[i] = taskQueue[i + 1];
        }
        taskCount--;

        // Desbloqueia a fila de tarefas
        pthread_mutex_unlock(&mutexQueue);

        // Executa a tarefa
        executeTask(&task);
    }
}

/**
 * Função para submeter uma nova tarefa à fila.
 * Adiciona uma tarefa à fila e sinaliza que há uma nova tarefa para ser processada.
 * 
 * @param task A tarefa a ser adicionada à fila.
 */
void submitTask(Task task) {
    pthread_mutex_lock(&mutexQueue);

    taskQueue[taskCount] = task;
    taskCount++;

    pthread_mutex_lock(&stdout_mutex);
    printf("Tarefa adicionada à fila.\n");
    pthread_mutex_unlock(&stdout_mutex);

    pthread_mutex_unlock(&mutexQueue);

    // Acorda uma thread para processar a nova tarefa
    pthread_cond_signal(&condQueue);
}

/**
 * Função para liberar recursos.
 * Destroi os mutexes e a variável condicional utilizados.
 */
void cleanupResources() {
    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);
}
