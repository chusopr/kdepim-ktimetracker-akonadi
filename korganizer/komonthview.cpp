/*
  This file is part of KOrganizer.
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#include "komonthview.h"

#include "koprefs.h"
#include "koglobals.h"
#include "koeventpopupmenu.h"
#include "kohelper.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#include "kodecorationlabel.h"
#endif

#include <kcal/calfilter.h>
#include <kcal/calendar.h>
#include <kcal/incidenceformatter.h>
#include <kcal/calendarresources.h>

#include <kcalendarsystem.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kwordwrap.h>

#include <QFont>
#include <QFontMetrics>
#include <QPushButton>
#include <QPainter>
#include <QCursor>
#include <QLayout>
#include <QLabel>
#include <QGridLayout>
#include <QKeyEvent>
#include <QFrame>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QMouseEvent>

#include "komonthview.moc"

using namespace KOrg;

//--------------------------------------------------------------------------

KNoScrollListBox::KNoScrollListBox( QWidget *parent )
  : QListWidget( parent )
{
  QPalette pal = palette();
  pal.setColor( QPalette::Foreground, KOPrefs::instance()->monthGridBackgroundColor().dark( 150 ) );
  pal.setColor( QPalette::Base, KOPrefs::instance()->monthGridBackgroundColor() );
  setPalette( pal );
}

void KNoScrollListBox::setBackground( bool primary, bool workDay )
{
  QColor color;
  if ( workDay ) {
    color = KOPrefs::instance()->monthGridWorkHoursBackgroundColor();
  } else {
    color = KOPrefs::instance()->monthGridBackgroundColor();
  }

  QPalette pal = palette();
  if ( primary ) {
    pal.setColor( QPalette::Base, color );
  } else {
    pal.setColor( QPalette::Base, color.dark( 115 ) );
  }
  setPalette( pal );
}

MonthViewItem::MonthViewItem( Incidence *incidence, const KDateTime &dt, const QString & s )
  : QListWidgetItem()
{
  setText( s );

  mIncidence = incidence;
  mDateTime = dt;

  mEventPixmap     = KOGlobals::self()->smallIcon( "view-calendar-day" );
  mTodoPixmap      = KOGlobals::self()->smallIcon( "view-calendar-tasks" );
  mTodoDonePixmap  = KOGlobals::self()->smallIcon( "checkedbox" );
  mJournalPixmap   = KOGlobals::self()->smallIcon( "view-calendar-journal" );
  mAlarmPixmap     = KOGlobals::self()->smallIcon( "bell" );
  mRecurPixmap     = KOGlobals::self()->smallIcon( "recur" );
  mReplyPixmap     = KOGlobals::self()->smallIcon( "mail-reply-sender" );
  mHolidayPixmap   = KOGlobals::self()->smallIcon( "emblem-favorite" );

  mResourceColor = QColor();
  mEvent = false;
  mTodo = false;
  mTodoDone = false;
  mJournal = false;
  mRecur = false;
  mAlarm = false;
  mReply = false;
  mHoliday = false;

  QString tipText;
  if ( incidence ) {
    tipText = IncidenceFormatter::toolTipString( incidence );
  } else {
    tipText = s;
  }
  if ( !tipText.isEmpty() ) {
    setToolTip( tipText );
  }
}

void MonthViewItem::drawIt()
{
  // Icon
  if ( mEvent ) {
    setIcon( mEventPixmap );
  }
  if ( mTodo ) {
    setIcon( mTodoPixmap );
  }
  if ( mTodoDone ) {
    setIcon( mTodoDonePixmap );
  }
  if ( mRecur ) {
    setIcon( mRecurPixmap );
  }
  if ( mAlarm ) {
    setIcon( mAlarmPixmap );
  }
  if ( mReply ) {
    setIcon( mReplyPixmap );
  }
  if ( mHoliday ) {
    setIcon( mHolidayPixmap );
  }

  // Background color
  QColor bgColor =
    palette().color( QPalette::Normal,
                     isSelected() ? QPalette::Highlight : QPalette::Background );
  if ( KOPrefs::instance()->monthViewUsesResourceColor() && mResourceColor.isValid() ) {
    setBackground( QBrush( mResourceColor ) );
  }
  if ( KOPrefs::instance()->monthViewUsesCategoryColor() ) {
      setBackground( QBrush( bgColor ) );
  }

  setForeground( QBrush( getTextColor( bgColor ) ) );
}

int MonthViewItem::height( const QListWidget *lw ) const
{
  return qMax( qMax( mRecurPixmap.height(), mReplyPixmap.height() ),
               qMax( mAlarmPixmap.height(), lw->fontMetrics().lineSpacing() + 1 ) );
}

int MonthViewItem::width( const QListWidget *lw ) const
{
  int x = 3;
  if( mRecur ) {
    x += mRecurPixmap.width()+2;
  }
  if( mAlarm ) {
    x += mAlarmPixmap.width()+2;
  }
  if( mReply ) {
    x += mReplyPixmap.width()+2;
  }

  return x + lw->fontMetrics().boundingRect( text() ).width() + 1;
}

MonthViewCell::MonthViewCell( KOMonthView *parent )
  : QWidget( parent ),
    mMonthView( parent ), mPrimary( false ), mHoliday( false )
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setMargin( 0 );
  topLayout->setSpacing( 0 );

/* TODO: Add code for the loading of the cell decorations around here? */
  mLabel = new QLabel( this );
  mLabel->setFrameStyle( QFrame::Box | QFrame::Raised );
  mLabel->setLineWidth( 1 );
  mLabel->setAlignment( Qt::AlignCenter );

  mItemList = new KNoScrollListBox( this );
  mItemList->setMinimumSize( 10, 10 );
  mItemList->setFrameStyle( QFrame::Panel | QFrame::Plain );
  mItemList->setLineWidth( 1 );
  mItemList->setContextMenuPolicy( Qt::CustomContextMenu );
  topLayout->addWidget( mItemList, 0, 0 );

  mLabel->raise();

  mStandardPalette = palette();

  enableScrollBars( false );

  updateConfig();

  connect( mItemList, SIGNAL(itemDoubleClicked( QListWidgetItem *)),
           SLOT(defaultAction(QListWidgetItem *)) );
  connect( mItemList, SIGNAL(customContextMenuRequested(const QPoint &)),
           SLOT(contextMenu(const QPoint &)) );
  connect( mItemList, SIGNAL(itemClicked(QListWidgetItem *)), SLOT(select()) );
}

