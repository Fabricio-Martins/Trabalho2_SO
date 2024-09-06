#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <string.h>

//#include "lime.h"

#define DEVICE_NAME "/dev/lime"
#define IOCTL_SEND_CMD _IOW('a', 'a', char*)

int main() {
    int fd;
    char command[256];

    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device...");
        return -1;
    }

    while (1) {
        printf("Enter command (start <path> or Exit()): ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';  // Remove newline

        if (ioctl(fd, IOCTL_SEND_CMD, command) == -1) {
            perror("Failed to send ioctl command");
            return -1;
        }

        if (strcmp(command, "Exit()") == 0) {
            break;
        }
    }

    close(fd);
    return 0;
}
