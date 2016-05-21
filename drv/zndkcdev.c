/**
 * @file     zndkcdev.c
 * @brief    Linux simple character device driver for test
 * 
 * @note     ** Linux Device Drivers, 3rd edition
 *           https://lwn.net/Kernel/LDD3/
 * 
 * @note     ** The Linux Kernel Module Programming Guide
 *           http://www.tldp.org/LDP/lkmpg/2.6/html/lkmpg.html
 * 
 * @note     https://github.com/zundoko/zndkcdev
 * 
 * @date     2016-05-14
 * @author   zundoko
 */

/* kernel include files:
 * on Ubuntu, you can find them at /lib/modules/<kernel version>/build/include/linux/.
 *
 * how to get your <kernel version>:
 * like this,
 * $ uname -r
 * 4.2.0-36-generic
 */
#include <linux/cdev.h>         /* cdev_add()                */
#include <linux/device.h>       /* device_create()           */
#include <linux/fs.h>           /* chrdev                    */
#include <linux/init.h>         /* macros: e.g., __init      */
#include <linux/kernel.h>       /* printk()                  */
#include <linux/mm.h>           /* remap_pfn_range()         */
#include <linux/module.h>       /* essential for all modules */
#include <linux/mutex.h>        /* mutex()                   */
#include <linux/sched.h>        /* send_sig_info()           */
#include <linux/slab.h>         /* kmalloc()/kfree()         */
#include <linux/types.h>        /* u32, pid_t                */

#include <asm/io.h>
#include <asm/uaccess.h>        /* copy_(to|from)_user()     */

#include "zndkcdev.h"           /* own header  */

MODULE_LICENSE    ("Dual BSD/GPL");
MODULE_DESCRIPTION("Linux simple character device driver for test");
MODULE_AUTHOR     ("zundoko");
MODULE_VERSION    (ZNDKCDEV_VERSION);

/**
 * @struct  TZndkCdevDCB
 * @brief   ZndkCdev Device Control Block (DCB)
 */
typedef struct {
    int            minor;            /* minor # of cdev         */

    dev_t          dev_num;          /* device numver           */
    struct cdev    c_dev;            /* character device        */
    struct device *dev;              /* device                  */

    char          *buf;              /* test buffer (kmalloc)   */
    int            len_buf;          /* test buffer size [B]    */

    struct mutex   mtx;              /* resource blocking       */

    int            init_done;        /* driver's been inited ?  */
} TZndkCdevDCB;
static  TZndkCdevDCB                 ZndkCdevDCB[N_ZNDKCDEV];
#define _get_zndkcdev_dcb(minor)   (&ZndkCdevDCB[minor     ])

/**
 * @struct  TZndkCdevInfo
 * @brief   driver management info
 */
typedef struct {
    char           ver[LEN_VER + 1]; /* versoin of this driver  */

    TZndkCdevDCB  *dcb;              /* device control block    */

    struct class  *cl;               /* device class            */
    dev_t          dev_num;          /* device number           */
    int            major;            /* major # of cdev         */
    int            n_dev;            /* # of devices to support */
} TZndkCdevInfo;

static TZndkCdevInfo              ZndkCdevInfo;
#define _get_zndkcdev_info()    (&ZndkCdevInfo)

/**
 * _init_zndkcdev_info()
 */
static int
_init_zndkcdev_info(TZndkCdevInfo *info)
{
    int     stat   =  0;

    memset(info, 0, sizeof(TZndkCdevInfo));
    snprintf(info->ver, LEN_VER, ZNDKCDEV_VERSION);

    info->dcb      = _get_zndkcdev_dcb(0); /* 0: top of DCB */

    info->cl       =  NULL;
    info->n_dev    =  N_ZNDKCDEV;

    return  stat;
}

/**
 * _cleanup_zndkcdev_info()
 * @info
 */
static int
_cleanup_zndkcdev_info(TZndkCdevInfo *info)
{
    int     stat   =  0;

    memset(info, 0, sizeof(TZndkCdevInfo));

    return  stat;
}

/**
 * _init_zndkcdev_dcb()
 */
