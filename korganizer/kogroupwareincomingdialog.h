/*
    This file is part of KOrganizer.

    Copyright (c) 2002-2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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
#ifndef KOGROUPWAREINCOMINGDIALOG_H
#define KOGROUPWAREINCOMINGDIALOG_H

#include "kogroupwareincomingdialog_base.h"

namespace KCal { class Incidence; }


class KOGroupwareIncomingDialog : public KOGroupwareIncomingDialog_base
{
    Q_OBJECT
  public:
    KOGroupwareIncomingDialog( KCal::Incidence*, QWidget* parent = 0,
                               const char* name = 0, bool modal = false,
                               WFlags fl = 0 );
    ~KOGroupwareIncomingDialog();

    bool isAccepted() const { return mAccepted; }
    bool isConditionallyAccepted() const { return mConditionallyAccepted; }
    bool isDeclined() const { return mDeclined; }

  protected slots:
    void slotAcceptEvent();
    void slotAcceptEventConditionally();
    void slotDeclineEvent();

  private:
    bool mAccepted;
    bool mConditionallyAccepted;
    bool mDeclined;
};

#endif
