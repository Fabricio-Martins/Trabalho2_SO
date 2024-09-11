### Compilação
Para compilar utilize o comando make

### execução
Após a compilação o arquivo executavel `lab3` será criado.
Para rodar o programa passe por parametro o nome do arquivo de entrada.
`./lab3 |nome do arquivo|`

Por padrão o programa vai utilizar 4 threads, mas pode ser alterado no arquivo `lab3.h`

### Descrição do código
Representação da Matriz:

A matriz armazenada em um array de inteiros linear de tamanho 𝑀 × 𝑁, acessada como matrix[i * N + j] para acessar o elemento da linha 𝑖 e coluna 𝑗.

Para as médias aritméticas, cada thread processa uma faixa de linhas.
Para as médias geométricas, cada thread processa uma faixa de colunas.

Uso de pthread_t para Gerenciar Threads:

O número de threads é ajustável, e a divisão do trabalho é feita em função do número de linhas ou colunas, de acordo com a faixa calculada.

