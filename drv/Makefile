# 
# @file     Makefile
# @brief    Linux simple charcter device driver for test
# 
# @note     https://github.com/zundoko/zndkcdev
# 
# @date     2016-05-14
# @author   zundoko
# 

PRJNAME = zndkcdev

TARGET  = $(PRJNAME)

KDIR    = /lib/modules/$(shell uname -r)/build/

obj-m  += $(TARGET).o

.PHONY: all
all:
	make -C $(KDIR) M=$(PWD) modules

.PHONY: clean
clean:
	make -C $(KDIR) M=$(PWD) clean
	-rm -rf *~ *.o *.mod* *.order *.symvers

# end
