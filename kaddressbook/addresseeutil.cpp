/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kabc/stdaddressbook.h>
#include <kabc/vcardconverter.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "addresseeutil.h"

QString AddresseeUtil::addresseesToClipboard( const KABC::Addressee::List &list )
{
  KABC::VCardConverter converter;

  return converter.createVCards( list );
}

KABC::Addressee::List AddresseeUtil::clipboardToAddressees( const QString &data )
{
  KABC::VCardConverter converter;

  return converter.parseVCards( data );
}

QString AddresseeUtil::addresseesToEmails( const KABC::Addressee::List &addrList )
{
  QStringList emails;

  KABC::Addressee::List::ConstIterator it;
  for ( it = addrList.begin(); it != addrList.end(); ++it )
    emails.append( (*it).fullEmail() );

  return emails.join( "," );
}
