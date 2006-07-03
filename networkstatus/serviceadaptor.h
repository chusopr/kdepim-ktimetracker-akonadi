/*
 * This file was generated by dbusidl2cpp version 0.5
 * when processing input file org.kde.networkstatus.service.xml
 *
 * dbusidl2cpp is Copyright (C) 2006 Trolltech AS. All rights reserved.
 *
 * This is an auto-generated file.
 */

#ifndef SERVICEADAPTOR_H_220721150471817
#define SERVICEADAPTOR_H_220721150471817

#include <QtCore/QObject>
#include <QtDBus>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface org.kde.networkstatus.service
 */
class ServiceAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.networkstatus.service")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.kde.networkstatus.service\" >\n"
"    <method name=\"setNetworkStatus\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"networkName\" />\n"
"      <arg direction=\"in\" type=\"i\" name=\"status\" />\n"
"    </method>\n"
"    <method name=\"registerNetwork\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"networkName\" />\n"
"      <arg direction=\"in\" type=\"i\" name=\"properties\" />\n"
"    </method>\n"
"    <method name=\"unregisterNetwork\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"networkName\" />\n"
"    </method>\n"
"    <method name=\"requestShutdown\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"networkName\" />\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    ServiceAdaptor(QObject *parent);
    virtual ~ServiceAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void registerNetwork(const QString &networkName, int properties);
    void requestShutdown(const QString &networkName);
    void setNetworkStatus(const QString &networkName, int status);
    void unregisterNetwork(const QString &networkName);
Q_SIGNALS: // SIGNALS
};

#endif
