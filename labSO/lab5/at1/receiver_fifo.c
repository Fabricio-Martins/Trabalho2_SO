#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include "fifo.h"

// Função para contar vogais, consoantes e espaços
void count_characters(const char *str, int *vogais, int *consoantes, int *spaces) {
    *vogais = 0;
    *consoantes = 0;
    *spaces = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        char ch = tolower(str[i]);
        if (ch == ' ') {
            (*spaces)++;
        } else if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u') {
            (*vogais)++;
        } else if (isalpha(ch)) {
            (*consoantes)++;
        }
    }
}

int main() {
    int fd;
    char received[100];
    int vogais, consoantes, spaces;

    FILE *fileLog = fopen("receiver_log.txt", "w");

    // Criação da FIFO
    mkfifo(FIFO_FILE, 0666);

    while (1) {
        // Abre a FIFO no modo de leitura e recebe a string
        fd = open(FIFO_FILE, O_RDONLY);
        read(fd, received, sizeof(received));
        close(fd);

        // Verifica se é "exit"
        if (strcmp(received, "exit") == 0) {
            break;
        }

        // Conta vogais, consoantes e espaços
        count_characters(received, &vogais, &consoantes, &spaces);

        fprintf(fileLog, "%s\n", received);

        // Exibe a string e suas informações
        printf("String recebida: %s\n", received);
        printf("Tamanho: %ld\n", strlen(received));
        printf("Vogais: %d\n", vogais);
        printf("Consoantes: %d\n", consoantes);
        printf("Espaços: %d\n\n", spaces);
    }
    fclose(fileLog);

    // Remove a FIFO
    unlink(FIFO_FILE);

    return 0;
}
