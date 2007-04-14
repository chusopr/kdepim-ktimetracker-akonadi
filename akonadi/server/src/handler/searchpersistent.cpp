/***************************************************************************
 *   Copyright (C) 2006 by Tobias Koenig <tokoe@kde.org>                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "akonadi.h"
#include "akonadiconnection.h"
#include "searchpersistent.h"
#include "imapparser.h"
#include "response.h"
#include "storage/datastore.h"
#include "storage/entity.h"
#include "storage/transaction.h"
#include "handlerhelper.h"
#include "searchproviderinterface.h"

#include <QtCore/QStringList>

using namespace Akonadi;

SearchPersistent::SearchPersistent()
  : Handler()
{
}

SearchPersistent::~SearchPersistent()
{
}

bool SearchPersistent::handleLine( const QByteArray& line )
{
  int pos = line.indexOf( ' ' ) + 1; // skip tag
  QByteArray command;
  pos = ImapParser::parseString( line, command, pos );

  QByteArray collectionName;
  pos = ImapParser::parseString( line, collectionName, pos );
  if ( collectionName.isEmpty() )
    return failureResponse( "No name specified" );

  DataStore *db = connection()->storageBackend();
  Transaction transaction( db );

  if ( command.toUpper() == "SEARCH_STORE" ) {

    QString mimeType;
    pos = ImapParser::parseString( line, mimeType, pos );
    if ( mimeType.isEmpty() )
      return failureResponse( "No mimetype specified" );

    MimeType mt = MimeType::retrieveByName( mimeType );
    if ( !mt.isValid() )
      return failureResponse( "Invalid mimetype" );

    QByteArray queryString;
    ImapParser::parseString( line, queryString, pos );
    if ( queryString.isEmpty() )
      return failureResponse( "No query specified" );

    if ( !db->appendPersisntentSearch( QString::fromUtf8(collectionName), queryString ) )
      return failureResponse( "Unable to create persistent search" );

    // get the responsible search providers
    QStringList providers = providerForMimetype( mimeType );
    if ( providers.isEmpty() )
      return failureResponse( "No search providers found for this mimetype" );

    // call search providers
// FIXME: actually provide that interface...
#if 0
    org::kde::Akonadi::SearchProvider *interface =
        new org::kde::Akonadi::SearchProvider(
          QLatin1String("org.kde.Akonadi.SearchProvider.") + provider,
          QLatin1String("/"), QDBusConnection::sessionBus(), this );

    if ( !interface || !interface->isValid() ) {
      qDebug() << "Cannot connect to search provider" << provider
          << (interface ? interface->lastError().message() : QString() );
    } else {
      interface->addSearch( collectionName, queryString );
    }
#endif

  } else if ( command.toUpper() == "SEARCH_DELETE" ) {

    Location search = HandlerHelper::collectionFromIdOrName( collectionName );
    if ( !search.isValid() )
      return failureResponse( "No such persistent search" );

    // get the responsible search providers
    // TODO: fetch mimetype from the database
    QStringList providers = providerForMimetype( QString::fromLatin1( "message/rfc822" ) );
    if ( providers.isEmpty() )
      return failureResponse( "No search providers found for this mimetype" );

    // unregister at search providers
// FIXME: actually provide that interface...
#if 0
    org::kde::Akonadi::SearchProvider *interface =
        new org::kde::Akonadi::SearchProvider(
          QLatin1String("org.kde.Akonadi.SearchProvider.") + provider,
          QLatin1String("/"), QDBusConnection::sessionBus(), this );

    if ( !interface || !interface->isValid() ) {
      qDebug() << "Cannot connect to search provider" << provider
          << (interface ? interface->lastError().message() : QString() );
    } else {
      interface->removeSearch( collectionName );
    }
#endif

    if ( !db->cleanupLocation( search ) )
      return failureResponse( "Unable to remove presistent search" );

  } else {
    return failureResponse( "Unknwon command" );
  }

  if ( !transaction.commit() )
    return failureResponse( "Unable to commit transaction" );

  Response response;
  response.setTag( tag() );
  response.setSuccess();
  response.setString( command + " completed" );
  emit responseAvailable( response );

  deleteLater();
  return true;
}
