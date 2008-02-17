/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include "koeditorgeneralevent.h"
#include "koprefs.h"
#include "ktimeedit.h"

#include <libkdepim/kdateedit.h>
#include <libkdepim/ktimezonecombobox.h>

#include <kcal/event.h>
#include <kcal/incidenceformatter.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <ktextedit.h>
#include <KComboBox>

#include <QLayout>
#include <QSpinBox>
#include <QDateTime>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QVBoxLayout>

#include "koeditorgeneralevent.moc"

KOEditorGeneralEvent::KOEditorGeneralEvent( QObject *parent )
  : KOEditorGeneral( parent )
{
  connect( this, SIGNAL(dateTimesChanged(const QDateTime &, const QDateTime &)),
           SLOT(setDuration()) );
  connect( this, SIGNAL(dateTimesChanged(const QDateTime &, const QDateTime &)),
           SLOT(emitDateTimeStr()) );
}

KOEditorGeneralEvent::~KOEditorGeneralEvent()
{
}

void KOEditorGeneralEvent::finishSetup()
{
  QWidget::setTabOrder( mSummaryEdit, mLocationEdit );
  QWidget::setTabOrder( mLocationEdit, mStartDateEdit );
  QWidget::setTabOrder( mStartDateEdit, mStartTimeEdit );
  QWidget::setTabOrder( mStartTimeEdit, mEndDateEdit );
  QWidget::setTabOrder( mEndDateEdit, mEndTimeEdit );
  QWidget::setTabOrder( mEndTimeEdit, mHasTimeCheckbox );
  QWidget::setTabOrder( mHasTimeCheckbox, mAlarmButton );
  QWidget::setTabOrder( mAlarmButton, mAlarmTimeEdit );
  QWidget::setTabOrder( mAlarmTimeEdit, mAlarmIncrCombo );
//   QWidget::setTabOrder( mAlarmIncrCombo, mAlarmSoundButton );
  QWidget::setTabOrder( mAlarmIncrCombo, mAlarmEditButton );
//   QWidget::setTabOrder( mAlarmSoundButton, mAlarmProgramButton );
//   QWidget::setTabOrder( mAlarmProgramButton, mFreeTimeCombo );
  QWidget::setTabOrder( mAlarmEditButton, mFreeTimeCombo );
  QWidget::setTabOrder( mFreeTimeCombo, mDescriptionEdit );
  QWidget::setTabOrder( mDescriptionEdit, mCategoriesButton );
  QWidget::setTabOrder( mCategoriesButton, mSecrecyCombo );
//  QWidget::setTabOrder( mSecrecyCombo, mDescriptionEdit );

  mSummaryEdit->setFocus();
}

