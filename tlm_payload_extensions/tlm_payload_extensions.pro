TARGET = tlm_payload_extensions

include(../common.pri)

HEADERS += ../tlm_memory_manager/memory_manager.h
HEADERS += ../tlm_at_1/initiator.h
HEADERS += ../tlm_at_1/target.h

SOURCES += main.cpp
SOURCES += ../tlm_memory_manager/memory_manager.cpp
