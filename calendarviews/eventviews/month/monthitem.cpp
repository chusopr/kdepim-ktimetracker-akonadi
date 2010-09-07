/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

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

#include "monthitem.h"
#include "helper.h"
#include "prefs.h"
#include "prefs_base.h" // Ugly, but needed for the Enums
#include "monthgraphicsitems.h"
#include "monthscene.h"
#include "monthview.h"

#include <libkdepim/pimmessagebox.h>

#include <calendarsupport/calendar.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <kcalcore/incidence.h>
#include <kcalutils/incidenceformatter.h>
#include <kcalcore/todo.h>

using namespace EventViews;
using namespace KCalUtils;

//-------------------------------------------------------------
MonthItem::MonthItem( MonthScene *monthScene )
  : mMonthScene( monthScene ),
    mSelected( false ),
    mMoving( false ),
    mResizing( false )
{
}

MonthItem::~MonthItem()
{
  deleteAll();
}

void MonthItem::deleteAll()
{
  qDeleteAll( mMonthGraphicsItemList );
  mMonthGraphicsItemList.clear();
}

QWidget *MonthItem::parentWidget() const
{
  return mMonthScene ? mMonthScene->monthView() : 0;
}

void MonthItem::updateMonthGraphicsItems()
{
  // Remove all items
  qDeleteAll( mMonthGraphicsItemList );
  mMonthGraphicsItemList.clear();

  // For each row of the month view, create an item to build the whole
  // MonthItem's MonthGraphicsItems.
  for ( QDate d = mMonthScene->mMonthView->actualStartDateTime().date();
        d < mMonthScene->mMonthView->actualEndDateTime().date(); d = d.addDays( 7 ) ) {
    QDate end = d.addDays( 6 );

    int span;
    QDate start;
    if ( startDate() <= d && endDate() >= end ) { // MonthItem takes the whole line
      span = 6;
      start = d;
    } else if ( startDate() >= d && endDate() <= end ) { // starts and ends on this line
      start = startDate();
      span = daySpan();
    } else if ( d <= endDate() && endDate() <= end ) { // MonthItem ends on this line
      span = mMonthScene->getLeftSpan( endDate() );
      start = d;
    } else if ( d <= startDate() && startDate() <= end ) { // MonthItem begins on this line
      span = mMonthScene->getRightSpan( startDate() );
      start = startDate();
    } else { // MonthItem is not on the line
      continue;
    }

    // A new item needs to be created
    MonthGraphicsItem *newItem = new MonthGraphicsItem( this );
    mMonthGraphicsItemList << newItem;
    newItem->setStartDate( start );
    newItem->setDaySpan( span );
  }

  if ( isMoving() || isResizing() ) {
    setZValue( 100 );
  } else {
    setZValue( 0 );
  }
}

void MonthItem::beginResize()
{
  mOverrideDaySpan = daySpan();
  mOverrideStartDate = startDate();
  mResizing = true;
  setZValue( 100 );
}

void MonthItem::endResize()
{
  setZValue( 0 );
  mResizing = false; // startDate() and daySpan() return real values again

  if ( mOverrideStartDate != startDate() || mOverrideDaySpan != daySpan() ) {
    finalizeResize( mOverrideStartDate, mOverrideStartDate.addDays( mOverrideDaySpan ) );
  }
}

void MonthItem::beginMove()
{
  mOverrideDaySpan = daySpan();
  mOverrideStartDate = startDate();
  mMoving = true;
  setZValue( 100 );
}

void MonthItem::endMove()
{
  setZValue( 0 );
  mMoving = false; // startDate() and daySpan() return real values again

  if ( mOverrideStartDate != startDate() ) {
    finalizeMove( mOverrideStartDate );
  }
}