static int
_init_zndkcdev_dcb(TZndkCdevDCB *dcb)
{
    int     stat   =  0;

    dcb->minor     =  0;
    dcb->dev_num   =  0;
    dcb->dev       =  NULL;

    dcb->buf       =  NULL;
    dcb->len_buf   =  LEN_ZNDKCDEV_BUF;

    mutex_init(&dcb->mtx);

    dcb->init_done = -1;

    return  stat;
}

/**
 * _cleanup_zndkcdev_dcb()
 */
static int
_cleanup_zndkcdev_dcb(TZndkCdevDCB *dcb)
{
    int     stat   =  0;

    mutex_destroy(&dcb->mtx);

    return  stat;
}

/**
 * zndkcdev_send_signal()
 */
static int
zndkcdev_send_signal(TZndkCdevDCB *dcb, TSigMsg *sigmsg)
{
    int                 stat   = 0;
    int                 signum;
    struct siginfo      sinfo;
    struct task_struct *task;

    task = get_pid_task(find_get_pid(sigmsg->pid), PIDTYPE_PID);

    memset(&sinfo, 0, sizeof(struct siginfo));
    signum         = SIGUSR1;
    sinfo.si_signo = signum;
    sinfo.si_code  = SI_QUEUE;
    sinfo.si_int   = sigmsg->dat;

    pr_info(" %s(): send SIGNAL: signum=%d, si_int=%d\n", __func__, signum, sigmsg->dat);
    send_sig_info(signum, &sinfo, task);

    return  stat;
}

/**
 * zndkcdev_open()
 */
static int
zndkcdev_open(struct inode *i, struct file *filp)
{
    TZndkCdevInfo  *info  = _get_zndkcdev_info();
    int             minor =  MINOR(i->i_rdev);
    TZndkCdevDCB   *dcb   = _get_zndkcdev_dcb(minor);

    pr_info(" %s[%2d]: %s(): major=%d, minor=%d\n",
            NAME_MODULE, minor, __func__, info->major, minor);

    /* save DCB as private data */
    filp->private_data = dcb;

    return  0;
}

/**
 * zndkcdev_close()
 */
static int
zndkcdev_close(struct inode *i, struct file *filp)
{
    TZndkCdevDCB  *dcb = (TZndkCdevDCB *)filp->private_data;

    pr_info(" %s[%2d]: %s()\n", NAME_MODULE, dcb->minor, __func__);

    return  0;
}

/**
 * zndkcdev_read()
 */
static ssize_t
zndkcdev_read(struct file *filp,       char __user *ubuf, size_t count, loff_t *fpos)
{
    int            stat  = 0;
    TZndkCdevDCB  *dcb   = (TZndkCdevDCB *)filp->private_data;
    size_t         len;
    size_t         remain;
    void          *buf;

    pr_info(" %s[%2d]: %s()\n", NAME_MODULE, dcb->minor, __func__);

//    if (mutex_lock_interruptible(&dcb->mtx)) {
//        return  -ERESTARTSYS;
//    }

    if (*fpos  >= dcb->len_buf) {
        stat    = 0;
        goto  read_unlock;
    }

    buf         =  dcb->buf;
    len         = (count < dcb->len_buf) ? count : dcb->len_buf;

    remain      =  copy_to_user(ubuf, buf, len);
    if (remain !=  0) {
        stat    =  0;
        goto  read_unlock;
    }

    stat        =  len;

read_unlock:
//    mutex_unlock(&dcb->mtx);

    return  stat;
}

/**
 * zndkcdev_write()
 */
static ssize_t
zndkcdev_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *fpos)
{
    int            stat  = 0;
    TZndkCdevDCB  *dcb   = (TZndkCdevDCB *)filp->private_data;
    size_t         len;
    size_t         remain;
    void          *buf;

    pr_info(" %s[%2d]: %s()\n", NAME_MODULE, dcb->minor, __func__);

//    if (mutex_lock_interruptible(&dcb->mtx)) {
//        return  -ERESTARTSYS;
//    }

    if (*fpos  >= dcb->len_buf) {
        stat    = 0;
        goto  read_unlock;
    }

    buf         =  dcb->buf;
    len         = (count < dcb->len_buf) ? count : dcb->len_buf;

    remain      =  copy_from_user(buf, ubuf, len);
    if (remain !=  0) {
        stat    =  0;
        goto  read_unlock;
    }

    stat        =  len;

read_unlock:
//    mutex_unlock(&dcb->mtx);

    return  stat;
}

