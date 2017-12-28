TARGET = tlm_at_1

include(../common.pri)

HEADERS += ../tlm_memory_manager/memory_manager.h \
    initiator.h \
    target.h \
    util.h

SOURCES += main.cpp
SOURCES += ../tlm_memory_manager/memory_manager.cpp