void MonthViewCell::setDate( const QDate &date )
{
  mDate = date;

  setFrameWidth();

  QString text;
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  if ( calSys->day( date ) == 1 ) {
    text = i18nc( "'Month day' for month view cells", "%1 %2",
                  calSys->monthName( date, KCalendarSystem::ShortName ),
                  calSys->day( mDate ) );
    QFontMetrics fm( mLabel->font() );
    mLabel->resize( mLabelSize + QSize( fm.width( text ), 0 ) );
  } else {
    mLabel->resize( mLabelSize );
    text = QString::number( calSys->day( mDate ) );
  }
  mLabel->setText( text );
/* TODO: Add code for the loading of the decorations around here */

  resizeEvent( 0 );
}

QDate MonthViewCell::date() const
{
  return mDate;
}

void MonthViewCell::setFrameWidth()
{
  // show current day with a thicker frame
  if ( mDate == QDate::currentDate() ) {
    mItemList->setLineWidth( 3 );
  } else {
    mItemList->setLineWidth( 1 );
  }
}

void MonthViewCell::setPrimary( bool primary )
{
  mPrimary = primary;
  if ( mPrimary ) {
    mLabel->setBackgroundRole( QPalette::Base );
  } else {
    mLabel->setBackgroundRole( QPalette::Background );
  }
  mItemList->setBackground( mPrimary, KOGlobals::self()->isWorkDay( mDate ) );
}

bool MonthViewCell::isPrimary() const
{
  return mPrimary;
}

