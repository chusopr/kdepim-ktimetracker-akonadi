/***************************************************************************
 *   Copyright (C) 2006 by Ingo Kloecker <kloecker@kde.org>                *
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
#include "status.h"

#include <QtCore/QDebug>

#include "akonadi.h"
#include "akonadiconnection.h"
#include "storage/datastore.h"
#include "storage/entity.h"
#include "storage/countquerybuilder.h"

#include "response.h"
#include "imapparser.h"
#include "handlerhelper.h"

using namespace Akonadi;

Status::Status(): Handler()
{
}


Status::~Status()
{
}


bool Status::handleLine( const QByteArray& line )
{
    // Arguments: mailbox name
    //            status data item names

    // Syntax:
    // status     = "STATUS" SP mailbox SP "(" status-att *(SP status-att) ")"
    // status-att = "MESSAGES" / "RECENT" / "UIDNEXT" / "UIDVALIDITY" / "UNSEEN"
    const int startOfCommand = line.indexOf( ' ' ) + 1;
    const int startOfMailbox = line.indexOf( ' ', startOfCommand ) + 1;
    QByteArray mailbox;
    const int endOfMailbox = ImapParser::parseString( line, mailbox, startOfMailbox );
    QList<QByteArray> attributeList;
    ImapParser::parseParenthesizedList( line, attributeList, endOfMailbox );

    Response response;

    DataStore *db = connection()->storageBackend();
    Location l = HandlerHelper::collectionFromIdOrName( mailbox );

    if ( !l.isValid() )
        return failureResponse( "No status for this folder" );

    // Responses:
    // REQUIRED untagged responses: STATUS

    // build STATUS response
    QByteArray statusResponse;
    // MESSAGES - The number of messages in the mailbox
    if ( attributeList.contains( "MESSAGES" ) ) {
        statusResponse += "MESSAGES ";
        const int count = HandlerHelper::itemCount( l );
        if ( count < 0 )
          return failureResponse( "Could not determine message count." );
        statusResponse += QByteArray::number( count );
    }
    // RECENT - The number of messages with the \Recent flag set
    if ( attributeList.contains( "RECENT" ) ) {
        if ( !statusResponse.isEmpty() )
            statusResponse += " RECENT ";
        else
            statusResponse += "RECENT ";
        const int count = HandlerHelper::itemWithFlagCount( l, QLatin1String( "\\Recent" ) );
        if ( count < 0 )
          return failureResponse( "Could not determine recent item count" );
        statusResponse += QByteArray::number( count );
    }
    // UIDNEXT - The next unique identifier value of the mailbox
    if ( attributeList.contains( "UIDNEXT" ) ) {
        if ( !statusResponse.isEmpty() )
            statusResponse += " UIDNEXT ";
        else
            statusResponse += "UIDNEXT ";
        statusResponse += QByteArray::number( db->uidNext() );
    }
    // UIDVALIDITY - The unique identifier validity value of the mailbox
    if ( attributeList.contains( "UIDVALIDITY" ) ) {
        if ( !statusResponse.isEmpty() )
            statusResponse += " UIDVALIDITY 1";
        else
            statusResponse += "UIDVALIDITY 1";
    }
    if ( attributeList.contains( "UNSEEN" ) ) {
        if ( !statusResponse.isEmpty() )
            statusResponse += " UNSEEN ";
        else
            statusResponse += "UNSEEN ";

        const int count = HandlerHelper::itemWithoutFlagCount( l, QLatin1String( "\\Seen" ) );
        if ( count < 0 )
          return failureResponse( "Unable to retrieve unread count" );
        statusResponse += QByteArray::number( count );
    }

    response.setUntagged();
    response.setString( "STATUS \"" + HandlerHelper::pathForCollection( l ).toUtf8() + "\" (" + statusResponse + ')' );
    emit responseAvailable( response );

    response.setSuccess();
    response.setTag( tag() );
    response.setString( "STATUS completed" );
    emit responseAvailable( response );

    deleteLater();
    return true;
}
