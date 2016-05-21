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

TARGET  = test$(PRJNAME)
LIBNAME = lib$(PRJNAME)

SRCS    = $(TARGET).c
OBJS    = $(SRCS:.c=.o)
DEPEND  = Makefile.depend

CC      = gcc
INC     = -I. -I../lib
CFLAGS  = -c -Wall -Werror $(INC)
LDFLAGS = -L. -L../lib
LIBS    = -l$(PRJNAME)

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $<

.PHONY: clean
clean:
	-rm -rf $(OBJS) $(TARGET) *~

.PHONY: depend
depend:
	-rm -rf $(DEPEND)
	$(CC) -MM -MG $(CFLAGS) $(SRCS) > $(DEPEND)

-include  $(DEPEND)

# end
