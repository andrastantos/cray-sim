#################################################################################
### Set up compiler and linker flags unless they're already defined
ifndef CXXFLAGS
#CXXFLAGS = -std=c++11 -c -ffunction-sections -fdata-sections -g -Wall -Wno-unused-local-typedefs -Wno-reorder -Wno-deprecated-declarations -fno-strict-aliasing -Werror
CXXFLAGS = $(CXX_BASE_FLAGS) -std=c++14 -c -ffunction-sections -fdata-sections -g -Wall -Wno-unused-local-typedefs -Wno-reorder -fno-strict-aliasing -Wno-unused-variable -Wno-unused-result -Wno-psabi -Werror
endif
ifndef DEP_CXXFLAGS
DEP_CXXFLAGS = -std=c++14
endif
ifndef CFLAGS
CFLAGS = -c -ffunction-sections -fdata-sections -g -fno-strict-aliasing
endif
ifndef DEP_CFLAGS
DEP_CFLAGS =
endif
ifndef CLFLAGS
CLFLAGS = -Wl,--relax -Wl,--no-whole-archive -Wl,-O,2 -Wl,--gc-sections
endif
ifndef CLSOFLAGS
CLSOFLAGS = -static -shared -Wl,-Bsymbolic -Wl,-export-dynamic -fPIC -Wl,--relax -Wl,--no-whole-archive -Wl,-O,2 -Wl,--gc-sections
endif
ifndef PROJECT_MAKEFILE
PROJECT_MAKEFILE = Makefile
endif

ifeq ($(SYSTEM),linux)
#CLFLAGS += -Wl,--whole-archive -lpthread -static -Wl,--no-whole-archive
CLFLAGS += -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
CFLAGS += -gdwarf-2
CXXFLAGS += -gdwarf-2
endif

ifeq ($(SYSTEM),mingw)
#CLFLAGS += -static
CXXFLAGS += -D_WIN32_WINNT=0x0600 -DWINVER=0x0600 -DBOOST_THREAD_USE_LIB -D__USE_W32_SOCKETS
CFLAGS += -D_WIN32_WINNT=0x0600 -DWINVER=0x0600 -DBOOST_THREAD_USE_LIB
endif

ifeq ($(SYSTEM),cygwin)
CXXFLAGS += -D_WIN32_WINNT=0x0600 -DWINVER=0x0600 -D_GNU_SOURCE
CFLAGS += -D_WIN32_WINNT=0x0600 -DWINVER=0x0600 -D_GNU_SOURCE
endif

DIR_MODE = $(MODE)

ifeq ($(MODE),release)
CFLAGS += -O3
CXXFLAGS += -O3
endif
ifeq ($(MODE),pgo_gen)
CFLAGS += -O3 -fprofile-generate
CXXFLAGS += -O3 -fprofile-generate
CLFLAGS += -fprofile-generate
DIR_MODE = pgo
endif
ifeq ($(MODE),pgo_use)
CFLAGS += -O3 -fprofile-use
CXXFLAGS += -O3 -fprofile-use
CLFLAGS += -fprofile-use
DIR_MODE = pgo
endif
ifeq ($(ARCH),32)
CFLAGS += -m32
CXXFLAGS += -m32
CLFLAGS += -m32
CLSOFLAGS += -m32
endif
ifeq ($(ARCH),64)
CFLAGS += -m64
CXXFLAGS += -m64
CLFLAGS += -m64
CLSOFLAGS += -m64
endif

#################################################################################
### Set up intermediate and output paths unless they're already defined
ifdef SYSTEM
DIR_POSTFIX=$(SYSTEM)_$(DIR_MODE)
else
DIR_POSTFIX=$(DIR_MODE)
endif
ifneq ($(ARCH),)
DIR_POSTFIX:=$(DIR_POSTFIX)_$(ARCH)
endif
ifndef BASE
BASE=..
endif

ifndef OBJ_DIR
OBJ_DIR = $(BASE)/_obj/$(DIR_POSTFIX)
endif
ifndef BIN_DIR
BIN_DIR = $(BASE)/_bin/$(DIR_POSTFIX)
endif
ifndef LIB_DIR
LIB_DIR = $(BASE)/_lib/$(DIR_POSTFIX)
endif
ifndef SO_DIR
SO_DIR = $(BASE)/_lib/$(DIR_POSTFIX)
endif


#################################################################################
### Make sure we can eat output from commands in a platform-independent way
ifeq ($(OS),Windows_NT)
ifndef SHELL
  NULL_REDIRECT = > nul 2>&1
else
  ifeq ($(origin SHELL), default)
    NULL_REDIRECT = > nul 2>&1
  else
    NULL_REDIRECT = > /dev/null 2>&1
  endif
endif
else
  NULL_REDIRECT = > /dev/null 2>&1
endif

#################################################################################
### Make sure DEP_INC_DIRS is reasonable, if not defined
ifndef DEP_INC_DIRS
DEP_INC_DIRS=$(INC_DIRS)
endif

