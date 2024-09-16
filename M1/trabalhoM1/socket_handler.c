#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include "socket_handler.h"
#include "task_queue.h"
#include "utils.h"

pthread_mutex_t stdout_mutex;
pthread_mutex_t client_count_mutex;
int online_clients;

int setupSocket(const char* path) {
    int server_socket;
    struct sockaddr_un local;

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket < 0) {
        logError("Falha ao criar o socket");
        exit(1);
    }

    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, path, sizeof(local.sun_path) - 1);
    unlink(local.sun_path);
    int len = strlen(local.sun_path) + sizeof(local.sun_family);

    if (bind(server_socket, (struct sockaddr*)&local, len) < 0) {
        logError("Falha ao fazer o bind no socket");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, 5) < 0) {
        logError("Falha ao escutar no socket");
        close(server_socket);
        exit(1);
    }

    return server_socket;
}

int acceptConnection(int server_socket, struct sockaddr_un* remote) {
    int client_socket;
    socklen_t len = sizeof(*remote);
    client_socket = accept(server_socket, (struct sockaddr*)remote, &len);
    return client_socket;
}

void handleStringConnection(int* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));  

    if (read(client_socket, buffer, sizeof(buffer)) < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em ler do socket");
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

    pthread_mutex_lock(&stdout_mutex);
    // printf("String recebida: %s\n", buffer);
    pthread_mutex_unlock(&stdout_mutex);

    // Converte a string para maiúsculas
    for (int i = 0; i < strlen(buffer); i++) {
        buffer[i] = toupper(buffer[i]);
    }

    if (write(client_socket, buffer, strlen(buffer) + 1) < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em escrever no socket");
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

    pthread_mutex_lock(&stdout_mutex);
    // printf("String processada e enviada de volta.\n");
    pthread_mutex_unlock(&stdout_mutex);

    close(client_socket);
    pthread_mutex_lock(&client_count_mutex);
    online_clients--;
    pthread_mutex_unlock(&client_count_mutex);
}

// Função para lidar com números
void handleNumberConnection(int* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);  // Lê os dados
    if (bytes_read < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em ler do socket");
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';  // Garante a terminação da string

    int number = atoi(buffer);
    pthread_mutex_lock(&stdout_mutex);
    printf("Número recebido: %d\n", number);
    pthread_mutex_unlock(&stdout_mutex);

    // Processa o número
    number *= 2;
    snprintf(buffer, sizeof(buffer), "%d", number);

    if (write(client_socket, buffer, strlen(buffer)) < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em escrever no socket");
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

    pthread_mutex_lock(&stdout_mutex);
    printf("Número processado e enviado de volta: %d\n", number);
    pthread_mutex_unlock(&stdout_mutex);

    close(client_socket);
    pthread_mutex_lock(&client_count_mutex);
    online_clients--;
    pthread_mutex_unlock(&client_count_mutex);
}

void handleClientConnection(int client_socket, void (*handler)(int *)) {
    int *pclient = malloc(sizeof(int));
    *pclient = client_socket;

    Task t = {
        .taskFunction = (void (*)(void *))handler,  
        .arg1 = pclient
    };

    submitTask(t);
}


