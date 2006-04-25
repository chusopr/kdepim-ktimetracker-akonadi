/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <qstring.h>
#include <qnamespace.h>
#include <qlayout.h>
#include <qtimer.h>
#include <q3frame.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QWheelEvent>
#include <QGridLayout>
#include <QEvent>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include "koglobals.h"
#include "koprefs.h"
#include "kodaymatrix.h"

#include <kcalendarsystem.h>

#include "navigatorbar.h"

#include "kdatenavigator.h"

KDateNavigator::KDateNavigator( QWidget *parent )
  : QFrame( parent ), mBaseDate( 1970, 1, 1 )
{
  QGridLayout* topLayout = new QGridLayout( this );

  mNavigatorBar = new NavigatorBar( this );
  topLayout->addWidget( mNavigatorBar, 0, 0, 1, 8 );

  connect( mNavigatorBar, SIGNAL( goPrevYear() ), SIGNAL( goPrevYear() ) );
  connect( mNavigatorBar, SIGNAL( goPrevMonth() ), SIGNAL( goPrevMonth() ) );
  connect( mNavigatorBar, SIGNAL( goNextMonth() ), SIGNAL( goNextMonth() ) );
  connect( mNavigatorBar, SIGNAL( goNextYear() ), SIGNAL( goNextYear() ) );
  connect( mNavigatorBar, SIGNAL( goMonth( int ) ), SIGNAL( goMonth( int ) ) );

  int i;
  QString generalFont = KGlobalSettings::generalFont().family();

  // Set up the heading fields.
  for( i = 0; i < 7; i++ ) {
    headings[i] = new QLabel( this );
    headings[i]->setFont( QFont( generalFont, 10, QFont::Bold ) );
    headings[i]->setAlignment( Qt::AlignCenter );

    topLayout->addWidget( headings[i], 1, i + 1 );
  }

  // Create the weeknumber labels
  for( i = 0; i < 6; i++ ) {
    weeknos[i] = new QLabel( this );
    weeknos[i]->setAlignment( Qt::AlignCenter );
    weeknos[i]->setFont( QFont( generalFont, 10 ) );
    weeknos[i]->installEventFilter( this );

    topLayout->addWidget( weeknos[i], i + 2, 0 );
  }

  mDayMatrix = new KODayMatrix( this );
  mDayMatrix->setObjectName( "KDateNavigator::dayMatrix" );

  connect( mDayMatrix, SIGNAL( selected( const KCal::DateList & ) ),
           SIGNAL( datesSelected( const KCal::DateList & ) ) );

  connect( mDayMatrix, SIGNAL( incidenceDropped( Incidence *, const QDate & ) ),
           SIGNAL( incidenceDropped( Incidence *, const QDate & ) ) );
  connect( mDayMatrix, SIGNAL( incidenceDroppedMove( Incidence * , const QDate & ) ),
           SIGNAL( incidenceDroppedMove( Incidence *, const QDate & ) ) );


  topLayout->addWidget( mDayMatrix, 2, 1, 6, 7 );

  // read settings from configuration file.
  updateConfig();
}

KDateNavigator::~KDateNavigator()
{
}

void KDateNavigator::setCalendar( Calendar *cal )
{
  mDayMatrix->setCalendar( cal );
}

void KDateNavigator::setBaseDate( const QDate &date )
{
  if ( date != mBaseDate ) {
    mBaseDate = date;

    updateDates();
    updateView();

    // Use the base date to show the monthname and year in the header
    KCal::DateList dates;
    dates.append( date );
    mNavigatorBar->selectDates( dates );

    repaint();
    mDayMatrix->repaint();
  }
}

QSizePolicy KDateNavigator::sizePolicy () const
{
  return QSizePolicy( QSizePolicy::MinimumExpanding,
                      QSizePolicy::MinimumExpanding );
}