/**
 * zndkcdev_mmap()
 */
static int
zndkcdev_mmap(struct file *filp, struct vm_area_struct *vma)
{
    int            stat;
    TZndkCdevDCB  *dcb     = (TZndkCdevDCB *)filp->private_data;
    unsigned long  len_req;

    len_req = vma->vm_end - vma->vm_start;

    pr_info(" %s[%2d]: %s(): len_buf=%08X, mmap size requested:%08lX\n",
            NAME_MODULE, dcb->minor, __func__, LEN_ZNDKCDEV_BUF, len_req);

    if (len_req > LEN_ZNDKCDEV_BUF) {
        pr_err(" %s():L%d: greed\n", __func__, __LINE__);
        return -EAGAIN;
    }

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    stat = remap_pfn_range(vma,
                           vma->vm_start,
                           virt_to_phys(dcb->buf) >> PAGE_SHIFT,
                           len_req,
                           vma->vm_page_prot);

    if (stat) {
        pr_err(" %s[%2d]: %s():L%d: remap_pfn_range() failed = %d\n",
               NAME_MODULE, dcb->minor, __func__, __LINE__, stat);
        return -EAGAIN;
    }

    return  0;
}

/**
 * zndkcdev_buf_rd()
 * @dcb
 * @mem
 */
static int
zndkcdev_buf_rd(TZndkCdevDCB *dcb, TZndkCdevMem *mem)
{
    int     stat = 0;
    int     ofs;
    int     len;
    int     remain;

    if (mem->ofs < dcb->len_buf) {
        /* correct params */
        ofs      =  mem->ofs;
        remain   =  dcb->len_buf - ofs - 1;
        len      = (mem->len < remain) ? mem->len : remain;

        if (copy_to_user((char *)mem->buf, (char *)(dcb->buf + ofs), len)) {
            return  -1;
        }
    } else {
        pr_err(" %s[%2d]: %s():L%d: out of range: ofs must be less than %d (your: %d))\n",
               NAME_MODULE, dcb->minor, __func__, __LINE__, dcb->len_buf, mem->ofs);
        return  -2;
    }

    return  stat;
}

/**
 * zndkcdev_buf_wr()
 * @dcb
 * @mem
 */
static int
zndkcdev_buf_wr(TZndkCdevDCB *dcb, TZndkCdevMem *mem)
{
    int     stat = 0;
    int     ofs;
    int     len;
    u32     remain;

    if (mem->ofs < dcb->len_buf) {
        /* correct params */
        ofs      =  mem->ofs;
        remain   =  dcb->len_buf - ofs - 1;
        len      = (mem->len < remain) ? mem->len : remain;

        if (copy_from_user((char *)(dcb->buf + ofs), (char *)mem->buf, len)) {
            return  -1;
        }
    } else {
        pr_err(" %s[%2d]: %s(): out of range: ofs must be less than %d (your: %d))\n",
               NAME_MODULE, dcb->minor, __func__, dcb->len_buf, mem->ofs);
        return  -2;
    }

    return  stat;
}

/**
 * zndkcdev_ioctl()
 */
