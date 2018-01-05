TARGET = tlm_cpu_example

include(../common.pri)

HEADERS += ../tlm_memory_manager/memory_manager.h
SOURCES += ../tlm_memory_manager/memory_manager.cpp

HEADERS += cpu.h
HEADERS += memory.h
SOURCES += main.cpp

DISTFILES += README
DISTFILES += assembler.pl
DISTFILES += test.asm
