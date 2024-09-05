#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ioctl.h"

int main() {
    int fd;
    
	LiME_path dadosParaModulo;
	
	//system("insmod lime.ko");

    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }
	
	do{
		
		scanf("%s", dadosParaModulo.path);
		
		if (ioctl(fd, IOCTL_WAIT_FOR_VALUE, &dadosParaModulo) == -1) {
			perror("Failed to send ioctl");
			close(fd);
			return -1;
		}

		printf("Valor %s enviado ao módulo kernel\n", dadosParaModulo.path);
		
	}while(strcmp(dadosParaModulo.path, "exit()") == 0);

	// Remove the module
    system("rmmod lime");

    close(fd);
    return 0;
}