void MonthViewCell::setHoliday( bool holiday )
{
  mHoliday = holiday;

  if ( holiday ) {
    setPalette( mHolidayPalette );
  } else {
    setPalette( mStandardPalette );
  }
}

void MonthViewCell::setHolidayString( const QString &holiday )
{
  mHolidayString = holiday;
}

void MonthViewCell::updateCell()
{
  setFrameWidth();

  if ( mDate == QDate::currentDate() ) {
    setPalette( mTodayPalette );

    QPalette pal = mItemList->palette();
    pal.setColor( QPalette::Foreground, KOPrefs::instance()->monthGridHighlightColor() );
    mItemList->setPalette( pal );
  } else {
    if ( mHoliday ) {
      setPalette( mHolidayPalette );
    } else {
      setPalette( mStandardPalette );
    }

    QPalette pal = mItemList->palette();
    pal.setColor( QPalette::Foreground,
                  KOPrefs::instance()->monthGridBackgroundColor().dark( 150 ) );
    mItemList->setPalette( pal );
  }

  mItemList->clear();

  if ( !mHolidayString.isEmpty() ) {
    MonthViewItem *item =
      new MonthViewItem( 0, KDateTime( mDate, KOPrefs::instance()->timeSpec() ), mHolidayString );
    item->setPalette( mHolidayPalette );
    item->setHoliday( true );
    item->drawIt();
    mItemList->addItem( item );
  }
}

class MonthViewCell::CreateItemVisitor
  : public IncidenceBase::Visitor
{
  public:
    CreateItemVisitor() : mItem(0) {}

    bool act( IncidenceBase *incidence, const QDate &date, const QPalette &stdPal, int multiDay )
    {
      mItem = 0;
      mDate = date;
      mStandardPalette = stdPal;
      mMultiDay = multiDay;
      return incidence->accept( *this );
    }
    MonthViewItem *item() const { return mItem; }

  protected:
    bool visit( Event *event ) {
      QString text;
      KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
      KDateTime dt( mDate, timeSpec );
      // take the time 0:00 into account, which is non-inclusive
      QDate dtEnd =
        event->dtEnd().toTimeSpec( timeSpec ).addSecs( event->allDay() ? 0 : -1 ).date();
      KDateTime dtStart = event->dtStart().toTimeSpec( timeSpec );
      int length = dtStart.date().daysTo( dtEnd );
      if ( event->isMultiDay() ) {
        if ( mDate == dtStart.date() ||
           ( mMultiDay == 0 && event->recursOn( mDate, timeSpec ) ) ) {
          text = "(-- " + event->summary();
          dt = event->dtStart();
        } else if ( !event->recurs() && mDate == dtEnd ||
                    // last day of a recurring multi-day event?
                    ( mMultiDay == length &&
                      event->recursOn( mDate.addDays( -length ), timeSpec ) ) ) {
          text = event->summary() + " --)";
        } else if ( !( event->dtStart().date().daysTo( mDate ) % 7 ) && length > 7 ) {
          text = "-- " + event->summary() + " --";
        } else {
          text = "----------------";
        }
      } else {
        if ( event->allDay() ) {
          text = event->summary();
        } else {
          QTime startTime = event->dtStart().toTimeSpec( timeSpec ).time();
          text = KGlobal::locale()->formatTime(startTime);
          dt.setTime( startTime );
          text += ' ' + event->summary();
        }
      }

      mItem = new MonthViewItem( event, dt, text );
      if ( KOPrefs::instance()->monthViewUsesCategoryColor() ) {
        QStringList categories = event->categories();
        QString cat;
        if ( !categories.isEmpty() ) {
          cat = categories.first();
        }
        if ( cat.isEmpty() ) {
          mItem->setPalette(
            QPalette( KOPrefs::instance()->monthCalendarItemsEventsBackgroundColor(),
                      KOPrefs::instance()->monthCalendarItemsEventsBackgroundColor() ) );
        } else {
          mItem->setPalette(
            QPalette( KOPrefs::instance()->categoryColor( cat ),
                      KOPrefs::instance()->categoryColor( cat ) ) );
        }
      } else {
        mItem->setPalette( mStandardPalette );
      }
      mItem->setEvent( true );

      Attendee *me = event->attendeeByMails( KOPrefs::instance()->allEmails() );
      if ( me != 0 ) {
        mItem->setReply( me->status() == Attendee::NeedsAction && me->RSVP() );
      } else {
        mItem->setReply( false );
      }
      mItem->drawIt();
      return true;
    }
    bool visit( Todo *todo ) {
      QString text;
      if ( !KOPrefs::instance()->showAllDayTodo() ) {
        return false;
      }
      KDateTime dt( mDate, KOPrefs::instance()->timeSpec() );
      if ( todo->hasDueDate() && !todo->allDay() ) {
        QTime dueTime = todo->dtDue().toTimeSpec( KOPrefs::instance()->timeSpec() ).time();
        text += KGlobal::locale()->formatTime( dueTime );
        text += ' ';
        dt.setTime( dueTime );
      }
      text += todo->summary();

      mItem = new MonthViewItem( todo, dt, text );
      if ( todo->recurs() ) {
        mDate < todo->dtDue().date() ? mItem->setTodoDone( true ) : mItem->setTodo( true );
      } else {
        todo->isCompleted() ? mItem->setTodoDone( true ) : mItem->setTodo( true );
      }
      mItem->setPalette( mStandardPalette );
      mItem->drawIt();
      return true;
    }
  protected:
    MonthViewItem *mItem;
    QDate mDate;
    QPalette mStandardPalette;
    int mMultiDay;
};

