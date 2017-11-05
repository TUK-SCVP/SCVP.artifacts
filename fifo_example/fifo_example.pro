TARGET = fifo_example

include(../common.pri)

SOURCES += main.cpp
SOURCES += producer.cpp
SOURCES += consumer.cpp

HEADERS += producer.h
HEADERS += consumer.h

DEFINES += DEBUG_SYSTEMC=1