void KDateNavigator::updateToday()
{
  mDayMatrix->recalculateToday();
  mDayMatrix->repaint();
}
QDate KDateNavigator::startDate() const
{
  // Find the first day of the week of the current month.
  QDate dayone( mBaseDate.year(), mBaseDate.month(), mBaseDate.day() );
  int d2 = KOGlobals::self()->calendarSystem()->day( dayone );
  //int di = d1 - d2 + 1;
  dayone = dayone.addDays( -d2 + 1 );


  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();
  int m_fstDayOfWkCalsys = calsys->dayOfWeek( dayone );

  // If month begins on Monday and Monday is first day of week,
  // month should begin on second line. Sunday doesn't have this problem.
  int nextLine = ( ( m_fstDayOfWkCalsys == 1) &&
                   ( KGlobal::locale()->weekStartDay() == 1 ) ) ? 7 : 0;

  // update the matrix dates
  int index = ( KGlobal::locale()->weekStartDay() == 1 ? 1 : 0 ) -
              m_fstDayOfWkCalsys - nextLine;

  dayone = dayone.addDays( index );

  return dayone;
}
QDate KDateNavigator::endDate() const
{
  return startDate().addDays( 6*7 );
}

void KDateNavigator::updateDates()
{
// kDebug(5850) << "KDateNavigator::updateDates(), this=" << this << endl;
  QDate dayone = startDate();

  mDayMatrix->updateView( dayone );

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();

  // set the week numbers.
  for( int i = 0; i < 6; i++ ) {
    // Use QDate's weekNumber method to determine the week number!
    QDate dtStart = mDayMatrix->getDate( i * 7 );
    QDate dtEnd = mDayMatrix->getDate( ( i + 1 ) * 7 - 1 );
    int weeknumstart = calsys->weekNumber( dtStart );
    int weeknumend = calsys->weekNumber( dtEnd );
    QString weeknum;

    if ( weeknumstart != weeknumend ) {
      weeknum = i18nc("start/end week number of line in date picker", "%1/%2",
                  weeknumstart, weeknumend );
    } else {
      weeknum.setNum( weeknumstart );
    }
    weeknos[i]->setText( weeknum );
  }

// each updateDates is followed by an updateView -> repaint is issued there !
//  mDayMatrix->repaint();
}

void KDateNavigator::updateDayMatrix()
{
  mDayMatrix->updateView();
  mDayMatrix->repaint();
}


void KDateNavigator::updateView()
{
//   kDebug(5850) << "KDateNavigator::updateView(), view " << this << endl;
  updateDayMatrix();
  repaint();
}

void KDateNavigator::updateConfig()
{
  int day;
  for( int i = 0; i < 7; i++ ) {
    // take the first letter of the day name to be the abbreviation
    if ( KGlobal::locale()->weekStartDay() == 1 ) {
      day = i + 1;
    } else {
      if ( i == 0 ) day = 7;
      else day = i;
    }
    QString dayName = KOGlobals::self()->calendarSystem()->weekDayName( day,
                                                                        true );
    if ( KOPrefs::instance()->mCompactDialogs ) dayName = dayName.left( 1 );
    headings[i]->setText( dayName );
  }

  // FIXME: Use actual config setting here
//  setShowWeekNums( true );
}

void KDateNavigator::setShowWeekNums( bool enabled )
{
  for( int i = 0; i < 6; i++ ) {
    if( enabled )
      weeknos[i]->show();
    else
      weeknos[i]->hide();
  }
}

void KDateNavigator::selectDates( const DateList &dateList )
{
  if ( dateList.count() > 0 ) {
    mSelectedDates = dateList;

    updateDates();

    mDayMatrix->setSelectedDaysFrom( *( dateList.begin() ),
                                     *( --dateList.end() ) );

    updateView();
  }
}

void KDateNavigator::wheelEvent ( QWheelEvent *e )
{
  if( e->delta() > 0 ) emit goPrevious();
  else emit goNext();

  e->accept();
}

bool KDateNavigator::eventFilter ( QObject *o, QEvent *e )
{
  if ( e->type() == QEvent::MouseButtonPress ) {
    int i;
    for( i = 0; i < 6; ++i ) {
      if ( o == weeknos[ i ] ) {
        QDate weekstart = mDayMatrix->getDate( i * 7 );
        emit weekClicked( weekstart );
        break;
      }
    }
    return true;
  } else {
    return false;
  }
}

#include "kdatenavigator.moc"
