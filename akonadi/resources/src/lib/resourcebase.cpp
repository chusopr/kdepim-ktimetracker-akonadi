/*
    This file is part of libakonadi.

    Copyright (c) 2006 Till Adam <adam@kde.org>

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

#include <QtCore/QTimer>
#include <QtGui/QApplication>

#include <stdlib.h>

#include "resourcebase.h"
#include "resourceadaptor.h"

#include "tracerinterface.h"

using namespace PIM;

class ResourceBase::Private
{
  public:
    Private()
      : mStatusCode( Ready ),
        mProgress( 0 )
    {
      mStatusMessage = defaultReadyMessage();
    }

    QString defaultReadyMessage() const;
    QString defaultSyncingMessage() const;
    QString defaultErrorMessage() const;

    org::kde::Akonadi::Tracer *mTracer;
    QString mId;

    int mStatusCode;
    QString mStatusMessage;

    uint mProgress;
    QString mProgressMessage;
};

QString ResourceBase::Private::defaultReadyMessage() const
{
  return tr( "Ready" );
}

QString ResourceBase::Private::defaultSyncingMessage() const
{
  return tr( "Syncing..." );
}

QString ResourceBase::Private::defaultErrorMessage() const
{
  return tr( "Error!" );
}

ResourceBase::ResourceBase( const QString & id )
  : d( new Private )
{
  d->mTracer = new org::kde::Akonadi::Tracer( "org.kde.Akonadi", "/tracing", QDBusConnection::sessionBus(), this );

  if ( !QDBusConnection::sessionBus().registerService( "org.kde.Akonadi.Resource." + id ) )
    error( QString( "Unable to register service at dbus: %1" ).arg( QDBusConnection::sessionBus().lastError().message() ) );

  new ResourceAdaptor( this );
  if ( !QDBusConnection::sessionBus().registerObject( "/", this, QDBusConnection::ExportAdaptors ) )
    error( QString( "Unable to register object at dbus: %1" ).arg( QDBusConnection::sessionBus().lastError().message() ) );

  d->mId = id;
}

ResourceBase::~ResourceBase()
{
  delete d;
}

int ResourceBase::status() const
{
  return d->mStatusCode;
}

QString ResourceBase::statusMessage() const
{
  return d->mStatusMessage;
}

uint ResourceBase::progress() const
{
  return d->mProgress;
}

QString ResourceBase::progressMessage() const
{
  return d->mProgressMessage;
}

void ResourceBase::warning( const QString& message )
{
  d->mTracer->warning( QString( "ResourceBase(%1)" ).arg( d->mId ), message );
}

void ResourceBase::error( const QString& message )
{
  d->mTracer->error( QString( "ResourceBase(%1)" ).arg( d->mId ), message );
}

void ResourceBase::changeStatus( Status status, const QString &message )
{
  d->mStatusMessage = message;
  d->mStatusCode = 0;

  switch ( status ) {
    case Ready:
      if ( d->mStatusMessage.isEmpty() )
        d->mStatusMessage = d->defaultReadyMessage();

      d->mStatusCode = 0;
      break;
    case Syncing:
      if ( d->mStatusMessage.isEmpty() )
        d->mStatusMessage = d->defaultSyncingMessage();

      d->mStatusCode = 1;
      break;
    case Error:
      if ( d->mStatusMessage.isEmpty() )
        d->mStatusMessage = d->defaultErrorMessage();

      d->mStatusCode = 2;
      break;
    default:
      Q_ASSERT( !"Unknown status passed" );
      break;
  }

  emit statusChanged( d->mStatusCode, d->mStatusMessage );
}

void ResourceBase::changeProgress( uint progress, const QString &message )
{
  d->mProgress = progress;
  d->mProgressMessage = message;

  emit progressChanged( d->mProgress, d->mProgressMessage );
}

void ResourceBase::configure()
{
  emit configurationChanged( QString() );
}

bool ResourceBase::setConfiguration( const QString &data )
{
  // TODO: add template method
  emit configurationChanged( data );

  return true;
}

QString ResourceBase::configuration() const
{
  return QString();
}

void ResourceBase::synchronize()
{
}

QString ResourceBase::parseArguments( int argc, char **argv )
{
  if ( argc < 3 ) {
    qDebug( "ResourceBase::parseArguments: Not enough arguments passed..." );
    exit( 1 );
  }

  QString identifier;
  for ( int i = 1; i < argc - 1; ++i ) {
    if ( QString( argv[ i ] ) == "--identifier" )
      identifier = QString( argv[ i + 1 ] );
  }

  if ( identifier.isEmpty() ) {
    qDebug( "ResourceBase::parseArguments: Identifier argument missing" );
    exit( 1 );
  }

  QApplication::setQuitOnLastWindowClosed( false );

  return identifier;
}

void ResourceBase::quit()
{
  aboutToQuit();

  QTimer::singleShot( 0, QCoreApplication::instance(), SLOT( quit() ) );
}

void ResourceBase::aboutToQuit()
{
  qDebug( "about to quit called" );
}

QString PIM::ResourceBase::identifier() const
{
  return d->mId;
}

