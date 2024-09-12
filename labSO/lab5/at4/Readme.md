Autores: Lucas A. Seemund e Fabricio

Foi utilizado shm_open para criar uma área de memória compartilhada chamada "/vetorResultado". Isso permite que todos os processos filhos e o processo pai compartilhem o vetor de resultado sem a necessidade de copiar os dados.

O número total de elementos é dividido igualmente entre os processos filhos. Se o número de elementos não for divisível de forma exata, o último processo filho processa o restante.

Cada processo filho lê o intervalo de trabalho do pipe e realiza a soma dos vetores no intervalo designado. Em seguida, o processo filho termina sua execução.

O processo pai cria os processos filhos, distribui o trabalho via pipes e aguarda que todos os processos terminem usando wait().