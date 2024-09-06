#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

//parte do lime
#include "lime.h"
static ssize_t write_lime_header(struct resource *);
static ssize_t write_padding(size_t);
static void write_range(struct resource *);
static int init(void);
static ssize_t write_vaddr(void *, size_t);
static ssize_t write_flush(void);
static ssize_t try_write(void *, ssize_t);
static int setup(void);
static void cleanup(void);

extern ssize_t write_vaddr_disk(void *, size_t);
extern int setup_disk(char *, int);
extern void cleanup_disk(void);

extern int ldigest_init(void);
extern int ldigest_update(void *, size_t);
extern int ldigest_final(void);
extern int ldigest_write_disk(void);
extern int ldigest_clean(void);

#ifdef LIME_SUPPORTS_DEFLATE
extern int deflate_begin_stream(void *, size_t);
extern int deflate_end_stream(void);
extern ssize_t deflate(const void *, size_t);
#endif

static char * format = 0;
static int mode = 0;
static int method = 0;
static void * vpage;
#ifdef LIME_SUPPORTS_DEFLATE
static void *deflate_page_buf;
#endif

char * path = "/home";
int dio = 0;
int port = 0;
int localhostonly = 0;
char * digest = 0;
int compute_digest = 0;
int no_overlap = 0;
extern struct resource iomem_resource;

//parte do ioctl
#define DEVICE_NAME "LiME"
#define IOCTL_SEND_CMD _IOW('a', 'a', char*)

static int major;
static char cmd_buffer[256];
static bool keep_running = true;

//código que chama o LiME
static void mDump(const char *path) {
    mode = LIME_MODE_RAW;
    method = LIME_METHOD_DISK;
    if (digest) compute_digest = LIME_DIGEST_COMPUTE;
}

//parte que valida a entrada do comandos
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    if (cmd == IOCTL_SEND_CMD) {
        if (copy_from_user(cmd_buffer, (char *)arg, sizeof(cmd_buffer))) {
            return -EFAULT;
        }

        printk(KERN_INFO "Received command: %s\n", cmd_buffer);

        if (strncmp(cmd_buffer, "start", 5) == 0) {
            char *path = cmd_buffer + 6;  // Skip "start "
            //mDump(path);
        } else if (strcmp(cmd_buffer, "Exit()") == 0) {
            keep_running = false;
        }
    }

    return 0;
}

static int device_open(struct inode *inode, struct file *file) {
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    return 0;
}

//estrutura para executar o ioctl
static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};

//cria o modulo
static int __init simple_module_init(void) {
	printk("oi lucas\n");
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return major;
    }

    printk(KERN_INFO "Module loaded with device major number %d\n", major);

    do {
        schedule();  // Allow the kernel to context switch
    //} while (keep_running);
	} while(0);

    return 0;
}

//deleta o modulo
static void __exit simple_module_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Module unloaded\n");
}

//chamadas para criar o modulo
module_init(simple_module_init);
module_exit(simple_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucas e Fabricio");
MODULE_DESCRIPTION("Simple Kernel Module with ioctl for command handling");