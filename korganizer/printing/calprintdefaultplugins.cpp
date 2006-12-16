/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KORG_NOPRINTER

#include <qpainter.h>
#include <qdatetimeedit.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qbuttongroup.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kcalendarsystem.h>

#include "calprintdefaultplugins.h"

#include "calprintincidenceconfig_base.h"
#include "calprintdayconfig_base.h"
#include "calprintweekconfig_base.h"
#include "calprintmonthconfig_base.h"
#include "calprinttodoconfig_base.h"


/**************************************************************
 *           Print Incidence
 **************************************************************/

CalPrintIncidence::CalPrintIncidence() : CalPrintPluginBase()
{
}

CalPrintIncidence::~CalPrintIncidence()
{
}

QWidget *CalPrintIncidence::createConfigWidget( QWidget *w )
{
  return new CalPrintIncidenceConfig_Base( w );
}

void CalPrintIncidence::readSettingsWidget()
{
  CalPrintIncidenceConfig_Base *cfg =
      dynamic_cast<CalPrintIncidenceConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    mUseColors = cfg->mColors->isChecked();
    mShowOptions = cfg->mShowDetails->isChecked();
    mShowSubitemsNotes = cfg->mShowSubitemsNotes->isChecked();
    mShowAttendees = cfg->mShowAttendees->isChecked();
    mShowAttachments = cfg->mShowAttachments->isChecked();
  }
}

void CalPrintIncidence::setSettingsWidget()
{
  CalPrintIncidenceConfig_Base *cfg =
      dynamic_cast<CalPrintIncidenceConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mColors->setChecked( mUseColors );
    cfg->mShowDetails->setChecked(mShowOptions);
    cfg->mShowSubitemsNotes->setChecked(mShowSubitemsNotes);
    cfg->mShowAttendees->setChecked(mShowAttendees);
    cfg->mShowAttachments->setChecked(mShowAttachments);
  }
}

void CalPrintIncidence::loadConfig()
{
  if ( mConfig ) {
    mUseColors = mConfig->readBoolEntry( "Use Colors", false );
    mShowOptions = mConfig->readBoolEntry( "Show Options", false );
    mShowSubitemsNotes = mConfig->readBoolEntry( "Show Subitems and Notes", false );
    mShowAttendees = mConfig->readBoolEntry( "Use Attendees", false );
    mShowAttachments = mConfig->readBoolEntry( "Use Attachments", false );
  }
  setSettingsWidget();
}

void CalPrintIncidence::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Use Colors", mUseColors );
    mConfig->writeEntry( "Show Options", mShowOptions );
    mConfig->writeEntry( "Show Subitems and Notes", mShowSubitemsNotes );
    mConfig->writeEntry( "Use Attendees", mShowAttendees );
    mConfig->writeEntry( "Use Attachments", mShowAttachments );
  }
}