void MonthViewCell::addIncidence( Incidence *incidence, int multiDay )
{
  CreateItemVisitor v;

  if ( v.act( incidence, mDate, mStandardPalette, multiDay ) ) {
    MonthViewItem *item = v.item();
    if ( item ) {
      item->setAlarm( incidence->isAlarmEnabled() );
      item->setRecur( incidence->recurrenceType() );

      QColor resourceColor = KOHelper::resourceColor( mCalendar, incidence );
      if ( !resourceColor.isValid() ) {
        resourceColor = KOPrefs::instance()->monthCalendarItemsEventsBackgroundColor();
      }
      item->setResourceColor( resourceColor );

      int i = 0;
      int pos = -1;
      KDateTime dt( item->incidenceDateTime() );

      while ( i < mItemList->count() && pos < 0 ) {
        QListWidgetItem *item = mItemList->item( i );
        MonthViewItem *mvitem = dynamic_cast<MonthViewItem*>( item );
        if ( mvitem && mvitem->incidenceDateTime() > dt ) {
          pos = i;
        }
        ++i;
      }
      if ( pos >= 0 ) {
        // insert chronologically
        mItemList->insertItem( pos, item );
      } else {
        // append to end of list
        mItemList->addItem( item );
      }
    }
  }
}

void MonthViewCell::removeIncidence( Incidence *incidence )
{
  for ( int i = 0; i < mItemList->count(); ++i ) {
    MonthViewItem *item = static_cast<MonthViewItem *>( mItemList->item( i ) );
    if ( item && item->incidence() && item->incidence()->uid() == incidence->uid() ) {
      mItemList->removeItemWidget( mItemList->item( i ) );
      break;
    }
  }
}