bool MonthItem::resizeBy( int offsetToPreviousDate )
{
  bool ret = false;
  if ( mMonthScene->resizeType() == MonthScene::ResizeLeft ) {
    if ( mOverrideDaySpan - offsetToPreviousDate >= 0 ) {
      mOverrideStartDate = mOverrideStartDate.addDays( offsetToPreviousDate );
      mOverrideDaySpan = mOverrideDaySpan - offsetToPreviousDate;
      ret = true;
    }
  } else if ( mMonthScene->resizeType() == MonthScene::ResizeRight ) {
    if ( mOverrideDaySpan + offsetToPreviousDate >= 0 ) {
      mOverrideDaySpan = mOverrideDaySpan + offsetToPreviousDate;
      ret = true;
    }
  }

  if ( ret ) {
    updateMonthGraphicsItems();
  }
  return ret;
}

void MonthItem::moveBy( int offsetToPreviousDate )
{
  mOverrideStartDate = mOverrideStartDate.addDays( offsetToPreviousDate );
  updateMonthGraphicsItems();
}

void MonthItem::updateGeometry()
{
  foreach ( MonthGraphicsItem *item, mMonthGraphicsItemList ) {
      item->updateGeometry();
  }
}

void MonthItem::setZValue( qreal z )
{
  foreach ( MonthGraphicsItem *item, mMonthGraphicsItemList ) {
    item->setZValue( z );
  }
}

QDate MonthItem::startDate() const
{
  if ( isMoving() || isResizing() ) {
    return mOverrideStartDate;
  }

  return realStartDate();
}

QDate MonthItem::endDate() const
{
  if ( isMoving() || isResizing() ) {
    return mOverrideStartDate.addDays( mOverrideDaySpan );
  }

  return realEndDate();
}

int MonthItem::daySpan() const
{
  if ( isMoving() || isResizing() ) {
    return mOverrideDaySpan;
  }

  QDateTime start( startDate() );
  QDateTime end( endDate() );

  if ( start.isValid() && end.isValid() ) {
    return start.daysTo( end );
  }

  return 0;
}

bool MonthItem::greaterThan( const MonthItem *e1, const MonthItem *e2 )
{
  if ( !e1->startDate().isValid() || !e2->startDate().isValid() ) {
    return false;
  }

  if ( e1->startDate() == e2->startDate() ) {
    if ( e1->daySpan() == e2->daySpan() ) {
      if ( e1->allDay() && !e2->allDay() ) {
        return true;
      }
      if ( !e1->allDay() && e2->allDay() ) {
        return false;
      }
      return e1->greaterThanFallback( e2 );
    } else {
      return e1->daySpan() >  e2->daySpan();
    }
  }

  return e1->startDate() < e2->startDate();
}

bool MonthItem::greaterThanFallback( const MonthItem *other ) const
{
  // Yeah, pointer comparison if there is nothing else to compare...
  return this < other;
}

void MonthItem::updatePosition()
{
  if ( !startDate().isValid() || !endDate().isValid() ) {
    return;
  }

  int firstFreeSpace = 0;
  for ( QDate d = startDate(); d <= endDate(); d = d.addDays( 1 ) ) {
    MonthCell *cell = mMonthScene->mMonthCellMap.value( d );
    if ( !cell ) {
      continue; // cell can be null if the item begins outside the month
    }
    int firstFreeSpaceTmp = cell->firstFreeSpace();
    if ( firstFreeSpaceTmp > firstFreeSpace ) {
      firstFreeSpace = firstFreeSpaceTmp;
    }
  }

  for ( QDate d = startDate(); d <= endDate(); d = d.addDays( 1 ) ) {
    MonthCell *cell = mMonthScene->mMonthCellMap.value( d );
    if ( !cell ) {
      continue;
    }
    cell->addMonthItem( this, firstFreeSpace );
  }

  mPosition = firstFreeSpace;
}

