/**
 * Descrição: Código do cliente
 * 
 * Autor: Lucas A. Seemund e Fabricio
 * Data: 11/09/2024
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "socket_traducao"
#define BUFFER_SIZE 256

int main() {
    int cliente_socket;
    struct sockaddr_un servidor_addr;
    char buffer[BUFFER_SIZE];
    char send[BUFFER_SIZE+5];

    // Criar o socket
    cliente_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (cliente_socket == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Configurar o endereço do servidor
    memset(&servidor_addr, 0, sizeof(struct sockaddr_un));
    servidor_addr.sun_family = AF_UNIX;
    strncpy(servidor_addr.sun_path, SOCKET_PATH, sizeof(servidor_addr.sun_path) - 1);

    // Conectar ao servidor
    if (connect(cliente_socket, (struct sockaddr *)&servidor_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Erro ao conectar ao servidor");
        exit(EXIT_FAILURE);
    }

    printf("Conectado ao servidor de tradução.\n");

    // Enviar requisições de tradução
    while (1) {
        printf("Digite o código de tradução e a palavra (ou NO-NO para sair): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        if(buffer[0] == '\n' || buffer[0] == '\0') continue; //se so foi clicado no enter

        strcpy(send, "pt-en:");

        // Verificar se o cliente deseja sair
        if (strncmp(buffer, "NO-NO", 5) == 0) {
            write(cliente_socket, buffer, strlen(buffer));
            break;
        }

        strcat(send, buffer);

        // Enviar para o servidor
        if (write(cliente_socket, send, strlen(send)) == -1) {
            perror("Erro ao enviar mensagem");
            exit(EXIT_FAILURE);
        }

        // Receber resposta do servidor
        memset(buffer, 0, BUFFER_SIZE);
        if (read(cliente_socket, buffer, BUFFER_SIZE) > 0) {
            printf("Resposta do servidor: %s\n", buffer);
        } else {
            perror("Erro ao receber resposta");
        }
    }

    close(cliente_socket);
    return 0;
}
