### Questão 1 do lab 3

## Descrição do código
Representação da Matriz:

A matriz armazenada em um array de inteiros linear de tamanho 𝑀 × 𝑁, acessada como matrix[i * N + j] para acessar o elemento da linha 𝑖 e coluna 𝑗.

Para as médias aritméticas, cada thread processa uma faixa de linhas.
Para as médias geométricas, cada thread processa uma faixa de colunas.

Uso de pthread_t para Gerenciar Threads:

O número de threads é ajustável, e a divisão do trabalho é feita em função do número de linhas ou colunas, de acordo com a faixa calculada.
