
/***************************************************************************
 *   Copyright (C) 2006 by Till Adam <adam@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include <QtCore/QDebug>
#include <QtCore/QLatin1String>

#include "handler.h"
#include "response.h"
#include "handler/aklist.h"
#include "handler/append.h"
#include "handler/capability.h"
#include "handler/create.h"
#include "handler/delete.h"
#include "handler/expunge.h"
#include "handler/fetch.h"
#include "handler/list.h"
#include "handler/login.h"
#include "handler/logout.h"
#include "handler/modify.h"
#include "handler/noop.h"
#include "handler/rename.h"
#include "handler/search.h"
#include "handler/searchpersistent.h"
#include "handler/select.h"
#include "handler/status.h"
#include "handler/store.h"
#include "handler/transaction.h"
#include "uid.h"

#include "dbusthread.h"

using namespace Akonadi;

Handler::Handler()
    :QObject()
    , m_connection( 0 )
{
}


Handler::~Handler()
{
}

bool Handler::handleLine( const QByteArray & command )
{
    Response response;
    response.setError();
    response.setTag( m_tag );
    response.setString( "Unrecognized command: " + command );

    emit responseAvailable( response );
    deleteLater();
    return true;
}

Handler * Handler::findHandlerForCommandNonAuthenticated( const QByteArray & command )
{
    // allowed are LOGIN and AUTHENTICATE
    if ( command == "LOGIN" )
        return new Login();

    return 0;
}

Handler * Handler::findHandlerForCommandAlwaysAllowed( const QByteArray & command )
{
    // allowed commands CAPABILITY, NOOP, and LOGOUT
    if ( command == "LOGOUT" )
        return new Logout();
    if ( command == "CAPABILITY" )
        return new Capability();
    return 0;
}

void Handler::setTag( const QByteArray & tag )
{
    m_tag = tag;
}

QByteArray Handler::tag( ) const
{
    return m_tag;
}

Handler * Handler::findHandlerForCommandAuthenticated( const QByteArray & command )
{
    // allowd commands are SELECT, EXAMINE, CREATE, DELETE, RENAME,
    // SUBSCRIBE, UNSUBSCRIBE, LIST, LSUB, and APPEND.
    if ( command == "APPEND" )
        return new Append();
    if ( command == "CREATE" )
        return new Create();
    if ( command == "LIST" )
        return new List();
    if ( command == "SELECT" )
        return new Select();
    if ( command == "SEARCH" )
        return new Search();
    if ( command == "SEARCH_STORE" || command == "SEARCH_DELETE" || command == "SEARCH_DEBUG" )
        return new SearchPersistent();
    if ( command == "NOOP" )
        return new Noop();
    if ( command == "FETCH" )
        return new Fetch();
    if ( command == "EXPUNGE" )
        return new Expunge();
    if ( command == "UID" )
        return new Uid();
    if ( command == "STORE" )
        return new Store();
    if ( command == "STATUS" )
        return new Status();
    if ( command == "DELETE" )
      return new Delete();
    if ( command == "MODIFY" )
      return new Modify();
    if ( command == "RENAME" )
      return new Rename();
    if ( command == "BEGIN" || command == "ROLLBACK" || command == "COMMIT" )
      return new TransactionHandler();
    if ( command == "X-AKLIST" )
      return new AkList();

    return 0;
}


void Akonadi::Handler::setConnection( AkonadiConnection* connection )
{
    m_connection = connection;
}


AkonadiConnection* Akonadi::Handler::connection()
{
    return m_connection;
}



bool Akonadi::Handler::failureResponse( const QString& failureMessage )
{
    Response response;
    response.setTag( tag() );
    response.setFailure();
    response.setString( failureMessage );
    emit responseAvailable( response );
    deleteLater();
    return true;
}

bool Akonadi::Handler::failureResponse( const QByteArray &failureMessage )
{
  return failureResponse( QString::fromLatin1( failureMessage ) );
}

bool Akonadi::Handler::failureResponse(const char * failureMessage)
{
  return failureResponse( QLatin1String( failureMessage ) );
}

bool Akonadi::Handler::startContinuation()
{
  Response response;
  response.setContinuation();
  response.setString( "Ready for literal data" );
  emit responseAvailable( response );
  return false;
}

QStringList Akonadi::Handler::providerForMimetype(const QByteArray & mimeType)
{
  if ( m_providerCache.contains( mimeType ) )
    return m_providerCache.value( mimeType );

  QList<QVariant> arguments;
  arguments << QString::fromUtf8( mimeType );
  DBusThread *dbusThread = static_cast<DBusThread*>( QThread::currentThread() );

  QList<QVariant> result = dbusThread->callDBus( QLatin1String( "org.kde.Akonadi.Control" ),
      QLatin1String( "/SearchProviderManager" ),
      QLatin1String( "org.kde.Akonadi.SearchProviderManager" ),
      QLatin1String( "providersForMimeType" ), arguments );

  // TODO error handling (eg. on failing call)
  if ( result.isEmpty() )
    return QStringList();

  QStringList providers = result.first().toStringList();
  if ( !providers.isEmpty() )
    m_providerCache.insert( mimeType, providers );

  return providers;
}

#include "handler.moc"