#################################################################################
### Build various file-lists
ifeq ($(SYSTEM),mingw)
BIN_EXT = .exe
endif
ifeq ($(SYSTEM),cygwin)
BIN_EXT = .exe
endif

OBJ_FILES = $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(basename $(notdir $(SOURCES)))))
DEP_FILES = $(addprefix $(OBJ_DIR)/,$(addsuffix .d,$(basename $(notdir $(SOURCES)))))
ifeq ($(TARGET_TYPE),LIBRARY)
LIB_FILES = $(addprefix $(LIB_DIR)/lib,$(addsuffix .a,$(basename $(notdir $(TARGETS)))))
endif
ifeq ($(TARGET_TYPE),EXECUTABLE)
BIN_FILES = $(addprefix $(BIN_DIR)/,$(addsuffix $(BIN_EXT),$(basename $(notdir $(TARGETS)))))
DBG_FILES = $(addprefix $(BIN_DIR)/,$(addsuffix .dbg,$(basename $(notdir $(TARGETS)))))
endif
ifeq ($(TARGET_TYPE),SHARED_OBJECT)
SO_FILES = $(addprefix $(SO_DIR)/,$(addsuffix .so,$(basename $(notdir $(TARGETS)))))
endif
SRCLIB_FILES = $(addprefix $(LIB_DIR)/lib,$(addsuffix .a,$(SOURCE_LIBS)))
INC_FLAGS = $(addprefix -I,$(INC_DIRS))
SYS_INC_FLAGS = $(addprefix -isystem,$(SYS_INC_DIRS))
DEP_INC_FLAGS = $(addprefix -I,$(DEP_INC_DIRS))
DEFINE_FLAGS = $(addprefix -D,$(DEFINES))
LIBDIR_FLAGS = $(addprefix -L,$(LIB_DIRS))

# OBJ_FILES is necessary otherwise make automatically deletes them :(
ifndef NO_DEP_LIBS
ALL_TARGETS = $(SOURCE_LIBS)
else
ALL_TARGETS =
endif
ALL_TARGETS += $(LIB_FILES) $(BIN_FILES) $(OBJ_FILES) $(SO_FILES)

#################################################################################
### All the explicit build rules
.PHONY: all clean build depclean

all: $(ALL_TARGETS)
clean:
	@-$(REMOVE) $(ALL_TARGETS) $(DBG_FILES) $(NULL_REDIRECT)
build: clean all
depclean: clean
	@-$(REMOVE) $(DEP_FILES) $(NULL_REDIRECT)

#################################################################################
### Include dependency files except for depclean to avoid building them before deleting
ifneq ($(MAKECMDGOALS),depclean)
-include $(OBJ_FILES:.o=.d)
endif

#################################################################################
### Finally all the implicit build rules

#ifndef SRC_DIRS
#SRC_DIRS = .
#endif
#
#define TEMPLATE
#$(OBJ_DIR)/%.d: $(1)/%.cpp
#	@-$(MKDIR) -p $(OBJ_DIR) $(NULL_REDIRECT)
#	$(GCXX) $(DEP_INC_FLAGS) $(DEFINE_FLAGS) -MM $$< -MP -MG -MT $(OBJ_DIR)/$(addsuffix .d,$(basename $(notdir $@))) -MF $$@
#$(OBJ_DIR)/%.o: $(1)/%.cpp $(OBJ_DIR)/%.d Makefile
#	$(GCXX) $(CXXFLAGS) $(INC_FLAGS) $(SYS_INC_FLAGS) $(DEFINE_FLAGS) $$< -c -g -o $$@
#$(OBJ_DIR)/%.d: $(1)/%.c
#	@-$(MKDIR) -p $(OBJ_DIR) $(NULL_REDIRECT)
#	$(GCC) $(DEP_INC_FLAGS) $(DEFINE_FLAGS) -MM $$< -MP -MG -MT $(OBJ_DIR)/$(addsuffix .d,$(basename $(notdir $@))) -MF $$@
#$(OBJ_DIR)/%.o: $(1)/%.c $(OBJ_DIR)/%.d Makefile
#	$(GCC) $(CFLAGS) $(INC_FLAGS) $(SYS_INC_FLAGS) $(DEFINE_FLAGS) $$< -c -g -o $$@
#endef
#
#$(foreach src_dir,$(SRC_DIRS),$(eval $(call TEMPLATE,$(src_dir))))

define TEMPLATE
ifeq ($(suffix $(1)),.cpp)
$(OBJ_DIR)/$(addsuffix .d,$(basename $(notdir $(1)))): $(1)
	@-$(call MKDIR,$(OBJ_DIR)) $(NULL_REDIRECT)
	$(GCXX) $(DEP_CXXFLAGS) $(DEP_INC_FLAGS) $(DEFINE_FLAGS) -MM $$< -MP -MG -MT $(OBJ_DIR)/$(addsuffix .d,$(basename $(notdir $(1)))) -MF $$@
