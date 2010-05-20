/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

#include "incidencedatetimeeditor.h"

#include <KCal/ICalTimeZones>
#include <KCal/IncidenceFormatter>
#include <KDebug>
#include <KSystemTimeZones>

#include "incidencerecurrencedialog.h"
#include "ui_incidencedatetime.h"

using namespace IncidenceEditorsNG;
using namespace KCal;

IncidenceDateTimeEditor::IncidenceDateTimeEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mTimeZones( new ICalTimeZones )
  , mUi( new Ui::IncidenceDateTimeEditor )
{
  mUi->setupUi( this );
  mUi->mAlarmBell->setPixmap( SmallIcon( "task-reminder" ) );
  mUi->mRecurrenceEditButton->setIcon(
    KIconLoader::global()->loadIcon(
      "task-recurring", KIconLoader::Desktop, KIconLoader::SizeSmall ) );

  connect( mUi->mRecurrenceEditButton, SIGNAL(clicked()), SLOT(editRecurrence()) );
}

IncidenceDateTimeEditor::~IncidenceDateTimeEditor()
{
  delete mTimeZones;
}


void IncidenceDateTimeEditor::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;

  // We can only handle events or todos.
  if ( KCal::Todo::ConstPtr todo = IncidenceDateTimeEditor::incidence<Todo>() ) {
    load( todo );
  } else if ( KCal::Event::ConstPtr event = IncidenceDateTimeEditor::incidence<Event>() ) {
    load( event );
  } else {
    kDebug() << "Not an event or an todo.";
  }

  mActiveStartDT.setTimeSpec( mUi->mTimeZoneComboStart->selectedTimeSpec() );
  mActiveEndDT.setTimeSpec( mUi->mTimeZoneComboEnd->selectedTimeSpec() );

  enableTimeEdits( mUi->mHasTimeCheck->isChecked() );

  mWasDirty = false;
}

void IncidenceDateTimeEditor::save( KCal::Incidence::Ptr incidence )
{ }

bool IncidenceDateTimeEditor::isDirty() const
{
  if ( KCal::Todo::ConstPtr todo = IncidenceDateTimeEditor::incidence<Todo>() ) {
    return isDirty( todo );
  } else if ( KCal::Event::ConstPtr event = IncidenceDateTimeEditor::incidence<Event>() ) {
    return isDirty( event );
  } else {
    Q_ASSERT_X( false, "IncidenceDateTimeEditor::isDirty", "Only implemented for todos and events" );
    return false;
  }
}

void IncidenceDateTimeEditor::setActiveDate( const QDate &activeDate )
{
  mActiveDate = activeDate;
}

/// private slots for General

void IncidenceDateTimeEditor::editRecurrence()
{
  QPointer<IncidenceRecurrenceDialog> dialog( new IncidenceRecurrenceDialog( this ) );
  dialog->exec();
  delete dialog;
}

void IncidenceDateTimeEditor::enableAlarm( bool enable )
{
  mUi->mAlarmStack->setEnabled( enable );
  mUi->mAlarmEditButton->setEnabled( enable );
}

void IncidenceDateTimeEditor::startTimeChanged( const QTime &/* newtime */ )
{
  if ( mUi->mStartCheck->isChecked() && mUi->mEndCheck->isChecked() ) {
    const KDateTime currStartDateTime = currentEndDateTime();
    KDateTime currEndDateTime = currentEndDateTime();
    const int secsep = currStartDateTime.secsTo( currEndDateTime );

    // adjust end time so that the event has the same duration as before.
    currEndDateTime = currStartDateTime.addSecs( secsep );
    mUi->mEndTimeEdit->setTime( currEndDateTime.time() );
    mUi->mEndDateEdit->setDate( currEndDateTime.date() );
  }

//   emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
  checkDirtyStatus();
}

