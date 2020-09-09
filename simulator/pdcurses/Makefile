#################################################################################
### MODE: determines what compiler flags to use
### SYSTEM: determines the intermediate and final file directory names (for now)
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

ifndef MODE
MODE=release
#MODE=debug
endif

include ../common.mak

DEP_INC_DIRS = .
INC_DIRS = $(DEP_INC_DIRS)

ifeq ($(SYSTEM),linux)
INC_DIRS += /usr/include/X11
endif

#TARGET_TYPE = EXECUTABLE
TARGET_TYPE = LIBRARY


SOURCES =
SOURCES += pdcurses/addch.c
SOURCES += pdcurses/addchstr.c
SOURCES += pdcurses/addstr.c
SOURCES += pdcurses/attr.c
SOURCES += pdcurses/beep.c
SOURCES += pdcurses/bkgd.c
SOURCES += pdcurses/border.c
SOURCES += pdcurses/clear.c
SOURCES += pdcurses/color.c
SOURCES += pdcurses/debug.c
SOURCES += pdcurses/delch.c
SOURCES += pdcurses/deleteln.c
SOURCES += pdcurses/deprec.c
SOURCES += pdcurses/getch.c
SOURCES += pdcurses/getstr.c
SOURCES += pdcurses/getyx.c
SOURCES += pdcurses/inch.c
SOURCES += pdcurses/inchstr.c
SOURCES += pdcurses/initscr.c
SOURCES += pdcurses/inopts.c
SOURCES += pdcurses/insch.c
SOURCES += pdcurses/insstr.c
SOURCES += pdcurses/instr.c
SOURCES += pdcurses/kernel.c
SOURCES += pdcurses/keyname.c
SOURCES += pdcurses/mouse.c
SOURCES += pdcurses/move.c
SOURCES += pdcurses/outopts.c
SOURCES += pdcurses/overlay.c
SOURCES += pdcurses/pad.c
SOURCES += pdcurses/panel.c
SOURCES += pdcurses/printw.c
SOURCES += pdcurses/refresh.c
SOURCES += pdcurses/scanw.c
SOURCES += pdcurses/scroll.c
SOURCES += pdcurses/scr_dump.c
SOURCES += pdcurses/slk.c
SOURCES += pdcurses/termattr.c
SOURCES += pdcurses/terminfo.c
SOURCES += pdcurses/touch.c
SOURCES += pdcurses/util.c
SOURCES += pdcurses/window.c
ifeq ("$(SYSTEM)","mingw")
SOURCES += win32/pdcclip.c
SOURCES += win32/pdcdisp.c
SOURCES += win32/pdcgetsc.c
SOURCES += win32/pdckbd.c
SOURCES += win32/pdcscrn.c
SOURCES += win32/pdcsetsc.c
SOURCES += win32/pdcutil.c
endif
ifeq ("$(SYSTEM)","linux")
SOURCES += x11/pdcclip.c
SOURCES += x11/pdcgetsc.c
SOURCES += x11/pdcscrn.c
SOURCES += x11/pdcutil.c
SOURCES += x11/sb.c
SOURCES += x11/x11.c
SOURCES += x11/pdcdisp.c
SOURCES += x11/pdckbd.c
SOURCES += x11/pdcsetsc.c
SOURCES += x11/pdcx11.c
SOURCES += x11/ScrollBox.c
endif

SOURCE_LIBS =

LINK_LIBS =

TARGETS = 
TARGETS += pdcurses

include ../engine.mak
