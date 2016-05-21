# 
# @file     Makefile
# @brief    Linux simple charcter device driver for test
# 
# @note     https://github.com/zundoko/zndkcdev
# 
# @date     2016-05-16
# @author   zundoko
# 

PRJNAME = zndkcdev

LIBNAME = lib$(PRJNAME)
LIBSO   = $(LIBNAME).so

SRCS    = $(LIBNAME).c
OBJS    = $(SRCS:.c=.o)
DEPEND  = Makefile.depend

CC      = gcc
INC     = -I. -I../drv
CFLAGS  = -fPIC -Wall -Werror $(INC)
LDFLAGS = -shared
LIBS    = -l$(PRJNAME)

.PHONY: all
all: $(LIBSO)

$(LIBSO): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	-rm -rf $(OBJS) $(LIBSO) *~

.PHONY: depend
depend:
	-rm -rf $(DEPEND)
	$(CC) -MM -MG $(CFLAGS) $(SRCS) > $(DEPEND)

-include  $(DEPEND)

# end
