# 
# @file     Makefile
# @brief    Linux simple character device driver for test
# 
# @note     https://github.com/zundoko/zndkcdev
# 
# @date     2016-05-14
# @author   zundoko
# 

TARGET  = zndkcdev

DRVKO   = $(TARGET).ko
LIBSO   = lib$(TARGET).so

DIRDRV  = drv
DIRLIB  = lib
DIRTST  = test

DRVCDEV = $(DIRDRV)/$(DRVKO)
LIBCDEV = $(DIRLIB)/$(LIBSO)

SUBS    = $(DIRDRV) $(DIRLIB) $(DIRTST)



.PHONY: all
all:
	@for sub in $(SUBS); do echo "---------- $$sub"; cd $$sub; make;       cd -; done

.PHONY: clean
clean:
	-rm -rf *~
	@for sub in $(SUBS); do echo "---------- $$sub"; cd $$sub; make clean; cd -; done

.PHONY: depend
depend:
	@for sub in $(SUBS); do echo "---------- $$sub"; cd $$sub; make depend; cd -; done

.PHONY: install
install:
#	@echo  $(LIBCDEV)
#	@echo  $(DRVCDEV)
	sudo insmod $(DRVCDEV)
	sudo cp $(LIBCDEV) /usr/lib/
	sudo chown $(USER) /usr/lib/$(LIBSO)
	sudo chown $(USER) /dev/$(TARGET)*
	@echo "########## lsmod    ##########"
	@lsmod      | grep --color $(TARGET)
	@echo "########## /dev     ##########"
	@ls -l /dev | grep --color $(TARGET)

.PHONY: uninstall
uninstall:
#	@echo  $(LIBCDEV)
#	@echo  $(DRVCDEV)
	sudo  rm    /usr/lib/$(LIBSO)
	sudo  rmmod $(DRVCDEV)

.PHONY: check
check:							# use "check" after installation
	@echo "########## lsmod    ##########"
	-@lsmod      | grep --color $(TARGET)
	@echo "########## /dev     ##########"
	-@ls -l /dev | grep --color $(TARGET)

# end
