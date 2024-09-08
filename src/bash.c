#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <string.h>

#define DEVICE_NAME "/dev/lime"   // Define o nome do dispositivo
#define IOCTL_SEND_CMD _IOW('a', 'a', char*)  // Define o comando ioctl para enviar dados

int main() {
    int fd;  // Variável para o descritor de arquivo
    char command[256];  // Buffer para armazenar o comando do usuário

    // Abre o dispositivo "/dev/lime" com permissões de leitura/escrita
    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        // Se a abertura falhar, exibe uma mensagem de erro e encerra o programa
        perror("Failed to open the device...");
        return -1;
    }
	
    // Loop principal para ler comandos do usuário
    while (1) {
		printf("Comandos: (start <path>, ping ou Exit()): ");
        // Lê o comando inserido pelo usuário
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';  // Remove o caractere de nova linha ao final do comando

        // Envia o comando via ioctl para o módulo do kernel
        if (ioctl(fd, IOCTL_SEND_CMD, command) == -1) {
            // Se ocorrer um erro ao enviar o comando, exibe uma mensagem de erro e encerra o programa
            perror("Failed to send ioctl command");
            return -1;
        }
		
		//system("sudo dmesg");

        // Se o comando for "Exit()", encerra o loop
        if (strcmp(command, "Exit()") == 0) {
            break;
        }
    }

    // Fecha o descritor de arquivo antes de encerrar o programa
    close(fd);
    return 0;
}
