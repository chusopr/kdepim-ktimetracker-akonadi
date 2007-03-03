/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KSYNC_ADDRESSEEDIFFALGO_H
#define KSYNC_ADDRESSEEDIFFALGO_H

#include <kabc/addressee.h>
#include "diffalgo.h"

namespace KSync {

class AddresseeDiffAlgo : public DiffAlgo
{
  public:
    AddresseeDiffAlgo( const KABC::Addressee &leftAddressee, const KABC::Addressee &rightAddressee );
    AddresseeDiffAlgo( const QString &leftAddressee, const QString &rightAddressee );

    void run();

  private:
    template <class L>
    void diffList( const QString &id, const QValueList<L> &left, const QValueList<L> &right );

    QString toString( const KABC::PhoneNumber &number );
    QString toString( const KABC::Address &address );

    KABC::Addressee mLeftAddressee;
    KABC::Addressee mRightAddressee;
};

}

#endif