void IncidenceDateTimeEditor::updateRecurrenceSummary( KCal::Incidence::ConstPtr incidence )
{
  if ( incidence->recurs() ) {
    mUi->mRecurrenceLabel->setText( IncidenceFormatter::recurrenceString( const_cast<Incidence *>( incidence.get() ) ) );
  } else {
    mUi->mRecurrenceLabel->setText( QString() );
  }
}

/// private slots for Todo

void IncidenceDateTimeEditor::enableStartEdit( bool enable )
{
  mUi->mStartDateEdit->setEnabled( enable );

  if( mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked() ) {
    mUi->mHasTimeCheck->setEnabled( true );
  } else {
    mUi->mHasTimeCheck->setEnabled( false );
    mUi->mHasTimeCheck->setChecked( false );
  }

  if ( enable ) {
    mUi->mStartTimeEdit->setEnabled( mUi->mHasTimeCheck->isChecked() );
    mUi->mTimeZoneComboStart->setEnabled( mUi->mHasTimeCheck->isChecked() );
  } else {
    mUi->mStartTimeEdit->setEnabled( false );
    mUi->mTimeZoneComboStart->setEnabled( false );
  }

  mUi->mTimeZoneComboStart->setFloating( !mUi->mTimeZoneComboStart->isEnabled() );
  checkDirtyStatus();
}

void IncidenceDateTimeEditor::enableEndEdit( bool enable )
{
  mUi->mEndDateEdit->setEnabled( enable );

  if( mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked() ) {
    mUi->mHasTimeCheck->setEnabled( true );
  } else {
    mUi->mHasTimeCheck->setEnabled( false );
  }

  if ( enable ) {
    mUi->mEndTimeEdit->setEnabled( mUi->mHasTimeCheck->isChecked() );
    mUi->mTimeZoneComboEnd->setEnabled( mUi->mHasTimeCheck->isChecked() );
  } else {
    mUi->mEndTimeEdit->setEnabled( false );
    mUi->mTimeZoneComboEnd->setEnabled( false );
  }

  mUi->mTimeZoneComboEnd->setFloating( !mUi->mTimeZoneComboEnd->isEnabled() );
  checkDirtyStatus();
}

void IncidenceDateTimeEditor::enableTimeEdits( bool enable )
{
  if( mUi->mStartCheck->isChecked() ) {
    mUi->mStartTimeEdit->setEnabled( enable );
    mUi->mTimeZoneComboStart->setEnabled( enable );
    mUi->mTimeZoneComboStart->setFloating( !enable, mActiveStartDT.timeSpec() );
  }
  if( mUi->mEndCheck->isChecked() ) {
    mUi->mEndTimeEdit->setEnabled( enable );
    mUi->mTimeZoneComboEnd->setEnabled( enable );
    mUi->mTimeZoneComboEnd->setFloating( !enable, mActiveEndDT.timeSpec() );
  }
}

bool IncidenceDateTimeEditor::isDirty( KCal::Todo::ConstPtr todo ) const
{
  Q_ASSERT( todo );

  // First check the start time/date of the todo
  if ( todo->hasStartDate() != mUi->mStartCheck->isChecked() )
    return true;

  if ( mUi->mStartCheck->isChecked() ) {
    // Use mActiveStartTime. This is the KDateTime::Spec selected on load comming from
    // the combobox. We use this one as it can slightly differ (e.g. missing
    // country code in the incidence time spec) from the incidence.
    if ( currentStartDateTime() != mActiveStartDT )
      return true;
  }

  if ( todo->hasDueDate() != mUi->mEndCheck->isChecked() )
    return true;

  if ( mUi->mEndCheck->isChecked() ) {
    if ( currentEndDateTime() != mActiveEndDT )
      return true;
  }

  // TODO Recurrence
  // TODO Alarms

  return false;
}

/// Event specific methods

bool IncidenceDateTimeEditor::isDirty( KCal::Event::ConstPtr event ) const
{
  if ( event->allDay() != mUi->mHasTimeCheck->isChecked() )
    return true;

  if ( !event->allDay() ) {
    if ( currentStartDateTime() != mActiveStartDT )
      return true;

    if ( currentEndDateTime() != mActiveEndDT )
      return true;
  }

  // TODO Recurrence
  // TODO Alarms
  
  return false;
}