void KOEditorGeneralEvent::initTime( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *timeLayout = new QVBoxLayout();
  topLayout->addItem( timeLayout );

  QGroupBox *timeGroupBox = new QGroupBox( i18n( "Date && Time" ), parent );
  timeGroupBox->setWhatsThis(
    i18n( "Sets options related to the date and time of the event or to-do." ) );
  timeLayout->addWidget( timeGroupBox );

  QGridLayout *layoutTimeBox = new QGridLayout( timeGroupBox );
  layoutTimeBox->setSpacing( KDialog::spacingHint() );

  mStartDateLabel = new QLabel( i18nc( "@label event start time", "&Start:" ), timeGroupBox );
  layoutTimeBox->addWidget( mStartDateLabel, 0, 0 );
  layoutTimeBox->setColStretch( 3, 1 );

  mStartDateEdit = new KPIM::KDateEdit( timeGroupBox );
  layoutTimeBox->addWidget( mStartDateEdit, 0, 1 );
  mStartDateLabel->setBuddy( mStartDateEdit );

  mStartTimeEdit = new KPIM::KTimeEdit( timeGroupBox );
  layoutTimeBox->addWidget( mStartTimeEdit, 0, 2 );

  // Timezone
  QString whatsThis = i18n( "Select the timezone for this event. "
                            "It will also affect recurrences" );
  mTimeZoneComboStart = new KPIM::KTimeZoneComboBox( timeGroupBox );
  mTimeZoneComboEnd = new KPIM::KTimeZoneComboBox( timeGroupBox );
  layoutTimeBox->addWidget( mTimeZoneComboStart, 0, 3 );
  layoutTimeBox->addWidget( mTimeZoneComboEnd, 1, 3 );
  mTimeZoneComboStart->setWhatsThis( whatsThis );
  mTimeZoneComboEnd->setWhatsThis( whatsThis );
  mTimeZoneComboStart->selectLocalTimeSpec();
  mTimeZoneComboEnd->selectLocalTimeSpec();
  mStartSpec = mTimeZoneComboStart->selectedTimeSpec();
  mEndSpec = mTimeZoneComboEnd->selectedTimeSpec();

  mEndDateLabel = new QLabel( i18n( "&End:" ), timeGroupBox );
  layoutTimeBox->addWidget( mEndDateLabel, 1, 0 );

  mEndDateEdit = new KPIM::KDateEdit( timeGroupBox );
  layoutTimeBox->addWidget( mEndDateEdit, 1, 1 );
  mEndDateLabel->setBuddy( mEndDateEdit );

  mEndTimeEdit = new KPIM::KTimeEdit( timeGroupBox );
  layoutTimeBox->addWidget( mEndTimeEdit, 1, 2 );

  mHasTimeCheckbox = new QCheckBox( i18n( "T&ime associated" ), timeGroupBox );
  layoutTimeBox->addWidget( mHasTimeCheckbox, 0, 4 );
  connect( mHasTimeCheckbox, SIGNAL(toggled(bool)), SLOT(slotHasTimeCheckboxToggled(bool)) );

  mDurationLabel = new QLabel( timeGroupBox );
  layoutTimeBox->addWidget( mDurationLabel, 1, 4 );

  // time widgets are checked if they contain a valid time
  connect( mStartTimeEdit, SIGNAL(timeChanged(QTime)),
           this, SLOT(startTimeChanged(QTime)) );
  connect( mEndTimeEdit, SIGNAL(timeChanged(QTime)),
           this, SLOT(endTimeChanged(QTime)) );

  // date widgets are checked if they contain a valid date
  connect( mStartDateEdit, SIGNAL(dateChanged(const QDate&)),
           this, SLOT(startDateChanged(const QDate&)) );
  connect( mEndDateEdit, SIGNAL(dateChanged(const QDate&)),
           this, SLOT(endDateChanged(const QDate&)) );

  connect( mTimeZoneComboStart, SIGNAL(currentIndexChanged(int)),
           this, SLOT(startSpecChanged()) );
  connect( mTimeZoneComboEnd, SIGNAL(currentIndexChanged(int)),
           this, SLOT(endSpecChanged()) );

  QBoxLayout *recLayout = new QHBoxLayout();
  layoutTimeBox->addMultiCellLayout( recLayout, 2, 2, 1, 4 );
  mRecurrenceSummary = new QLabel( QString(), timeGroupBox );
  recLayout->addWidget( mRecurrenceSummary );
  QPushButton *recEditButton = new QPushButton( i18n( "Edit..." ), timeGroupBox );
  recLayout->addWidget( recEditButton );
  connect( recEditButton, SIGNAL(clicked()), SIGNAL(editRecurrence()) );
  recLayout->addStretch( 1 );

  QLabel *label = new QLabel( i18n( "Reminder:" ), timeGroupBox );
  layoutTimeBox->addWidget( label, 3, 0 );
  QBoxLayout *alarmLineLayout = new QHBoxLayout();
  layoutTimeBox->addMultiCellLayout( alarmLineLayout, 3, 3, 1, 3 );
  initAlarm( timeGroupBox, alarmLineLayout );
  alarmLineLayout->addStretch( 1 );

  QBoxLayout *secLayout = new QHBoxLayout();
  layoutTimeBox->addLayout( secLayout, 0, 5 );
  initSecrecy( timeGroupBox, secLayout );

  QBoxLayout *classLayout = new QHBoxLayout();
  layoutTimeBox->addLayout( classLayout, 1, 5 );
  initClass( timeGroupBox, classLayout );
}

void KOEditorGeneralEvent::initClass( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *classLayout = new QHBoxLayout();
  classLayout->setSpacing( topLayout->spacing() );
  topLayout->addItem( classLayout );

  QLabel *freeTimeLabel = new QLabel( i18n( "S&how time as:" ), parent );
  QString whatsThis = i18n( "Sets how this time will appear on your Free/Busy information." );
  freeTimeLabel->setWhatsThis( whatsThis );
  classLayout->addWidget( freeTimeLabel );

  mFreeTimeCombo = new KComboBox( parent );
  mFreeTimeCombo->setEditable( false );
  mFreeTimeCombo->setWhatsThis( whatsThis );
  mFreeTimeCombo->addItem( i18nc( "show event as busy time", "Busy" ) );
  mFreeTimeCombo->addItem( i18nc( "show event as free time", "Free" ) );
  classLayout->addWidget( mFreeTimeCombo );
  freeTimeLabel->setBuddy( mFreeTimeCombo );
}

