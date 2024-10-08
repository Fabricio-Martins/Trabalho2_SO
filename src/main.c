#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/delay.h>

#include "lime.h"

// This file
static ssize_t write_lime_header(struct resource *);
static ssize_t write_padding(size_t);
static void write_range(struct resource *);
static ssize_t write_vaddr(void *, size_t);
static ssize_t write_flush(void);
static ssize_t try_write(void *, ssize_t);
static int setup(void);
static void cleanup(void);
static int limeInit(void);

// External
extern ssize_t write_vaddr_tcp(void *, size_t);
extern int setup_tcp(void);
extern void cleanup_tcp(void);

extern ssize_t write_vaddr_disk(void *, size_t);
extern int setup_disk(char *, int);
extern void cleanup_disk(void);

extern int ldigest_init(void);
extern int ldigest_update(void *, size_t);
extern int ldigest_final(void);
extern int ldigest_write_tcp(void);
extern int ldigest_write_disk(void);
extern int ldigest_clean(void);

#ifdef LIME_SUPPORTS_DEFLATE
extern int deflate_begin_stream(void *, size_t);
extern int deflate_end_stream(void);
extern ssize_t deflate(const void *, size_t);
#endif

static int mode = 0;
static int method = 0;

static void * vpage;

#ifdef LIME_SUPPORTS_DEFLATE
static void *deflate_page_buf;
#endif

char * path = 0;
int dio = 0;
int port = 0;
int localhostonly = 0;

char * digest = 0;
char *format = "raw";
int compute_digest = 0;

int no_overlap = 0;

extern struct resource iomem_resource;

#ifdef LIME_SUPPORTS_TIMING
long timeout = 1000;
module_param(timeout, long, S_IRUGO);
#endif

#ifdef LIME_SUPPORTS_DEFLATE
int compress = 0;
module_param(compress, int, S_IRUGO);
#endif



// Parte do ioctl
#define DEVICE_NAME "LiME"
#define IOCTL_SEND_CMD _IOW('a', 'a', char*)

static int major;
static char cmd_buffer[256];
static bool keep_running = true;

// Função que chama o LiME para realizar o dump de memória
static void mDump(void) {
    mode = LIME_MODE_RAW;          // Configura o modo de operação
    method = LIME_METHOD_DISK;     // Define o método de escrita em disco
    if (digest) compute_digest = LIME_DIGEST_COMPUTE; // Calcula o digest, se definido
	limeInit(); //chama o comando de inicio do LiME
}

// Função que valida a entrada dos comandos via ioctl
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    if (cmd == IOCTL_SEND_CMD) {
		
		// Copia o comando do espaço de usuário para o kernel
        if (copy_from_user(cmd_buffer, (char *)arg, sizeof(cmd_buffer))) {
            return -EFAULT;
        }

        printk(KERN_INFO "Recebido pelo programa ./bash: %s\n", cmd_buffer);

		// Verifica se o comando é "start <path>"
        if (strncmp(cmd_buffer, "start", 5) == 0) {
            path = cmd_buffer + 6;  // Extrai o caminho após "start "
			if(!path) {
				printk(KERN_INFO "No path parameter specified");
				return -EINVAL;
			}
			printk(KERN_INFO "Iniciando despejo de memória no caminho: \" %s \"\n", path);
            mDump();  // Chama a função mDump para realizar o dump
			printk(KERN_INFO "Despejo de memória terminado\n");
		} else if (strcmp(cmd_buffer, "ping") == 0) {
			printk(KERN_INFO "...Pong...\n");	//Responde o ping
        } else if (strcmp(cmd_buffer, "Exit()") == 0) {
            keep_running = false; // Sinaliza para encerrar o loop
			printk(KERN_INFO "Saindo do Modulo\n");
        }
    }
	mdelay(100);
    return 0;
}

// Funções open e release do dispositivo
static int device_open(struct inode *inode, struct file *file) {
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    return 0;
}

// Estrutura de operações do dispositivo
static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release,
};

