/**
 * Descrição: Atividade 4 do Lab 5
 * 
 * Autor: Lucas A. Seemund e Fabricio
 * Data: 11/09/2024
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

void somar_vetor(int* v1, int* v2, int* resultado, int inicio, int fim) {
    for (int i = inicio; i < fim; i++) {
        resultado[i] = v1[i] + v2[i];
    }
}

int main() {
    int numElementos, numProcessos;

    // Solicitar ao usuário o número de elementos e processos
    printf("Digite o número de elementos no vetor: ");
    scanf("%d", &numElementos);
    printf("Digite o número de processos filhos: ");
    scanf("%d", &numProcessos);

    // Vetores para somar
    int *v1 = malloc(sizeof(int) * numElementos);
    int *v2 = malloc(sizeof(int) * numElementos);

    // Preencher vetores com valores aleatórios
    srand(time(NULL));
    for (int i = 0; i < numElementos; i++) {
        v1[i] = rand() % 100;
        v2[i] = rand() % 100;
    }

    //mostra os valores dos 2 vetores
    printf("V1  V2 \n");
    for(int i=0; i<numElementos; i++ ){
        printf("%d %d\n", v1[i], v2[i]);
    }

    // Criar memória compartilhada para o vetor resultado
    int tamanhoMemoria = sizeof(int) * numElementos;
    int shm_fd = shm_open("/vetorResultado", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, tamanhoMemoria);
    int *resultado = mmap(NULL, tamanhoMemoria, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Inicializar vetor resultado
    memset(resultado, 0, tamanhoMemoria);

    // Criar pipes para comunicação entre processos
    int pipes[numProcessos][2];
    for (int i = 0; i < numProcessos; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Erro ao criar pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Definir quantos elementos cada processo filho deve processar
    int dadosPorFilho = numElementos / numProcessos;
    int sobra = numElementos % numProcessos; // Para lidar com divisões não exatas

    // Criar processos filhos
    for (int i = 0; i < numProcessos; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Erro ao criar processo filho");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {  // Código para os processos filhos
            close(pipes[i][1]); // Fechar o lado de escrita do pipe

            int inicio, fim;
            read(pipes[i][0], &inicio, sizeof(int));
            read(pipes[i][0], &fim, sizeof(int));
            close(pipes[i][0]); // Fechar o lado de leitura do pipe

            // Executar a soma dos vetores no intervalo designado
            somar_vetor(v1, v2, resultado, inicio, fim);
            exit(0); // Finalizar processo filho
        }
    }

    // Código do processo pai
    for (int i = 0; i < numProcessos; i++) {
        int inicio = i * dadosPorFilho;
        int fim = (i == numProcessos - 1) ? (inicio + dadosPorFilho + sobra) : (inicio + dadosPorFilho);

        close(pipes[i][0]); // Fechar o lado de leitura do pipe

        // Enviar intervalo de trabalho para o filho via pipe
        write(pipes[i][1], &inicio, sizeof(int));
        write(pipes[i][1], &fim, sizeof(int));
        close(pipes[i][1]); // Fechar o lado de escrita do pipe
    }

    // Esperar os processos filhos terminarem
    for (int i = 0; i < numProcessos; i++) {
        wait(NULL);
    }

    // Exibir resultado
    printf("Resultado da soma dos vetores:\n");
    for (int i = 0; i < numElementos; i++) {
        printf("v1[%d] + v2[%d] = %d\n", i, i, resultado[i]);
    }

    // Limpeza: liberar memória e remover memória compartilhada
    munmap(resultado, tamanhoMemoria);
    shm_unlink("/vetorResultado");

    free(v1);
    free(v2);

    return 0;
}
