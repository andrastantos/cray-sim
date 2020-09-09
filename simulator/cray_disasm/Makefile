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

TARGET_TYPE = EXECUTABLE

SOURCES =
SOURCES += cray_disasm.cpp

SOURCE_LIBS =
SOURCE_LIBS += sim_lib

LINK_LIBS =
LINK_LIBS += $(NCURSES_LIBS)
LINK_LIBS += $(COMMON_LIBS)

TARGETS =
TARGETS += cray_disasm.exe

include ../engine.mak