void MonthViewCell::updateConfig()
{
  setFont( KOPrefs::instance()->mMonthViewFont );

  QFontMetrics fm( font() );
  mLabelSize = fm.size( 0, "30" ) +
               QSize( mLabel->frameWidth() * 2, mLabel->frameWidth() * 2 ) +
               QSize( 2, 2 );
//  mStandardPalette = mOriginalPalette;
  QColor bg = mStandardPalette.color( QPalette::Active, QPalette::Background );
  int h, s, v;
  bg.getHsv( &h, &s, &v );
  if ( date().month() % 2 == 0 ) {
    if ( v < 128 ) {
      bg = bg.light( 125 );
    } else {
      bg = bg.dark( 125 );
    }
  }
  QPalette pal;
  pal.setColor( backgroundRole(), bg );
  setPalette( pal );

  mHolidayPalette = mStandardPalette;
  mHolidayPalette.setColor( QPalette::Foreground,
                            KOPrefs::instance()->monthHolidaysBackgroundColor() );
  mHolidayPalette.setColor( QPalette::Text,
                            KOPrefs::instance()->monthHolidaysBackgroundColor() );
  mTodayPalette = mStandardPalette;
  mTodayPalette.setColor( QPalette::Foreground,
                          KOPrefs::instance()->monthGridHighlightColor() );
  mTodayPalette.setColor( QPalette::Text,
                          KOPrefs::instance()->monthGridHighlightColor() );
  updateCell();

  mItemList->setBackground( mPrimary, KOGlobals::self()->isWorkDay( mDate ) );
}

void MonthViewCell::enableScrollBars( bool enabled )
{
  if ( enabled ) {
    mItemList->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    mItemList->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  } else {
    mItemList->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    mItemList->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  }
}

Incidence *MonthViewCell::selectedIncidence()
{
  int index = mItemList->currentRow();
  if ( index < 0 ) {
    return 0;
  }

  MonthViewItem *item = static_cast<MonthViewItem *>( mItemList->item( index ) );

  if ( !item ) {
    return 0;
  }

  return item->incidence();
}

QDate MonthViewCell::selectedIncidenceDate()
{
  QDate qd;
  int index = mItemList->currentRow();
  if ( index < 0 ) {
    return qd;
  }

  MonthViewItem *item = static_cast<MonthViewItem *>( mItemList->item( index ) );

  if ( !item ) {
    return qd;
  }

  return item->incidenceDateTime().date();
}

void MonthViewCell::select()
{
  // setSelectedCell will deselect currently selected cells
  mMonthView->setSelectedCell( this );

  if( KOPrefs::instance()->enableMonthScroll() ) {
    enableScrollBars( true );
  }

  // don't mess up the cell when it represents today
  if( mDate != QDate::currentDate() ) {
    mItemList->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    mItemList->setLineWidth( 3 );
  }
}

void MonthViewCell::deselect()
{
  mItemList->clearSelection();
  mItemList->setFrameStyle( QFrame::Plain | QFrame::Panel );
  setFrameWidth();

  enableScrollBars( false );
}

void MonthViewCell::resizeEvent ( QResizeEvent * )
{
  mLabel->move( width() - mLabel->width(), height() - mLabel->height() );
/* TODO: Add code to move cell decorations around here */
}

void MonthViewCell::defaultAction( QListWidgetItem *item )
{
  select();

  if ( !item ) {
    emit newEventSignal( date() );
  } else {
    MonthViewItem *eventItem = static_cast<MonthViewItem *>( item );
    Incidence *incidence = eventItem->incidence();
    if ( incidence ) {
      mMonthView->defaultAction( incidence );
    }
  }
}

void MonthViewCell::contextMenu( const QPoint &pos )
{
  select();

  QListWidgetItem *item = mItemList->itemAt( pos );
  if ( item ) {
    MonthViewItem *eventItem = static_cast<MonthViewItem *>( item );
    Incidence *incidence = eventItem->incidence();
    if ( incidence ) {
      mMonthView->showEventContextMenu( incidence, date() );
    }
  } else {
    mMonthView->showGeneralContextMenu();
  }
}

