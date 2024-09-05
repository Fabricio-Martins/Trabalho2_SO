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

#include "lime.h"

// This file
static ssize_t write_lime_header(struct resource *);
static ssize_t write_padding(size_t);
static void write_range(struct resource *);
static int init(void);
static ssize_t write_vaddr(void *, size_t);
static ssize_t write_flush(void);
static ssize_t try_write(void *, ssize_t);
static int setup(void);
static void cleanup(void);

// External
//extern ssize_t write_vaddr_tcp(void *, size_t);
//extern int setup_tcp(void);
//extern void cleanup_tcp(void);

extern ssize_t write_vaddr_disk(void *, size_t);
extern int setup_disk(char *, int);
extern void cleanup_disk(void);

extern int ldigest_init(void);
extern int ldigest_update(void *, size_t);
extern int ldigest_final(void);
//extern int ldigest_write_tcp(void);
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

char * path = 0;
int dio = 0;
int port = 0;
int localhostonly = 0;

char * digest = 0;
int compute_digest = 0;

int no_overlap = 0;

extern struct resource iomem_resource;

//module_param(path, charp, S_IRUGO);
//module_param(dio, int, S_IRUGO);
//module_param(format, charp, S_IRUGO);
//module_param(localhostonly, int, S_IRUGO);
//module_param(digest, charp, S_IRUGO);

//#ifdef LIME_SUPPORTS_TIMING
//long timeout = 1000;
//module_param(timeout, long, S_IRUGO);
//#endif

#ifdef LIME_SUPPORTS_DEFLATE
int compress = 0;
module_param(compress, int, S_IRUGO);
#endif

#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/sched.h>

#include "ioctl.h"

LiME_path dadosLiME;
static int ioctl_value_set = 0;
static wait_queue_head_t ioctl_wait_queue;
static DEFINE_MUTEX(ioctl_mutex);

