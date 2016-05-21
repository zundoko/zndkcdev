/**
 * @file     zndkcdev.h
 * @brief    Linux simple character device driver for test
 *
 * @note     https://github.com/zundoko/zndkcdev
 * 
 * @note     ** The Linux Kernel Module Programming Guide
 *           http://www.tldp.org/LDP/lkmpg/2.6/html/lkmpg.html
 * 
 * @date     2016-05-14
 * @author   zundoko
 */

#ifndef    ZNDKCDEV_H
#define    ZNDKCDEV_H

/* definitions */
#define  NAME_MODULE                   "zndkcdev"

#define  ZNDKCDEV_VERSION              "0.0.1" /* <major>.<minor>.<revision>   */
#define  LEN_VER                       20      /* length of version string [B] */

#define  N_ZNDKCDEV                     2

#define  LEN_ZNDKCDEV_BUF          (1024 * 1024 * 1) /* unit: [B] */

/**
 * @struct  TZndkCdevMem
 * @brief   ZndkCdev Memory Buffer structure
 */
typedef struct {
    void    *buf;               /* R/W buffer                         */
    int      ofs;               /* offset address of R/W buffer above */
    int      len;               /* R/W buffer lenght (unit: [B])      */
} TZndkCdevMem;

/**
 * @struct  TSigMsg
 * @brief   signal info
 */
typedef struct {
    pid_t    pid;               /* PID of user space programme  */
    int      dat;               /* message data from user space */
} TSigMsg;

/* IOCTL commands */
#define  ZNDKCDEV_IOCTL_BASE           'Z'
#define  ZNDKCDEV_GET_VERSION       _IO(ZNDKCDEV_IOCTL_BASE,  0) /* IOCTL: get a driver version */
#define  ZNDKCDEV_BUF_RD            _IO(ZNDKCDEV_IOCTL_BASE,  1) /* IOCTL: copy_to_user()       */
#define  ZNDKCDEV_BUF_WR            _IO(ZNDKCDEV_IOCTL_BASE,  2) /* IOCTL: copy_from_user()     */
#define  ZNDKCDEV_PRINTK            _IO(ZNDKCDEV_IOCTL_BASE,  3) /* IOCTL: printk() test        */
#define  ZNDKCDEV_SIGNAL            _IO(ZNDKCDEV_IOCTL_BASE,  4) /* IOCTL: send signal to user  */
#define  ZNDKCDEV_TEST              _IO(ZNDKCDEV_IOCTL_BASE, 99) /* IOCTL: ioctl    test        */

#endif  /* ZNDKCDEV_H */

/* end */
