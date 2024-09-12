Programa feito por Fabricio e Lucas A. Seemund

Faça um programa que manipule arquivos (ler e escrever) e que, ao receber o signal 2 (SIGINT)
ou signal 15 (SIGTERM), faça uma finalização limpa (graceful stop) – armazenar as informações
pendentes e fechar o arquivo.


Para testar a finalização limpa, você pode:
* Pressionar Ctrl+C no terminal, que envia o sinal SIGINT.

* Usar o comando kill para enviar o sinal SIGTERM:
* * kill -15 <PID_DO_PROGRAMA>