void CalPrintIncidence::print( QPainter &p, int width, int height )
{
  KLocale *local = KGlobal::locale();

  QFont oldFont(p.font());
//   QFont textFont( "helvetica", 12, QFont::Normal );
//   QFont captionFont( "helvetica", 12, QFont::Normal );
  QFont textFont( "Tahoma", 11, QFont::Normal );
  QFont captionFont( "Tahoma", 11, QFont::Normal );
  p.setFont( textFont );
  int lineHeight = p.fontMetrics().lineSpacing();


  Incidence::List::ConstIterator it;
  for ( it=mSelectedIncidences.begin(); it!=mSelectedIncidences.end(); ++it ) {
    // don't do anything on a 0-pointer!
    if ( !(*it) ) continue;
    if ( it != mSelectedIncidences.begin() ) mPrinter->newPage();


    // PAGE Layout:
    //  +-----------------------------------+
    //  | Header:  Summary                  |
    //  +===================================+
    //  | start: ______   end: _________    |
    //  | repeats: ___________________      |
    //  | reminder: __________________      |
    //  +-----------------------------------+
    //  | Location: ______________________  |
    //  +------------------------+----------+
    //  | Description:           | Notes or |
    //  |                        | Subitems |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  +------------------------+----------+
    //  | Attachments:           | Settings |
    //  |                        |          |
    //  +------------------------+          |
    //  | Attendees:             |          |
    //  |                        |          |
    //  +------------------------+----------+
    //  | Categories: _____________________ |
    //  +-----------------------------------+

    QRect box( 0, 0, width, height );
    QRect titleBox( box );
    titleBox.setHeight( headerHeight() );
    // Draw summary as header, no small calendars in title bar, expand height if needed
    int titleBottom = drawHeader( p, (*it)->summary(), QDate(), QDate(), titleBox, true );
    titleBox.setBottom( titleBottom );

    QRect timesBox( titleBox );
    timesBox.setTop( titleBox.bottom() + padding() );
    // TODO: Good default height of the times box
    timesBox.setHeight( height / 8 );
    drawBox( p, BOX_BORDER_WIDTH, timesBox );
    // TODO: Contents of the times box

QString temp = i18n("Start date: ");
temp += i18n("End date: " );
temp += i18n("Due date: " );

temp += i18n("Start time: ");
temp += i18n("End time: " );
temp += i18n("Due time: " );

temp += i18n("No start date");
temp += i18n("No end date");
temp += i18n("No due date");

temp += i18n("Repeats: ");


if ( (*it)->alarms().count()==1 ) {
  temp += i18n("Reminder: ");
}

if ( (*it)->alarms().count() == 0 )
  temp += i18n("No reminders");
else
  temp += i18n("1 reminder: ", "%n reminders: ", (*it)->alarms().count() );
temp += i18n("Organizer: ");
temp += i18n("%1 %2 before start");
temp += i18n("%1 %2 before end");
temp += i18n("%1 %2 before due time");
temp += i18n("%1 %2 after start");
temp += i18n("%1 %2 after end");
temp += i18n("%1 %2 after due time");




    QRect locationBox( timesBox );
    locationBox.setTop( timesBox.bottom() + padding() );
    locationBox.setHeight( 0 );
    int locationBottom = drawBoxWithCaption( p, locationBox, i18n("Location: "),
         (*it)->location(), /*sameLine=*/true, /*expand=*/true );
    locationBox.setBottom( locationBottom );

    // Now start constructing the boxes from the bottom:
    QRect categoriesBox( locationBox );
    categoriesBox.setBottom( box.bottom() );
    categoriesBox.setTop( categoriesBox.bottom() - lineHeight - 2*padding() );

    QRect attendeesBox( box.left(), categoriesBox.top()-padding()-box.height()/9, box.width()*3/4 - padding(), box.height()/9 );
    QRect attachmentsBox( box.left(), attendeesBox.top()-padding()-box.height()/9, box.width()*3/4 - padding(), box.height()/9 );
    QRect optionsBox( attachmentsBox.right() + padding(), attachmentsBox.top(), 0, 0 );
    optionsBox.setRight( box.right() );
    optionsBox.setBottom( attendeesBox.bottom() );

    QRect notesBox( optionsBox.left(), locationBox.bottom() + padding(), optionsBox.width(), 0 );
    notesBox.setBottom( optionsBox.top() - padding() );

    QRect descriptionBox( notesBox );
    descriptionBox.setLeft( box.left() );
    descriptionBox.setRight( attachmentsBox.right() );

    drawBoxWithCaption( p, descriptionBox, i18n("Description:"), (*it)->description(), /*sameLine=*/false );
    QString subitemCaption = i18n("Subitems:");
    if ( (*it)->relations().isEmpty() ) {
      int notesStart = drawBoxWithCaption( p, notesBox, i18n("Notes:"), QString::null, /*sameLine=*/false );
      // TODO: Draw lines for writing notes
    } else {
      int subitemsStart = drawBoxWithCaption( p, notesBox, i18n("Subitems:"), (*it)->description(), /*sameLine=*/false );
      // TODO: Draw subitems
    }

    int attachStart = drawBoxWithCaption( p, attachmentsBox, i18n("Attachments:"), QString::null );
    // TODO: Print out the attachments somehow

    Attendee::List attendees = (*it)->attendees();
    QString attendeeCaption;
    if ( attendees.count() == 0 )
      attendeeCaption = i18n("No Attendees");
    else
      attendeeCaption = i18n("1 Attendee:", "%n Attendees:", attendees.count() );
    QString attendeeString;
    for ( Attendee::List::ConstIterator ait = attendees.begin(); ait != attendees.end(); ++ait ) {
      if ( !attendeeString.isEmpty() ) attendeeString += "\n";
      attendeeString += i18n("Formatting of an attendee: "
             "'Name (Role): Status', e.g. 'Reinhold Kainhofer "
             "<reinhold@kainhofer.com> (Participant): Awaiting Response'",
             "%1 (%2): %3")
                     .arg( (*ait)->fullName() )
                     .arg( (*ait)->roleStr() ).arg( (*ait)->statusStr() );
    }
    int attendeesStart = drawBoxWithCaption( p, attendeesBox, i18n("Attendees:"), attendeeString, /*sameLine=*/false );

    QString optionsString = i18n("Status: %1").arg( (*it)->statusStr() );
    optionsString += "\n";
    optionsString += i18n("Secrecy: %1").arg( (*it)->secrecyStr() );
    optionsString += "\n";
//     optionsString += i18n("Privacy: %1").arg( (*it)->secrecy() );
//     optionsString += "\n";
    if ( (*it)->type() == "Event" ) {
      Event *e = static_cast<Event*>(*it);
      if ( e->transparency() == Event::Opaque ) {
        optionsString += i18n("Show as: Busy");
      } else {
        optionsString += i18n("Show as: Free");
      }
      optionsString += "\n";
    } else if ( (*it)->type() == "Todo" ) {
      Todo *t = static_cast<Todo*>(*it);
      if ( t->isOverdue() ) {
        optionsString += i18n("This task is overdue!");
        optionsString += "\n";
      }
    } else if ( (*it)->type() == "Journal" ) {
      //TODO: Anything Journal-specific?
    }

    drawBoxWithCaption( p, optionsBox, i18n("Settings: "),
           optionsString, /*sameLine=*/false );
    drawBoxWithCaption( p, categoriesBox, i18n("Categories: "),
           (*it)->categories().join( i18n("Spacer for the joined list of categories", ", ") ),
           /*sameLine=*/true, /*expand=*/false );

  }
  p.setFont( oldFont );
}