//-----------------------------------------------------------------
// INCIDENCEMONTHITEM
IncidenceMonthItem::IncidenceMonthItem( MonthScene *monthScene,
                                        const Akonadi::Item &aitem,
                                        const QDate &recurStartDate )
  : MonthItem( monthScene ), mIncidence( aitem ), mCloned( false )
{
  mIsEvent = CalendarSupport::hasEvent( mIncidence );
  mIsJournal = CalendarSupport::hasJournal( mIncidence );
  mIsTodo = CalendarSupport::hasTodo( mIncidence );

  Incidence::Ptr inc = CalendarSupport::incidence( mIncidence );
  if ( inc->customProperty( "KABC", "BIRTHDAY" ) == "YES" ||
       inc->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
    int years = EventViews::yearDiff( inc->dtStart().date(), recurStartDate );
    if ( years > 0 ) {
      inc = Incidence::Ptr( inc->clone() );
      inc->setReadOnly( false );
      inc->setSummary( i18np( "%2 (1 year)", "%2 (%1 years)", years, inc->summary() ) );
      inc->setReadOnly( true );
      mIncidence = Akonadi::Item();
      mIncidence.setPayload( inc );
      mCloned = true;
    }
  }

  connect( monthScene, SIGNAL(incidenceSelected(const Akonadi::Item &, const QDate &)),
           this, SLOT(updateSelection(const Akonadi::Item &, const QDate &)) );

  // first set to 0, because it's used in startDate()
  mRecurDayOffset = 0;
  if ( startDate().isValid() && recurStartDate.isValid() ) {
    mRecurDayOffset = startDate().daysTo( recurStartDate );
  }
}

IncidenceMonthItem::~IncidenceMonthItem()
{
}

bool IncidenceMonthItem::greaterThanFallback( const MonthItem *other ) const
{

  const IncidenceMonthItem *o = qobject_cast<const IncidenceMonthItem *>( other );
  if ( !o ) {
    return MonthItem::greaterThanFallback( other );
  }

  if ( allDay() != o->allDay() ) {
    return allDay();
  }
  const Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
  const Incidence::Ptr otherIncidence = CalendarSupport::incidence( o->mIncidence );

  if ( incidence->dtStart().time() != otherIncidence->dtStart().time() ) {
    return incidence->dtStart().time() < otherIncidence->dtStart().time();
  }

  // as a last resort, compare the item id's
  return mIncidence.id() < o->mIncidence.id();
}

QDate IncidenceMonthItem::realStartDate() const
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
  if ( !incidence ) {
    return QDate();
  }

  KDateTime dt;
  if ( mIsEvent || mIsJournal ) {
    dt = incidence->dtStart();
  } else if ( mIsTodo ) {
    dt = CalendarSupport::todo( mIncidence )->dtDue();
  }

  QDate start;
  if ( dt.isDateOnly() ) {
    start = dt.date();
  } else {
    start = dt.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date();
  }

  return start.addDays( mRecurDayOffset );
}
QDate IncidenceMonthItem::realEndDate() const
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
  if ( !incidence ) {
    return QDate();
  }

  const KDateTime dt = incidence->dateTime( Incidence::RoleDisplayEnd );

  QDate end;
  if ( dt.isDateOnly() ) {
    end = dt.date();
  } else {
    end = dt.toTimeSpec( CalendarSupport::KCalPrefs::instance()->timeSpec() ).date();
  }

  return end.addDays( mRecurDayOffset );
}
bool IncidenceMonthItem::allDay() const
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
  return incidence->allDay();
}

bool IncidenceMonthItem::isMoveable() const
{
  return monthScene()->mMonthView->calendar()->hasChangeRights( mIncidence );
}
bool IncidenceMonthItem::isResizable() const
{
  return mIsEvent && monthScene()->mMonthView->calendar()->hasChangeRights( mIncidence );
}

