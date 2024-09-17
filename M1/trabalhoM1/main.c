#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "task_queue.h"
#include "socket_handler.h"
#include "utils.h"

#define SOCK_PATH_STRING "/tmp/pipestring"
#define SOCK_PATH_NUMBER "/tmp/pipenumber"
#define MAX_EVENTS 10  // Máximo de eventos a serem processados de uma ve z

int main(int argc, char* argv[]) {
    int string_socket, number_socket, client_socket, epoll_fd;
    struct sockaddr_un remote;
    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, i;

    // Inicializa mutexes e condições
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_mutex_init(&client_count_mutex, NULL);
    pthread_cond_init(&condQueue, NULL);

    setvbuf(stdout, NULL, _IONBF, 0);

    // Inicializa o pool de threads
    initializeThreadPool();

    // Configura dois sockets - um para strings e outro para números
    string_socket = setupSocket(SOCK_PATH_STRING);
    number_socket = setupSocket(SOCK_PATH_NUMBER);

    printf("Servidor ouvindo em %s e %s...\n", SOCK_PATH_STRING, SOCK_PATH_NUMBER);

    // Cria uma instância de epoll para monitorar múltiplos sockets
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Erro ao criar epoll");
        exit(EXIT_FAILURE);
    }

    // Adiciona o socket de strings ao epoll para monitoramento de eventos
    ev.events = EPOLLIN;
    ev.data.fd = string_socket;   // Associamos o evento ao socket de strings
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, string_socket, &ev) == -1) {
        perror("Erro ao adicionar string_socket ao epoll");
        exit(EXIT_FAILURE);
    }

    // Adiciona o socket de números ao epoll para monitoramento de eventos
    ev.events = EPOLLIN;
    ev.data.fd = number_socket; // Associamos o evento ao socket de números
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, number_socket, &ev) == -1) {
        perror("Erro ao adicionar number_socket ao epoll");
        exit(EXIT_FAILURE);
    }

    // Loop principal que espera por eventos e processa conexões
    while (1) {
        // Aguarda por eventos em qualquer socket monitorado
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue; // Ignora interrupções e continua o loop
            } else {
                perror("Erro no epoll_wait");
                exit(EXIT_FAILURE);
            }
        }

        // Processa os eventos recebidos
        for (i = 0; i < nfds; i++) {
            if (events[i].data.fd == string_socket) {
                // Se o evento ocorreu no socket de strings, aceita a conexão
                client_socket = acceptConnection(string_socket, &remote);
                if (client_socket >= 0) {
                    handleClientConnection(client_socket, handleStringConnection);  // Processa a conexão de strings
                }
            } else if (events[i].data.fd == number_socket) {
                // Se o evento ocorreu no socket de números, aceita a conexão
                client_socket = acceptConnection(number_socket, &remote);
                if (client_socket >= 0) {
                    handleClientConnection(client_socket, handleNumberConnection); // Processa a conexão de números
                }
            }
        }
    }

    // Limpeza de recursos
    close(string_socket);
    close(number_socket);
    close(epoll_fd);
    cleanupResources();

    return 0;
}
