/*
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef AKONADI_NOTIFICATIONMANAGER_H
#define AKONADI_NOTIFICATIONMANAGER_H

#include <QtCore/QObject>

namespace Akonadi {

/**
  Notification manager D-Bus interface.
*/
class NotificationManager : public QObject
{
  Q_OBJECT
  Q_CLASSINFO( "D-Bus Interface", "org.kde.Akonadi.NotificationManager" );
  public:
    NotificationManager();
    virtual ~NotificationManager() {}

  public slots:
    void monitorCollection( const QByteArray &path );
    void monitorItem( const QByteArray &uid );

  signals:
    void itemChanged( const QByteArray &uid, const QByteArray &collection );
    void itemAdded( const QByteArray &uid, const QByteArray &collection );
    void itemRemoved( const QByteArray &uid, const QByteArray &collection );
    void collectionAdded( const QByteArray &path );
    void collectionChanged( const QByteArray &path );
    void collectionRemoved( const QByteArray &path );

  private slots:
    // ### just for testing
    void emitSignal();
};

}

#endif
