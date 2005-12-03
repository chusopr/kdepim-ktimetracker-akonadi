/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef SUMMARYWIDGET_H
#define SUMMARYWIDGET_H

#include "summary.h"

#include <qmap.h>
#include <q3ptrlist.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QLabel>
#include <Q3GridLayout>
#include <QEvent>

#include <libkcal/resourcelocal.h>
#include <libkcal/calendarresources.h>

class Q3GridLayout;
class QLabel;

namespace Kontact {
    class Plugin;
}

class KNotesSummaryWidget : public Kontact::Summary
{
  Q_OBJECT

  public:
    KNotesSummaryWidget( Kontact::Plugin *plugin, QWidget *parent, const char *name = 0 );

    void updateSummary( bool force = false ) { Q_UNUSED( force ); updateView(); }

  protected:
    virtual bool eventFilter( QObject *obj, QEvent* e );

  protected slots:
    void urlClicked( const QString& );
    void updateView();
    void addNote( KCal::Journal* );
    void removeNote( KCal::Journal* );

  private:
    KCal::CalendarLocal *mCalendar;
    KCal::Journal::List mNotes;

    Q3GridLayout *mLayout;

    Q3PtrList<QLabel> mLabels;
    Kontact::Plugin *mPlugin;
};

#endif
