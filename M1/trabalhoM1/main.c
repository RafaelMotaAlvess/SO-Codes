#include <pthread.h>
#include <stdio.h>
#include <sys/un.h>
#include "task_queue.h"
#include "socket_handler.h"
#include "utils.h"

#define SOCK_PATH_STRING "/tmp/pipestring"
#define SOCK_PATH_NUMBER "/tmp/pipenumber"

int main(int argc, char* argv[]) {
    int string_socket, number_socket, client_socket;
    struct sockaddr_un remote;

    pthread_mutex_init(&mutexQueue, NULL);
    pthread_mutex_init(&client_count_mutex, NULL);
    pthread_cond_init(&condQueue, NULL);

    setvbuf(stdout, NULL, _IONBF, 0); // Desabilita o buffering de stdout

    initializeThreadPool();  // Inicializa o pool de threads

    string_socket = setupSocket(SOCK_PATH_STRING);
    number_socket = setupSocket(SOCK_PATH_NUMBER);

    printf("Servidor ouvindo em %s e %s...\n", SOCK_PATH_STRING, SOCK_PATH_NUMBER);

    // Loop principal de aceitação de conexões
    while (1) {
        // Lida com conexões de strings
        client_socket = acceptConnection(string_socket, &remote);
        if (client_socket >= 0) {
            handleClientConnection(client_socket, handleStringConnection);
        }

        // Lida com conexões de números
        client_socket = acceptConnection(number_socket, &remote);
        if (client_socket >= 0) {
            handleClientConnection(client_socket, handleNumberConnection);
        }
    }

    cleanupResources();
    return 0;
}
