# zndkcdev
Linux simple character device driver for test

This Linux driver has been written to run on x86/x64 environment.

My development environment are as follows:
   CPU: Core i7 x64
   OS: Ubuntu 15.10 on VirtualBox on Window 7
   uname -a: Linux vbox 4.2.0-36-generic #42-Ubuntu SMP Thu May 12 22:05:35 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

## About this driver

This driver consist of:
 1. (kernel space) drv/zndkcdev.ko   : a character device driver (/dev/zndkcdev_[01])
 2. (user   space) lib/libzndkcdev.so: a library which controls zndkcdev.ko
 3. (user   space) test/testapp      : a test application for zndkcdev.ko

This driver will test that:
- open/close character deivce file.
- read/write kernel buffer from user space w/ read/write syscalls.
- read/write kernel buffer from user space w/ mmap.
- send SIGNAL from kernel to user space.

## Compile/Installation/Run/Uninstallation

```bash
make
make install
cd ./test
./testzndkcdev
cd ..
make uninstall
```


## Whole operation log

```bash
~/work/learning/linux/chrdev/zndkcdev$ make clean
rm -rf *~
---------- drv
make[1]: Entering directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/drv'
make -C /lib/modules/4.2.0-36-generic/build/ M=/home/<foo>/work/learning/linux/chrdev/zndkcdev/drv clean
make[2]: Entering directory '/usr/src/linux-headers-4.2.0-36-generic'
make[2]: Leaving directory '/usr/src/linux-headers-4.2.0-36-generic'
rm -rf *~ *.o *.mod* *.order *.symvers
make[1]: Leaving directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/drv'
/home/<foo>/work/learning/linux/chrdev/zndkcdev
---------- lib
make[1]: Entering directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/lib'
rm -rf libzndkcdev.o libzndkcdev.so *~
make[1]: Leaving directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/lib'
/home/<foo>/work/learning/linux/chrdev/zndkcdev
---------- test
make[1]: Entering directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/test'
rm -rf testzndkcdev.o testzndkcdev *~
make[1]: Leaving directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/test'
/home/<foo>/work/learning/linux/chrdev/zndkcdev

~/work/learning/linux/chrdev/zndkcdev$ make depend
---------- drv
make[1]: Entering directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/drv'
make[1]: *** No rule to make target 'depend'.  Stop.
make[1]: Leaving directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/drv'
/home/<foo>/work/learning/linux/chrdev/zndkcdev
---------- lib
make[1]: Entering directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/lib'
rm -rf Makefile.depend
gcc -MM -MG -fPIC -Wall -Werror -I. -I../drv libzndkcdev.c > Makefile.depend
make[1]: Leaving directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/lib'
/home/<foo>/work/learning/linux/chrdev/zndkcdev
---------- test
make[1]: Entering directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/test'
rm -rf Makefile.depend
gcc -MM -MG -c -Wall -Werror -I. -I../lib testzndkcdev.c > Makefile.depend
make[1]: Leaving directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/test'
/home/<foo>/work/learning/linux/chrdev/zndkcdev

~/work/learning/linux/chrdev/zndkcdev$ make
---------- drv
make[1]: Entering directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/drv'
make -C /lib/modules/4.2.0-36-generic/build/ M=/home/<foo>/work/learning/linux/chrdev/zndkcdev/drv modules
make[2]: Entering directory '/usr/src/linux-headers-4.2.0-36-generic'
  CC [M]  /home/<foo>/work/learning/linux/chrdev/zndkcdev/drv/zndkcdev.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/<foo>/work/learning/linux/chrdev/zndkcdev/drv/zndkcdev.mod.o
  LD [M]  /home/<foo>/work/learning/linux/chrdev/zndkcdev/drv/zndkcdev.ko
make[2]: Leaving directory '/usr/src/linux-headers-4.2.0-36-generic'
make[1]: Leaving directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/drv'
/home/<foo>/work/learning/linux/chrdev/zndkcdev
---------- lib
make[1]: Entering directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/lib'
gcc -fPIC -Wall -Werror -I. -I../drv -c libzndkcdev.c
gcc -shared -o libzndkcdev.so libzndkcdev.o
make[1]: Leaving directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/lib'
/home/<foo>/work/learning/linux/chrdev/zndkcdev
---------- test
make[1]: Entering directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/test'
gcc -c -Wall -Werror -I. -I../lib testzndkcdev.c
gcc -L. -L../lib -o testzndkcdev testzndkcdev.o -lzndkcdev
make[1]: Leaving directory '/home/<foo>/work/learning/linux/chrdev/zndkcdev/test'
/home/<foo>/work/learning/linux/chrdev/zndkcdev

~/work/learning/linux/chrdev/zndkcdev$ make install
sudo insmod drv/zndkcdev.ko
[sudo] password for <foo>: 
sudo cp lib/libzndkcdev.so /usr/lib/
sudo chown <foo> /usr/lib/libzndkcdev.so
sudo chown <foo> /dev/zndkcdev*
########## lsmod    ##########
zndkcdev               16384  0
########## /dev     ##########
crw-------  1 <foo>    root    242,   0 May 21 18:56 zndkcdev_0
crw-------  1 <foo>    root    242,   1 May 21 18:56 zndkcdev_1

~/work/learning/linux/chrdev/zndkcdev/test$ cd ./test/

~/work/learning/linux/chrdev/zndkcdev/test$ ./testzndkcdev 
 zndkcdev_open(): open
 zndkcdev_get_version(): ioctl: get vresion
  -> zndkcdev driver version: 0.0.1
  -> write (clear): 
  -> read  (check): 
  -> write (again): zndkcdev R/W test
  -> read  (check): zndkcdev R/W test
 zndkcdev_mmap(): mmap
  -> write (mmap ): ABCDEFGHIJKLMNOPQRSTUVWXYZ
  -> read  (check): ABCDEFGHIJKLMNOPQRSTUVWXYZ
 zndkcdev_buf_write(): ioctl: buf write
  -> write (mmap ): abcdefghijklmnopqrstuvwxyz
 zndkcdev_buf_read(): ioctl: read buffer
  -> read  (check): abcdefghijklmnopqrstuvwxyz
 zndkcdev_send_signal(): ioctl: signal
 _zndkcdev_sigaction(): got the SIGNAL, signum=10, si_int=12345
  -> _test_zndkcdev_callback(): signum=10, si_int=12345
 zndkcdev_close(): close

~/work/learning/linux/chrdev/zndkcdev/test$ cd ..

~/work/learning/linux/chrdev/zndkcdev$ make uninstall
sudo  rm    /usr/lib/libzndkcdev.so
sudo  rmmod drv/zndkcdev.ko

~/work/learning/linux/chrdev/zndkcdev$ sudo dmesg -c
[25463.515719]  zndkcdev[--]: zndkcdev_init(): ##### INIT  #####
[25463.515725]  zndkcdev[--]: zndkcdev_init(): 0.0.1
[25468.605999]  zndkcdev[ 0]: zndkcdev_open(): major=242, minor=0
[25468.606045]  zndkcdev[ 0]: zndkcdev_ioctl: ioctl: ZNDKCDEV_GET_VERSION
[25468.606047]   -> zndkcdev version 0.0.1
[25468.606062]  zndkcdev[ 0]: zndkcdev_write()
[25468.606119]  zndkcdev[ 0]: zndkcdev_read()
[25468.606130]  zndkcdev[ 0]: zndkcdev_write()
[25468.606142]  zndkcdev[ 0]: zndkcdev_read()
[25468.606169]  zndkcdev[ 0]: zndkcdev_mmap(): len_buf=00100000, mmap size requested:00100000
[25468.606181] x86/PAT: testzndkcdev:13528 map pfn RAM range req uncached-minus for [mem 0x39e00000-0x39efffff], got write-back
[25468.606298]  zndkcdev[ 0]: zndkcdev_read()
[25468.606321]  zndkcdev[ 0]: zndkcdev_ioctl: ioctl: ZNDKCDEV_BUF_WR
[25468.606350]  zndkcdev[ 0]: zndkcdev_ioctl: ioctl: ZNDKCDEV_BUF_RD
[25468.606482]  zndkcdev[ 0]: zndkcdev_ioctl: ioctl: ZNDKCDEV_SIGNAL
[25468.606485]  zndkcdev_send_signal(): send SIGNAL: signum=10, si_int=12345
[25469.606865]  zndkcdev[ 0]: zndkcdev_close()
[25474.715832]  zndkcdev[--]: _zndkcdev_cleanup(): ----- start -----
[25474.716245]  _zndkcdev_cleanup(): ----- done  -----
[25474.716248]  zndkcdev[--]: zndkcdev_exit(): ##### EXIT  #####

~/work/learning/linux/chrdev/zndkcdev$ 
```