void IncidenceMonthItem::finalizeMove( const QDate &newStartDate )
{
  Q_ASSERT( isMoveable() );

  if ( startDate().isValid() && newStartDate.isValid() ) {
    updateDates( startDate().daysTo( newStartDate ),
                 startDate().daysTo( newStartDate ) );
  }
}
void IncidenceMonthItem::finalizeResize( const QDate &newStartDate,
                                         const QDate &newEndDate )
{
  Q_ASSERT( isResizable() );

  if ( startDate().isValid() && endDate().isValid() &&
       newStartDate.isValid() && newEndDate.isValid() ) {
    updateDates( startDate().daysTo( newStartDate ),
                 endDate().daysTo( newEndDate ) );
  }
}

void IncidenceMonthItem::updateDates( int startOffset, int endOffset )
{
  if ( startOffset == 0 && endOffset == 0 ) {
    return;
  }
  Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );

  CalendarSupport::IncidenceChanger *changer = monthScene()->incidenceChanger();
  if ( !changer ) {
    KMessageBox::sorry( parentWidget(), i18n( "Unable to save %1 \"%2\".",
                        i18n( incidence->typeStr() ), incidence->summary() ) );
    return;
  }

  bool modify = true;

  if ( incidence->recurs() ) {
    int res = monthScene()->mMonthView->showMoveRecurDialog( mIncidence, startDate() );
    switch ( res ) {
      case KMessageBox::Ok: // All occurrences
        modify = true;
        break;
      case KMessageBox::Yes: // Just this occurrence
      {
        modify = true;
        Incidence::Ptr oldIncSaved( incidence->clone() );
        Incidence::Ptr newInc( monthScene()->mMonthView->calendar()->dissociateOccurrence( mIncidence, startDate(), CalendarSupport::KCalPrefs::instance()->timeSpec() ) );
        if ( newInc ) {
           //TODO check return values
          changer->changeIncidence( oldIncSaved, mIncidence,
                                    CalendarSupport::IncidenceChanger::RECURRENCE_MODIFIED_ONE_ONLY, 0 );

          changer->addIncidence( newInc, mIncidence.parentCollection(), parentWidget() );
        } else {
          KMessageBox::sorry(
            parentWidget(),
            i18n( "Unable to add the exception item to the calendar. "
                  "No change will be done." ),
            i18n( "Error Occurred" ) );
          modify = false;
        }
        break;
      }
      case KMessageBox::No: // All future occurrences
      {
        modify = true;
        Incidence::Ptr oldIncSaved( incidence->clone() );
        Incidence::Ptr newInc( monthScene()->mMonthView->calendar()->dissociateOccurrence(
            mIncidence, startDate(), CalendarSupport::KCalPrefs::instance()->timeSpec(), false ) );
        if ( newInc ) {
           //TODO check return values
          changer->changeIncidence( oldIncSaved, mIncidence,
                                    CalendarSupport::IncidenceChanger::RECURRENCE_MODIFIED_ALL_FUTURE, 0 );

          changer->addIncidence( newInc, mIncidence.parentCollection(), parentWidget() );
        } else {
          KMessageBox::sorry(
            parentWidget(),
            i18n( "Unable to add the future items to the calendar. "
                  "No change will be done." ),
            i18n( "Error Occurred" ) );
          modify = false;
        }
        break;
      }
      default:
        modify = false;
    }
  }

  if ( modify ) {
    Incidence::Ptr oldInc( incidence->clone() );

    if ( !mIsTodo ) {
      incidence->setDtStart( incidence->dtStart().addDays( startOffset ) );

      if ( mIsEvent ) {
        Event::Ptr event = CalendarSupport::event( mIncidence );
        event->setDtEnd( event->dtEnd().addDays( endOffset ) );
      }
    } else {
      Todo::Ptr todo = CalendarSupport::todo( mIncidence );
      todo->setDtDue( todo->dtDue().addDays( startOffset ) );
    }

    changer->changeIncidence( oldInc, mIncidence, CalendarSupport::IncidenceChanger::DATE_MODIFIED, 0 );
  }

}