// Função de inicialização do módulo
static int __init simple_module_init(void) {
	printk("Iniciando Modulo\n");
    major = register_chrdev(0, DEVICE_NAME, &fops);	// Registra o dispositivo
    if (major < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return major;
    }

    printk(KERN_INFO "Module loaded with device major number %d\n", major);

    do {	// Loop principal que aguarda comandos até receber "Exit()"
        schedule();  // Permite que o kernel faça a troca de contexto
    } while (keep_running);

    return 0;
}

// Função de remoção do módulo
static void __exit simple_module_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);	// Remove o dispositivo registrado
    printk(KERN_INFO "Module unloaded\n");
}

// Chamadas para inicialização e remoção do módulo
module_init(simple_module_init);
module_exit(simple_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fabricio e Lucas A. Seemund");
MODULE_DESCRIPTION("Simple Kernel Module with ioctl for command handling");



//Daqui para baixo é parte do antigo modulo LiME e não foi alterado

/*
 * LiME - Linux Memory Extractor
 * Copyright (c) 2011-2014 Joe Sylve - 504ENSICS Labs
 *
 *
 * Author:
 * Joe Sylve       - joe.sylve@gmail.com, @jtsylve
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

static int limeInit() {
    struct resource *p;
    int err = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
    resource_size_t p_last = -1;
#else
    __PTRDIFF_TYPE__ p_last = -1;
#endif

    DBG("Initializing Dump...");

    if ((err = setup())) {
        DBG("Setup Error");
        cleanup();
        return err;
    }

    if (digest) {
        compute_digest = ldigest_init();
        no_overlap = 1;
    }

    vpage = (void *) __get_free_page(GFP_NOIO);

#ifdef LIME_SUPPORTS_DEFLATE
    if (compress) {
        deflate_page_buf = kmalloc(PAGE_SIZE, GFP_NOIO);
        err = deflate_begin_stream(deflate_page_buf, PAGE_SIZE);
        if (err < 0) {
            DBG("ZLIB begin stream failed");
            return err;
        }
        no_overlap = 1;
    }
#endif

    for (p = iomem_resource.child; p ; p = p->sibling) {

        if (!p->name || strcmp(p->name, LIME_RAMSTR))
            continue;

        if (mode == LIME_MODE_LIME && write_lime_header(p) < 0) {
            //DBG("Error writing header 0x%lx - 0x%lx", (long) p->start, (long) p->end);
            break;
        } else if (mode == LIME_MODE_PADDED && write_padding((size_t) ((p->start - 1) - p_last)) < 0) {
            //DBG("Error writing padding 0x%lx - 0x%lx", (long) p_last, (long) p->start - 1);
            break;
        }

        write_range(p);

        p_last = p->end;
    }

    write_flush();

    DBG("Memory Dump Complete...");

    cleanup();

    if (compute_digest == LIME_DIGEST_COMPUTE) {
        DBG("Writing Out Digest.");

        compute_digest = ldigest_final();

        if (compute_digest == LIME_DIGEST_COMPLETE) {
            if (method == LIME_METHOD_TCP)
                err = ldigest_write_tcp();
            else
                err = ldigest_write_disk();

            DBG("Digest Write %s.", (err == 0) ? "Complete" : "Failed");
        }
    }

    if (digest)
        ldigest_clean();

#ifdef LIME_SUPPORTS_DEFLATE
    if (compress) {
        deflate_end_stream();
        kfree(deflate_page_buf);
    }
#endif

    free_page((unsigned long) vpage);

    return 0;
}

static ssize_t write_lime_header(struct resource * res) {
    lime_mem_range_header header;

    memset(&header, 0, sizeof(lime_mem_range_header));
    header.magic = LIME_MAGIC;
    header.version = 1;
    header.s_addr = res->start;
    header.e_addr = res->end;

    return write_vaddr(&header, sizeof(lime_mem_range_header));
}

static ssize_t write_padding(size_t s) {
    size_t i = 0;
    ssize_t r;

    memset(vpage, 0, PAGE_SIZE);

    while(s -= i) {

        i = min((size_t) PAGE_SIZE, s);
        r = write_vaddr(vpage, i);

        if (r != i) {
            DBG("Error sending zero page: %zd", r);
            return r;
        }
    }

    return 0;
}

static void write_range(struct resource * res) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
    resource_size_t i, is;
#else
    __PTRDIFF_TYPE__ i, is;
#endif
    struct page * p;
    void * v;

    ssize_t s;

#ifdef LIME_SUPPORTS_TIMING
    ktime_t start,end;
#endif

    DBG("Writing range %llx - %llx.", res->start, res->end);

    for (i = res->start; i <= res->end; i += is) {
#ifdef LIME_SUPPORTS_TIMING
        start = ktime_get_real();
#endif
        p = pfn_to_page((i) >> PAGE_SHIFT);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        is = min((resource_size_t) PAGE_SIZE, (resource_size_t) (res->end - i + 1));
#else
        is = min((size_t) PAGE_SIZE, (size_t) (res->end - i + 1));
#endif

        if (is < PAGE_SIZE) {
            // We can't map partial pages and
            // the linux kernel doesn't use them anyway
            DBG("Padding partial page: vaddr %p size: %lu", (void *) i, (unsigned long) is);
            write_padding(is);
        } else {
#ifdef LIME_USE_KMAP_ATOMIC
            v = kmap_atomic(p);
#else
            v = kmap(p);
#endif
            /*
             * If we need to compute the digest or compress the output
             * take a snapshot of the page. Otherwise save some cycles.
             */
#ifdef LIME_USE_KMAP_ATOMIC
            preempt_enable();
#endif
            if (no_overlap) {
                copy_page(vpage, v);
                s = write_vaddr(vpage, is);
            } else {
                s = write_vaddr(v, is);
            }
#ifdef LIME_USE_KMAP_ATOMIC
            preempt_disable();
            kunmap_atomic(v);
#else
            kunmap(p);
#endif
            if (s < 0) {
                DBG("Failed to write page: vaddr %p. Skipping Range...", v);
                break;
            }
        }

