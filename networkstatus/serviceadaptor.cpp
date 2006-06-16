/*
 * This file was generated by dbusidl2cpp version 0.5
 * when processing input file org.kde.networkstatus.service.xml
 *
 * dbusidl2cpp is Copyright (C) 2006 Trolltech AS. All rights reserved.
 *
 * This is an auto-generated file.
 */

#include "serviceadaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class ServiceAdaptor
 */

ServiceAdaptor::ServiceAdaptor(QObject *parent)
   : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ServiceAdaptor::~ServiceAdaptor()
{
    // destructor
}

void ServiceAdaptor::registerNetwork(const QString &networkName, int properties)
{
    // handle method call org.kde.networkstatus.service.registerNetwork
    QMetaObject::invokeMethod(parent(), "registerNetwork", Q_ARG(QString, networkName), Q_ARG(int, properties));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->registerNetwork(networkName, properties);
}

void ServiceAdaptor::requestShutdown(const QString &networkName)
{
    // handle method call org.kde.networkstatus.service.requestShutdown
    QMetaObject::invokeMethod(parent(), "requestShutdown", Q_ARG(QString, networkName));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->requestShutdown(networkName);
}

void ServiceAdaptor::setNetworkStatus(const QString &networkName, int status)
{
    // handle method call org.kde.networkstatus.service.setNetworkStatus
    QMetaObject::invokeMethod(parent(), "setNetworkStatus", Q_ARG(QString, networkName), Q_ARG(int, status));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->setNetworkStatus(networkName, status);
}

void ServiceAdaptor::unregisterNetwork(const QString &networkName)
{
    // handle method call org.kde.networkstatus.service.unregisterNetwork
    QMetaObject::invokeMethod(parent(), "unregisterNetwork", Q_ARG(QString, networkName));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->unregisterNetwork(networkName);
}


#include "serviceadaptor.moc"
