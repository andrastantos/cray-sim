#################################################################################
### MODE: determines what compiler flags to use
### SYSTEM: determines the intermediate and final file directory names (for now)
### DEFINES:       list of predefined macros to pass to the compiler
### INC_DIRS:      list of include paths to pass to the compiler
### DEP_INC_DIRS:  list of include paths to search for dependencies in
### LIB_DIRS:      list of library paths to pass to the linker
### TARGET_TYPE:   determines whether to build executables or libraries
### SOURCES:       list of source (c++) files
### SOURCE_LIBS:   list of locally built libs (they are included as a dependency)
### LINK_LIBS:     list of libraries to link againsg on top of SOURCE_LIBS
### TARGETS:       list of target final targets (library names or executable files)

ifndef MODE
MODE=release
#MODE=debug
endif

PROJECT_MAKEFILE=

include ../common.mak

#DEFINES = CRAY_TURBO

#TARGET_TYPE = EXECUTABLE
TARGET_TYPE = LIBRARY

SOURCES =
SOURCES += commands.cpp
SOURCES += config_file.cpp
SOURCES += cray_channels.cpp
SOURCES += cray_softcpu.cpp
SOURCES += cray_float.cpp
SOURCES += cray_iop.cpp
SOURCES += cray_iop_built_in_channels.cpp
SOURCES += cray_logger.cpp
SOURCES += cray_mainframe.cpp
SOURCES += cray_types.cpp
SOURCES += curses_win.cpp
SOURCES += console.cpp
SOURCES += iop_bmx.cpp
SOURCES += iop_concentrator.cpp
SOURCES += iop_console.cpp
SOURCES += iop_cluster_if.cpp
SOURCES += iop_cluster.cpp
SOURCES += iop_disk.cpp
SOURCES += iop_era.cpp
SOURCES += iop_expander.cpp
SOURCES += iop_host_memory.cpp
SOURCES += iop_iop2cray.cpp
SOURCES += iop_iop2iop.cpp
SOURCES += logger.cpp
SOURCES += parser.cpp
SOURCES += poked_tap_file.cpp
SOURCES += sys_task_req.cpp
SOURCES += tap_file.cpp
SOURCES += utils.cpp
SOURCES += sim_iop_con.cpp
SOURCES += sim_iop.cpp
SOURCES += sim_iop_disk.cpp
SOURCES += sim_iop_eth.cpp
SOURCES += sim_iop_periph.cpp
SOURCES += ui.cpp
SOURCES += unicos_proc_list.cpp
ifeq ("$(SYSTEM)","mingw")
SOURCES += vtap_win32.cpp
endif
ifeq ("$(SYSTEM)","cygwin")
SOURCES += vtap_win32.cpp
endif
ifeq ("$(SYSTEM)","linux")
SOURCES += vtap_linux.cpp
endif



SOURCE_LIBS =

LINK_LIBS =

TARGETS = 
TARGETS += sim_lib

include ../engine.mak
