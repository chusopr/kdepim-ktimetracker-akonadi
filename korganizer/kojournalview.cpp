/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$
//
// View of Journal entries

#include <qlayout.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kdebug.h>

#include <libkcal/calendar.h>

#include "journalentry.h"

#include "kojournalview.h"
#include "kojournalview.moc"

KOJournalView::KOJournalView(Calendar *calendar, QWidget *parent,
		       const char *name)
  : KOrg::BaseView(calendar, parent, name)
{
  mEntry = new JournalEntry(calendar,this);
  
  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->addWidget(mEntry);
}

KOJournalView::~KOJournalView()
{
}

int KOJournalView::currentDateCount()
{
  return 0;
}

QPtrList<Incidence> KOJournalView::getSelected()
{
  QPtrList<Incidence> eventList;

  return eventList;
}

void KOJournalView::updateView()
{
  kdDebug() << "KOJournalView::updateView() does nothing" << endl;
}

void KOJournalView::flushView()
{
  mEntry->flushEntry();
}

void KOJournalView::selectDates(const QDateList dateList)
{
//  kdDebug() << "KOJournalView::selectDates()" << endl;

  QDateList dates = dateList;

  if (dateList.count() == 0) {
    kdDebug() << "KOJournalView::selectDates() called with empty list." << endl;
    return;
  }
  
  QDate date = *dates.first();

  mEntry->setDate(date);

  Journal *j = mCalendar->journal(date);
  if (j) mEntry->setJournal(j);
  else mEntry->clear();
  
//  emit eventsSelected(false);
}

void KOJournalView::selectEvents(QPtrList<Event> eventList)
{
  // After new creation of list view no events are selected.
//  emit eventsSelected(false);
}

void KOJournalView::changeEventDisplay(Event *event, int action)
{
  updateView();
}
