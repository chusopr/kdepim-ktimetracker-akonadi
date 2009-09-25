/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>
    Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

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

#ifndef AKONADI_NEPOMUK_EMAIL_FEEDER_H
#define AKONADI_NEPOMUK_EMAIL_FEEDER_H

#include <akonadi/agentbase.h>

#include "personcontact.h"

#include <QtCore/QList>

#include <kmime/kmime_header_parsing.h>
#include <nepomukfeederagent.h>

namespace Akonadi {

class NepomukEMailFeeder : public NepomukFeederAgent
{
  Q_OBJECT
  public:
    NepomukEMailFeeder( const QString &id );
    ~NepomukEMailFeeder();

    void updateItem( const Akonadi::Item &item );

  private:
    QList<NepomukFast::Contact> extractContactsFromMailboxes( const KMime::Types::Mailbox::List& mbs, const QUrl& );
    NepomukFast::PersonContact findContact( const QByteArray& address, const QUrl&, bool *found );
};

}

#endif
