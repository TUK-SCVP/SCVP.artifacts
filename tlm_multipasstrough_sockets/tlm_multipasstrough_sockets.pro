TARGET = tlm_multipasstrough_sockets

include(../common.pri)

HEADERS += ../tlm_memory_manager/memory_manager.h
HEADERS += target.h
HEADERS += initiator.h

SOURCES += main.cpp
SOURCES += ../tlm_memory_manager/memory_manager.cpp