#ifdef LIME_SUPPORTS_TIMING
        end = ktime_get_real();

        if (timeout > 0 && ktime_to_ms(ktime_sub(end, start)) > timeout) {
            DBG("Reading is too slow.  Skipping Range...");
            write_padding(res->end - i + 1 - is);
            break;
        }
#endif

    }
}

static ssize_t write_vaddr(void * v, size_t is) {
    ssize_t ret;

    if (compute_digest == LIME_DIGEST_COMPUTE)
        compute_digest = ldigest_update(v, is);

#ifdef LIME_SUPPORTS_DEFLATE
    if (compress) {
        /* Run deflate() on input until output buffer is not full. */
        do {
            ret = try_write(deflate_page_buf, deflate(v, is));
            if (ret < 0)
                return ret;
        } while (ret == PAGE_SIZE);
        return is;
    }
#endif

    ret = try_write(v, is);
    return ret;
}

static ssize_t write_flush(void) {
#ifdef LIME_SUPPORTS_DEFLATE
    if (compress) {
        try_write(deflate_page_buf, deflate(NULL, 0));
    }
#endif
    return 0;
}

static ssize_t try_write(void * v, ssize_t is) {
    ssize_t ret;

    if (is <= 0)
        return is;

    ret = RETRY_IF_INTERRUPTED(
        (method == LIME_METHOD_TCP) ? write_vaddr_tcp(v, is) : write_vaddr_disk(v, is)
    );

    if (ret < 0) {
        DBG("Write error: %zd", ret);
    } else if (ret != is) {
        DBG("Short write %zu instead of %zu.", ret, is);
        ret = -1;
    }

    return ret;
}

static int setup(void) {
    return (method == LIME_METHOD_TCP) ? setup_tcp() : setup_disk(path, dio);
}

static void cleanup(void) {
    return (method == LIME_METHOD_TCP) ? cleanup_tcp() : cleanup_disk();
}
