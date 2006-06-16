/*
 * This file was generated by dbusidl2cpp version 0.5
 * when processing input file org.kde.networkstatus.provider.xml
 *
 * dbusidl2cpp is Copyright (C) 2006 Trolltech AS. All rights reserved.
 *
 * This is an auto-generated file.
 */

#ifndef PROVIDERADAPTOR_H_220901150471955
#define PROVIDERADAPTOR_H_220901150471955

#include <QtCore/QObject>
#include <dbus/qdbus.h>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface org.kde.networkstatus.provider
 */
class ProviderAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.networkstatus.provider")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.kde.networkstatus.provider\" >\n"
"    <method name=\"status\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"network\" />\n"
"      <arg direction=\"out\" type=\"i\" />\n"
"    </method>\n"
"    <method name=\"establish\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"network\" />\n"
"      <arg direction=\"out\" type=\"i\" />\n"
"    </method>\n"
"    <method name=\"shutdown\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"network\" />\n"
"      <arg direction=\"out\" type=\"i\" />\n"
"    </method>\n"
"    <method name=\"simulateFailure\" />\n"
"    <method name=\"simulateDisconnect\" />\n"
"  </interface>\n"
        "")
public:
    ProviderAdaptor(QObject *parent);
    virtual ~ProviderAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    int establish(const QString &network);
    int shutdown(const QString &network);
    void simulateDisconnect();
    void simulateFailure();
    int status(const QString &network);
Q_SIGNALS: // SIGNALS
};

#endif