void IncidenceMonthItem::updateSelection( const Akonadi::Item &incidence, const QDate &date )
{
  Q_UNUSED( date );
  setSelected( incidence == mIncidence );
}

QString IncidenceMonthItem::text( bool end ) const
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
  QString ret = incidence->summary();
  if ( !allDay() ) { // Prepend the time str to the text
    QString timeStr;
    if ( mIsTodo ) {
      Todo::Ptr todo = CalendarSupport::todo( mIncidence );
      timeStr = IncidenceFormatter::timeToString(
        todo->dtDue(), true, CalendarSupport::KCalPrefs::instance()->timeSpec() );
    } else {
      if ( !end ) {
        timeStr = IncidenceFormatter::timeToString(
          incidence->dtStart(), true, CalendarSupport::KCalPrefs::instance()->timeSpec() );
      } else {
        Event::Ptr event = CalendarSupport::event( mIncidence );
        timeStr = IncidenceFormatter::timeToString(
          event->dtEnd(), true, CalendarSupport::KCalPrefs::instance()->timeSpec() );
      }
    }
    if ( !timeStr.isEmpty() ) {
      if ( !end ) {
        ret = timeStr + ' ' + ret;
      } else {
        ret = ret + ' ' + timeStr;
      }
    }
  }

  return ret;
}

QString IncidenceMonthItem::toolTipText( const QDate &date ) const
{
  return IncidenceFormatter::toolTipStr(
    CalendarSupport::displayName( mIncidence.parentCollection() ),
    CalendarSupport::incidence( mIncidence ),
    date, true, CalendarSupport::KCalPrefs::instance()->timeSpec() );
}

QList<QPixmap *> IncidenceMonthItem::icons() const
{
  QList<QPixmap *> ret;

  if ( !CalendarSupport::hasIncidence( mIncidence ) ) {
    return ret;
  }
  const Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );

  bool specialEvent = false;
  if ( mIsEvent ) {
    if ( incidence->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
      specialEvent = true;
      ret << monthScene()->anniversaryPixmap();
    } else if ( incidence->customProperty( "KABC", "BIRTHDAY" ) == "YES" ) {
      specialEvent = true;
      ret << monthScene()->birthdayPixmap();
    }
    // smartins: Disabling the event Pixmap because:
    // 1. Save precious space so we can read the event's title better.
    // 2. We don't need a pixmap to tell us an item is an event we
    //    only need one to tell us it's not, as month view was designed for events.
    // 3. If only to-dos and journals have a pixmap they will be distinguished
    //    from event's much easier.

    // ret << monthScene()->eventPixmap();

  } else if ( mIsTodo ) {

    bool isCompleted = monthScene()->monthView()->usesCompletedTodoPixmap( mIncidence, realStartDate() );

    if ( isCompleted ) {
      ret << monthScene()->todoDonePixmap();
    } else {
      ret << monthScene()->todoPixmap();
    }
  } else if ( mIsJournal ) {
    ret << monthScene()->journalPixmap();
  }

  if ( !monthScene()->mMonthView->calendar()->hasChangeRights( mIncidence ) && !specialEvent ) {
    ret << monthScene()->readonlyPixmap();
  }
#if 0
  /* sorry, this looks too cluttered. disable until we can
     make something prettier; no idea at this time -- allen */
  if ( mIncidence->hasEnabledAlarms() && !specialEvent ) {
    ret << monthScene()->alarmPixmap();
  }
  if ( mIncidence->recurs() && !specialEvent ) {
    ret << monthScene()->recurPixmap();
  }
  //TODO: check what to do with Reply
#endif
  return ret;
}

QColor IncidenceMonthItem::catColor() const
{
  QColor retColor;
  const Incidence::Ptr incidence = CalendarSupport::incidence( mIncidence );
  Q_ASSERT( incidence );
  QStringList categories = incidence->categories();
  QString cat;
  if ( !categories.isEmpty() ) {
    cat = categories.first();
  }
  if ( cat.isEmpty() ) {
    retColor = monthScene()->monthView()->preferences()->unsetCategoryColor();
  } else {
    retColor = monthScene()->monthView()->preferences()->categoryColor( cat );
  }
  return retColor;
}

