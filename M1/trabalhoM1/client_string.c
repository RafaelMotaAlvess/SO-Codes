#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define SOCK_PATH "/tmp/pipestring"

// gcc -o client_string client_string.c

int main()
{
    int sockfd, len;
    struct sockaddr_un remote;
    char buffer[8096];

    // Cria o socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Erro: Falha ao criar o socket");
        return 1;
    }

    // Conecta ao servidor
    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path) - 1);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(sockfd, (struct sockaddr *)&remote, len) < 0)
    {
        perror("Erro: Falha ao conectar ao servidor");
        close(sockfd);
        return 1;
    }

    printf("Conectado ao servidor com sucesso!\n");

    // Solicita o dado ao usuário
    printf("Digite a string que deseja enviar ao servidor: ");
    fgets(buffer, sizeof(buffer), stdin);

    // Envia os dados ao servidor
    if (write(sockfd, buffer, strlen(buffer) + 1) < 0) // Inclui o caractere null no envio
    {
        perror("Erro: Falha ao enviar os dados ao servidor");
        close(sockfd);
        return 1;
    }

    printf("String enviada ao servidor com sucesso.\n");

    // Recebe resposta do servidor
    if (read(sockfd, buffer, sizeof(buffer)) < 0)
    {
        perror("Erro: Falha ao receber dados do servidor");
        close(sockfd);
        return 1;
    }

    printf("Resposta do servidor: %s", buffer);

    // Fecha o socket e encerra
    close(sockfd);
    return 0;
}