/**************************************************************
 *           Print Day
 **************************************************************/

CalPrintDay::CalPrintDay() : CalPrintPluginBase()
{
}

CalPrintDay::~CalPrintDay()
{
}

QWidget *CalPrintDay::createConfigWidget( QWidget *w )
{
  return new CalPrintDayConfig_Base( w );
}

void CalPrintDay::readSettingsWidget()
{
  CalPrintDayConfig_Base *cfg =
      dynamic_cast<CalPrintDayConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mStartTime = cfg->mFromTime->time();
    mEndTime = cfg->mToTime->time();
    mIncludeAllEvents = cfg->mIncludeAllEvents->isChecked();

    mIncludeTodos = cfg->mIncludeTodos->isChecked();
    mUseColors = cfg->mColors->isChecked();
  }
}

void CalPrintDay::setSettingsWidget()
{
  CalPrintDayConfig_Base *cfg =
      dynamic_cast<CalPrintDayConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mFromTime->setTime( mStartTime );
    cfg->mToTime->setTime( mEndTime );
    cfg->mIncludeAllEvents->setChecked( mIncludeAllEvents );

    cfg->mIncludeTodos->setChecked( mIncludeTodos );
    cfg->mColors->setChecked( mUseColors );
  }
}

void CalPrintDay::loadConfig()
{
  if ( mConfig ) {
    QDate dt;
    QTime tm1( dayStart() );
    QDateTime startTm( dt, tm1 );
    QDateTime endTm( dt, tm1.addSecs( 12 * 60 * 60 ) );
    mStartTime = mConfig->readDateTimeEntry( "Start time", &startTm ).time();
    mEndTime = mConfig->readDateTimeEntry( "End time", &endTm ).time();
    mIncludeTodos = mConfig->readBoolEntry( "Include todos", false );
    mIncludeAllEvents = mConfig->readBoolEntry( "Include all events", false );
  }
  setSettingsWidget();
}

void CalPrintDay::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Start time", QDateTime( QDate(), mStartTime ) );
    mConfig->writeEntry( "End time", QDateTime( QDate(), mEndTime ) );
    mConfig->writeEntry( "Include todos", mIncludeTodos );
    mConfig->writeEntry( "Include all events", mIncludeAllEvents );
  }
}