/// Private methods

KDateTime IncidenceDateTimeEditor::currentStartDateTime() const
{
  return KDateTime(
    mUi->mStartDateEdit->date(),
    mUi->mEndTimeEdit->time(),
    mUi->mTimeZoneComboStart->selectedTimeSpec() );
}

KDateTime IncidenceDateTimeEditor::currentEndDateTime() const
{
  return KDateTime(
    mUi->mEndDateEdit->date(),
    mUi->mEndTimeEdit->time(),
    mUi->mTimeZoneComboEnd->selectedTimeSpec() ).toTimeSpec( currentStartDateTime().timeSpec() );
}

void IncidenceDateTimeEditor::load( KCal::Event::ConstPtr event )
{
  // First en/disable the necessary ui bits and pieces
  mUi->mStartLabel->setVisible( true );
  mUi->mEndLabel->setVisible( true );
  mUi->mStartCheck->setVisible( false );
  mUi->mStartCheck->setChecked( true ); // Set to checked so we can reuse enableTimeEdits.
  mUi->mEndCheck->setVisible( false );
  mUi->mEndCheck->setChecked( true ); // Set to checked so we can reuse enableTimeEdits.

  connect( mUi->mHasTimeCheck, SIGNAL(toggled(bool)), SLOT(enableTimeEdits(bool)) );
  connect( mUi->mHasTimeCheck, SIGNAL(toggled(bool)), SLOT(enableAlarm(bool)) );

  mUi->mHasTimeCheck->setChecked( !event->allDay() );
  enableTimeEdits( !event->allDay() );

  bool isTemplate = false; // TODO
  if ( !isTemplate ) {
    mActiveStartDT = event->dtStart();
    mActiveEndDT = event->dtEnd();
    if ( event->recurs() && mActiveDate.isValid() ) {
      // Consider the active date when editing recurring Events.
      KDateTime kdt( mActiveDate, QTime( 0, 0, 0 ), KSystemTimeZones::local() );
      const int eventLength = mActiveStartDT.daysTo( mActiveEndDT );
      kdt = kdt.addSecs( -1 );
      mActiveStartDT.setDate( event->recurrence()->getNextDateTime( kdt ).date() );
      if ( event->hasEndDate() ) {
        mActiveEndDT.setDate( mActiveStartDT.addDays( eventLength ).date() );
      } else {
        if ( event->hasDuration() ) {
          mActiveEndDT = mActiveStartDT.addSecs( event->duration().asSeconds() );
        } else {
          mActiveEndDT = mActiveStartDT;
        }
      }
    }
    // Convert UTC to local timezone, if needed (i.e. for kolab #204059)
    if ( mActiveStartDT.isUtc() ) {
      mActiveStartDT = mActiveStartDT.toLocalZone();
    }
    if ( mActiveEndDT.isUtc() ) {
      mActiveEndDT = mActiveEndDT.toLocalZone();
    }
    setDateTimes( mActiveStartDT, mActiveEndDT );
  } else {
    // set the start/end time from the template, only as a last resort #190545
    if ( !event->dtStart().isValid() || !event->dtEnd().isValid() ) {
      setTimes( event->dtStart(), event->dtEnd() );
    }
  }

  switch( event->transparency() ) {
  case Event::Transparent:
    mUi->mFreeBusyCombo->setCurrentIndex( 1 );
    break;
  case Event::Opaque:
    mUi->mFreeBusyCombo->setCurrentIndex( 0 );
    break;
  }

  updateRecurrenceSummary( event );
}

