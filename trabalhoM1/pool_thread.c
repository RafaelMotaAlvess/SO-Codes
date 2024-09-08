#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#define SOCK_PATH "/tmp/pipeso"

#define THREAD_NUM 4

typedef struct Task {
    void (*taskFunction)(int, int);
    int arg1, arg2;
} Task;


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

int sockfd, newsockfd, len;
struct sockaddr_un local, remote;
char buffer[1024];


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

// funcao para iniciliazar o pipe de string

void createStringPipe(){
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sockfd < 0){
        perror("Falha em criar o pipe");
        return 1;
    }

    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, SOCK_PATH, sizeof(local.sun_path) - 1);
    unlink(local.sun_path);

    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(sockfd, (struct sockaddr *)&local, len) < 0){
        perror("Falha em capturar o socket");
        close(sockfd);
        return 1;
    }

    // Listen for connections
    if (listen(sockfd, 5) < 0){
        perror("Falha em escutar o socket");
        close(sockfd);
        return 1;
    }

    printf("Servidor Named pipe ouvindo em %s...\n", SOCK_PATH);

    memset(&remote, 0, sizeof(remote));
    len = sizeof(remote);
    newsockfd = accept(sockfd, (struct sockaddr *)&remote, &len);
    if (newsockfd < 0)
    {
        perror("Falha em aceitar coneccao");
        close(sockfd);
        return 1;
    }

    printf("Cliente conectado!\n");
}

void listenStringPipe(){
    while(1){
        // Read data from client
        if (read(newsockfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Falha em ler do socket");
            close(newsockfd);
            close(sockfd);
            return 1;
        }

        printf("Dado recebido: %s\n", buffer);

        // Process data
        // In this example, we just convert the string to uppercase
        for (int i = 0; i < strlen(buffer); i++)
        {
            buffer[i] = toupper(buffer[i]);
        }

        // Write processed data back to client
        if (write(newsockfd, buffer, strlen(buffer) + 1) < 0)
        {
            perror("Falha em escrever no socket");
            close(newsockfd);
            close(sockfd);
            return 1;
        }

        printf("Dado enviado de volta para o cliente.\n");

        // Close sockets and exit
        // if(buffer == "FECHAR"){
        //     printf("caiu aqui");
        //     close(newsockfd);
        //     close(sockfd);
        //     return 0;
        // }
    }
}

int main(int argc, char *argv[]){
    // pthread_t th[THREAD_NUM];
    // pthread_mutex_init(&mutexQueue, NULL);
    // pthread_cond_init(&condQueue, NULL);

    // int i;

    // // inicializa todas as threads
    // for(i = 0; i < THREAD_NUM; i++){
    //     if(pthread_create(&th[i], NULL, &startThread, NULL) != 0){
    //         perror("Failed to create the thread");
    //     }
    // }

    // // cria as tasks que as threads irão fazer
    // for(i = 0; i < 100; i++){
    //     Task t = {
    //         .taskFunction = i % 2 == 0 ?  &sum : &product,
    //         .arg1 = rand() % 100,
    //         .arg2 = rand() % 100
    //     };
        
    //     submitTask(t);
    // }

    // // espera o retorno de todas as chamadas de threads
    // for(i = 0; i < THREAD_NUM; i++){
    //     if(pthread_join(th[i], NULL) != 0){
    //         perror("Failed to join the thread");
    //     }
    // }

    // pthread_mutex_destroy(&mutexQueue);
    // pthread_cond_destroy(&condQueue);

    createStringPipe();
    listenStringPipe();
    
    return 0;
}