$(OBJ_DIR)/$(addsuffix .o,$(basename $(notdir $(1)))): $(1) $(OBJ_DIR)/$(addsuffix .d,$(basename $(notdir $(1)))) $(PROJECT_MAKEFILE)
	$(GCXX) $(CXXFLAGS) $(INC_FLAGS) $(SYS_INC_FLAGS) $(DEFINE_FLAGS) $$< -c -g -o $$@
endif
ifeq ($(suffix $(1)),.c)
$(OBJ_DIR)/$(addsuffix .d,$(basename $(notdir $(1)))): $(1)
	@-$(call MKDIR,$(OBJ_DIR)) $(NULL_REDIRECT)
	$(GCC) $(DEP_CFLAGS) $(DEP_INC_FLAGS) $(DEFINE_FLAGS) -MM $$< -MP -MG -MT $(OBJ_DIR)/$(addsuffix .d,$(basename $(notdir $(1)))) -MF $$@
$(OBJ_DIR)/$(addsuffix .o,$(basename $(notdir $(1)))): $(1) $(OBJ_DIR)/$(addsuffix .d,$(basename $(notdir $(1)))) $(PROJECT_MAKEFILE)
	$(GCC) $(CFLAGS) $(INC_FLAGS) $(SYS_INC_FLAGS) $(DEFINE_FLAGS) $$< -c -g -o $$@
endif
endef

$(foreach src,$(SOURCES),$(eval $(call TEMPLATE,$(src))))

#ifeq ($(TARGET_TYPE),LIBRARY)
#$(LIB_DIR)/lib%.a: $(OBJ_FILES)
#	@-$(MKDIR) -p $(LIB_DIR) $(NULL_REDIRECT)
#	-$(REMOVE) $@ $(NULL_REDIRECT)
#	$(AR) -qcv $@ $^
#endif
#
#ifeq ($(TARGET_TYPE),EXECUTABLE)
#$(BIN_DIR)/%$(BIN_EXT): $(OBJ_FILES) $(SRCLIB_FILES)
#	@-$(MKDIR) -p $(BIN_DIR) $(NULL_REDIRECT)
#	$(GCXX) $(CLFLAGS) $(OBJ_FILES) $(LIBDIR_FLAGS) -L$(LIB_DIR) -Wl,--start-group $(addprefix -l,$(SOURCE_LIBS)) $(addprefix -l,$(LINK_LIBS)) -Wl,--end-group -o $@
#endif

ifeq ($(TARGET_TYPE),LIBRARY)
define LIB_TARGET_TEMPLATE
$(1): $(OBJ_FILES)
	-$(call MKDIR,$(LIB_DIR)) $(NULL_REDIRECT)
	-$(REMOVE) $(1) $(NULL_REDIRECT)
	$(AR) -qcv $(1) $(OBJ_FILES)
endef
$(foreach trg,$(LIB_FILES),$(eval $(call LIB_TARGET_TEMPLATE,$(trg))))
endif

ifeq ($(TARGET_TYPE),EXECUTABLE)
define BIN_TARGET_TEMPLATE
$(1): $(OBJ_FILES) $(SRCLIB_FILES)
	@-$(call MKDIR,$(BIN_DIR)) $(NULL_REDIRECT)
	$(GCXX) $(CLFLAGS) $(OBJ_FILES) $(LIBDIR_FLAGS) -L$(LIB_DIR) -Wl,--start-group $(addprefix -l,$(SOURCE_LIBS)) $(addprefix -l,$(LINK_LIBS)) -Wl,--end-group -o $(1)
	$(OBJCOPY) --only-keep-debug $(1) $(addsuffix .dbg,$(basename $(1)))
	$(STRIP) --strip-debug --strip-unneeded $(1)
	$(OBJCOPY) --add-gnu-debuglink=$(addsuffix .dbg,$(basename $(1))) $(1)
endef
$(foreach trg,$(BIN_FILES),$(eval $(call BIN_TARGET_TEMPLATE,$(trg))))
endif

ifeq ($(TARGET_TYPE),SHARED_OBJECT)
define SO_TARGET_TEMPLATE
$(1): $(OBJ_FILES)
	@-$(call MKDIR,$(SO_DIR)) $(NULL_REDIRECT)
	-$(REMOVE) $(1) $(NULL_REDIRECT)
	$(GCXX) $(CLSOFLAGS) $(OBJ_FILES) $(LIBDIR_FLAGS) -L$(LIB_DIR) -Wl,--start-group $(addprefix -l,$(SOURCE_LIBS)) $(addprefix -l,$(LINK_LIBS)) -Wl,--end-group -o $(1)
endef
$(foreach trg,$(SO_FILES),$(eval $(call SO_TARGET_TEMPLATE,$(trg))))
endif

