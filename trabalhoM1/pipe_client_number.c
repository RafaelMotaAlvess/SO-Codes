#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCK_PATH "/tmp/pipenumber"  // Caminho para o socket de números

int main()
{
    int sockfd, len;
    struct sockaddr_un remote;
    char buffer[1024];

    // Cria o socket Unix
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Falha em criar o socket");
        return 1;
    }

    // Conecta ao servidor
    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path) - 1);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(sockfd, (struct sockaddr *)&remote, len) < 0)
    {
        perror("Falha em conectar no servidor");
        close(sockfd);
        return 1;
    }

    printf("Conectado ao servidor de números!\n");

    // Solicita ao usuário para entrar com um número
    printf("Entre com um número a ser enviado: ");
    fgets(buffer, sizeof(buffer), stdin);

    // Envia o número ao servidor
    if (write(sockfd, buffer, strlen(buffer) + 1) < 0)
    {
        perror("Falha em escrever no socket");
        close(sockfd);
        return 1;
    }

    printf("Número enviado ao servidor.\n");

    // Lê o número processado do servidor (dobrado, por exemplo)
    if (read(sockfd, buffer, sizeof(buffer)) < 0)
    {
        perror("Falha em ler do socket");
        close(sockfd);
        return 1;
    }

    printf("Número processado recebido do servidor: %s\n", buffer);

    close(sockfd);
    return 0;
}