void IncidenceDateTimeEditor::load( KCal::Todo::ConstPtr todo )
{
  // First en/disable the necessary ui bits and pieces
  mUi->mStartLabel->setVisible( false );
  mUi->mEndLabel->setVisible( false );

  mUi->mStartCheck->setVisible( true );
  mUi->mStartCheck->setChecked( todo->hasStartDate() );
  mUi->mStartDateEdit->setEnabled( todo->hasStartDate() );
  mUi->mStartTimeEdit->setEnabled( todo->hasStartDate() );
  mUi->mTimeZoneComboStart->setEnabled( todo->hasStartDate() );
  
  mUi->mEndCheck->setVisible( true );
  mUi->mEndCheck->setChecked( todo->hasDueDate() );
  mUi->mEndDateEdit->setEnabled( todo->hasDueDate() );
  mUi->mEndTimeEdit->setEnabled( todo->hasDueDate() );
  mUi->mTimeZoneComboEnd->setEnabled( todo->hasDueDate() );

  // These fields where not enabled in the old code either:
  mUi->mDurationLabel->setVisible( false );
  mUi->mFreeBusyLabel->setVisible( false );
  mUi->mFreeBusyCombo->setVisible( false );

  mUi->mHasTimeCheck->setChecked( !todo->allDay() );

  // Connect to the right logic
  connect( mUi->mStartCheck, SIGNAL(toggled(bool)), SLOT(enableStartEdit(bool)) );
  connect( mUi->mStartDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDirtyStatus()) );
  connect( mUi->mStartTimeEdit, SIGNAL(timeChanged(QTime)), SLOT(startTimeChanged(QTime)) );
  connect( mUi->mTimeZoneComboStart, SIGNAL(currentIndexChanged(int)), SLOT(checkDirtyStatus()) );

  connect( mUi->mEndCheck, SIGNAL(toggled(bool)), SLOT(enableEndEdit(bool)) );
  connect( mUi->mEndCheck, SIGNAL(toggled(bool)), SLOT(enableAlarm(bool)) );
  //   connect( mDueCheck, SIGNAL(toggled(bool)), SIGNAL(dueDateEditToggle(bool)) );
  connect( mUi->mEndDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDirtyStatus()) );
  connect( mUi->mEndTimeEdit, SIGNAL(timeChanged(const QTime&)), SLOT(checkDirtyStatus()) );
  connect( mUi->mTimeZoneComboEnd, SIGNAL(currentIndexChanged(int)), SLOT(checkDirtyStatus()) );

  connect( mUi->mHasTimeCheck, SIGNAL(toggled(bool)), SLOT(enableTimeEdits(bool)));

  //TODO: do something with tmpl, note: this wasn't used in the old code either.
//   Q_UNUSED( tmpl );

  mActiveEndDT = KDateTime( QDate::currentDate(), QTime::currentTime() );
  if ( todo->hasDueDate() ) {
    mActiveEndDT = todo->dtDue();
    if ( todo->recurs() && mActiveDate.isValid() ) {
      KDateTime dt( mActiveDate, QTime( 0, 0, 0 ) );
      dt = dt.addSecs( -1 );
      mActiveEndDT.setDate( todo->recurrence()->getNextDateTime( dt ).date() );
    }
    if ( mActiveEndDT.isUtc() )
      mActiveEndDT = mActiveEndDT.toLocalZone();
  }

  mActiveStartDT = KDateTime( QDate::currentDate(), QTime::currentTime() );
  if ( todo->hasStartDate() ) {
    mActiveStartDT = todo->dtStart();
    if ( todo->recurs() && mActiveDate.isValid() && todo->hasDueDate() ) {
      int days = todo->dtStart( true ).daysTo( todo->dtDue( true ) );
      mActiveStartDT.setDate( mActiveStartDT.date().addDays( -days ) );
    }
    if ( mActiveStartDT.isUtc() )
      mActiveStartDT = mActiveStartDT.toLocalZone();
  }

  setDateTimes( mActiveStartDT, mActiveEndDT );
  enableAlarm( todo->hasDueDate() );
  updateRecurrenceSummary( todo );
}

