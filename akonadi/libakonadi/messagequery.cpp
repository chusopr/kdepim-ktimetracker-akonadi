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

#include "messagequery.h"

#include <QtCore/QTimer>

using namespace Akonadi;

class MessageQuery::Private
{
  public:
    QString query;
    QString part;
    Message::List messages;
};

MessageQuery::MessageQuery( const QString & query, const QString & part ) :
    Job(),
    d ( new Private() )
{
  d->query = query;
  d->part = part;
}

MessageQuery::~MessageQuery( )
{
  delete d;
}

Message::List MessageQuery::messages( ) const
{
  return d->messages;
}

void MessageQuery::doStart( )
{
  // TODO
  QTimer::singleShot( 0, this, SLOT( slotEmitDone() ) );
}

void MessageQuery::doHandleResponse( const QByteArray &tag,
  const QByteArray &data )
{
}

void MessageQuery::slotEmitDone( )
{
  emit done( this );
}

#include "messagequery.moc"