void CalPrintDay::setDateRange( const QDate& from, const QDate& to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintDayConfig_Base *cfg =
      dynamic_cast<CalPrintDayConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintDay::print( QPainter &p, int width, int height )
{
  QDate curDay( mFromDate );

  do {
    QTime curStartTime( mStartTime );
    QTime curEndTime( mEndTime );

    // For an invalid time range, simply show one hour, starting at the hour
    // before the given start time
    if ( curEndTime <= curStartTime ) {
      curStartTime = QTime( curStartTime.hour(), 0, 0 );
      curEndTime = curStartTime.addSecs( 3600 );
    }

    KLocale *local = KGlobal::locale();
    QRect headerBox( 0, 0, width, headerHeight() );
    drawHeader( p, local->formatDate( curDay ), curDay, QDate(), headerBox );


    Event::List eventList = mCalendar->events( curDay,
                                               EventSortStartDate,
                                               SortDirectionAscending );

    p.setFont( QFont( "helvetica", 12 ) );

    // TODO: Find a good way to determine the height of the all-day box
    QRect allDayBox( TIMELINE_WIDTH + padding(), headerBox.bottom() + padding(),
                     0, height / 20 );
    allDayBox.setRight( width );
    int allDayHeight = drawAllDayBox( p, eventList, curDay, true, allDayBox );

    QRect dayBox( allDayBox );
    dayBox.setTop( allDayHeight /*allDayBox.bottom()*/ );
    dayBox.setBottom( height );
    drawAgendaDayBox( p, eventList, curDay, mIncludeAllEvents,
                      curStartTime, curEndTime, dayBox );

    QRect tlBox( dayBox );
    tlBox.setLeft( 0 );
    tlBox.setWidth( TIMELINE_WIDTH );
    drawTimeLine( p, curStartTime, curEndTime, tlBox );
    curDay = curDay.addDays( 1 );
    if ( curDay <= mToDate ) mPrinter->newPage();
  } while ( curDay <= mToDate );
}



/**************************************************************
 *           Print Week
 **************************************************************/

CalPrintWeek::CalPrintWeek() : CalPrintPluginBase()
{
}

CalPrintWeek::~CalPrintWeek()
{
}

QWidget *CalPrintWeek::createConfigWidget( QWidget *w )
{
  return new CalPrintWeekConfig_Base( w );
}

void CalPrintWeek::readSettingsWidget()
{
  CalPrintWeekConfig_Base *cfg =
      dynamic_cast<CalPrintWeekConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mWeekPrintType = (eWeekPrintType)( cfg->mPrintType->id(
      cfg->mPrintType->selected() ) );

    mStartTime = cfg->mFromTime->time();
    mEndTime = cfg->mToTime->time();

    mIncludeTodos = cfg->mIncludeTodos->isChecked();
    mUseColors = cfg->mColors->isChecked();
  }
}

void CalPrintWeek::setSettingsWidget()
{
  CalPrintWeekConfig_Base *cfg =
      dynamic_cast<CalPrintWeekConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mPrintType->setButton( mWeekPrintType );

    cfg->mFromTime->setTime( mStartTime );
    cfg->mToTime->setTime( mEndTime );

    cfg->mIncludeTodos->setChecked( mIncludeTodos );
    cfg->mColors->setChecked( mUseColors );
  }
}

void CalPrintWeek::loadConfig()
{
  if ( mConfig ) {
    QDate dt;
    QTime tm1( dayStart() );
    QDateTime startTm( dt, tm1  );
    QDateTime endTm( dt, tm1.addSecs( 43200 ) );
    mStartTime = mConfig->readDateTimeEntry( "Start time", &startTm ).time();
    mEndTime = mConfig->readDateTimeEntry( "End time", &endTm ).time();
    mIncludeTodos = mConfig->readBoolEntry( "Include todos", false );
    mWeekPrintType =(eWeekPrintType)( mConfig->readNumEntry( "Print type", (int)Filofax ) );
  }
  setSettingsWidget();
}

void CalPrintWeek::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Start time", QDateTime( QDate(), mStartTime ) );
    mConfig->writeEntry( "End time", QDateTime( QDate(), mEndTime ) );
    mConfig->writeEntry( "Include todos", mIncludeTodos );
    mConfig->writeEntry( "Print type", int( mWeekPrintType ) );
  }
}

