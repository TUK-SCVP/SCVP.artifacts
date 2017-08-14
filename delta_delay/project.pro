TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11
TARGET = delta_delay
INCLUDEPATH += /opt/systemc/include/
LIBS += -L/opt/systemc/lib-linux64/ -lsystemc 
SOURCES += delta_delay.cpp
