/**
 * Header: lab3.h
 * Descrição: funções básicas para manipular arquivo e para calculo de média
 * 
 * Autor: Fabricio e Lucas A. Seemund
 * Data: 11/09/2024
 */

 typedef struct {
    unsigned int *matrix;
    unsigned int M;
    unsigned int N;
    double *result;
    int start;
    int end;
} ThreadArgs;

//Quantidade de THREADS
#define NTHREADS 4 // Ajustável de acordo com o número de núcleos

// Função para calcular a média aritmética das linhas dentro de uma faixa
void* calcMediaAritmetica(void* arg);

// Função para calcular a média geométrica das colunas dentro de uma faixa
void* calcMediaGeometrica(void* arg);

// Função para ler a matriz de um arquivo
unsigned int* leMatrixArquivo(const char* filename, unsigned int* M, unsigned int* N);

// Função para gravar o resultado em um arquivo
void escreveArquivo(const char* filename, double* mediaArit, double* mediaGeo, int M, int N);