KOMonthView::KOMonthView( Calendar *calendar, QWidget *parent )
  : KOEventView( calendar, parent ),
    mDaysPerWeek( 7 ), mNumWeeks( 6 ), mNumCells( mDaysPerWeek * mNumWeeks ),
    mShortDayLabels( false ), mWidthLongDayLabel( 0 ), mSelectedCell( 0 )
{
  QGridLayout *dayLayout = new QGridLayout( this );
  dayLayout->setSpacing( 0 );
  dayLayout->setMargin( 0 );

  QFont bfont = font();
  bfont.setBold( true );

  QFont mfont = bfont;
  mfont.setPointSize( 20 );

  // Top box with month name and decorations
  mTopBox = new KHBox( this );
  mTitle = new QLabel( mTopBox );
  mTitle->setFont( mfont );
  mTitle->setLineWidth( 0 );
  mTitle->setFrameStyle( QFrame::Plain );
  mDecorationsFrame = 0;

  dayLayout->addWidget( mTopBox, 0, 0, 1, -1, Qt::AlignCenter );

  // Create the day of the week labels (Sun, Mon, etc)
  mDayLabels.resize( mDaysPerWeek );
  int i;
  for ( i = 0; i < mDaysPerWeek; i++ ) {
    QLabel *label = new QLabel( this );
    label->setFont( bfont );
    label->setFrameStyle( QFrame::Panel | QFrame::Raised );
    label->setLineWidth( 1 );
    label->setAlignment( Qt::AlignCenter );

    mDayLabels[i] = label;

    dayLayout->addWidget( label, 1, i );
    dayLayout->addItem( new QSpacerItem( 10, 0 ), 0, i );
    dayLayout->setColumnStretch( i, 1 );
  }

  mCells.resize( mNumCells );
  int row, col;
  for ( row = 0; row < mNumWeeks; ++row ) {
    for ( col = 0; col < mDaysPerWeek; ++col ) {
      MonthViewCell *cell = new MonthViewCell( this );
      cell->setCalendar( calendar );
      mCells[row * mDaysPerWeek + col] = cell;
      dayLayout->addWidget( cell, row + 2, col );

      connect( cell, SIGNAL( defaultAction( Incidence * ) ),
               SLOT( defaultAction( Incidence * ) ) );
      connect( cell, SIGNAL( newEventSignal( const QDate & ) ),
               SIGNAL( newEventSignal( const QDate & ) ) );
    }
    dayLayout->setRowStretch( row + 2, 1 );
  }

  mEventContextMenu = eventPopup();

  updateConfig();

  emit incidenceSelected( 0 );
}

KOMonthView::~KOMonthView()
{
  qDeleteAll( mCells );
  mCells.clear();
  delete mEventContextMenu;
}

int KOMonthView::maxDatesHint()
{
  return mNumCells;
}

int KOMonthView::currentDateCount()
{
  return mNumCells;
}

Incidence::List KOMonthView::selectedIncidences()
{
  Incidence::List selected;

  if ( mSelectedCell ) {
    Incidence *incidence = mSelectedCell->selectedIncidence();
    if ( incidence ) {
      selected.append( incidence );
    }
  }

  return selected;
}

DateList KOMonthView::selectedDates()
{
  DateList selected;

  if ( mSelectedCell ) {
    QDate qd = mSelectedCell->selectedIncidenceDate();
    if ( qd.isValid() ) {
      selected.append( qd );
    }
  }

  return selected;
}

bool KOMonthView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  if ( mSelectedCell ) {
    startDt.setDate( mSelectedCell->date() );
    endDt.setDate( mSelectedCell->date() );
    allDay = true;
    return true;
  }
  return false;
}

void KOMonthView::updateConfig()
{
  mWeekStartDay = KGlobal::locale()->weekStartDay();

  QFontMetrics fontmetric( mDayLabels[0]->font() );
  mWidthLongDayLabel = 0;

  for ( int i = 0; i < 7; ++i ) {
    int width = fontmetric.width( KOGlobals::self()->calendarSystem()->weekDayName( i + 1 ) );
    if ( width > mWidthLongDayLabel ) {
      mWidthLongDayLabel = width;
    }
  }

  updateDayLabels();

  for ( int i = 0; i < mCells.count(); ++i ) {
    mCells[i]->updateConfig();
  }
}

void KOMonthView::updateDayLabels()
{
  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();
  int currDay;
  for ( int i = 0; i < 7; i++ ) {
    currDay = i + mWeekStartDay;
    if ( currDay > 7 ) {
      currDay -= 7;
    }
    if ( mShortDayLabels ) {
      mDayLabels[i]->setText( calsys->weekDayName( currDay, KCalendarSystem::ShortDayName ) );
    } else {
      mDayLabels[i]->setText( calsys->weekDayName( currDay, KCalendarSystem::LongDayName ) );
    }
  }
}

