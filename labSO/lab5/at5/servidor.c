/**
 * Descrição: Código do servidor
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
#include <ctype.h>

#define SOCKET_PATH "socket_traducao"
#define BUFFER_SIZE 256

// Função para realizar a tradução lendo o arquivo de traduções
char* traduzir(const char* codigo) {
    FILE* arquivo = fopen("dicionario.txt", "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo de tradução");
        return "ERROR:FILE";
    }

    static char resultado[BUFFER_SIZE];
    char linha[BUFFER_SIZE];

    while (fgets(linha, sizeof(linha), arquivo)) {
        // Remover nova linha
        linha[strcspn(linha, "\n")] = 0;

        // Procurar a correspondência de tradução
        char* token = strtok(linha, "=");
        if (strcmp(token, codigo) == 0) {
            token = strtok(NULL, "=");
            fclose(arquivo);
            strcpy(resultado, token);
            return resultado;
        }
    }

    fclose(arquivo);
    return "ERROR:UNKNOWN";
}

int main() {
    int servidor_socket, cliente_socket;
    struct sockaddr_un servidor_addr;
    char buffer[BUFFER_SIZE];

    // Criar o socket
    servidor_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (servidor_socket == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Configurar o endereço do servidor
    memset(&servidor_addr, 0, sizeof(struct sockaddr_un));
    servidor_addr.sun_family = AF_UNIX;
    strncpy(servidor_addr.sun_path, SOCKET_PATH, sizeof(servidor_addr.sun_path) - 1);

    // Remover qualquer socket existente
    unlink(SOCKET_PATH);

    // Vincular o socket ao endereço
    if (bind(servidor_socket, (struct sockaddr *)&servidor_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Erro ao fazer bind");
        exit(EXIT_FAILURE);
    }

    // Escutar por conexões
    if (listen(servidor_socket, 5) == -1) {
        perror("Erro ao escutar");
        exit(EXIT_FAILURE);
    }

    printf("Servidor de tradução iniciado...\n");

    // Aceitar e processar conexões
    while (1) {
        cliente_socket = accept(servidor_socket, NULL, NULL);
        if (cliente_socket == -1) {
            perror("Erro ao aceitar conexão");
            continue;
        }
        printf("Cliente %d, conectou.\n",cliente_socket);

        // Criar processo filho para tratar o cliente
        pid_t pid = fork();
        if (pid == 0) {  // Processo filho
            close(servidor_socket);

            while (1) {
                memset(buffer, 0, BUFFER_SIZE);
                int leitura = read(cliente_socket, buffer, BUFFER_SIZE);
                if (leitura <= 0) {
                    perror("Erro ao ler do cliente");
                    break;
                }

                for(int i=0; buffer[i]!='\0'; i++ ){
                    buffer[i] = tolower(buffer[i]);
                }

                printf("Cliente %d, mandou: %s", cliente_socket, buffer);

                // Remover nova linha e espaços extras
                buffer[strcspn(buffer, "\n")] = 0;

                // Verificar se o cliente deseja encerrar
                if (strcmp(buffer, "no-no") == 0) {
                    printf("Encerrando comunicação com o cliente.\n");
                    break;
                }

                // Realizar a tradução
                char* resultado = traduzir(buffer);
                printf("Enviando: %s\n", resultado);
                write(cliente_socket, resultado, strlen(resultado) + 1);
            }

            close(cliente_socket);
            exit(EXIT_SUCCESS);
        } else if (pid > 0) {  // Processo pai
            close(cliente_socket);  // Pai não precisa do socket do cliente
        } else {
            perror("Erro ao criar processo filho");
        }
    }

    close(servidor_socket);
    unlink(SOCKET_PATH);
    return 0;
}
