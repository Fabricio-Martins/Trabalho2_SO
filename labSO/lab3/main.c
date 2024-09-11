/**
 * Header: lab3.h
 * Descrição: funções básicas para manipular arquivo e para calculo de média
 * 
 * Autor: Fabricio e Lucas A. Seemund
 * Data: 11/09/2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "lab3.h"

//operação matemática que representa a multiplicação sucessiva de um número por ele mesmo
//base^exp
int potencia(int base, int exp){
    if(exp < 0)
    return -1;

    int result = 1;
    while (exp){
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}


// Função para calcular a média aritmética das linhas dentro de uma faixa
void* calcMediaAritmetica(void* arg) {
    ThreadArgs* args = (ThreadArgs*) arg;
    int N = args->N;
    
    for (int i = args->start; i < args->end; i++) {
        double sum = 0;
        for (int j = 0; j < N; j++) {
            sum += args->matrix[i * N + j];
        }
        args->result[i] = sum / N;
    }
    pthread_exit(NULL);
}

// Função para calcular a média geométrica das colunas dentro de uma faixa
void* calcMediaGeometrica(void* arg) {
    ThreadArgs* args = (ThreadArgs*) arg;
    int M = args->M;

    for (int j = args->start; j < args->end; j++) {
        double product = 1.0;
        for (int i = 0; i < M; i++) {
            product *= args->matrix[i * args->N + j];
        }
        args->result[j] = potencia(product, 1.0 / M);
    }
    pthread_exit(NULL);
}

// Função para ler a matriz de um arquivo
//  Código de leitura de arquivo pego da internet
unsigned int* leMatrixArquivo(const char* filename, unsigned int* M, unsigned int* N) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        exit(1);
    }
    
    fscanf(file, "%dx%d", M, N);

    printf("%d x %d\n", *M, *N);  //Mostra no prompt qual a dimenção da matriz encontrada no arquivo

    unsigned int *matrix = (unsigned int*) malloc((*M) * (*N) * sizeof(unsigned int)); //aloca a matriz

    for (int i = 0; i < (*M) * (*N); i++) {
        fscanf(file, "%d", &matrix[i]);  // Lê os elementos da matriz
    }

    fclose(file);
    return matrix;
}

// Função para gravar o resultado em um arquivo
void escreveArquivo(const char* filename, double* mediaArit, double* mediaGeo, int M, int N) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo para escrever.\n");
        exit(1);
    }

    fprintf(file, "Médias Aritméticas das Linhas:\n");
    for (int i = 0; i < M; i++) {
        fprintf(file, "Linha %d: %.2f\n", i, mediaArit[i]);
    }

    fprintf(file, "\nMédias Geométricas das Colunas:\n");
    for (int j = 0; j < N; j++) {
        fprintf(file, "Coluna %d: %.2f\n", j, mediaGeo[j]);
    }

    fclose(file);
}

int main(int argc, char *argv[]){
    unsigned int M, N;

    int qtdThreads = NTHREADS / 2; // dividido por 2 por cause que metade vai para media aritimetica e a outra para media geometrica

    char input_file[100] = "matriz.txt";
    if(argc >= 2){
        strcpy(input_file, argv[1]);
    }
    printf("Arquivo de entrada: %s\n", input_file);

    const char *output_file = "resultado.txt";

    // Ler a matriz do arquivo
    unsigned int *matrix = leMatrixArquivo(input_file, &M, &N);

    // Vetores para armazenar os resultados das médias
    double *mediaAritmetica = (double*) malloc(M * sizeof(double));
    double *mediaGeometrica = (double*) malloc(N * sizeof(double));

    // Criação de threads para as médias aritméticas (linhas)
    pthread_t *ListaThreadsArit = (pthread_t*) malloc(qtdThreads * sizeof(pthread_t));
    ThreadArgs *tArgsAritmetica = (ThreadArgs*) malloc(qtdThreads * sizeof(ThreadArgs));
    int partition_size = M / qtdThreads;

    for (int i = 0; i < qtdThreads; i++) {
        tArgsAritmetica[i].matrix = matrix;
        tArgsAritmetica[i].M = M;
        tArgsAritmetica[i].N = N;
        tArgsAritmetica[i].result = mediaAritmetica;
        tArgsAritmetica[i].start = i * partition_size;
        tArgsAritmetica[i].end = (i == qtdThreads - 1) ? M : (i + 1) * partition_size;
        pthread_create(&ListaThreadsArit[i], NULL, calcMediaAritmetica, (void*) &tArgsAritmetica[i]);
    }

    // Criação de threads para as médias geométricas (colunas)
    pthread_t *ListaThreadsGeo = (pthread_t*) malloc(qtdThreads * sizeof(pthread_t));
    ThreadArgs *tArgsGeometrica = (ThreadArgs*) malloc(qtdThreads * sizeof(ThreadArgs));
    partition_size = N / qtdThreads;

    for (int j = 0; j < qtdThreads; j++) {
        tArgsGeometrica[j].matrix = matrix;
        tArgsGeometrica[j].M = M;
        tArgsGeometrica[j].N = N;
        tArgsGeometrica[j].result = mediaGeometrica;
        tArgsGeometrica[j].start = j * partition_size;
        tArgsGeometrica[j].end = (j == qtdThreads - 1) ? N : (j + 1) * partition_size;
        pthread_create(&ListaThreadsGeo[j], NULL, calcMediaGeometrica, (void*) &tArgsGeometrica[j]);
    }

    // Aguardar as threads das médias aritméticas terminarem
    for (int i = 0; i < qtdThreads; i++) {
        pthread_join(ListaThreadsArit[i], NULL);
    }

    // Aguardar as threads das médias geométricas terminarem
    for (int j = 0; j < qtdThreads; j++) {
        pthread_join(ListaThreadsGeo[j], NULL);
    }

    // Gravar os resultados no arquivo
    escreveArquivo(output_file, mediaAritmetica, mediaGeometrica, M, N);

    // Limpar a memória alocada
    free(matrix);
    free(mediaAritmetica);
    free(mediaGeometrica);
    free(ListaThreadsArit);
    free(ListaThreadsGeo);
    free(tArgsAritmetica);
    free(tArgsGeometrica);

    printf("Resultados gravados em %s\n", output_file);
    return 0;
}
