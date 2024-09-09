# Simple Kernel Module with LiME Integration
##[LiME](https://github.com/504ensicsLabs/LiME.git)

## Autores
- **Lucas**
- **Fabricio**

## Descrição
Este módulo de kernel é um dispositivo de caractere que utiliza `ioctl` para receber comandos de um programa de espaço de usuário. Ele integra parte do código do LiME (Linux Memory Extractor) para realizar o despejo de memória com base em comandos enviados pelo usuário.
O Modulo [LiME](https://github.com/504ensicsLabs/LiME.git) pode ser encontrado aqui -> https://github.com/504ensicsLabs/LiME.git

### Funcionalidades
- **Comando `start <path>`**: Realiza um despejo de memória usando o caminho fornecido.
- **Comando `Exit()`**: Termina a execução do módulo.

## Como usar

### Compilar o Módulo
1. Compile o módulo:
	make
	O make vai compilar o modulo e o programa de usuário
### Usar o modulo
1. Carregue o módulo:
	sudo insmod simple_module.ko
2. Verifique o número major do dispositivo:
	dmesg | grep "Module loaded with device major number"
3. Crie o dispositivo de caractere:
	sudo mknod /dev/lime c <major_number> 0
	Substitua <major_number> pelo número major exibido no log.
4. Inicie o programa de usuário
	./bash
5. Iniciar o despejo
	No prompt de comando rodando o ./bash digite o comando start <path>
	No qual o `path` é o caminho para o despejo de memória do `LiME`.
