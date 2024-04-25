BINDIR = ../_bin/
LIBDIR = ../_lib/
OBJDIR = ../_obj/

# set COS_BASE if tools are not in PATH
#COS_BASE =

CAL = $(COS_BASE)cal
LIB = $(COS_BASE)lib
LDR = $(COS_BASE)ldr

TARGET = $(BINDIR)helloa.bin

LIBCOS = $(LIBDIR)libcos.lib
LIBCOS_SRC = ../libcos

OBJS = \
  $(OBJDIR)helloa.obj

all: $(LIBCOS) $(TARGET)

build: clean all

$(TARGET): $(OBJS)
	-@mkdir -p $(BINDIR)
	$(LDR) -m - -o $@ $< $(LIBCOS)

$(OBJDIR)%.obj: %.cal
	-@mkdir -p $(OBJDIR)
	$(CAL) -t $(LIBCOS_SRC)/systxt $< -o $@ -l $(@:.obj=.lst)

.PHONY: $(LIBCOS)
$(LIBCOS):
	$(MAKE) -C $(LIBCOS_SRC) $(MAKECMDGOALS)

clean:
	rm -f $(OBJDIR)*.obj $(OBJDIR)*.lst
	rm -f $(TARGET)

