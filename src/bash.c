#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_WAIT_FOR_VALUE _IOW('a', 1, int)

int main() {
    int fd;
    int value = 123;  // Exemplo de valor para enviar ao ioctl
	
	//system("insmod lime.ko");

    fd = open("/dev/byte_string_device", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    if (ioctl(fd, IOCTL_WAIT_FOR_VALUE, &value) == -1) {
        perror("Failed to send ioctl");
        close(fd);
        return -1;
    }

    printf("Valor %d enviado ao módulo kernel\n", value);
	
	// Remove the module
    system("rmmod lime");

    close(fd);
    return 0;
}
