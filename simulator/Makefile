export NO_DEP_LIBS = true

include common.mak

SOURCES = 
SOURCES += sim_lib
SOURCES += httpd
ifeq ($(SYSTEM),mingw)
SOURCES += pdcurses
endif
SOURCES += cray_disasm
SOURCES += cray_sim
SOURCES += ds_edit
SOURCES += ddisk_extractor
SOURCES += dump
SOURCES += bin_patch
SOURCES += exp_disk_create
SOURCES += file_splitter
SOURCES += iop_disasm
SOURCES += ovl_split
SOURCES += pl_extractor
SOURCES += tap_edit
SOURCES += tap_extractor
SOURCES += mkfs_unicos
SOURCES += bcpio_extractor
SOURCES += fp_test

define TEMPLATE
.PHONY: $(1)
all: $(1)
build: $(1)
clean: $(1)
depclean: $(1)
$(1):
	$(MAKE) $(MAKECMDGOALS) -C $(1)
endef

$(foreach src,$(SOURCES),$(eval $(call TEMPLATE,$(src))))