static long
zndkcdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int            stat  =  0;
    TZndkCdevInfo *info  = _get_zndkcdev_info();
    TZndkCdevDCB  *dcb   = (TZndkCdevDCB *)filp->private_data;
    TZndkCdevMem   mem;
    TSigMsg        sigmsg;

    switch(cmd) {
    case ZNDKCDEV_GET_VERSION:
        pr_info(" %s[%2d]: %s: ioctl: ZNDKCDEV_GET_VERSION\n", NAME_MODULE, dcb->minor, __func__);
        pr_info("  -> %s version %s\n"              , NAME_MODULE, info->ver);
        if (copy_to_user((char *)arg, (char *)info->ver, sizeof(info->ver))) {
            return -EFAULT;
        }
        break;
    case ZNDKCDEV_BUF_RD     :
        pr_info(" %s[%2d]: %s: ioctl: ZNDKCDEV_BUF_RD\n"     , NAME_MODULE, dcb->minor, __func__);
        if (copy_from_user((void *)&mem, (const void __user *)arg, sizeof(TZndkCdevMem))) {
            return -EFAULT;
        }
        zndkcdev_buf_rd(dcb, &mem);
        break;
    case ZNDKCDEV_BUF_WR     :
        pr_info(" %s[%2d]: %s: ioctl: ZNDKCDEV_BUF_WR\n"     , NAME_MODULE, dcb->minor, __func__);
        if (copy_from_user((void *)&mem, (const void __user *)arg, sizeof(TZndkCdevMem))) {
            return -EFAULT;
        }
        zndkcdev_buf_wr(dcb, &mem);
        break;
    case ZNDKCDEV_PRINTK     :
        pr_info(" %s[%2d]: %s: ioctl: ZNDKCDEV_PRINTK\n"     , NAME_MODULE, dcb->minor, __func__);
        pr_info("  -> %s\n", (const char __user *)arg);
        break;
    case ZNDKCDEV_SIGNAL:
        pr_info(" %s[%2d]: %s: ioctl: ZNDKCDEV_SIGNAL\n"     , NAME_MODULE, dcb->minor, __func__);
        if (copy_from_user((void *)&sigmsg, (const void __user *)arg, sizeof(TSigMsg))) {
            return -EFAULT;
        }
        zndkcdev_send_signal(dcb, &sigmsg);
        break;
    case ZNDKCDEV_TEST       :
        pr_info(" %s[%2d]: %s: ioctl: ZNDKCDEV_TEST\n"       , NAME_MODULE, dcb->minor, __func__);
        break;
    default:
        pr_info(" %s[%2d]: %s: ioctl: unknown cmand\n"       , NAME_MODULE, dcb->minor, __func__);
        break;
    }

    return  stat;
}

/**
 * zndkcdev_fops
 */
static const struct file_operations zndkcdev_fops = {
    .open           = zndkcdev_open ,
    .release        = zndkcdev_close,
    .read           = zndkcdev_read ,
    .write          = zndkcdev_write,
    .mmap           = zndkcdev_mmap ,
    .unlocked_ioctl = zndkcdev_ioctl,
};

/**
 * zndkcdev_probe()
 * @info
 * @idx_minor
 */
static int
zndkcdev_probe(TZndkCdevInfo *info, int idx_minor)
{
    int             stat;
    TZndkCdevDCB   *dcb     = _get_zndkcdev_dcb(idx_minor);
    dev_t           dev_num;
    struct device  *dev;
    struct cdev    *c_dev;
    char           *buf;

    _init_zndkcdev_dcb(dcb);

    /* create a device file: /dev/zndkcdev_<n> */
    dev_num        = MKDEV(MAJOR(info->dev_num), idx_minor);
    dev            = device_create(info->cl, NULL, dev_num, NULL, NAME_MODULE "_%d", idx_minor);
    if (dev == NULL) {
        pr_err(" %s[%2d]: %s():L%d: could not create a device\n", NAME_MODULE, dcb->minor, __func__, __LINE__);
        return -1;
    }
    dcb->dev       = dev;
    dcb->dev_num   = dev_num;

    /* add a character device */
    c_dev           = &dcb->c_dev;
    cdev_init(c_dev, &zndkcdev_fops);
    stat = cdev_add(c_dev, dev_num, 1);
    if (stat < 0) {
        pr_err(" %s[%2d]: %s():L%d: could not add a chrdev\n", NAME_MODULE, dcb->minor, __func__, __LINE__);
        return -2;
    }

    /* prepare test buffer */
    buf = kzalloc(dcb->len_buf, GFP_KERNEL);
    if (buf == NULL) {
        pr_err(" %s[%2d]: %s():L%d: could not allocate memory buffer\n", NAME_MODULE, dcb->minor, __func__, __LINE__);
        return -3;
    }
    dcb->buf       = buf;
    dcb->minor     = idx_minor;

    dcb->init_done = 1;

    return  stat;
}

/**
 * zndkcdev_remove()
 * @info
 * @idx_minor
 */
