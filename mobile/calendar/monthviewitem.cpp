/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#include "monthviewitem.h"

#include <calendarsupport/calendar.h>
#include <calendarviews/eventviews/month/monthview.h>

#include <KGlobal>
#include <KGlobalSettings>

using namespace EventViews;

MonthViewItem::MonthViewItem( QDeclarativeItem* parent )
  : DeclarativeAkonadiItem( parent )
  , mView( new MonthView() )
{
  const KDateTime start = KDateTime::currentLocalDateTime().addDays( -1 );
  const KDateTime end = KDateTime::currentLocalDateTime().addDays( 1 );
  
  // start with the oxygen palette (which is not necessarily the default on all platforms)
  QPalette pal = KGlobalSettings::createApplicationPalette( KGlobal::config() );

  mView->setPalette( pal );
  mView->setDateRange( start, end );
  
  setWidget( mView );
}

MonthViewItem::~MonthViewItem()
{
  delete mView;
}

QObject* MonthViewItem::calendar() const
{
  return mView->calendar();
}

void MonthViewItem::setCalendar( QObject* calendarObj )
{
  CalendarSupport::Calendar* cal = qobject_cast<CalendarSupport::Calendar*>( calendarObj );
  kDebug() << calendarObj << cal;
  if ( cal ) {
    mView->setCalendar( cal );
    mView->updateConfig();
  }
}
