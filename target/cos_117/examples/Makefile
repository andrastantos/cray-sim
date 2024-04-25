BINDIR=_bin

all: $(BINDIR)/helloa $(BINDIR)/helloc
clean: all
build: all

$(BINDIR)/%: %
	$(MAKE) -C $< $(MAKECMDGOALS)
