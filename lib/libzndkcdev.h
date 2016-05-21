/**
 * @file     libzndkcdev.h
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

#ifndef    LIBZNDKCDEV_H
#define    LIBZNDKCDEV_H

/* definitions */

/**
 * @struct TDevHandle
 * @brief  ZndkCdev handle info
 */
typedef struct {
    int      fd;                /* file descriptor                             */

    uint8_t *buf_virt;          /* driver managed buffer (virt)                */
    int      len_buf;           /* lenght of driver managed buffer (unit: [B]) */
} TDevHandle;

typedef int (* TSigCallback)(int signum, int dat);

/* extern declarations */
extern  int            zndkcdev_open       (const char *filepaht);
extern  int            zndkcdev_close      (int fd);
extern uint8_t *       zndkcdev_mmap       (int fd);
extern  int            zndkcdev_munmap     (int fd);
extern  int            zndkcdev_get_version(int fd, char *ver);
extern  int            zndkcdev_buf_read   (int fd, int   ofs, int len, const void *rbuf);
extern  int            zndkcdev_buf_write  (int fd, int   ofs, int len,       void *wbuf);
extern  int            zndkcdev_print      (int fd, const char *msg);
extern  int            zndkcdev_send_signal(int fd, TSigCallback sigcb, pid_t pid, int dat);
extern  int            zndkcdev_test       (int fd);

#endif  /* LIBZNDKCDEV_H */
/* end */
