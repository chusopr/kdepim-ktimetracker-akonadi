/*  -*- c++ -*-
    identitydrag.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "identitydrag.h"


namespace KMail {

  static const char kmailIdentityMimeType[] = "application/x-kmail-identity-drag";

  IdentityDrag::IdentityDrag( const KPIM::Identity & ident,
			      QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name ), mIdent( ident )
  {

  }

  const char * IdentityDrag::format( int i ) const {
    if ( i == 0 )
      return kmailIdentityMimeType;
    else
      return 0;
  }

  QByteArray IdentityDrag::encodedData( const char * mimetype ) const {
    QByteArray a;

    if ( !qstrcmp( mimetype, kmailIdentityMimeType ) ) {
      QDataStream s( a, IO_WriteOnly );
      s << mIdent;
    }

    return a;
  }

  bool IdentityDrag::canDecode( const QMimeSource * e ) {
    // ### feel free to add vCard and other stuff here and in decode...
    return e->provides( kmailIdentityMimeType );
  }

  bool IdentityDrag::decode( const QMimeSource * e, KPIM::Identity & i ) {

    if ( e->provides( kmailIdentityMimeType ) ) {
      QDataStream s( e->encodedData( kmailIdentityMimeType ), IO_ReadOnly );
      s >> i;
      return true;
    }

    return false;
  }

}

#include "identitydrag.moc"
