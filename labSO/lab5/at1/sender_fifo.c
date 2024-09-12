/**
 * Descrição: atividade 1 a do laboratorio 5
 *      * Esse é o programa que vai enviar as informações por fifo para o receiver
 * 
 * Autor: Fabricio e Lucas A. Seemund
 * Data: 11/09/2024
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fifo.h"

int main() {
    int fd;
    char input[100];

    // Criação da FIFO
    mkfifo(FIFO_FILE, 0666);

    while (1) {
        // Recebendo string do usuário
        printf("Digite uma string (ou 'exit' para sair): ");
        fgets(input, sizeof(input), stdin);

        // Remover a nova linha no final da string (causada pelo fgets)
        input[strcspn(input, "\n")] = 0;

        // Se o usuário digitar "exit", o programa encerra
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Abre a FIFO no modo de escrita e envia a string
        fd = open(FIFO_FILE, O_WRONLY);
        write(fd, input, strlen(input) + 1);
        close(fd);
    }

    // Remove a FIFO
    unlink(FIFO_FILE);

    return 0;
}
