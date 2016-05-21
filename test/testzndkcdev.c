/**
 * @file     testzndkcdev.c
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
 * @date     2016-05-16
 * @author   zundoko
 */

#include <stdio.h>              /* printf()    */
#include <stdint.h>             /* uint32_t    */
#include <unistd.h>             /* getpid()    */
#include <sys/types.h>          /* pid_t       */

#include "libzndkcdev.h"        /* zndk lib    */

/**
 * _test_zndkcdev_callback()
 * @brief    callback function for sending SIGNAL from the ZndkCdev driver
 *
 * @param    [in] signum        int ::= SIGNAL #
 * @oaram    [in] dat           int ::= si_int
 * @return        stat          int ::= process status
 */
static int
_test_zndkcdev_callback(int signum, int dat)
{
    int     stat = 0;

    printf("  -> %s(): signum=%d, si_int=%d\n", __func__, signum, dat);

    return  stat;
}

/**
 * main()
 * @brief    zndkcdev device driver test application
 *
 * @param    [in]   argc        int ::= # of args
 * @param    [in]  *argv[]     char ::= entity of args
 */
int
main(void)
{
    int     stat      =   0;
    int     fd;

    /* open */
    fd = zndkcdev_open("/dev/zndkcdev_0");
    if (fd < 0) {
        printf(" %s(): open error\n", __func__);
        return  1;
    }

    /* version */
    {
        char    ver [256] = { 0 };

        stat = zndkcdev_get_version(fd, ver);
        if (stat == 0) {
            printf("  -> zndkcdev driver version: %s\n", ver);
        }
    }

    /* R/W w/ syscals */
    {
        char    rbuf[256] = { 0 };
        char    wbuf[256] = { 0 };
        int     len;

        write(fd, "", 1); 
        printf("  -> write (clear): %s\n", wbuf);

        read (fd, rbuf, 10);
        printf("  -> read  (check): %s\n", rbuf);

        snprintf(wbuf, 20, "%s", "zndkcdev R/W test");
        len = sizeof(wbuf);
        write(fd, wbuf, len); 
        printf("  -> write (again): %s\n", wbuf);

        read (fd, rbuf, 20);
        printf("  -> read  (check): %s\n", rbuf);
    }

    /* R/W w/ mmap */
    {
        char *buf;
        int   idx;
        char  rbuf[256] = { 0 };
        char  wbuf[256] = { 0 };
        int   len;

        buf = (char *)zndkcdev_mmap(fd);

        printf("  -> write (mmap ): ");
        for (idx = 0; idx < 26; idx++) {
            buf[idx] = 'A' + idx;
            printf("%c", buf[idx]);
        }
        printf("\n");

        read (fd, rbuf, 26);
        printf("  -> read  (check): %s\n", rbuf);

        snprintf(wbuf, 27, "%s", "abcdefghijklmnopqrstuvwxyz");
        len = sizeof(wbuf);
        zndkcdev_buf_write(fd, 0, len, wbuf); 

        printf("  -> write (mmap ): ");
        for (idx = 0; idx < len; idx++) {
            printf("%c", buf[idx]);
        }
        printf("\n");

        zndkcdev_buf_read(fd, 0, len, rbuf);
        printf("  -> read  (check): %s\n", rbuf);
    }

    /* send signal from kernel */
    {
        stat = zndkcdev_send_signal(fd, _test_zndkcdev_callback, getpid(), 12345);
    }

    /* close */
    sleep(1);                   /* wait for log message out */
    zndkcdev_close(fd);

    return  0;
}

/* end */
