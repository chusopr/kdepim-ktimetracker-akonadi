/*
 * This file was generated by dbusxml2cpp version 0.6
 * Command line was: dbusxml2cpp -m -a resourceadaptor -i resourcebase.h -l Akonadi::Resource /usr/src/kde4/kdepim/akonadi/resources/include/org.kde.Akonadi.Resource.xml
 *
 * dbusxml2cpp is Copyright (C) 2006 Trolltech AS. All rights reserved.
 *
 * This is an auto-generated file and manually modified file.
 */

#include "resourceadaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class ResourceAdaptor
 */

ResourceAdaptor::ResourceAdaptor(Akonadi::Resource *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ResourceAdaptor::~ResourceAdaptor()
{
    // destructor
}

void ResourceAdaptor::cleanup()
{
    // handle method call org.kde.Akonadi.Resource.cleanup
    parent()->cleanup();
}

QString ResourceAdaptor::configuration()
{
    // handle method call org.kde.Akonadi.Resource.configuration
    return parent()->configuration();
}

void ResourceAdaptor::configure()
{
    // handle method call org.kde.Akonadi.Resource.configure
    parent()->configure();
}

QString ResourceAdaptor::name()
{
    // handle method call org.kde.Akonadi.Resource.name
    return parent()->name();
}

uint ResourceAdaptor::progress()
{
    // handle method call org.kde.Akonadi.Resource.progress
    return parent()->progress();
}

QString ResourceAdaptor::progressMessage()
{
    // handle method call org.kde.Akonadi.Resource.progressMessage
    return parent()->progressMessage();
}

void ResourceAdaptor::quit()
{
    // handle method call org.kde.Akonadi.Resource.quit
    parent()->quit();
}

bool ResourceAdaptor::requestItemDelivery(const QString &uid, const QString &remotedId, const QString &collection, int type, const QDBusMessage &msg)
{
    // handle method call org.kde.Akonadi.Resource.requestItemDelivery
    return parent()->requestItemDelivery(uid, remotedId, collection, type, msg);
}

bool ResourceAdaptor::setConfiguration(const QString &data)
{
    // handle method call org.kde.Akonadi.Resource.setConfiguration
    return parent()->setConfiguration(data);
}

void ResourceAdaptor::setName(const QString &name)
{
    // handle method call org.kde.Akonadi.Resource.setName
    parent()->setName(name);
}

int ResourceAdaptor::status()
{
    // handle method call org.kde.Akonadi.Resource.status
    return parent()->status();
}

QString ResourceAdaptor::statusMessage()
{
    // handle method call org.kde.Akonadi.Resource.statusMessage
    return parent()->statusMessage();
}

void ResourceAdaptor::synchronize()
{
    // handle method call org.kde.Akonadi.Resource.synchronize
    parent()->synchronize();
}


#include "resourceadaptor.moc"
