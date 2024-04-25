BINDIR = ../_bin/
LIBDIR = ../_lib/
OBJDIR = _obj/

CC = $(COS_BASE)ack

#ACK=$(shell which $(CC))
#var PLATFORMDIR={EM}/share/ack/cos

# set COS_BASE if tools are not in PATH
#COS_BASE =

TARGET = $(BINDIR)helloc.bin

SRCS = \
  helloc.c

all: $(TARGET)

build: clean all

$(TARGET): $(SRCS)
	-@mkdir -p $(BINDIR)
	$(CC) -o $@ $<

clean:
	rm -f $(TARGET)

