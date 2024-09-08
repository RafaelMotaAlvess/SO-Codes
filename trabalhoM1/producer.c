#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCK_PATH "/tmp/pipeso"
#define BUFFER_SIZE 1024
#define THREAD_POOL_SIZE 4
#define MAX_CLIENTS 2

pthread_t th_pool[THREAD_POOL_SIZE];

typedef struct Task {
    void (*taskFunction)(int *);
    int *arg1;
} Task;

pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;
pthread_mutex_t client_count_mutex;

Task taskQueue[2];
int online_clients = 0;
int taskCount = 0;

// Função a ser executada pela thread relacionada à fila.
void executeTask(Task* task) {
    task->taskFunction(task->arg1);
}

// Função para adicionar uma task à fila de tasks
void submitTask(Task task) {
    pthread_mutex_lock(&mutexQueue);
    taskQueue[taskCount] = task;
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);

    // Sinaliza a condição de task para liberar uma thread para realizar a nova tarefa
    pthread_cond_signal(&condQueue);
}

void* startThread(void* args) {
    while (1) {
        Task task;

        pthread_mutex_lock(&mutexQueue);

        // Coloca as threads em stand-by
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

void handleConnection(void* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);  // Libera o valor de memória alocado

    char buffer[BUFFER_SIZE];
    if (read(client_socket, buffer, sizeof(buffer)) < 0) {
        perror("Falha em ler do socket");
        close(client_socket);
        return;
    }

    printf("Dado recebido: %s\n", buffer);

    // Processa os dados (converte para letras maiúsculas como exemplo)
    for (int i = 0; i < strlen(buffer); i++) {
        buffer[i] = toupper(buffer[i]);
    }

    // Escreve os dados processados de volta para o cliente
    if (write(client_socket, buffer, strlen(buffer) + 1) < 0) {
        perror("Falha em escrever no socket");
        close(client_socket);
        return;
    }

    printf("Dado enviado de volta para o cliente.\n");
    close(client_socket);
    printf("Fechando conexão!\n");

    // Decrementa o número de clientes conectados ao finalizar a conexão
    pthread_mutex_lock(&client_count_mutex);
    online_clients--;
    pthread_mutex_unlock(&client_count_mutex);
}

int main(int argc, char* argv[]) {
    int server_socket, client_socket, len;
    struct sockaddr_un local, remote;

    pthread_mutex_init(&mutexQueue, NULL);
    pthread_mutex_init(&client_count_mutex, NULL);
    pthread_cond_init(&condQueue, NULL);

    // Inicializa as threads
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&th_pool[i], NULL, &startThread, NULL) != 0) {
            perror("Falha ao criar a thread");
        }
    }

    // Cria o socket
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Falha ao criar o pipe");
        return 1;
    }

    // Bind do socket ao endereço local
    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, SOCK_PATH, sizeof(local.sun_path) - 1);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(server_socket, (struct sockaddr*)&local, len) < 0) {
        perror("Falha ao capturar o socket");
        close(server_socket);
        return 1;
    }

    // Escuta por conexões
    if (listen(server_socket, 5) < 0) {
        perror("Falha ao escutar o socket");
        close(server_socket);
        return 1;
    }

    printf("Servidor Named pipe ouvindo em %s...\n", SOCK_PATH);

    // Aceita conexões
    while (1) {
        // Protege o contador de clientes conectados com um mutex
        pthread_mutex_lock(&client_count_mutex);
        if (online_clients >= MAX_CLIENTS) {
            pthread_mutex_unlock(&client_count_mutex);
            printf("Limite de clientes atingido. Aguardando vaga...\n");
            sleep(1);
            continue;
        }
        online_clients++;
        pthread_mutex_unlock(&client_count_mutex);

        memset(&remote, 0, sizeof(remote));
        len = sizeof(remote);
        client_socket = accept(server_socket, (struct sockaddr*)&remote, &len);
        if (client_socket < 0) {
            perror("Falha ao aceitar conexão");
            close(server_socket);
            return 1;
        }

        printf("Cliente conectado!\n");

        int* pclient = malloc(sizeof(int));
        *pclient = client_socket;

        Task t = {
            .taskFunction = &handleConnection,
            .arg1 = pclient
        };

        submitTask(t);
    }

    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);
    pthread_mutex_destroy(&client_count_mutex);

    close(server_socket);
    return 0;
}
