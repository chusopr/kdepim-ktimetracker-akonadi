/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef MESSAGEVIEWER_VCARDVIEWER_H
#define MESSAGEVIEWER_VCARDVIEWER_H

#include <kdialog.h>
#include <kabc/addressee.h>

namespace KPIM {
  class AddresseeView;
}

class VCardViewer : public KDialog
{
    Q_OBJECT
    public:
      VCardViewer(QWidget *parent, const QByteArray& vCard);
      virtual ~VCardViewer();

    protected:
      virtual void slotUser1();
      virtual void slotUser2();
      virtual void slotUser3();

    private:
      KPIM::AddresseeView *  mAddresseeView;
      KABC::Addressee::List  mAddresseeList;

      KABC::Addressee::List::Iterator itAddresseeList;
};


#endif // VCARDVIEWER_H

