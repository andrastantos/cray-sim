#################################################################################
### MODE:          release|debug determines what compiler flags to use
### ARCH:          32|64 determines 32- or 64-bit build. If not specified, system default is used
### SYSTEM:        mingw|linux|cygwin determines the compilation environment. Autodetect if not specified
### DEFINES:       list of predefined macros to pass to the compiler
### INC_DIRS:      list of include paths to pass to the compiler
### DEP_INC_DIRS:  list of include paths to search for dependencies in
### LIB_DIRS:      list of library paths to pass to the linker
### TARGET_TYPE:   determines whether to build executables or libraries
### SOURCES:       list of source (c++) files
### SRC_DIRS:      list of source paths. Needed to generate implicit rules
### SOURCE_LIBS:   list of locally built libs (they are included as a dependency)
### LINK_LIBS:     list of libraries to link againsg on top of SOURCE_LIBS
### TARGETS:       list of target final targets (library names or executable files)
### SIM_BASE:      location for the simulation sources. Defaults to ..
#################################################################################
### CPATH:         make sure boost is in the search path. If not, adjust this environment variable so that it is
### LIBRARY_PATH:  make sure boost libraries are in the search path. If not, adjust this environment variable so that they are

# Since we'll define goals here before Makefile.inc defines all, make sure that's the one that's picked up
.DEFAULT_GOAL := all

#################################################################################
### Define commands if they're not already defined

ifndef GCXX
GCXX = g++
endif
ifndef GCC
GCC = gcc
endif
ifndef AR
AR = ar
endif
ifndef OBJDUMP
OBJDUMP = objdump
endif
ifndef OBJCOPY
OBJCOPY = objcopy
endif
ifndef STRIP
STRIP = strip
endif

ifndef SYSTEM
GCC_TARGET:=$(shell $(GCC) -dumpmachine)
ifneq (, $(findstring linux, $(GCC_TARGET)))
SYSTEM=linux
HOST=posix
else
ifneq (, $(findstring mingw, $(GCC_TARGET)))
SYSTEM=mingw
# Decide if we're under MSYS or plain DOS shell...
ifndef HOST
ifeq (, $(findstring msys, $(OSTYPE)))
HOST=posix
SUBSYSTEM=msys
else
HOST=dos
endif
endif
else
ifneq (, $(findstring cygwin, $(GCC_TARGET)))
SYSTEM=cygwin
HOST=posix
else
ifneq (, $(findstring cygnus, $(GCC_TARGET)))
SYSTEM=cygwin
HOST=posix
else
$(error Couldn't detect system. Please specify manually \(make SYSTEM=<linux|mingw|cygwin>\)
endif
endif
endif
endif
endif

ifndef SYSTEM
$(error Couldn't detect system. Please specify manually \(make SYSTEM=<linux|mingw|cygwin>\)
endif

ifeq ($(SYSTEM),cygwin)
ifndef REMOVE
REMOVE = rm
endif
#ifndef MKDIR
MKDIR = mkdir -p $(1)
#endif
endif

ifeq ($(SYSTEM),mingw)
ifndef REMOVE
REMOVE = del
endif
#ifndef MKDIR
MKDIR = mkdir $(subst /,\,$(1))
#endif
endif

ifeq ($(SYSTEM),linux)
ifndef REMOVE
REMOVE = rm
endif
#ifndef MKDIR
MKDIR = mkdir -p $(1)
#endif
endif


ifeq ($(SYSTEM),mingw)
BIN_EXT = .exe
endif
ifeq ($(SYSTEM),cygwin)
BIN_EXT = .exe
endif

ifndef MODE
MODE=release
#MODE=debug
endif

ifeq ("$(SYSTEM)","mingw")
endif
ifeq ("$(SYSTEM)","cygwin")
EXTRA_INC += /usr/include/ncurses
#DEFINES += __USE_W32_SOCKETS
endif
ifeq ("$(SYSTEM)","linux")
endif
DEFINES += CRAY_HOST_SYSTEM=$(SYSTEM)

ifndef DEP_INC_DIRS
DEP_INC_DIRS = . ../sim_lib ../httpd
ifeq ($(SYSTEM),mingw)
DEP_INC_DIRS += ../pdcurses
endif
endif

INC_DIRS += $(DEP_INC_DIRS) $(EXTRA_INC)

DEFINES += _FILE_OFFSET_BITS=64

#	$(if ("$(SYSTEM)","cygwin"), $(1), :lib$(strip $(1)).a)
define static_lib
	:lib$(strip $(1)).a
endef
ifeq ("$(SYSTEM)","cygwin")
define static_lib
	$(1)
endef
endif

#################################################################################
### Setting up some common libraries
#################################################################################
NCURSES_LIBS =
BOOST_LIBS =
COMMON_LIBS =

BOOST_LIBS += $(call static_lib, boost_system)
BOOST_LIBS += $(call static_lib, boost_filesystem)
BOOST_LIBS += $(call static_lib, boost_regex)
BOOST_LIBS += $(call static_lib, boost_thread)
BOOST_LIBS += $(call static_lib, boost_timer)
BOOST_LIBS += $(call static_lib, boost_chrono)

COMMON_LIBS += $(BOOST_LIBS)
COMMON_LIBS += supc++
COMMON_LIBS += stdc++

ifeq ("$(SYSTEM)","mingw")
COMMON_LIBS += wsock32
COMMON_LIBS += ws2_32
endif
ifeq ($(SYSTEM),linux)
NCURSES_LIBS += $(call static_lib, ncurses)
NCURSES_LIBS += $(call static_lib, termcap)
NCURSES_LIBS += $(call static_lib, gpm)
NCURSES_LIBS += $(call static_lib, panel)
BOOST_LIBS += $(call static_lib, rt)
COMMON_LIBS += pthread
endif
ifeq ($(SYSTEM),cygwin)
NCURSES_LIBS += ncurses
NCURSES_LIBS += panel
endif

#################################################################################
### We have to inform Make about how to build each entry SOURCE_LIBS
#################################################################################

ifndef SIM_BASE
SIM_BASE = ..
endif

ifndef NO_DEP_LIBS
.PHONY: $(SOURCE_LIBS)

sim_lib:
	$(MAKE) $(MAKECMDGOALS) -C $(SIM_BASE)/sim_lib

httpd:
	$(MAKE) $(MAKECMDGOALS) -C $(SIM_BASE)/httpd

pdcurses:
	$(MAKE) $(MAKECMDGOALS) -C $(SIM_BASE)/pdcurses
endif
