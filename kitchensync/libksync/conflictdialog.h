/*
    This file is part of KitchenSync.

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef CONFLICTDIA_H
#define CONFLICTDIA_H

#include <kdialogbase.h>
#include <kdepimmacros.h>

namespace KPIM {
class DiffAlgo;
class HTMLDiffAlgoDisplay;
}

namespace KSync {

class SyncEntry;

class KDE_EXPORT ConflictDialog : public KDialogBase
{
  Q_OBJECT

  public:
    ConflictDialog( SyncEntry *syncEntry, SyncEntry *target, QWidget *parent, const char *name = 0 );
    ~ConflictDialog();

  public slots:
    virtual void slotUser1();
    virtual void slotUser2();

  private:
    void initGUI();

    KPIM::DiffAlgo *mDiffAlgo;
    KPIM::HTMLDiffAlgoDisplay *mDisplay;
};

}

#endif
