include(../../../meegotouch_config.pri)
TEMPLATE = app
TARGET = ut_stacking
target.path = /usr/lib/mcompositor-tests/
INSTALLS += target
DEPENDPATH += /usr/include/meegotouch/mcompositor
INCLUDEPATH += ../../../src /usr/include/meegotouch/mcompositor

DEFINES += TESTS

LIBS += -lmcompositor

# Input
HEADERS += ut_stacking.h
SOURCES += ut_stacking.cpp
#OBJECTS += \
#	../../../src/mcompositemanager.o	\
#	../../../src/moc_mcompositemanager.o

QT += testlib core gui opengl dbus
CONFIG += debug