KPrinter::Orientation CalPrintWeek::defaultOrientation()
{
  if ( mWeekPrintType == Filofax ) return KPrinter::Portrait;
  else if ( mWeekPrintType == SplitWeek ) return KPrinter::Portrait;
  else return KPrinter::Landscape;
}

void CalPrintWeek::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintWeekConfig_Base *cfg =
      dynamic_cast<CalPrintWeekConfig_Base*>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintWeek::print( QPainter &p, int width, int height )
{
  QDate curWeek, fromWeek, toWeek;

  // correct begin and end to first and last day of week
  int weekdayCol = weekdayColumn( mFromDate.dayOfWeek() );
  fromWeek = mFromDate.addDays( -weekdayCol );
  weekdayCol = weekdayColumn( mFromDate.dayOfWeek() );
  toWeek = mToDate.addDays( 6 - weekdayCol );

  curWeek = fromWeek.addDays( 6 );
  KLocale *local = KGlobal::locale();

  QString line1, line2, title;
  QRect headerBox( 0, 0, width, headerHeight() );
  QRect weekBox( headerBox );
  weekBox.setTop( headerBox.bottom() + padding() );
  weekBox.setBottom( height );

  switch ( mWeekPrintType ) {
    case Filofax:
      do {
        line1 = local->formatDate( curWeek.addDays( -6 ) );
        line2 = local->formatDate( curWeek );
        if ( orientation() == KPrinter::Landscape ) {
          title = i18n("date from-to", "%1 - %2");
        } else {
          title = i18n("date from-\nto", "%1 -\n%2");;
        }
        title = title.arg( line1 ).arg( line2 );
        drawHeader( p, title, curWeek.addDays( -6 ), QDate(), headerBox );
        drawWeek( p, curWeek, weekBox );
        curWeek = curWeek.addDays( 7 );
        if ( curWeek <= toWeek )
          mPrinter->newPage();
      } while ( curWeek <= toWeek );
      break;

    case Timetable:
    default:
      do {
        line1 = local->formatDate( curWeek.addDays( -6 ) );
        line2 = local->formatDate( curWeek );
        if ( orientation() == KPrinter::Landscape ) {
          title = i18n("date from - to (Week %1)", "%1 - %2 (Week %3)");
        } else {
          title = i18n("date from -\nto (Week %1)", "%1 -\n%2 (Week %3)");
        }
        title = title.arg( line1 ).arg( line2 ).arg( curWeek.weekNumber() );
        drawHeader( p, title, curWeek, QDate(), headerBox );
        QRect weekBox( headerBox );
        weekBox.setTop( headerBox.bottom() + padding() );
        weekBox.setBottom( height );

        drawTimeTable( p, fromWeek, curWeek, mStartTime, mEndTime, weekBox );
        fromWeek = fromWeek.addDays( 7 );
        curWeek = fromWeek.addDays( 6 );
        if ( curWeek <= toWeek )
          mPrinter->newPage();
      } while ( curWeek <= toWeek );
      break;

    case SplitWeek: {
      QRect weekBox1( weekBox );
      // On the left side there are four days (mo-th) plus the timeline,
      // on the right there are only three days (fr-su) plus the timeline. Don't
      // use the whole width, but rather give them the same width as on the left.
      weekBox1.setRight( int( ( width - TIMELINE_WIDTH ) * 3. / 4. + TIMELINE_WIDTH ) );
      do {
        QDate endLeft( fromWeek.addDays( 3 ) );
        int hh = headerHeight();

        drawTimeTable( p, fromWeek, endLeft,
                       mStartTime, mEndTime, weekBox );
        mPrinter->newPage();
        drawSplitHeaderRight( p, fromWeek, curWeek, QDate(), width, hh );
        drawTimeTable( p, endLeft.addDays( 1 ), curWeek,
                       mStartTime, mEndTime, weekBox1 );

        fromWeek = fromWeek.addDays( 7 );
        curWeek = fromWeek.addDays( 6 );
        if ( curWeek <= toWeek )
          mPrinter->newPage();
      } while ( curWeek <= toWeek );
      }
      break;
  }
}




/**************************************************************
 *           Print Month
 **************************************************************/

CalPrintMonth::CalPrintMonth() : CalPrintPluginBase()
{
}