void KOMonthView::showDates( const QDate &start, const QDate & )
{
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  mDateToCell.clear();

  // show first day of month on top for readability issues
  mStartDate = start.addDays( -start.day() + 1 );
  // correct begin of week
  int weekdayCol=( mStartDate.dayOfWeek() + 7 - mWeekStartDay ) % 7;
  mStartDate = mStartDate.addDays( -weekdayCol );

  mTitle->setText( i18nc( "monthname year", "%1 %2",
                          calSys->monthName( start ),
                          calSys->yearString( start ) ) );

  delete mDecorationsFrame;
  mDecorationsFrame = new QFrame( mTopBox );
  mDecorationsFrame->setLineWidth( 0 );
  mDecorationsFrame->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
#ifndef KORG_NOPLUGINS
  // Month decoration labels
  foreach ( QString decoName, KOPrefs::instance()->decorationsAtMonthViewTop() ) {
    if ( KOPrefs::instance()->selectedPlugins().contains( decoName ) ) {
      CalendarDecoration::Decoration *deco = KOCore::self()->loadCalendarDecoration( decoName );

      CalendarDecoration::Element::List elements;
      elements = deco->monthElements( start );
      if ( elements.count() > 0 ) {
        KHBox *decoHBox = new KHBox( mDecorationsFrame );
        decoHBox->setFrameShape( QFrame::StyledPanel );
        decoHBox->setMinimumWidth( 1 );

        foreach ( CalendarDecoration::Element *it, elements ) {
          kDebug(5850) << "adding Element " << it->id()
                       << " of Decoration " << deco->info()
                       << " to the top of the month view";
          KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
          label->setAlignment( Qt::AlignBottom );
        }
      }
    }
  }
#endif

  bool primary = false;
  int i;
  for ( i = 0; i < mCells.size(); ++i ) {
    QDate date = mStartDate.addDays( i );
    if ( calSys->day( date ) == 1 ) {
      primary = !primary;
    }

    mCells[i]->setDate( date );
    mDateToCell[date] = mCells[i];
    if ( date == start ) {
      mCells[i]->select();
    }
    mCells[i]->setPrimary( primary );

    bool isHoliday = ( calSys->dayOfWeek( date ) == calSys->weekDayOfPray() ||
                       !KOGlobals::self()->isWorkDay( date ) );
    mCells[i]->setHoliday( isHoliday );

    // add holiday, if present
    QStringList holidays( KOGlobals::self()->holiday( date ) );
    mCells[i]->setHolidayString( holidays.join(
                                   i18nc( "delimiter for joining holiday names", "," ) ) );
  }

  updateView();
}

void KOMonthView::showIncidences( const Incidence::List & )
{
  kDebug(5850) << "KOMonthView::showIncidences( const Incidence::List & ) is not implemented yet.";
}

class KOMonthView::GetDateVisitor : public IncidenceBase::Visitor
{
  public:
    GetDateVisitor() {}

    bool act( IncidenceBase *incidence )
    {
      return incidence->accept( *this );
    }
    KDateTime startDate() const { return mStartDate; }
    KDateTime endDate() const { return mEndDate; }

  protected:
    bool visit( Event *event ) {
      mStartDate = event->dtStart();
      mEndDate = event->dtEnd();
      return true;
    }
    bool visit( Todo *todo ) {
      if ( todo->hasDueDate() ) {
        mStartDate = todo->dtDue();
        mEndDate = todo->dtDue();
      } else {
        return false;
      }
      return true;
    }
    bool visit( Journal *journal ) {
      mStartDate = journal->dtStart();
      mEndDate = journal->dtStart();
      return true;
    }
  protected:
    KDateTime mStartDate;
    KDateTime mEndDate;
};