void IncidenceDateTimeEditor::setDateTimes( const KDateTime &start, const KDateTime &end )
{
  if ( start.isValid() ) {
    mUi->mStartDateEdit->setDate( start.date() );
    mUi->mStartTimeEdit->setTime( start.time() );
    mUi->mTimeZoneComboStart->selectTimeSpec( start.timeSpec() );
  } else {
    KDateTime dt( QDate::currentDate(), QTime::currentTime() );
    mUi->mStartDateEdit->setDate( dt.date() );
    mUi->mStartTimeEdit->setTime( dt.time() );
    mUi->mTimeZoneComboStart->selectTimeSpec( dt.timeSpec() );
  }

  if ( end.isValid() ) {
    mUi->mEndDateEdit->setDate( end.date() );
    mUi->mEndTimeEdit->setTime( end.time() );
    mUi->mTimeZoneComboEnd->selectTimeSpec( end.timeSpec() );
  } else {
    KDateTime dt( QDate::currentDate(), QTime::currentTime() );
    mUi->mEndDateEdit->setDate( dt.date() );
    mUi->mEndTimeEdit->setTime( dt.time() );
    mUi->mTimeZoneComboEnd->selectTimeSpec( dt.timeSpec() );
  }

  setDuration();
}

void IncidenceDateTimeEditor::setTimes( const KDateTime &start, const KDateTime &end )
{
  // like setDateTimes(), but it set only the start/end time, not the date
  // it is used while applying a template to an event.
  mUi->mStartTimeEdit->blockSignals( true );
  mUi->mStartTimeEdit->setTime( start.time() );
  mUi->mStartTimeEdit->blockSignals( false );

  mUi->mEndTimeEdit->setTime( end.time() );

  mUi->mTimeZoneComboStart->selectTimeSpec( start.timeSpec() );
  mUi->mTimeZoneComboEnd->selectTimeSpec( end.timeSpec() );

  setDuration();
//   emitDateTimeStr();
}

void IncidenceDateTimeEditor::setDuration()
{
  // Those checks are always checked for events, but not for todos. If one of them
  // isn't checked we don't show the duration.
  if ( !mUi->mStartCheck->isChecked() || !mUi->mEndCheck->isChecked() ) {
    mUi->mDurationLabel->setVisible( false );
    return;
  }

  mUi->mDurationLabel->setVisible( true );
  
  QString tmpStr, catStr;
  int hourdiff, minutediff;
  // end date is an accepted temporary state while typing, but don't show
  // any duration if this happens
  KDateTime startDateTime = currentStartDateTime();
  KDateTime endDateTime = currentEndDateTime();
    
  if ( startDateTime < endDateTime ) {

    if ( !mUi->mHasTimeCheck->isChecked() ) {
      int daydiff = startDateTime.date().daysTo( endDateTime.date() ) + 1;
      tmpStr = i18nc( "@label", "Duration: " );
      tmpStr.append( i18ncp( "@label", "1 Day", "%1 Days", daydiff ) );
    } else {
      hourdiff = startDateTime.date().daysTo( endDateTime.date() ) * 24;
      hourdiff += endDateTime.time().hour() - startDateTime.time().hour();
      minutediff = endDateTime.time().minute() - startDateTime.time().minute();
      // If minutediff is negative, "borrow" 60 minutes from hourdiff
      if ( minutediff < 0 && hourdiff > 0 ) {
        hourdiff -= 1;
        minutediff += 60;
      }
      if ( hourdiff || minutediff ) {
        tmpStr = i18nc( "@label", "Duration: " );
        if ( hourdiff ){
          catStr = i18ncp( "@label", "1 hour", "%1 hours", hourdiff );
          tmpStr.append( catStr );
        }
        if ( hourdiff && minutediff ) {
          tmpStr += i18nc( "@label", ", " );
        }
        if ( minutediff ){
          catStr = i18ncp( "@label", "1 minute", "%1 minutes", minutediff );
          tmpStr += catStr;
        }
      } else {
        tmpStr = "";
      }
    }
  }
  mUi->mDurationLabel->setText( tmpStr );
  mUi->mDurationLabel->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Shows the duration of the event or to-do with the "
           "current start and end dates and times." ) );
}
