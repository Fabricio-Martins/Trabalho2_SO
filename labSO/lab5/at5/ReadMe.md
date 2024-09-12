autores: Lucas A. Seemund e Fabricio

### Descrição
## servidor
O servidor cria um socket UNIX para comunicação com clientes.
Ele aceita conexões e, para cada cliente, cria um processo filho que realiza a tradução.
O processo filho lê as mensagens do cliente, faz a tradução buscando no arquivo dicionario.txt e responde.
O servidor e cliente se comunicam até que a mensagem "NO-NO" seja enviada, o que encerra a conexão.

## Cliente
O cliente cria um socket e se conecta ao servidor.
Ele envia palavras para tradução no formato pt-en:cachorro.
Após enviar a tradução, o cliente recebe a resposta e exibe o resultado.
O cliente pode continuar enviando palavras até que envie "NO-NO" para encerrar a comunicação.