void KOMonthView::changeIncidenceDisplayAdded( Incidence *incidence )
{
  GetDateVisitor gdv;

  if ( !gdv.act( incidence ) ) {
    return;
  }

  bool allDay = incidence->allDay();

  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  if ( incidence->recurs() ) {
    for ( int i = 0; i < mCells.count(); ++i ) {
      if ( incidence->recursOn( mCells[i]->date(), timeSpec ) ) {
        // handle multiday events
        int length =
          gdv.startDate().date().daysTo( gdv.endDate().addSecs( allDay ? 0 : -1 ).date() );
        for ( int j = 0; j <= length && i+j < mCells.count(); ++j ) {
          mCells[i + j]->addIncidence( incidence, j );
        }
      }
    }
  } else {
    // addSecs(-1) is added to handle 0:00 cases (because it's non-inclusive according to rfc)
    if ( gdv.endDate().isValid() ) {
      QDate endDate = gdv.endDate().toTimeSpec( timeSpec ).addSecs( allDay ? 0 : -1 ).date();
      for ( QDate date = gdv.startDate().toTimeSpec( timeSpec ).date();
            date <= endDate; date = date.addDays( 1 ) ) {
        MonthViewCell *mvc = mDateToCell[ date ];
        if ( mvc ) {
          mvc->addIncidence( incidence );
        }
      }
    }
  }
}

void KOMonthView::changeIncidenceDisplay( Incidence *incidence, int action )
{
  switch ( action ) {
  case KOGlobals::INCIDENCEADDED:
    changeIncidenceDisplayAdded( incidence );
    break;
  case KOGlobals::INCIDENCEEDITED:
    for ( int i = 0; i < mCells.count(); i++ ) {
      mCells[i]->removeIncidence( incidence );
    }
    changeIncidenceDisplayAdded( incidence );
    break;
  case KOGlobals::INCIDENCEDELETED:
    for ( int i = 0; i < mCells.count(); i++ ) {
      mCells[i]->removeIncidence( incidence );
    }
    break;
  default:
    return;
  }
  updateView();
}

void KOMonthView::updateView()
{
  for ( int i = 0; i < mCells.count(); ++i ) {
    mCells[i]->updateCell();
  }

  Incidence::List incidences = calendar()->incidences();
  Incidence::List::ConstIterator it;

  for ( it = incidences.begin(); it != incidences.end(); ++it ) {
    changeIncidenceDisplayAdded( *it );
  }

  processSelectionChange();
}

void KOMonthView::resizeEvent( QResizeEvent * )
{
  // select the appropriate heading string size. E.g. "Wednesday" or "Wed".
  // note this only changes the text if the requested size crosses the
  // threshold between big enough to support the full name and not big
  // enough.
  if( mDayLabels[0]->width() < mWidthLongDayLabel ) {
    if ( !mShortDayLabels ) {
      mShortDayLabels = true;
      updateDayLabels();
    }
  } else {
    if ( mShortDayLabels ) {
      mShortDayLabels = false;
      updateDayLabels();
    }
  }
}

void KOMonthView::showEventContextMenu( Incidence *incidence, const QDate &qd )
{
  mEventContextMenu->showIncidencePopup( incidence, qd );
}

void KOMonthView::showGeneralContextMenu()
{
  showNewEventPopup();
}

void KOMonthView::setSelectedCell( MonthViewCell *cell )
{
  if ( mSelectedCell && cell != mSelectedCell ) {
    mSelectedCell->deselect();
  }

  mSelectedCell = cell;

  if ( !mSelectedCell ) {
    emit incidenceSelected( 0 );
  } else {
    emit incidenceSelected( mSelectedCell->selectedIncidence() );
  }
}

void KOMonthView::processSelectionChange()
{
  Incidence::List incidences = selectedIncidences();
  if ( incidences.count() > 0 ) {
    emit incidenceSelected( incidences.first() );
  } else {
    emit incidenceSelected( 0 );
  }
}

void KOMonthView::clearSelection()
{
  if ( mSelectedCell ) {
    mSelectedCell->deselect();
    mSelectedCell = 0;
  }
}