void KOEditorGeneralEvent::initInvitationBar( QWidget *parent, QBoxLayout *layout )
{
  QBoxLayout *topLayout = new QHBoxLayout( layout );
  mInvitationBar = new QFrame( parent );
  topLayout->addWidget( mInvitationBar );

  QBoxLayout *barLayout = new QHBoxLayout( mInvitationBar );
  barLayout->setSpacing( layout->spacing() );
  QLabel *label =
    new QLabel( i18n( "You have not yet definitely responded to this invitation." ),
                mInvitationBar );
  barLayout->addWidget( label );
  barLayout->addStretch( 1 );
  QPushButton *button = new QPushButton( i18n( "Accept" ), mInvitationBar );
  connect( button, SIGNAL(clicked()), SIGNAL(acceptInvitation()) );
  connect( button, SIGNAL(clicked()), mInvitationBar, SLOT(hide()) );
  barLayout->addWidget( button );
  button = new QPushButton( i18n( "Decline" ), mInvitationBar );
  connect( button, SIGNAL(clicked()), SIGNAL(declineInvitation()) );
  connect( button, SIGNAL(clicked()), mInvitationBar, SLOT(hide()) );
  barLayout->addWidget( button );

  mInvitationBar->hide();
}

void KOEditorGeneralEvent::setTimeEditorsEnabled( bool enabled )
{
  mStartTimeEdit->setEnabled( enabled );
  mEndTimeEdit->setEnabled( enabled );

  if ( !enabled ) {
    mTimeZoneComboStart->setFloating( true );
    mTimeZoneComboEnd->setFloating( true );
  } else {
    mTimeZoneComboStart->selectLocalTimeSpec();
    mTimeZoneComboEnd->selectLocalTimeSpec();
    mStartSpec = mTimeZoneComboStart->selectedTimeSpec();
    mEndSpec = mTimeZoneComboEnd->selectedTimeSpec();
  }
  mTimeZoneComboStart->setEnabled( enabled );
  mTimeZoneComboEnd->setEnabled( enabled );

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::slotHasTimeCheckboxToggled( bool checked )
{
  setTimeEditorsEnabled( checked );
  //if(alarmButton->isChecked()) alarmStuffDisable(noTime);
  emit allDayChanged( !checked );
}

void KOEditorGeneralEvent::setDateTimes( const QDateTime &start, const QDateTime &end )
{
  setDateTimes( KDateTime( start, KOPrefs::instance()->timeSpec() ),
                KDateTime( end, KOPrefs::instance()->timeSpec() ) );
}

void KOEditorGeneralEvent::setDateTimes( const KDateTime &start, const KDateTime &end )
{
//  kDebug(5850) <<"KOEditorGeneralEvent::setDateTimes(): Start DateTime:" << start.toString();

  mStartDateEdit->setDate( start.date() );
  // KTimeEdit seems to emit some signals when setTime() is called.
  mStartTimeEdit->blockSignals( true );
  mStartTimeEdit->setTime( start.time() );
  mStartTimeEdit->blockSignals( false );
  mEndDateEdit->setDate( end.date() );
  mEndTimeEdit->setTime( end.time() );

  mCurrStartDateTime = start.dateTime();
  mCurrEndDateTime = end.dateTime();

  mTimeZoneComboStart->selectTimeSpec( start.timeSpec() );
  mTimeZoneComboEnd->selectTimeSpec( end.timeSpec() );

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::startTimeChanged( QTime newtime )
{
  int secsep = mCurrStartDateTime.secsTo( mCurrEndDateTime );

  mCurrStartDateTime.setTime( newtime );

  // adjust end time so that the event has the same duration as before.
  mCurrEndDateTime = mCurrStartDateTime.addSecs( secsep );
  mEndTimeEdit->setTime( mCurrEndDateTime.time() );
  mEndDateEdit->setDate( mCurrEndDateTime.date() );

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::endTimeChanged( QTime newtime )
{
  QDateTime newdt( mCurrEndDateTime.date(), newtime );
  mCurrEndDateTime = newdt;

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::startDateChanged( const QDate &newdate )
{
  if ( !newdate.isValid() ) {
    return;
  }

  int daysep = mCurrStartDateTime.daysTo( mCurrEndDateTime );
  mCurrStartDateTime.setDate( newdate );

  // adjust end date so that the event has the same duration as before
  mCurrEndDateTime.setDate( mCurrStartDateTime.date().addDays( daysep ) );
  mEndDateEdit->setDate( mCurrEndDateTime.date() );

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::endDateChanged( const QDate &newdate )
{
  if ( !newdate.isValid() ) {
    return;
  }

  QDateTime newdt( newdate, mCurrEndDateTime.time() );
  mCurrEndDateTime = newdt;

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::startSpecChanged()
{
  if ( mEndSpec == mStartSpec ) {
    mTimeZoneComboEnd->selectTimeSpec( mTimeZoneComboStart->selectedTimeSpec() );
  }
  mStartSpec = mTimeZoneComboStart->selectedTimeSpec();

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::endSpecChanged()
{
  mEndSpec = mTimeZoneComboEnd->selectedTimeSpec();

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::setDefaults( const QDateTime &from,
                                        const QDateTime &to, bool allDay )
{
  KOEditorGeneral::setDefaults( allDay );
  mHasTimeCheckbox->setChecked( !allDay );
  setTimeEditorsEnabled( !allDay );

  mTimeZoneComboStart->selectLocalTimeSpec();
  mTimeZoneComboEnd->selectLocalTimeSpec();
  mStartSpec = mTimeZoneComboStart->selectedTimeSpec();
  mEndSpec = mTimeZoneComboEnd->selectedTimeSpec();

  setDateTimes( from, to );
}

void KOEditorGeneralEvent::readEvent( Event *event, Calendar *calendar, bool isTemplate )
{
  mHasTimeCheckbox->setChecked( !event->allDay() );
  setTimeEditorsEnabled( !event->allDay() );

  if ( !isTemplate ) {
    // the rest is for the events only
    setDateTimes( event->dtStart(), event->dtEnd() );
  }

  switch( event->transparency() ) {
  case Event::Transparent:
    mFreeTimeCombo->setCurrentIndex( 1 );
    break;
  case Event::Opaque:
    mFreeTimeCombo->setCurrentIndex( 0 );
    break;
  }

  mRecurrenceSummary->setText( IncidenceFormatter::recurrenceString( event ) );

  Attendee *me = event->attendeeByMails( KOPrefs::instance()->allEmails() );
  if ( me &&
       ( me->status() == Attendee::NeedsAction ||
         me->status() == Attendee::Tentative ||
         me->status() == Attendee::InProcess ) ) {
    mInvitationBar->show();
  } else {
    mInvitationBar->hide();
  }

  readIncidence( event, calendar );
}

void KOEditorGeneralEvent::writeEvent( Event *event )
{
  writeIncidence( event );

  QDate tmpDate;
  QTime tmpTime;
  KDateTime tmpDT;

  // temp. until something better happens.
  QString tmpStr;

  if ( !mHasTimeCheckbox->isChecked() ) {
    event->setAllDay( true );

    // need to change this.
    tmpDate = mStartDateEdit->date();
    tmpDT.setDate( tmpDate );
    tmpDT.setDateOnly( true );
    tmpDT.setTimeSpec( mTimeZoneComboStart->selectedTimeSpec() );
    event->setDtStart( tmpDT );

    tmpDT.setTimeSpec( mTimeZoneComboEnd->selectedTimeSpec( ) );
    tmpDT.setDate( mEndDateEdit->date() );
    event->setDtEnd( tmpDT );
  } else {
    event->setAllDay( false );

    // set date/time end
    tmpDate = mEndDateEdit->date();
    tmpTime = mEndTimeEdit->getTime();
    tmpDT.setDate( tmpDate );
    tmpDT.setTime( tmpTime );
    tmpDT.setTimeSpec( mTimeZoneComboEnd->selectedTimeSpec() );
    event->setDtEnd( tmpDT );

    // set date/time start
    tmpDate = mStartDateEdit->date();
    tmpTime = mStartTimeEdit->getTime();
    event->setDtStart( KDateTime( tmpDate, tmpTime, mTimeZoneComboStart->selectedTimeSpec() ) );
  } // check for all-day

  event->setTransparency( mFreeTimeCombo->currentIndex() > 0 ?
                          KCal::Event::Transparent : KCal::Event::Opaque );
}

void KOEditorGeneralEvent::setDuration()
{
  QString tmpStr, catStr;
  int hourdiff, minutediff;
  // end<date is an accepted temporary state while typing, but don't show
  // any duration if this happens
  KDateTime startDateTime =
    KDateTime( mCurrStartDateTime, mTimeZoneComboStart->selectedTimeSpec() );
  KDateTime endDateTime =
    KDateTime( mCurrEndDateTime, mTimeZoneComboEnd->selectedTimeSpec() ).
    toTimeSpec( startDateTime.timeSpec() );
  if ( startDateTime < endDateTime ) {

    if ( !mHasTimeCheckbox->isChecked() ) {
      int daydiff = startDateTime.date().daysTo( endDateTime.date() ) + 1;
      tmpStr = i18n( "Duration: " );
      tmpStr.append( i18np( "1 Day", "%1 Days", daydiff ) );
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
        tmpStr = i18n( "Duration: " );
        if ( hourdiff ){
          catStr = i18np( "1 hour", "%1 hours", hourdiff );
          tmpStr.append( catStr );
        }
        if ( hourdiff && minutediff ) {
          tmpStr += i18n( ", " );
        }
        if ( minutediff ){
          catStr = i18np( "1 minute", "%1 minutes", minutediff );
          tmpStr += catStr;
        }
      } else {
        tmpStr = "";
      }
    }
  }
  mDurationLabel->setText( tmpStr );
  mDurationLabel->setWhatsThis(
    i18n( "Shows the duration of the event or to-do with the "
          "current start and end dates and times." ) );
}

void KOEditorGeneralEvent::emitDateTimeStr()
{
  KLocale *l = KGlobal::locale();

  QString from, to;
  if ( !mHasTimeCheckbox->isChecked() ) {
    from = l->formatDate( mCurrStartDateTime.date() );
    to = l->formatDate( mCurrEndDateTime.date() );
  } else {
    from = l->formatDateTime( mCurrStartDateTime );
    to = l->formatDateTime( mCurrEndDateTime );
  }
  QString str = i18n( "From: %1   To: %2   %3", from, to, mDurationLabel->text() );

  emit dateTimeStrChanged( str );
}

bool KOEditorGeneralEvent::validateInput()
{
  if ( mHasTimeCheckbox->isChecked() ) {
    if ( !mStartTimeEdit->inputIsValid() ) {
      KMessageBox::sorry( 0,
                          i18n( "Please specify a valid start time, for example '%1'.",
                                KGlobal::locale()->formatTime( QTime::currentTime() ) ) );
      return false;
    }

    if ( !mEndTimeEdit->inputIsValid() ) {
      KMessageBox::sorry( 0,
                          i18n( "Please specify a valid end time, for example '%1'.",
                                KGlobal::locale()->formatTime( QTime::currentTime() ) ) );
      return false;
    }
  }

  if ( !mStartDateEdit->date().isValid() ) {
    KMessageBox::sorry( 0,
                        i18n( "Please specify a valid start date, for example '%1'.",
                              KGlobal::locale()->formatDate( QDate::currentDate() ) ) );
    return false;
  }

  if ( !mEndDateEdit->date().isValid() ) {
    KMessageBox::sorry( 0,
                        i18n( "Please specify a valid end date, for example '%1'.",
                              KGlobal::locale()->formatDate( QDate::currentDate() ) ) );
    return false;
  }

  KDateTime startDt, endDt;
  startDt.setTimeSpec( mTimeZoneComboStart->selectedTimeSpec() );
  endDt.setTimeSpec( mTimeZoneComboEnd->selectedTimeSpec() );
  startDt.setDate( mStartDateEdit->date() );
  endDt.setDate( mEndDateEdit->date() );
  if ( mHasTimeCheckbox->isChecked() ) {
    startDt.setTime( mStartTimeEdit->getTime() );
    endDt.setTime( mEndTimeEdit->getTime() );
  }

  if ( startDt > endDt ) {
    KMessageBox::sorry( 0, i18n( "The event ends before it starts.\n"
                                 "Please correct dates and times." ) );
    return false;
  }

  return KOEditorGeneral::validateInput();
}

void KOEditorGeneralEvent::updateRecurrenceSummary( const QString &summary )
{
  mRecurrenceSummary->setText( summary );
}
