include(../../meegotouch_config.pri)
TEMPLATE = app
DEPENDPATH += ../libdecorator
INCLUDEPATH += ../libdecorator
CONFIG += meegotouch release
QT += opengl dbus

LIBS += ../libdecorator/libdecorator.so -lX11 -lXfixes

SOURCES += main.cpp mdecoratorwindow.cpp
HEADERS += mdecoratorwindow.h

QMAKE_EXTRA_TARGETS += check
check.depends = $$TARGET
check.commands = $$system(true)
target.path = /usr/bin
INSTALLS += target
