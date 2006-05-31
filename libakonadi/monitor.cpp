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

#include "monitor.h"
#include "notificationmanager.h"

#include <dbus/qdbusinterface.h>
#include <dbus/qdbusconnection.h>

#include <QDebug>

using namespace PIM;

class PIM::MonitorPrivate
{
  public:
    org::kde::pim::Akonadi::NotificationManager *nm;
    QList<QByteArray> collections;

    bool isCollectionMonitored( const QByteArray &path )
    {
      foreach ( const QByteArray ba, collections ) {
        if ( path.startsWith( ba ) )
          return true;
      }
      return false;
    }
};

PIM::Monitor::Monitor( QObject *parent ) :
    QObject( parent ),
    d( new MonitorPrivate() )
{
  d->nm = QDBus::sessionBus().findInterface<org::kde::pim::Akonadi::NotificationManager>("org.kde.pim.Akonadi.NotificationManager", "/");
  if ( !d->nm ) {
    // TODO: error handling
    qWarning() << "Unable to connect to notification manager";
  } else {
    connect( d->nm, SIGNAL(itemChanged(QByteArray,QByteArray)), SLOT(slotItemChanged(QByteArray,QByteArray)) );
    connect( d->nm, SIGNAL(itemAdded(QByteArray,QByteArray)), SLOT(slotItemAdded(QByteArray,QByteArray)) );
    connect( d->nm, SIGNAL(itemRemoved(QByteArray,QByteArray)), SLOT(slotItemRemoved(QByteArray,QByteArray)) );
    connect( d->nm, SIGNAL(collectionChanged(QByteArray)), SLOT(slotCollectionChanged(QByteArray)) );
    connect( d->nm, SIGNAL(collectionAdded(QByteArray)), SLOT(slotCollectionAdded(QByteArray)) );
    connect( d->nm, SIGNAL(collectionRemoved(QByteArray)), SLOT(slotCollectionRemoved(QByteArray)) );
  }
}

PIM::Monitor::~Monitor()
{
  delete d->nm;
  delete d;
}

void PIM::Monitor::monitorCollection( const QByteArray & path )
{
  d->collections.append( path );
  d->nm->monitorCollection( path );
}

void PIM::Monitor::monitorItem( const DataReference & ref )
{
  d->nm->monitorItem( ref.persistanceID().toLatin1() );
}

void PIM::Monitor::slotItemChanged( const QByteArray & uid, const QByteArray & collection )
{
  if ( d->isCollectionMonitored( collection ) )
    emit itemChanged( DataReference( uid, QString() ) );
}

void PIM::Monitor::slotItemAdded( const QByteArray & uid, const QByteArray & collection )
{
  if ( d->isCollectionMonitored( collection ) )
    emit itemAdded( DataReference( uid, QString() ) );
}

void PIM::Monitor::slotItemRemoved( const QByteArray & uid, const QByteArray & collection )
{
  if ( d->isCollectionMonitored( collection ) )
    emit itemRemoved( DataReference( uid, QString() ) );
}

void PIM::Monitor::slotCollectionChanged( const QByteArray & path )
{
  qDebug() << "hi there";
  if ( d->isCollectionMonitored( path ) )
    emit collectionChanged( path );
}

void PIM::Monitor::slotCollectionAdded( const QByteArray & path )
{
  if ( d->isCollectionMonitored( path ) )
    emit collectionAdded( path );
}

void PIM::Monitor::slotCollectionRemoved( const QByteArray & path )
{
  if ( d->isCollectionMonitored( path ) )
    emit collectionRemoved( path );
}

#include "monitor.moc"
