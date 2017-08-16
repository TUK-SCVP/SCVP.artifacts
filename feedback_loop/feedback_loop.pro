TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11
TARGET = feedback_loop
INCLUDEPATH += /opt/systemc/include/
LIBS += -L/opt/systemc/lib-linux64/ -lsystemc 
SOURCES += feedback_loop.cpp