CalPrintMonth::~CalPrintMonth()
{
}

QWidget *CalPrintMonth::createConfigWidget( QWidget *w )
{
  return new CalPrintMonthConfig_Base( w );
}

void CalPrintMonth::readSettingsWidget()
{
  CalPrintMonthConfig_Base *cfg =
      dynamic_cast<CalPrintMonthConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mWeekNumbers =  cfg->mWeekNumbers->isChecked();
    mRecurDaily = cfg->mRecurDaily->isChecked();
    mRecurWeekly = cfg->mRecurWeekly->isChecked();
    mIncludeTodos = cfg->mIncludeTodos->isChecked();
//    mUseColors = cfg->mColors->isChecked();
  }
}

void CalPrintMonth::setSettingsWidget()
{
  CalPrintMonthConfig_Base *cfg =
      dynamic_cast<CalPrintMonthConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mWeekNumbers->setChecked( mWeekNumbers );
    cfg->mRecurDaily->setChecked( mRecurDaily );
    cfg->mRecurWeekly->setChecked( mRecurWeekly );
    cfg->mIncludeTodos->setChecked( mIncludeTodos );
//    cfg->mColors->setChecked( mUseColors );
  }
}

void CalPrintMonth::loadConfig()
{
  if ( mConfig ) {
    mWeekNumbers = mConfig->readBoolEntry( "Print week numbers", true );
    mRecurDaily = mConfig->readBoolEntry( "Print daily incidences", true );
    mRecurWeekly = mConfig->readBoolEntry( "Print weekly incidences", true );
    mIncludeTodos = mConfig->readBoolEntry( "Include todos", false );
  }
  setSettingsWidget();
}

void CalPrintMonth::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Print week numbers", mWeekNumbers );
    mConfig->writeEntry( "Print daily incidences", mRecurDaily );
    mConfig->writeEntry( "Print weekly incidences", mRecurWeekly );
    mConfig->writeEntry( "Include todos", mIncludeTodos );
  }
}

void CalPrintMonth::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintMonthConfig_Base *cfg =
      dynamic_cast<CalPrintMonthConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintMonth::print( QPainter &p, int width, int height )
{
  QDate curMonth, fromMonth, toMonth;

  fromMonth = mFromDate.addDays( -( mFromDate.day() - 1 ) );
  toMonth = mToDate.addDays( mToDate.daysInMonth() - mToDate.day() );

  curMonth = fromMonth;
  const KCalendarSystem *calSys = calendarSystem();
  if ( !calSys ) return;

  QRect headerBox( 0, 0, width, headerHeight() );
  QRect monthBox( 0, 0, width, height );
  monthBox.setTop( headerBox.bottom() + padding() );

  do {
    QString title( i18n("monthname year", "%1 %2") );
    title = title.arg( calSys->monthName( curMonth ) )
                 .arg( curMonth.year() );
    QDate tmp( fromMonth );
    int weekdayCol = weekdayColumn( tmp.dayOfWeek() );
    tmp = tmp.addDays( -weekdayCol );

    drawHeader( p, title, curMonth.addMonths( -1 ), curMonth.addMonths( 1 ),
                headerBox );
    drawMonthTable( p, curMonth, mWeekNumbers, mRecurDaily, mRecurWeekly, monthBox );
    curMonth = curMonth.addDays( curMonth.daysInMonth() );
    if ( curMonth <= toMonth ) mPrinter->newPage();
  } while ( curMonth <= toMonth );

}




/**************************************************************
 *           Print Todos
 **************************************************************/

CalPrintTodos::CalPrintTodos() : CalPrintPluginBase()
{
  mTodoSortField = TodoFieldUnset;
  mTodoSortDirection = TodoDirectionUnset;
}

CalPrintTodos::~CalPrintTodos()
{
}

QWidget *CalPrintTodos::createConfigWidget( QWidget *w )
{
  return new CalPrintTodoConfig_Base( w );
}

