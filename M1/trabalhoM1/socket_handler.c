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

/**
 * Função para configurar o socket.
 * Cria, configura e faz o bind do socket para comunicação usando o caminho fornecido.
 * 
 * @param path O caminho do arquivo de socket UNIX.
 * @return O descritor de arquivo do socket configurado.
 */
int setupSocket(const char* path) {
    int server_socket;
    struct sockaddr_un local;

    // Cria um socket UNIX
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket < 0) {
        logError("Falha ao criar o socket");
        exit(1);
    }

    // Configura a estrutura sockaddr_un
    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, path, sizeof(local.sun_path) - 1);
    unlink(local.sun_path);
    int len = strlen(local.sun_path) + sizeof(local.sun_family);

    // Faz o bind do socket ao caminho especificado
    if (bind(server_socket, (struct sockaddr*)&local, len) < 0) {
        logError("Falha ao fazer o bind no socket");
        close(server_socket);
        exit(1);
    }

    // Coloca o socket em modo de escuta (aguarda conexões)
    if (listen(server_socket, 5) < 0) {
        logError("Falha ao escutar no socket");
        close(server_socket);
        exit(1);
    }

    return server_socket;
}

/**
 * Função para aceitar uma conexão de cliente.
 * Aceita uma nova conexão do cliente e retorna o descritor de arquivo do socket do cliente.
 * 
 * @param server_socket O descritor de arquivo do socket do servidor.
 * @param remote Estrutura para armazenar informações do cliente conectado.
 * @return O descritor de arquivo do socket do cliente.
 */
int acceptConnection(int server_socket, struct sockaddr_un* remote) {
    int client_socket;
    socklen_t len = sizeof(*remote);
    client_socket = accept(server_socket, (struct sockaddr*)remote, &len);
    return client_socket;
}

/**
 * Função para lidar com conexões que enviam strings.
 * Recebe uma string, converte para maiúsculas e devolve ao cliente.
 * 
 * @param p_client_socket Ponteiro para o socket do cliente.
 */
void handleStringConnection(int* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket); // Libera o ponteiro do cliente
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));  

    // Lê dados do cliente
    if (read(client_socket, buffer, sizeof(buffer)) < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em ler do socket");
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

     // Exibe a string recebida
    pthread_mutex_lock(&stdout_mutex);
    printf("String recebida: %s\n", buffer);
    pthread_mutex_unlock(&stdout_mutex);

    // Converte a string para maiúsculas
    for (int i = 0; i < strlen(buffer); i++) {
        buffer[i] = toupper(buffer[i]);
    }

    // Envia a string de volta para o cliente
    if (write(client_socket, buffer, strlen(buffer) + 1) < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em escrever no socket");
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);

        return;
    }

    // Exibe confirmação de envio
    pthread_mutex_lock(&stdout_mutex);
    printf("String processada e enviada de volta.\n") ;
    pthread_mutex_unlock(&stdout_mutex);

    // Fecha a conexão do cliente e atualiza o contador de clientes online
    close(client_socket);
    pthread_mutex_lock(&client_count_mutex);
    online_clients--;
    pthread_mutex_unlock(&client_count_mutex);
}

/**
 * Função para lidar com conexões que enviam números.
 * Recebe um número, o multiplica por dois e devolve o resultado ao cliente.
 * 
 * @param p_client_socket Ponteiro para o socket do cliente.
 */
void handleNumberConnection(int* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // Lê dados do cliente
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em ler do socket");
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';  // Garante a terminação da string

    // Converte o buffer para um número inteiro
    int number = atoi(buffer);
    pthread_mutex_lock(&stdout_mutex);
    printf("Número recebido: %d\n", number);
    pthread_mutex_unlock(&stdout_mutex);

    number *= 2;
    snprintf(buffer, sizeof(buffer), "%d", number);

    // Envia o número processado de volta ao cliente
    if (write(client_socket, buffer, strlen(buffer)) < 0) {
        pthread_mutex_lock(&stdout_mutex);
        perror("Falha em escrever no socket");
        pthread_mutex_unlock(&stdout_mutex);
        close(client_socket);
        return;
    }

    // Exibe confirmação de envio
    pthread_mutex_lock(&stdout_mutex);
    printf("Número processado e enviado de volta: %d\n", number);
    pthread_mutex_unlock(&stdout_mutex);

    // Fecha a conexão do cliente e atualiza o contador de clientes online
    close(client_socket);
    pthread_mutex_lock(&client_count_mutex);
    online_clients--;
    pthread_mutex_unlock(&client_count_mutex);
}

/**
 * Função para gerenciar uma conexão de cliente e passar para o handler correto.
 * Aloca memória para o socket do cliente e cria uma tarefa para o handler especificado.
 * 
 * @param client_socket O descritor de arquivo do socket do cliente.
 * @param handler Ponteiro para a função handler (que processa o tipo de conexão).
 */
void handleClientConnection(int client_socket, void (*handler)(int *)) {
    // Aloca memória para armazenar o descritor de arquivo do cliente
    int *pclient = malloc(sizeof(int));
    *pclient = client_socket;

    // Verifica se a alocação de memória falhou
    if (pclient == NULL) {
        perror("Erro ao alocar memória para client_socket");
        close(client_socket); 
        return;
    }

    // Cria e envia uma tarefa para processar a conexão usando a função handler apropriada
    Task t = {
        .taskFunction = (void (*)(void *))handler,  
        .arg1 = pclient
    };

    submitTask(t);
}


