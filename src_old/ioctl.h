#ifndef MY_IOCTL_H
#define MY_IOCTL_H

#include <linux/ioctl.h>

typedef struct{
    int status;
	char *path;
} LiME_path;

#define IOCTL_WAIT_FOR_VALUE _IOW('a', 1, LiME_path*)

#define QUERY_GET_VARIABLES _IOR('q', 1, query_arg_t *)
#define QUERY_CLR_VARIABLES _IO('q', 2)
#define QUERY_SET_VARIABLES _IOW('q', 3, query_arg_t *)

#define DEVICE_NAME "/dev/LiME"

#endif // MY_IOCTL_H
