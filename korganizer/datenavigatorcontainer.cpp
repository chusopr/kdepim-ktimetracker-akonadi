/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kdebug.h>
#include <klocale.h>

#include "koglobals.h"

#include <kcalendarsystem.h>

#include "datenavigatorcontainer.h"

DateNavigatorContainer::DateNavigatorContainer( QWidget *parent,
                                                const char *name )
  : QWidget( parent, name ), mCalendar( 0 ), 
    mHorizontalCount( 1 ), mVerticalCount( 1 )
{
  mExtraViews.setAutoDelete( true );

  mNavigatorView = new KDateNavigator( this, name );

  connectNavigatorView( mNavigatorView );
}

DateNavigatorContainer::~DateNavigatorContainer()
{
}

void DateNavigatorContainer::connectNavigatorView( KDateNavigator *v )
{
  connect( v, SIGNAL( datesSelected( const KCal::DateList & ) ),
           SIGNAL( datesSelected( const KCal::DateList & ) ) );
  connect( v, SIGNAL( incidenceDropped( Incidence * ) ),
           SIGNAL( incidenceDropped( Incidence * ) ) );
  connect( v, SIGNAL( incidenceDroppedMove( Incidence *, Incidence * ) ),
           SIGNAL( incidenceDroppedMove( Incidence *, Incidence * ) ) );
  connect( v, SIGNAL( weekClicked( const QDate & ) ),
           SIGNAL( weekClicked( const QDate & ) ) );

  connect( v, SIGNAL( goPrevious() ), SIGNAL( goPrevious() ) );
  connect( v, SIGNAL( goNext() ), SIGNAL( goNext() ) );

  connect( v, SIGNAL( goNextMonth() ), SIGNAL( goNextMonth() ) );
  connect( v, SIGNAL( goPrevMonth() ), SIGNAL( goPrevMonth() ) );
  connect( v, SIGNAL( goNextYear() ), SIGNAL( goNextYear() ) );
  connect( v, SIGNAL( goPrevYear() ), SIGNAL( goPrevYear() ) );

  connect( v, SIGNAL( goMonth( int ) ), SIGNAL( goMonth( int ) ) );
}

void DateNavigatorContainer::setCalendar( Calendar *cal )
{
  mCalendar = cal;
  mNavigatorView->setCalendar( cal );
  KDateNavigator *n;
  for( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->setCalendar( cal );
  }
}

void DateNavigatorContainer::updateDayMatrix()
{
  mNavigatorView->updateDayMatrix();
  KDateNavigator *n;
  for( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->updateDayMatrix();
  }
}

void DateNavigatorContainer::updateToday()
{
  mNavigatorView->updateToday();
  KDateNavigator *n;
  for( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->updateToday();
  }
}

void DateNavigatorContainer::updateView()
{
  mNavigatorView->updateView();
  KDateNavigator *n;
  for( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->updateView();
  }
}

void DateNavigatorContainer::updateConfig()
{
  mNavigatorView->updateConfig();
  KDateNavigator *n;
  for( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    n->updateConfig();
  }
}

void DateNavigatorContainer::selectDates( const DateList &dateList )
{
  mNavigatorView->selectDates( dateList );
  setBaseDates();
}

void DateNavigatorContainer::setBaseDates()
{
  KCal::DateList dateList = mNavigatorView->selectedDates();
  if ( dateList.isEmpty() ) {
    kdError() << "DateNavigatorContainer::selectDates() empty list." << endl;
  }
  QDate baseDate = dateList.first();
  KDateNavigator *n;
  for( n = mExtraViews.first(); n; n = mExtraViews.next() ) {
    baseDate = KOGlobals::self()->calendarSystem()->addMonths( baseDate, 1 );
    n->setBaseDate( baseDate );
  }
}

void DateNavigatorContainer::resizeEvent( QResizeEvent * )
{
#if 0
  kdDebug() << "DateNavigatorContainer::resizeEvent()" << endl;
  kdDebug() << "  CURRENT SIZE: " << size() << endl;
  kdDebug() << "  MINIMUM SIZEHINT: " << minimumSizeHint() << endl;
  kdDebug() << "  SIZEHINT: " << sizeHint() << endl;
  kdDebug() << "  MINIMUM SIZE: " << minimumSize() << endl;
#endif

  QSize minSize = mNavigatorView->minimumSizeHint();

//  kdDebug() << "  NAVIGATORVIEW minimumSizeHint: " << minSize << endl;

  int verticalCount = size().height() / minSize.height();
  int horizontalCount = size().width() / minSize.width();

  if ( horizontalCount != mHorizontalCount ||
       verticalCount != mVerticalCount ) {
    uint count = horizontalCount * verticalCount;
    if ( count == 0 ) return;

    while ( count > ( mExtraViews.count() + 1 ) ) {
      KDateNavigator *n = new KDateNavigator( this );
      mExtraViews.append( n );
      n->setCalendar( mCalendar );
      setBaseDates();
      connectNavigatorView( n );
      n->show();
    }

    while ( count < ( mExtraViews.count() + 1 ) ) {
      mExtraViews.removeLast();
    }

    mHorizontalCount = horizontalCount;
    mVerticalCount = verticalCount;
  }
  
  int height = size().height() / verticalCount;
  int width = size().width() / horizontalCount;
  
  mNavigatorView->setGeometry( 0, 0, width, height );
  for( uint i = 0; i < mExtraViews.count(); ++i ) {
    int x = ( i + 1 ) % horizontalCount;
    int y = ( i + 1 ) / horizontalCount;

    mExtraViews.at( i )->setGeometry( x * width, y * height, width, height );
  }
}

QSize DateNavigatorContainer::minimumSizeHint() const
{
  return mNavigatorView->minimumSizeHint();
}

QSize DateNavigatorContainer::sizeHint() const
{
  return mNavigatorView->sizeHint();
}

#include "datenavigatorcontainer.moc"
