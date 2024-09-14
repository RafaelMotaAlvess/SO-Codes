#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#define BUFFER_SIZE 1024

extern pthread_mutex_t stdout_mutex;
extern pthread_mutex_t client_count_mutex;
extern int online_clients;

int setupSocket(const char* path);
int acceptConnection(int server_socket, struct sockaddr_un* remote);
void handleStringConnection(int *p_client_socket);
void handleNumberConnection(int *p_client_socket);

void handleClientConnection(int client_socket, void (*handler)(int*));

#endif // SOCKET_HANDLER_H