void CalPrintTodos::readSettingsWidget()
{
  CalPrintTodoConfig_Base *cfg =
      dynamic_cast<CalPrintTodoConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    mPageTitle = cfg->mTitle->text();

    mTodoPrintType = (eTodoPrintType)( cfg->mPrintType->id(
      cfg->mPrintType->selected() ) );

    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mIncludeDescription = cfg->mDescription->isChecked();
    mIncludePriority = cfg->mPriority->isChecked();
    mIncludeDueDate = cfg->mDueDate->isChecked();
    mIncludePercentComplete = cfg->mPercentComplete->isChecked();
    mConnectSubTodos = cfg->mConnectSubTodos->isChecked();
    mStrikeOutCompleted = cfg->mStrikeOutCompleted->isChecked();

    mTodoSortField = (eTodoSortField)cfg->mSortField->currentItem();
    mTodoSortDirection = (eTodoSortDirection)cfg->mSortDirection->currentItem();
  }
}

void CalPrintTodos::setSettingsWidget()
{
  kdDebug() << "CalPrintTodos::setSettingsWidget" << endl;

  CalPrintTodoConfig_Base *cfg =
      dynamic_cast<CalPrintTodoConfig_Base *>( mConfigWidget );
  if ( cfg ) {
    cfg->mTitle->setText( mPageTitle );

    cfg->mPrintType->setButton( mTodoPrintType );

    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mDescription->setChecked( mIncludeDescription );
    cfg->mPriority->setChecked( mIncludePriority );
    cfg->mDueDate->setChecked( mIncludeDueDate );
    cfg->mPercentComplete->setChecked( mIncludePercentComplete );
    cfg->mConnectSubTodos->setChecked( mConnectSubTodos );
    cfg->mStrikeOutCompleted->setChecked( mStrikeOutCompleted );

    if ( mTodoSortField != TodoFieldUnset ) {
      // do not insert if already done so.
      cfg->mSortField->insertItem( i18n("Summary") );
      cfg->mSortField->insertItem( i18n("Start Date") );
      cfg->mSortField->insertItem( i18n("Due Date") );
      cfg->mSortField->insertItem( i18n("Priority") );
      cfg->mSortField->insertItem( i18n("Percent Complete") );
      cfg->mSortField->setCurrentItem( (int)mTodoSortField );
    }

    if ( mTodoSortDirection != TodoDirectionUnset ) {
      // do not insert if already done so.
      cfg->mSortDirection->insertItem( i18n("Ascending") );
      cfg->mSortDirection->insertItem( i18n("Descending") );
      cfg->mSortDirection->setCurrentItem( (int)mTodoSortDirection );
    }
  }
}

void CalPrintTodos::loadConfig()
{
  if ( mConfig ) {
    mPageTitle = mConfig->readEntry( "Page title", i18n("To-do list") );
    mTodoPrintType = (eTodoPrintType)mConfig->readNumEntry( "Print type", (int)TodosAll );
    mIncludeDescription = mConfig->readBoolEntry( "Include description", true );
    mIncludePriority = mConfig->readBoolEntry( "Include priority", true );
    mIncludeDueDate = mConfig->readBoolEntry( "Include due date", true );
    mIncludePercentComplete = mConfig->readBoolEntry( "Include percentage completed", true );
    mConnectSubTodos = mConfig->readBoolEntry( "Connect subtodos", true );
    mStrikeOutCompleted = mConfig->readBoolEntry( "Strike out completed summaries",  true );
    mTodoSortField = (eTodoSortField)mConfig->readNumEntry( "Sort field", (int)TodoFieldSummary );
    mTodoSortDirection = (eTodoSortDirection)mConfig->readNumEntry( "Sort direction", (int)TodoDirectionAscending );
  }
  setSettingsWidget();
}

void CalPrintTodos::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    mConfig->writeEntry( "Page title", mPageTitle );
    mConfig->writeEntry( "Print type", int( mTodoPrintType ) );
    mConfig->writeEntry( "Include description", mIncludeDescription );
    mConfig->writeEntry( "Include priority", mIncludePriority );
    mConfig->writeEntry( "Include due date", mIncludeDueDate );
    mConfig->writeEntry( "Include percentage completed", mIncludePercentComplete );
    mConfig->writeEntry( "Connect subtodos", mConnectSubTodos );
    mConfig->writeEntry( "Strike out completed summaries", mStrikeOutCompleted );
    mConfig->writeEntry( "Sort field", mTodoSortField );
    mConfig->writeEntry( "Sort direction", mTodoSortDirection );
  }
}

