#include <pthread.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>

#define THREAD_NUM 4

typedef struct Task {
    void (*taskFunction)(int, int);
    int arg1, arg2;
} Task;

// void sumAndProduct(int a, int b){
//     int sum = a + b;
//     int prod = a * b;

//     printf("Sum and product of %d and %d is %d and %d\n", a, b, sum, prod);
// }

void sum(int a, int b) {
    int sum = a + b;
    printf("Sum of %d and %d is %d\n", a, b, sum);
}

void product(int a, int b) {
    int prod = a * b;
    printf("Product of %d and %d is %d\n", a, b, prod);
}

Task taskQueue[256];
int taskCount = 0;

pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

// funcao a ser executada pela thread relacionado a fila.
void executeTask(Task* task){
    task->taskFunction(task->arg1, task->arg2);
    // int result = task->a + task->b;
    // printf("The sum of %d and %d is %d\n", task->a, task->b, result);
}

// funcao para adicionar uma task a fila de tasks
void submitTask(Task task) {
    pthread_mutex_lock(&mutexQueue);
    taskQueue[taskCount] = task;
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);

    // Envia um sinal para a condiciona de task para poder realizar a liberação de uma thread para realização de uma nova tarefa
    pthread_cond_signal(&condQueue);
}


// estrutura que vai servir para a thread pool
void* startThread(void* args){
    while(1){
        Task task;

        pthread_mutex_lock(&mutexQueue);

        // necessita colocar as threads em condição de espera (stand-by)
        // se não a thread na pool vai ficar sendo repetida diversas vezes e vai utilizar 100% do processamento
        // isso pode ser visualizado utilzando o HTOP do linux
        while(taskCount == 0) {
            pthread_cond_wait(&condQueue, &mutexQueue);
        }

        task = taskQueue[0];

        int i;

        for(i = 0; i < taskCount - 1; i++){
            taskQueue[i] = taskQueue[i + 1];
        }

        taskCount--;

        pthread_mutex_unlock(&mutexQueue);

        executeTask(&task);
    }
}

int main(int argc, char *argv[]){
    pthread_t th[THREAD_NUM];
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);

    int i;

    // inicializa todas as threads
    for(i = 0; i < THREAD_NUM; i++){
        if(pthread_create(&th[i], NULL, &startThread, NULL) != 0){
            perror("Failed to create the thread");
        }
    }

    // cria as tasks que as threads irão fazer
    for(i = 0; i < 100; i++){
        Task t = {
            .taskFunction = i % 2 == 0 ?  &sum : &product,
            .arg1 = rand() % 100,
            .arg2 = rand() % 100
        };
        
        submitTask(t);
    }

    // espera o retorno de todas as chamadas de threads
    for(i = 0; i < THREAD_NUM; i++){
        if(pthread_join(th[i], NULL) != 0){
            perror("Failed to join the thread");
        }
    }

    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);

    return 0;
}