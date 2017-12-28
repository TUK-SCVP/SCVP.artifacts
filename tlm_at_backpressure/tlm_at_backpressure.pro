TARGET = tlm_at_backpressure

include(../common.pri)

HEADERS += ../tlm_memory_manager/memory_manager.h
HEADERS += initiator.h
HEADERS += target.h

SOURCES += main.cpp
SOURCES += ../tlm_memory_manager/memory_manager.cpp