QColor IncidenceMonthItem::bgColor() const
{
  QColor bgColor = QColor(); // Default invalid color;

  PrefsPtr prefs = monthScene()->monthView()->preferences();
  if ( mIsTodo && !prefs->todosUseCategoryColors() ) {
    if ( CalendarSupport::todo( mIncidence )->isOverdue() ) {
      bgColor = prefs->agendaCalendarItemsToDosOverdueBackgroundColor();
    } else if ( CalendarSupport::todo( mIncidence )->dtDue().date() == QDate::currentDate() ) {
      bgColor = prefs->agendaCalendarItemsToDosDueTodayBackgroundColor();
    }
  }

  if ( !bgColor.isValid() ) {
    if ( prefs->monthViewColors() == PrefsBase::MonthItemResourceOnly ||
         prefs->monthViewColors() == PrefsBase::MonthItemResourceInsideCategoryOutside ) {
      Q_ASSERT( mIncidence.isValid() );
      const QString id = QString::number( mIncidence.storageCollectionId() );
      Q_ASSERT( ! id.isEmpty() );
      bgColor = monthScene()->monthView()->preferences()->resourceColor( id );
    } else {
      bgColor = catColor();
    }
  }

  if ( !bgColor.isValid() ) {
    bgColor = Qt::white;
  }

  return bgColor;
}

QColor IncidenceMonthItem::frameColor() const
{
  QColor frameColor;

  PrefsPtr prefs = monthScene()->monthView()->preferences();
  if ( prefs->monthViewColors() == PrefsBase::MonthItemResourceOnly ||
       prefs->monthViewColors() == PrefsBase::MonthItemCategoryInsideResourceOutside ) {
    Q_ASSERT( mIncidence.isValid() );
    const QString id = QString::number( mIncidence.storageCollectionId() );
    Q_ASSERT( ! id.isEmpty() );
    frameColor = prefs->resourceColor( id );
  } else {
    frameColor = catColor();
  }

  if ( !frameColor.isValid() ) {
    frameColor = Qt::black;
  }

  return frameColor;
}

//-----------------------------------------------------------------
// HOLIDAYMONTHITEM
HolidayMonthItem::HolidayMonthItem( MonthScene *monthScene, const QDate &date,
                                    const QString &name )
  : MonthItem( monthScene ), mDate( date ), mName( name )
{
}

HolidayMonthItem::~HolidayMonthItem()
{
}

bool HolidayMonthItem::greaterThanFallback( const MonthItem *other ) const
{
  const HolidayMonthItem *o = qobject_cast<const HolidayMonthItem *>( other );
  if ( o ) {
    return MonthItem::greaterThanFallback( other );
  }

  // always put holidays on top
  return false;
}

void HolidayMonthItem::finalizeMove( const QDate &newStartDate )
{
  Q_UNUSED( newStartDate );
  Q_ASSERT( false );
}
void HolidayMonthItem::finalizeResize( const QDate &newStartDate,
                                       const QDate &newEndDate )
{
  Q_UNUSED( newStartDate );
  Q_UNUSED( newEndDate );
  Q_ASSERT( false );
}

QList<QPixmap *> HolidayMonthItem::icons() const
{
  QList<QPixmap *> ret;
  ret << monthScene()->holidayPixmap();

  return ret;
}

QColor HolidayMonthItem::bgColor() const
{
  // FIXME: Currently, only this value is settable in the options.
  // There is a monthHolidaysBackgroundColor() option too. Maybe it would be
  // wise to merge those two.
  return monthScene()->monthView()->preferences()->agendaHolidaysBackgroundColor();
}

QColor HolidayMonthItem::frameColor() const
{
  return Qt::black;
}