void CalPrintTodos::print( QPainter &p, int width, int height )
{
  // TODO: Find a good way to guarantee a nicely designed output
  int pospriority = 10;
  int possummary = 60;
  int posdue = width - 65;
  int poscomplete = posdue - 70; //Complete column is to right of the Due column
  int lineSpacing = 15;
  int fontHeight = 10;

  // Draw the First Page Header
  drawHeader( p, mPageTitle, mFromDate, QDate(),
                       QRect( 0, 0, width, headerHeight() ) );

  // Draw the Column Headers
  int mCurrentLinePos = headerHeight() + 5;
  QString outStr;
  QFont oldFont( p.font() );

  p.setFont( QFont( "helvetica", 10, QFont::Bold ) );
  lineSpacing = p.fontMetrics().lineSpacing();
  mCurrentLinePos += lineSpacing;
  if ( mIncludePriority ) {
    outStr += i18n( "Priority" );
    p.drawText( pospriority, mCurrentLinePos - 2, outStr );
  } else {
    possummary = 10;
    pospriority = -1;
  }

  outStr.truncate( 0 );
  outStr += i18n( "Summary" );
  p.drawText( possummary, mCurrentLinePos - 2, outStr );

  if ( mIncludePercentComplete ) {
    if ( !mIncludeDueDate ) //move Complete column to the right
      poscomplete = posdue; //if not print the Due Date column
    outStr.truncate( 0 );
    outStr += i18n( "Complete" );
    p.drawText( poscomplete, mCurrentLinePos - 2, outStr );
  } else {
    poscomplete = -1;
  }

  if ( mIncludeDueDate ) {
    outStr.truncate( 0 );
    outStr += i18n( "Due" );
    p.drawText( posdue, mCurrentLinePos - 2, outStr );
  } else {
    posdue = -1;
  }

  p.setFont( QFont( "helvetica", 10 ) );
  fontHeight = p.fontMetrics().height();

  Todo::List todoList;
  Todo::List tempList;
  Todo::List::ConstIterator it;

  // Convert sort options to the corresponding enums
  TodoSortField sortField = TodoSortSummary;
  switch( mTodoSortField ) {
  case TodoFieldSummary:
    sortField = TodoSortSummary; break;
  case TodoFieldStartDate:
    sortField = TodoSortStartDate; break;
  case TodoFieldDueDate:
    sortField = TodoSortDueDate; break;
  case TodoFieldPriority:
    sortField = TodoSortPriority; break;
  case TodoFieldPercentComplete:
    sortField = TodoSortPercentComplete; break;
  case TodoFieldUnset:
    break;
  }

  SortDirection sortDirection;
  switch( mTodoSortDirection ) {
  case TodoDirectionAscending:
    sortDirection = SortDirectionAscending; break;
  case TodoDirectionDescending:
    sortDirection = SortDirectionDescending; break;
  case TodoDirectionUnset:
    break;
  }

  // Create list of to-dos which will be printed
  todoList = mCalendar->todos( sortField,  sortDirection );
  switch( mTodoPrintType ) {
  case TodosAll:
    break;
  case TodosUnfinished:
    for( it = todoList.begin(); it!= todoList.end(); ++it ) {
      if ( !(*it)->isCompleted() )
        tempList.append( *it );
    }
    todoList = tempList;
    break;
  case TodosDueRange:
    for( it = todoList.begin(); it!= todoList.end(); ++it ) {
      if ( (*it)->hasDueDate() ) {
        if ( (*it)->dtDue().date() >= mFromDate &&
             (*it)->dtDue().date() <= mToDate )
          tempList.append( *it );
      } else {
        tempList.append( *it );
      }
    }
    todoList = tempList;
    break;
  }

  // Print to-dos
  int count = 0;
  for ( it=todoList.begin(); it!=todoList.end(); ++it ) {
    Todo *currEvent = *it;

    // Skip sub-to-dos. They will be printed recursively in drawTodo()
    if ( !currEvent->relatedTo() ) {
      count++;
      drawTodo( count, currEvent, p,
                         sortField, sortDirection,
                         mConnectSubTodos,
                         mStrikeOutCompleted, mIncludeDescription,
                         pospriority, possummary, posdue, poscomplete,
                         0, 0, mCurrentLinePos, width, height, todoList );
    }
  }
  p.setFont( oldFont );
}


#endif
