/**
 * @file     libzndkcdev.c
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

#include <stdio.h>              /* printf()    */
#include <stdint.h>             /* uint32_t    */
#include <string.h>             /* memset()    */
#include <fcntl.h>              /* open()      */
#include <unistd.h>             /* close()     */
#include <signal.h>             /* SIGNAL      */
#include <sys/mman.h>           /* mmap()      */
#include <sys/ioctl.h>          /* ioctl()     */
#include <sys/types.h>          /* pid_t       */

#include    "zndkcdev.h"        /* zndk driver */
#include "libzndkcdev.h"        /* zndk lib    */

/**
 * @struct TLibZndkCdevInfo
 * @brief  libzndkcdev info
 */
typedef struct {
    TDevHandle   *hdl;          /* zndkcdevdriver handle info */

    TSigCallback  sigcb;        /* callback() for SIGNAL      */
} TLibZndkCdevInfo;
static TLibZndkCdevInfo              LibZndkCdevInfo;
#define _get_libzndkcdev_info()    (&LibZndkCdevInfo)

static TDevHandle                    DevHandle;
#define _get_zndkcdev_hdl()        (&DevHandle)

/**
 * _init_libzndkcdev_info()
 * @brief    LibZndkCdevInfo intialisation
 *
 * @param    - none -
 * @return   - none -
 */
static int
_init_libzndkcdev_info(TLibZndkCdevInfo *info)
{
    int         stat = 0;
    TDevHandle *hdl;

    /* init handle info */
    hdl            = _get_zndkcdev_hdl();
    memset(hdl, 0, sizeof(TDevHandle));
    hdl->fd        = -1;
    hdl->buf_virt  =  NULL;
    hdl->len_buf   =  LEN_ZNDKCDEV_BUF;

    /* int lib info */
    memset(info, 0, sizeof(TLibZndkCdevInfo));
    info->hdl      =  hdl;
    info->sigcb    = (void *)NULL;

    return  stat;
}

/**
 * _zndkcdev_sigaction()
 * @brief    sa_sigaction routine
 *
 * @param    [in]   signum      int ::= SIGNAL #
 * @param    [in]  *sinfo siginfo_t ::= siginfo_t
 * @param    [in]  *xtra       void ::= user context
 * @return   - none -
 */
static void
_zndkcdev_sigaction(int signum, siginfo_t *sinfo, void *xtra)
{
    TLibZndkCdevInfo *info  = _get_libzndkcdev_info();
    TSigCallback      sigcb =  info->sigcb;

    printf(" %s(): got the SIGNAL, signum=%d, si_int=%d\n", __func__, signum, sinfo->si_int);

    if (sigcb != NULL) {
        sigcb(signum, sinfo->si_int);
    } else {
        printf(" %s(): no sigcb() found\n", __func__);
    }
}

/**
 * zndkcdev_open()
 * @brief    open the zndkcdev driver
 *
 * @param    [in]  *filepath       char ::= /dev/<filename>
 * @return          stat            int ::= process status
 */
int
zndkcdev_open(const char *filepath)
{
    int               fd;
    TLibZndkCdevInfo *info = _get_libzndkcdev_info();
    TDevHandle       *hdl;

    printf(" %s(): open\n", __func__);

    /* open the cdev */
    fd       = open(filepath, O_RDWR);
    if (fd   < 0) {
        printf(" %s(): open error, file = %s (%d)\n", __func__, filepath, fd);
        return  fd;
    }

    _init_libzndkcdev_info(info);
    hdl      = info->hdl;
    hdl->fd  = fd;

    return  fd;
}

/**
 * zndkcdev_close()
 * @brief    close the zndkcdev driver
 *
 * @param    [in]   fd              int ::= file descriptor
 * @return          stat            int ::= process status
 */
int
zndkcdev_close(int fd)
{
    int               stat = 0;
    TLibZndkCdevInfo *info = _get_libzndkcdev_info();
    TDevHandle       *hdl  = info->hdl;

    printf(" %s(): close\n", __func__);

    /* un-map */
    if (hdl->buf_virt != NULL) {
        stat = munmap(hdl->buf_virt, hdl->len_buf);
        if (stat < 0) {
            printf(" %s(): munmap error (%d)\n", __func__, stat);
        }
    }

    /* close the cdev */
    close(fd);

    return  stat;
}

/**
 * zndkcdev_mmap()
 * @brief    create a new mapping in the virtual address (user space)
 */
