#include <linux/ioctl.h>

#define IOCTL_CMD_START _IO(IOCTL_MAGIC, 1)
#define IOCTL_CMD_STOP  _IO(IOCTL_MAGIC, 2)

static long ioctl(struct file *f, unsigned int cmd, unsigned long arg);

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case IOCTL_CMD_START:
            pr_info("ioctlmod: Start command received\n");
            break;
        case IOCTL_CMD_STOP:
            pr_info("ioctlmod: Stop command received\n");
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .open = dev_open,
    .unlocked_ioctl = dev_ioctl,
    .release = dev_release,
};

static struct cdev cdev;
static dev_t dev_num;
static struct class *cl;
