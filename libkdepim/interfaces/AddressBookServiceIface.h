/*
    This file is part of libkdepim.

    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef ADDRESSBOOKSERVICEIFACE_H
#define ADDRESSBOOKSERVICEIFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <kurl.h>
#include <qstring.h>
#include <qcstring.h>

namespace KPIM {

  class AddressBookServiceIface : virtual public DCOPObject
  {
    K_DCOP

    k_dcop:
      /**
        This method will add a vcard to the address book.

        @param vCard The vCard in string representation.
        @param showPreview Whether a preview should be shown.
       */
      virtual void importVCard( const QString& vCard, bool showPreview ) = 0L;

      /**
        This method will add a vcard to the address book.

        @param url The url where the vcard is located.
        @param showPreview Whether a preview should be shown.
       */
      virtual void importVCard( const KURL& url, bool showPreview ) = 0L;
  };

}

#endif

