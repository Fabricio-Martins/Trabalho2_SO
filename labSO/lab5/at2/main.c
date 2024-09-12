/**
 * Descrição: atividade 2 b do laboratorio 5
 * 
 * Autor: Fabricio e Lucas A. Seemund
 * Data: 11/09/2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define FILENAME "data.txt"
#define BUFFERW_W_SIZE 256
#define BUFFER_R_SIZE 64

FILE *file;        // Ponteiro global para o arquivo
char bufferEscrita[BUFFERW_W_SIZE]; // Buffer para armazenar dados pendentes
int countBufferOcupado; //Armazena a posicao do ultimo caracter no buffer de escrita

void handle_signal(int sig);

// Escreve no buffer
void flushBuffer(){
    // Escreve o buffer no arquivo
    fprintf(file, "%s", bufferEscrita);
    fflush(file); // Garante que os dados sejam gravados imediatamente

    //apaga o que está no buffer
    countBufferOcupado = 0;
    bufferEscrita[0] = '\0';
}

// Verifica se o buffer está cheio
void verificaBuffer(){
    if(countBufferOcupado >= BUFFERW_W_SIZE - 1){
        flushBuffer();
    }
}

// Passa os valores lidos no terminal para o buffer de esrita
int armazenaBuffer(char *bufferLeitura){
    char c = '\0';
    
    for(int i=0; i<BUFFER_R_SIZE; i++){
        c = bufferLeitura[i];
        if(c == '\0'){ //se o buffer de leitura terminou para o loop
            break;
        }

        //escreve no buffer de escrita o que foi digitado na tela
        bufferEscrita[countBufferOcupado] = c; //coloca no buffer de escrita
        countBufferOcupado++;
        verificaBuffer();
    }

    return(0);
}

int main() {
    char BufferLeituraTerminal[BUFFER_R_SIZE];
    countBufferOcupado = 0; //

    // Registra os sinais SIGINT e SIGTERM
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Abre o arquivo para escrita e leitura (cria se não existir)
    file = fopen(FILENAME, "a+");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    printf("Arquivo %s aberto. Digite para armazenar no arquivo.\n", FILENAME);
    printf("Pressione Ctrl+C para terminar (SIGINT) ou envie um signal (SIGTERM) para finalização limpa.\n");

    // Loop principal
    while (1) {
        printf("Digite uma string (ou 'exit' para sair): ");
        if (fgets(BufferLeituraTerminal, BUFFER_R_SIZE, stdin) != NULL) {
            // Se o usuário digitar "exit", encerra o programa normalmente
            if (strncmp(BufferLeituraTerminal, "exit", 4) == 0) {
                break;
            }

            armazenaBuffer(BufferLeituraTerminal);
        }
    }

    // Finalização limpa quando o usuário encerra com "exit"
    handle_signal(0);

    return 0;
}

// Função que faz o tratamento de sinais
void handle_signal(int sig) {
    printf("\nRecebido sinal %d, finalizando...\n", sig);

    // Escreve dados pendentes no arquivo antes de fechar
    if (strlen(bufferEscrita) > 0) {
        flushBuffer();
        printf("Dados pendentes escritos no arquivo.\n");
    }

    // Fecha o arquivo
    if (file) {
        fclose(file);
        printf("Arquivo fechado com sucesso.\n");
    }

    exit(0); // Termina o programa de forma limpa
}