uint8_t *
zndkcdev_mmap(int fd)
{
    TLibZndkCdevInfo *info = _get_libzndkcdev_info();
    TDevHandle       *hdl  =  info->hdl;
    uint8_t          *map  =  NULL;

    printf(" %s(): mmap\n", __func__);

    /* mmap the file to get access to driver memory buffer */
    if (hdl->buf_virt == NULL) {
      map      = mmap(NULL, hdl->len_buf, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
      if (map == MAP_FAILED) {
          printf(" %s(): mapping error\n", __func__);
          return  NULL;
      }
      hdl->buf_virt = map;
    }

    return  map;
}

/**
 * zndkcdev_munmap()
 * @brief    create a new mapping in the virtual address (user space)
 */
int
zndkcdev_munmap(int fd)
{
    int               stat = 0;
    TLibZndkCdevInfo *info = _get_libzndkcdev_info();
    TDevHandle       *hdl  =  info->hdl;

    printf(" %s(): mmap\n", __func__);

    /* un-map */
    if (hdl->buf_virt != NULL) {
        stat = munmap(hdl->buf_virt, hdl->len_buf);
        if (stat < 0) {
            printf(" %s(): munmap error (%d)\n", __func__, stat);
            stat = -1;
        } else {
            hdl->buf_virt = NULL;
        }
    }

    return  stat;
}

/**
 * zndkcdev_get_version()
 * @brief    get the version of the zndkcdev driver via ioctl
 *
 * @param    [in]   fd              int ::= file descriptor
 * @param    [out] *ver            char ::= device driver version string
 * @return          stat            int ::= process status
 */
int
zndkcdev_get_version(int fd, char *ver)
{
    int     stat = 0;

    printf(" %s(): ioctl: get vresion\n", __func__);

    stat = ioctl(fd, ZNDKCDEV_GET_VERSION, ver);
    if (stat < 0) {
        printf(" %s(): error: ioctl (%d)\n", __func__, stat);
    }

    return  stat;
}

/**
 * zndkcdev_buf_read()
 * @brief    read data from buffer of the zndkcdev driver via ioctl
 *
 * @param    [in]   fd              int ::= file descriptor
 * @param    [in]   ofs             int ::= offset address from top of driver buffer
 * @param    [in]   len             int ::= length to be read
 * @param    [out] *rbuf           void ::= read buffer
 * @return          stat            int ::= process status
 */
int
zndkcdev_buf_read(int fd, int ofs, int len, const void *rbuf)
{
    int           stat = 0;
    TZndkCdevMem  mem = { (void *)rbuf, ofs, len };

    printf(" %s(): ioctl: read buffer\n", __func__);

    stat = ioctl(fd, ZNDKCDEV_BUF_RD, &mem);
    if (stat < 0) {
        printf(" %s(): error: ioctl (%d)\n", __func__, stat);
    }

    return  stat;
}

/**
 * zndkcdev_buf_write()
 * @brief    write data to buffer of the zndkcdev driver via ioctl
 *
 * @param    [in]   fd              int ::= file descriptor
 * @param    [in]   ofs             int ::= offset address from top of driver buffer
 * @param    [in]   len             int ::= length to be written
 * @param    [in]  *wbuf           void ::= write buffer
 * @return          stat            int ::= process status
 */
int
zndkcdev_buf_write(int fd, int ofs, int len, void *wbuf)
{
    int           stat = 0;
    TZndkCdevMem  mem = { (void *)wbuf, ofs, len };

    printf(" %s(): ioctl: buf write\n", __func__);

    stat = ioctl(fd, ZNDKCDEV_BUF_WR, &mem);
    if (stat < 0) {
        printf(" %s(): error: ioctl (%d)\n", __func__, stat);
    }

    return  stat;
}

/**
 * zndkcdev_print()
 * @brief    print a message by the zndkcdev driver via ioctl (dmesg)
 *
 * @param    - none -
 * @param    [in]   fd              int ::= file descriptor
 * @return          stat            int ::= process status
 */
int
zndkcdev_print(int fd, const char *msg)
{
    int     stat = 0;

    printf(" %s(): ioctl: printk() \n", __func__);

    stat = ioctl(fd, ZNDKCDEV_PRINTK, msg);
    if (stat < 0) {
        printf(" %s(): error: ioctl (%d)\n", __func__, stat);
    }

    return  stat;
}

/**
 * zndkcdev_send_signal()
 * @brief    send a SIGNAL from the zndkcdev driver via ioctl
 *
 * @param    [in]   fd              int ::= file descriptor
 * @param    - none -
 * @return          stat            int ::= process status
 */
int
zndkcdev_send_signal(int fd, TSigCallback sigcb, pid_t pid, int dat)
{
    int               stat   =   0;
    TLibZndkCdevInfo *info   =  _get_libzndkcdev_info();
    struct sigaction  sa;
    TSigMsg           sigmsg = { pid, dat };

    printf(" %s(): ioctl: signal\n", __func__);

    /* prepare for the sigaction */
    info->sigcb     = sigcb;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_sigaction = _zndkcdev_sigaction;
    sa.sa_flags     =  SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);

    /* request the driver to send a SIGNAL */
    stat = ioctl(fd, ZNDKCDEV_SIGNAL, &sigmsg);
    if (stat < 0) {
        printf(" %s(): error: ioctl (%d)\n", __func__, stat);
    }

    return  stat;
}

/**
 * zndkcdev_test()
 * @brief    test the zndkcdev driver via ioctl
 *
 * @param    [in]   fd              int ::= file descriptor
 * @param    - none -
 * @return          stat            int ::= process status
 */
int
zndkcdev_test(int fd)
{
    int     stat = 0;

    printf(" %s(): ioctl: test\n", __func__);

    stat = ioctl(fd, ZNDKCDEV_SIGNAL, NULL);
    if (stat < 0) {
        printf(" %s(): error: ioctl (%d)\n", __func__, stat);
    }

    return  stat;
}


/* end */
