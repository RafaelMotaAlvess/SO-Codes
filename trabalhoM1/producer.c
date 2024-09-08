#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define SOCK_PATH_STRING "/tmp/pipestring"
#define SOCK_PATH_NUMBER "/tmp/pipenumber"

#define BUFFER_SIZE 1024
#define THREAD_POOL_SIZE 4
#define MAX_CLIENTS 20

pthread_t th_pool[THREAD_POOL_SIZE];

typedef struct Task {
    void (*taskFunction)(int *);
    int *arg1;
} Task;


pthread_mutex_t mutexQueue;
pthread_mutex_t client_count_mutex;
pthread_mutex_t stdout_mutex;
pthread_cond_t condQueue;

Task taskQueue[50];

int taskCount = 0;
int online_clients = 0;

// Função a ser executada pela thread relacionada à fila.
void executeTask(Task* task) {
    pthread_mutex_lock(&stdout_mutex);
    printf("====\n");
    printf("Thread %lu processando tarefa.\n", pthread_self());
    pthread_mutex_unlock(&stdout_mutex);

    task->taskFunction(task->arg1);

    pthread_mutex_lock(&mutexQueue);
    if (taskCount == 0) {
        pthread_mutex_lock(&stdout_mutex);
        printf("Fila vazia. Nenhuma conexão pendente.\n");
        printf("====\n");
        pthread_mutex_unlock(&stdout_mutex);
    }
    pthread_mutex_unlock(&mutexQueue);
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
            printf("Thread %lu aguardando tarefas...\n", pthread_self());  // Log quando a thread está aguardando
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

// Função para lidar com strings
void handleStringConnection(void* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);

    sleep(2);
    char buffer[BUFFER_SIZE];

    // Ler dados do cliente
    if (read(client_socket, buffer, sizeof(buffer)) < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em ler do socket");
        fflush(stdout);  // Garante que o buffer seja esvaziado
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

    // Bloqueia o stdout antes de imprimir
    pthread_mutex_lock(&stdout_mutex);
    printf("String recebida: %s\n", buffer);
    fflush(stdout);  // Força a impressão imediata
    pthread_mutex_unlock(&stdout_mutex);

    // Processa a string (exemplo: converte para maiúsculas)
    for (int i = 0; i < strlen(buffer); i++) {
        buffer[i] = toupper(buffer[i]);
    }

    // Envia a string de volta ao cliente
    if (write(client_socket, buffer, strlen(buffer) + 1) < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em escrever no socket");
        fflush(stdout);  // Garante que o buffer seja esvaziado
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

    // Bloqueia o stdout antes de imprimir
    pthread_mutex_lock(&stdout_mutex);
    printf("String processada e enviada de volta.\n");
    fflush(stdout);  // Força a impressão imediata
    pthread_mutex_unlock(&stdout_mutex);

    close(client_socket);

    pthread_mutex_lock(&client_count_mutex);
    online_clients--;
    pthread_mutex_unlock(&client_count_mutex);
}

// Função para lidar com números
void handleNumberConnection(void* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);

    sleep(2);
    char buffer[BUFFER_SIZE];
    int number;

    // Ler dados do cliente
    if (read(client_socket, buffer, sizeof(buffer)) < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em ler do socket");
        fflush(stdout);  // Garante que o buffer seja esvaziado
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

    number = atoi(buffer);

    // Bloqueia o stdout antes de imprimir
    pthread_mutex_lock(&stdout_mutex);
    printf("Número recebido: %d\n", number);
    fflush(stdout);  // Força a impressão imediata
    pthread_mutex_unlock(&stdout_mutex);

    // Processar o número (exemplo: dobrar o valor)
    number *= 2;
    snprintf(buffer, sizeof(buffer), "%d", number);

    // Envia o número de volta ao cliente
    if (write(client_socket, buffer, strlen(buffer) + 1) < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em escrever no socket");
        fflush(stdout);  // Garante que o buffer seja esvaziado
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

    // Bloqueia o stdout antes de imprimir
    pthread_mutex_lock(&stdout_mutex);
    printf("Número processado e enviado de volta: %d\n", number);
    fflush(stdout);  // Força a impressão imediata
    pthread_mutex_unlock(&stdout_mutex);

    close(client_socket);

    pthread_mutex_lock(&client_count_mutex);
    online_clients--;
    pthread_mutex_unlock(&client_count_mutex);
}

// Função para configurar um socket Unix
int setupSocket(const char* path) {
    int server_socket;
    struct sockaddr_un local;

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Falha ao criar o socket");
        exit(1);
    }

    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, path, sizeof(local.sun_path) - 1);
    unlink(local.sun_path);
    int len = strlen(local.sun_path) + sizeof(local.sun_family);

    if (bind(server_socket, (struct sockaddr*)&local, len) < 0) {
        perror("Falha ao fazer o bind no socket");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Falha ao escutar no socket");
        close(server_socket);
        exit(1);
    }

    return server_socket;
}

int main(int argc, char* argv[]) {
    int string_socket, number_socket, client_socket, len;
    struct sockaddr_un remote;

    pthread_mutex_init(&mutexQueue, NULL);
    pthread_mutex_init(&client_count_mutex, NULL);
    pthread_cond_init(&condQueue, NULL);

    // Desabilita o buffering de stdout
    setvbuf(stdout, NULL, _IONBF, 0);

    // Inicializa as threads
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&th_pool[i], NULL, &startThread, NULL) != 0) {
            perror("Falha ao criar a thread");
        }
    }

    string_socket = setupSocket(SOCK_PATH_STRING);
    number_socket = setupSocket(SOCK_PATH_NUMBER);

    printf("Servidor ouvindo em %s e %s...\n", SOCK_PATH_STRING, SOCK_PATH_NUMBER);

    // Aceita conexões, um bloco para cada socket
    while (1) {
        // Lida com conexões no socket de strings
        memset(&remote, 0, sizeof(remote));
        len = sizeof(remote);
        client_socket = accept(string_socket, (struct sockaddr*)&remote, &len);
        if (client_socket >= 0) {
            pthread_mutex_lock(&client_count_mutex);
            if (online_clients >= MAX_CLIENTS) {
                pthread_mutex_unlock(&client_count_mutex);

                // Envia uma mensagem de erro ao cliente
                const char* error_message = "Número máximo de clientes atingido. Conexão recusada.\n";
                write(client_socket, error_message, strlen(error_message));
                close(client_socket);
                printf("Conexão recusada: número máximo de clientes atingido.\n");
                continue;
            }
            online_clients++;
            pthread_mutex_unlock(&client_count_mutex);

            printf("Cliente conectado (socket de string)!\n");

            int* pclient = malloc(sizeof(int));
            *pclient = client_socket;

            Task t = {
                .taskFunction = &handleStringConnection,
                .arg1 = pclient
            };

            submitTask(t);
        }

        // Lida com conexões no socket de números
        memset(&remote, 0, sizeof(remote));
        len = sizeof(remote);
        client_socket = accept(number_socket, (struct sockaddr*)&remote, &len);
        if (client_socket >= 0) {
            pthread_mutex_lock(&client_count_mutex);
            if (online_clients >= MAX_CLIENTS) {
                pthread_mutex_unlock(&client_count_mutex);

                // Envia uma mensagem de erro ao cliente
                const char* error_message = "Número máximo de clientes atingido. Conexão recusada.\n";
                write(client_socket, error_message, strlen(error_message));
                close(client_socket);
                printf("Conexão recusada: número máximo de clientes atingido.\n");
                continue;
            }
            online_clients++;
            pthread_mutex_unlock(&client_count_mutex);

            printf("Cliente conectado (socket de número)!\n");

            int* pclient = malloc(sizeof(int));
            *pclient = client_socket;

            Task t = {
                .taskFunction = &handleNumberConnection,
                .arg1 = pclient
            };

            submitTask(t);
        }
    }

    pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);
    pthread_mutex_destroy(&client_count_mutex);

    close(string_socket);
    close(number_socket);

    return 0;
}
