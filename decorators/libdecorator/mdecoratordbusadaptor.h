/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp inteface.xml -a mdecoratordbusadaptor -i mabstractappinterface.h -c MDecoratorDBusAdaptor
 *
 * qdbusxml2cpp is Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef MDECORATORDBUSADAPTOR_H_1295881711
#define MDECORATORDBUSADAPTOR_H_1295881711

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "mabstractappinterface.h"
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface com.nokia.MDecorator
 */
class MDecoratorDBusAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.nokia.MDecorator")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.nokia.MDecorator\">\n"
"    <method name=\"setActions\">\n"
"      <arg direction=\"in\" type=\"a(ssbbuay)\" name=\"actions\"/>\n"
"      <annotation value=\"IPCActionList\" name=\"com.trolltech.QtDBus.QtTypeName.In0\"/>\n"
"      <arg direction=\"in\" type=\"u\" name=\"window\"/>\n"
"    </method>\n"
"    <signal name=\"triggered\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"uuid\"/>\n"
"      <arg direction=\"out\" type=\"b\" name=\"checked\"/>\n"
"    </signal>\n"
"    <signal name=\"toggled\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"uuid\"/>\n"
"      <arg direction=\"out\" type=\"b\" name=\"checked\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    MDecoratorDBusAdaptor(QObject *parent);
    virtual ~MDecoratorDBusAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void setActions(IPCActionList actions, uint window);
Q_SIGNALS: // SIGNALS
    void toggled(const QString &uuid, bool checked);
    void triggered(const QString &uuid, bool checked);
};

#endif