int init_module (void)
{
    if(!path) {
        printk("[LiME] No path parameter specified\n");
        return -EINVAL;
    }

    //if(!format) {
    //    DBG("No format parameter specified");
    //    return -EINVAL;
    //}

    printk("[LiME] Parameters\n");
    printk("[LiME]   PATH: %s\n", path);
    printk("[LiME]   DIO: %u\n", dio);
    printk("[LiME]   FORMAT: %s\n", format);
    printk("[LiME]   LOCALHOSTONLY: %u\n", localhostonly);
    printk("[LiME]   DIGEST: %s\n", digest);

//#ifdef LIME_SUPPORTS_TIMING
//    DBG("  TIMEOUT: %lu", timeout);
//#endif

//#ifdef LIME_SUPPORTS_DEFLATE
//    DBG("  COMPRESS: %u", compress);
//#endif

    if (!strcmp(format, "raw")) mode = LIME_MODE_RAW;
    else if (!strcmp(format, "lime")) mode = LIME_MODE_LIME;
    else if (!strcmp(format, "padded")) mode = LIME_MODE_PADDED;
    else {
        printk("[LiME] Invalid format parameter specified.\n");
        return -EINVAL;
    }

    method = LIME_METHOD_DISK;
    if (digest) compute_digest = LIME_DIGEST_COMPUTE;
	
	do{
		printk("[LiME] Esperando valor via ioctl...\n");

        // Espera pelo ioctl com timeout infinito (HZ * segundos se precisar de timeout)
        wait_event_interruptible(ioctl_wait_queue, ioctl_value_set != 0);

        mutex_lock(&ioctl_mutex);
        //printk("[LiME] Valor recebido do ioctl: %d\n", ioctl_value);
        ioctl_value_set = 0;  // Resetar para que possa esperar outro valor no futuro
        mutex_unlock(&ioctl_mutex);

        // Aqui, o código pode continuar
		init();
	}while(0);


    return 0;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    LiME_path dados;

    switch (cmd) {
        case IOCTL_WAIT_FOR_VALUE:
            if (copy_from_user(&dados, (LiME_path *)arg, sizeof(LiME_path))) {
                return -EFAULT;
            }

            mutex_lock(&ioctl_mutex);
            dadosLiME.status = dados.status;
			dadosLiME.path = dados.path;
            ioctl_value_set = 1;
            mutex_unlock(&ioctl_mutex);

            wake_up_interruptible(&ioctl_wait_queue);  // Acordar o módulo para continuar
            //printk(KERN_INFO "ioctl: valor recebido = %d\n", ioctl_value);
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
};

static int init() {
    struct resource *p;
    int err = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
    resource_size_t p_last = -1;
#else
    __PTRDIFF_TYPE__ p_last = -1;
#endif

    printk("[LiME] Initializing Dump...\n");

    if ((err = setup())) {
        printk("[LiME] Setup Error\n");
        cleanup();
        return err;
    }

    if (digest) {
        compute_digest = ldigest_init();
        no_overlap = 1;
    }

    vpage = (void *) __get_free_page(GFP_NOIO);

#ifdef LIME_SUPPORTS_DEFLATE
    printk("[deflate] [compress] se vc ta me vendo isso, ta ativado.\n");
    if (compress) {
        deflate_page_buf = kmalloc(PAGE_SIZE, GFP_NOIO);
        err = deflate_begin_stream(deflate_page_buf, PAGE_SIZE);
        if (err < 0) {
            printk("[LiME] ZLIB begin stream failed\n");
            return err;
        }
        no_overlap = 1;
    }
#endif

    for (p = iomem_resource.child; p ; p = p->sibling) {

        if (!p->name || strcmp(p->name, LIME_RAMSTR))
            continue;

        if (mode == LIME_MODE_LIME && write_lime_header(p) < 0) {
            printk("[LiME] Error writing header 0x%lx - 0x%lx\n", (long) p->start, (long) p->end);
            break;
        } else if (mode == LIME_MODE_PADDED && write_padding((size_t) ((p->start - 1) - p_last)) < 0) {
            printk("[LiME] Error writing padding 0x%lx - 0x%lx\n", (long) p_last, (long) p->start - 1);
            break;
        }

        write_range(p);

        p_last = p->end;
    }

    write_flush();

    printk("[LiME] Memory Dump Complete...\n");

    cleanup();

    if (compute_digest == LIME_DIGEST_COMPUTE) {
        printk("[LiME] Writing Out Digest.\n");

        compute_digest = ldigest_final();

        if (compute_digest == LIME_DIGEST_COMPLETE) {
            err = ldigest_write_disk();

            printk("[LiME] Digest Write %s.\n", (err == 0) ? "Complete" : "Failed");
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
            printk("[LiME] Error sending zero page: %zd\n", r);
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

//#ifdef LIME_SUPPORTS_TIMING
//    ktime_t start,end;
//#endif

    printk("[LiME] Writing range %llx - %llx.\n", res->start, res->end);

    for (i = res->start; i <= res->end; i += is) {
//#ifdef LIME_SUPPORTS_TIMING
//        start = ktime_get_real();
//#endif
        p = pfn_to_page((i) >> PAGE_SHIFT);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        is = min((resource_size_t) PAGE_SIZE, (resource_size_t) (res->end - i + 1));
#else
        is = min((size_t) PAGE_SIZE, (size_t) (res->end - i + 1));
#endif

        if (is < PAGE_SIZE) {
            // We can't map partial pages and
            // the linux kernel doesn't use them anyway
            //DBG("Padding partial page: vaddr %p size: %lu", (void *) i, (unsigned long) is);
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
                //DBG("Failed to write page: vaddr %p. Skipping Range...", v);
                break;
            }
        }

//#ifdef LIME_SUPPORTS_TIMING
//        end = ktime_get_real();

//        if (timeout > 0 && ktime_to_ms(ktime_sub(end, start)) > timeout) {
//            DBG("Reading is too slow.  Skipping Range...");
//            write_padding(res->end - i + 1 - is);
//            break;
//        }
//#endif

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
        write_vaddr_disk(v, is)
    );

    /* if (ret < 0) {
        //DBG("Write error: %zd", ret);
    } else */
    if (ret != is) {
        //DBG("Short write %zu instead of %zu.", ret, is);
        ret = -1;
    }

    return ret;
}

static int setup(void) {
    return setup_disk(path, dio);
}

static void cleanup(void) {
    return cleanup_disk();
}

void cleanup_module(void) {
    
}

MODULE_LICENSE("GPL");
