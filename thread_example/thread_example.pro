TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11
TARGET = thread_example
INCLUDEPATH += /opt/systemc/include/
LIBS += -L/opt/systemc/lib-linux64/ -lsystemc 
SOURCES += thread_example.cpp