static int
zndkcdev_remove(TZndkCdevInfo *info, int idx_minor)
{
    int             stat =  0;
    TZndkCdevDCB   *dcb  = _get_zndkcdev_dcb(idx_minor);

    if (dcb->buf       != NULL) {
        pr_debug(" %s[%2d]: %s(): kfree()\n"         , NAME_MODULE, dcb->minor, __func__);
        kfree(dcb->buf);
    }
    if (dcb->init_done == 1   ) {
        pr_debug(" %s[%2d]: %s(): cdev_del()\n"      , NAME_MODULE, dcb->minor, __func__);
        cdev_del(&dcb->c_dev);
    }
    if (dcb->dev       != NULL) {
        pr_debug(" %s[%2d]: %s(): device_destroy()\n", NAME_MODULE, dcb->minor, __func__);
        device_destroy(info->cl, dcb->dev_num);
    }

    _cleanup_zndkcdev_dcb(dcb);

    return  stat;
}

/**
 * _zndkcdev_cleanup()
 */
static int
_zndkcdev_cleanup(void)
{
    int             stat =  0;
    TZndkCdevInfo  *info = _get_zndkcdev_info();
    int             idx_minor;

    pr_info(" %s[--]: %s(): ----- start -----\n", NAME_MODULE, __func__);

    /* destroy device files */
    for (idx_minor = 0; idx_minor < info->n_dev; idx_minor++) {
        zndkcdev_remove(info, idx_minor);
    }
    
    if (info->cl        != NULL) {
        pr_debug(" %s[--]: %s(): class_destroy()\n", NAME_MODULE, __func__);
        class_destroy(info->cl);
    }
    if (info->dev_num   != 0   ) {
        pr_debug(" %s[--]: %s(): unregister_chrdev_region()\n", NAME_MODULE, __func__);
        unregister_chrdev_region(info->dev_num, info->n_dev);
    }

    _cleanup_zndkcdev_info(info);

    pr_info(" %s(): ----- done  -----\n", __func__);

    return  stat;
}

/**
 * zndkcdev_init()
 */
static int __init
zndkcdev_init(void)
{
    int             stat    =  0;
    TZndkCdevInfo  *info    = _get_zndkcdev_info();
    dev_t           dev_num;
    struct class   *cl;
    int             idx_minor;

    pr_info(" %s[--]: %s(): ##### INIT  #####\n", NAME_MODULE, __func__);

    _init_zndkcdev_info(info);
    pr_info(" %s[--]: %s(): %s\n", NAME_MODULE, __func__, info->ver);

    /* register a character deivce */
    stat = alloc_chrdev_region(&dev_num, 0, 1, NAME_MODULE);
    if (stat < 0) {
        pr_err(" %s[--]: %s():L%d: could not register a chrdev\n", NAME_MODULE, __func__, __LINE__);
        goto  err_init;
    }
    info->dev_num   =       dev_num;
    info->major     = MAJOR(dev_num);

    /* create a class: /sys/class */
    cl   = class_create(THIS_MODULE, NAME_MODULE);
    if (cl == NULL) {
        pr_err(" %s[--]: %s():L%d: could not create a class\n", NAME_MODULE, __func__, __LINE__);
        goto  err_init;
    }
    info->cl        = cl;

    /* create device files */
    for (idx_minor = 0; idx_minor < info->n_dev; idx_minor++) {
        stat = zndkcdev_probe(info, idx_minor);
        if (stat < 0) {
            pr_err( " %s[--]: %s():L%d: could not create a device file(minor=%d)\n",
                    NAME_MODULE, __func__, __LINE__, idx_minor);
            goto  err_init;
        }
    }

    return  0;

 err_init:
    pr_err(" %s(); error\n", __func__);
    _zndkcdev_cleanup();

    return  0;
}

/**
 * zndkcdev_exit()
 */
static void __exit
zndkcdev_exit(void)
{
    _zndkcdev_cleanup();

    pr_info(" %s[--]: %s(): ##### EXIT  #####\n", NAME_MODULE, __func__);

    return;
}

module_init(zndkcdev_init);
module_exit(zndkcdev_exit);

/* end */
