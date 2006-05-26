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
#include <QDebug>

#include "akonadi.h"
#include "akonadiconnection.h"
#include "storagebackend.h"
#include "storage/datastore.h"
#include "storage/entity.h"

#include "select.h"
#include "response.h"

using namespace Akonadi;

Select::Select(): Handler()
{
}


Select::~Select()
{
}


bool Select::handleLine(const QByteArray& line )
{
    // parse out the reference name and mailbox name
    int startOfCommand = line.indexOf( ' ' ) + 1;
    int startOfMailbox = line.indexOf( ' ', startOfCommand ) + 1;
    QByteArray mailbox = stripQuotes( line.right( line.size() - startOfMailbox ) );
    mailbox.prepend( connection()->selectedCollection() );

    // Responses:  REQUIRED untagged responses: FLAGS, EXISTS, RECENT
    // OPTIONAL OK untagged responses: UNSEEN, PERMANENTFLAGS
    Response response;
    response.setUntagged();
    response.setString( "FLAGS (Answered)");
    emit responseAvailable( response );

    DataStore *db = connection()->storageBackend();
    int secondSlash = mailbox.indexOf( '/', 2 ) + 1;
    qDebug() << "Select: " << mailbox.mid( secondSlash, mailbox.size() - secondSlash )
             << " in resource: " << mailbox.mid( 1, secondSlash - 2 );
    Resource resource = db->getResourceByName( mailbox.left( secondSlash ) );
    Location l = db->getLocationByName( resource, mailbox.right( mailbox.size() - secondSlash ) );

    int exists = 5;
    response.setString( QString::number(exists) + " EXISTS" );
    emit responseAvailable( response );

    int recent = 5;
    response.setString( QString::number(recent) + " RECENT" );
    emit responseAvailable( response );


    int unseen = 1;
    int firstUnseen = 4;

    response.setString( "OK [UNSEEN " + QString::number(unseen) + "] Message " + QString::number(firstUnseen) + " is first unseen" );
    emit responseAvailable( response );

    unsigned int uidValidity = 3857529045;
    response.setString( "OK [UIDVALIDITY " + QString::number(uidValidity) + "] UIDs valid" );
    emit responseAvailable( response );

    response.setSuccess();
    response.setTag( tag() );
    response.setString( "Completed" );
    emit responseAvailable( response );
    
    connection()->setSelectedCollection( mailbox );
    deleteLater();
    return true;
}

