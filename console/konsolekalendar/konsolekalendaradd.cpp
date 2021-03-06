/*******************************************************************************
 * konsolekalendaradd.cpp                                                      *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>                      *
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation; either version 2 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/
/**
 * @file konsolekalendaradd.cpp
 * Provides the KonsoleKalendarAdd class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include "konsolekalendaradd.h"

#include <stdlib.h>
#include <iostream>

#include <QtCore/QDateTime>
#include <QtCore/QObject>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <ksystemtimezone.h>
#include <klocale.h>

#include <kcal/calendarlocal.h>
#include <kcal/calendar.h>
#include <kcal/event.h>

using namespace KCal;
using namespace std;

KonsoleKalendarAdd::KonsoleKalendarAdd( KonsoleKalendarVariables *vars )
{
  m_variables = vars;
}

KonsoleKalendarAdd::~KonsoleKalendarAdd()
{
}

/**
 * Adds event to Calendar
 */

bool KonsoleKalendarAdd::addEvent()
{
  bool status = true;

  kDebug() << "konsolekalendaradd.cpp::addEvent()";

  if ( m_variables->isDryRun() ) {
    cout << i18n( "Insert Event &lt;Dry Run&gt;:" ).toLocal8Bit().data()
         << endl;
    printSpecs();
  } else {
    if ( m_variables->isVerbose() ) {
      cout << i18n( "Insert Event &lt;Verbose&gt;:" ).toLocal8Bit().data()
           << endl;
      printSpecs();
    }

    Event *event = new Event();

    KDateTime::Spec timeSpec = m_variables->getCalendar()->timeSpec();
    event->setDtStart( KDateTime( m_variables->getStartDateTime(), timeSpec ) );
    event->setDtEnd( KDateTime( m_variables->getEndDateTime(), timeSpec ) );
    event->setSummary( m_variables->getSummary() );
    event->setAllDay( m_variables->getFloating() );
    event->setDescription( m_variables->getDescription() );
    event->setLocation( m_variables->getLocation() );

    if ( m_variables->getCalendar()->addEvent( event ) ) {
      cout << i18n( "Success: \"%1\" inserted",
                    m_variables->getSummary() ).toLocal8Bit().data()
           << endl;

        m_variables->getCalendar()->save();

    } else {
      cout << i18n( "Failure: \"%1\" not inserted",
                    m_variables->getSummary() ).toLocal8Bit().data()
           << endl;
      status = false;
    }
  }

  kDebug() << "konsolekalendaradd.cpp::addEvent() | Done";
  return status;
}

bool KonsoleKalendarAdd::addImportedCalendar()
{

// If --file specified, then import into that file
// else, import into the standard calendar

  QString fileName;
  if ( m_variables->getCalendarFile().isEmpty() ) {
    fileName = KStandardDirs::locateLocal( "data", "korganizer/std.ics" );
  } else {
    fileName = m_variables->getCalendarFile();
  }

  CalendarLocal *cal = new CalendarLocal( KSystemTimeZones::local() );
  if ( !cal->load( fileName ) ||
       !cal->load( m_variables->getImportFile() ) ||
       !cal->save( fileName ) ) {
    kDebug() << "konsolekalendaradd.cpp::importCalendar() |"
             << "Can't import file:"
             << m_variables->getImportFile();
    return false;
  }
  kDebug() << "konsolekalendaradd.cpp::importCalendar() |"
           << "Successfully imported file:"
           << m_variables->getImportFile();
  return true;
}

void KonsoleKalendarAdd::printSpecs()
{
  cout << i18n( "  What:  %1",
                m_variables->getSummary() ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  Begin: %1",
                m_variables->getStartDateTime().toString( Qt::TextDate ) ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  End:   %1",
                m_variables->getEndDateTime().toString( Qt::TextDate ) ).toLocal8Bit().data()
       << endl;

  if ( m_variables->getFloating() == true ) {
    cout << i18n( "  No Time Associated with Event" ).toLocal8Bit().data()
         << endl;
  }

  cout << i18n( "  Desc:  %1",
                m_variables->getDescription() ).toLocal8Bit().data()
       << endl;

  cout << i18n( "  Location:  %1",
                m_variables->getLocation() ).toLocal8Bit().data()
       << endl;
}
