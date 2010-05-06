/*
  This file is part of the kcal library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>
  Copyright (c) 2010 Laurent Montel <montel@kde.org>

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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the DndFactory class.

  @brief
  vCalendar/iCalendar Drag-and-Drop object factory.

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "dndfactory.h"
#include "calendaradaptor.h"

#include <kcal/calendarlocal.h>
#include <kcal/dndfactory.h>
#include <kcal/vcaldrag.h>
#include <kcal/icaldrag.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QDropEvent>
#include <QtGui/QPixmap>

using namespace Akonadi;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Akonadi::DndFactory::Private
{
  public:
  Private( CalendarAdaptor *cal, bool deleteCalendar )
    : mDeleteCalendar( deleteCalendar ), mCalendar( cal ),
      mDndFactory( new KCal::DndFactory( cal ) )
    {}

  ~Private() {
    delete mDndFactory;
  }

  bool mDeleteCalendar;
  CalendarAdaptor *mCalendar;
  KCal::DndFactory *mDndFactory;

};
//@endcond
namespace Akonadi {
DndFactory::DndFactory( CalendarAdaptor *cal, bool deleteCalendarHere )
  : d( new Akonadi::DndFactory::Private ( cal, deleteCalendarHere ) )
{
}

DndFactory::~DndFactory()
{
  delete d;
}

QMimeData *DndFactory::createMimeData()
{
  return d->mDndFactory->createMimeData();
}

QDrag *DndFactory::createDrag( QWidget *owner )
{
  return d->mDndFactory->createDrag( owner );
}

QMimeData *DndFactory::createMimeData( Incidence *incidence )
{
  return d->mDndFactory->createMimeData( incidence );
}

QDrag *DndFactory::createDrag( Incidence *incidence, QWidget *owner )
{
  return d->mDndFactory->createDrag( incidence, owner );
}

KCal::Calendar *DndFactory::createDropCalendar( const QMimeData *md )
{
  return d->mDndFactory->createDropCalendar( md );
}

/* static */
KCal::Calendar *DndFactory::createDropCalendar( const QMimeData *md, const KDateTime::Spec &timeSpec )
{
 KCal::Calendar *cal = new KCal::CalendarLocal( timeSpec );

 if ( ICalDrag::fromMimeData( md, cal ) ||
      VCalDrag::fromMimeData( md, cal ) ) {
   return cal;
 }
 delete cal;
 return 0;
}

KCal::Calendar *DndFactory::createDropCalendar( QDropEvent *de )
{
  return d->mDndFactory->createDropCalendar( de );
}

KCal::Event *DndFactory::createDropEvent( const QMimeData *md )
{
  return d->mDndFactory->createDropEvent( md );
}

KCal::Event *DndFactory::createDropEvent( QDropEvent *de )
{
  return d->mDndFactory->createDropEvent( de );
}

KCal::Todo *DndFactory::createDropTodo( const QMimeData *md )
{
  return d->mDndFactory->createDropTodo( md );
}

KCal::Todo *DndFactory::createDropTodo( QDropEvent *de )
{
  return d->mDndFactory->createDropTodo( de );
}

void DndFactory::cutIncidence( const Akonadi::Item &selectedInc )
{
  if ( copyIncidence( selectedInc ) ) {
    // Don't call the kcal's version, call deleteIncidence( Item, )
    // which creates a ItemDeleteJob.
    d->mCalendar->deleteIncidence( selectedInc, d->mDeleteCalendar );
  }
}

bool DndFactory::copyIncidence( const Akonadi::Item &item )
{
  if ( Akonadi::hasIncidence( item ) ) {
    return d->mDndFactory->copyIncidence( Akonadi::incidence( item ).get() );
  } else {
    return false;
  }
}

Incidence *DndFactory::pasteIncidence( const QDate &newDate, const QTime *newTime )
{
  return d->mDndFactory->pasteIncidence( newDate, newTime );
}

} // namespace
