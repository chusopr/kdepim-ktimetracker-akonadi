/*******************************************************************************
 * konsolekalendarexports.cpp                                                  *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2004  Allen Winter <awinterz@users.sourceforge.net>      *
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/

#include <stdlib.h>
#include <iostream>

#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendar.h>
#include <libkcal/event.h>

#include "konsolekalendarexports.h"

using namespace KCal;
using namespace std;

KonsoleKalendarExports::KonsoleKalendarExports( KonsoleKalendarVariables
                                                *variables ) {
  m_variables = variables;
  m_firstEntry = true;
}


KonsoleKalendarExports::~KonsoleKalendarExports() {
}

bool KonsoleKalendarExports::exportAsTxt( QTextStream *ts,
                                          Event *event, QDate date ) {

  // Export "Text" Format:
  //
  // Date:\t<Incidence Date>(dddd yyyy-MM-dd)
  // [\t<Incidence Start Time>(hh:mm) - <Incidence End Time>(hh:mm)]
  // Summary:
  // \t<Incidence Summary | "(no summary available)">
  // Location:
  // \t<Incidence Location | "(no location available)">
  // Description:
  // \t<Incidence Description | "(no description available)">
  // UID:
  // \t<Incidence UID>
  // --------------------------------------------------

  // Print Event Date (in our standard format)
  *ts << i18n( "Date:" )
      << "\t"
      << date.toString("dddd yyyy-MM-dd")
      << endl;

  // Print Event Starttime - Endtime, for Non-Floating Events Only
  if ( !event->doesFloat() ) {
    *ts << "\t"
        << event->dtStart().time().toString("hh:mm")
        << " - "
        << event->dtEnd().time().toString("hh:mm");
  }
  *ts << endl;

  // Print Event Summary
  *ts << i18n( "Summary:" )
      << endl;
  if ( !event->summary().isEmpty() ) {
    *ts << "\t"
        << event->summary()
        << endl;
  } else {
    *ts << "\t"
        << i18n( "(no summary available)" )
        << endl;
  }

  // Print Event Location
  *ts << i18n( "Location:" )
      << endl;
  if ( !event->location().isEmpty() ) {
    *ts << "\t"
        <<event->location()
        << endl;
  } else {
    *ts << "\t"
        << i18n( "(no location available)" )
        << endl;
  }

  // Print Event Description
  *ts << i18n( "Description:" )
      << endl;
  if ( !event->description().isEmpty() ) {
    *ts << "\t"
        << event->description()
        << endl;
  } else {
    *ts << "\t"
        << i18n( "(no description available)" )
        << endl;
  }

  // Print Event UID
  *ts << i18n( "UID:" )
      << endl
      << "\t"
      << event->uid()
      << endl;

  // Print Line Separator
  *ts << "--------------------------------------------------"
      << endl;

  return true;
}

bool KonsoleKalendarExports::exportAsTxtShort( QTextStream *ts,
                                               Event *event, QDate date,
                                               bool sameday ) {

  // Export "Text-Short" Format:
  //
  // [--------------------------------------------------]
  // {<Incidence Date>(dddd yyyy-MM-dd)]
  // [<Incidence Start Time>(hh:mm) - <Incidence End Time>(hh:mm) | "\t"]
  // \t<Incidence Summary | \t>[, <Incidence Location>]
  // \t\t<Incidence Description | "\t">

  if ( !sameday ) {
    // If a new date, then print the day separator
    *ts << "--------------------------------------------------"
        << endl;

    // Print Event Date (in our standard format)
    *ts << date.toString("dddd yyyy-MM-dd")
        << endl;
  }

  // Print Event Starttime - Endtime
  if ( !event->doesFloat() ) {
    *ts << event->dtStart().time().toString("hh:mm")
        << " - "
        << event->dtEnd().time().toString("hh:mm");
  } else {
    *ts << "\t";
  }
  *ts << "\t";

  // Print Event Summary
  *ts << event->summary();

  // Print Event Location
  if ( !event->location().isEmpty() ) {
    if ( !event->summary().isEmpty() ) {
      *ts << ", ";
    }
    *ts << event->location();
  }
  *ts << endl;

  // Print Event Description
  *ts << "\t\t";
  if ( !event->description().isEmpty() ) {
    *ts << event->description()
        << endl;
  }

  // Print Event UID
// By user request, no longer print UIDs if export-type==short
//      << event->uid()
//      << endl;

  *ts << endl;  // blank line between events

  return true;
}

QString KonsoleKalendarExports::processField( QString field, QString dquote ) {

  // little function that processes a field for CSV compliance:
  //   1. Replaces double quotes by a pair of consecutive double quotes
  //   2. Surrounds field with double quotes

  QString double_dquote = dquote + dquote;
  QString retField = dquote + field.replace( dquote, double_dquote ) + dquote;
  return retField;
}

#define pF( x )  processField( ( x ), dquote )

bool KonsoleKalendarExports::exportAsCSV( QTextStream *ts,
                                          Event *event, QDate date ) {

  // Export "CSV" Format:
  //
  // startdate,starttime,enddate,endtime,summary,location,description,UID

  QString delim = i18n( "," );   // character to use as CSV field delimiter
  QString dquote = i18n( "\"" ); // character to use to quote CSV fields

  if ( !event->doesFloat() ) {
    *ts <<          pF( date.toString("yyyy-MM-dd") )
        << delim << pF( event->dtStart().time().toString("hh:mm") )
        << delim << pF( date.toString("yyyy-MM-dd") )
        << delim << pF( event->dtEnd().time().toString("hh:mm") );
  } else {
    *ts <<          pF( date.toString("yyyy-MM-dd") )
        << delim << pF( "" )
        << delim << pF( date.toString("yyyy-MM-dd") )
        << delim << pF( "" );
  }

  *ts << delim << pF( event->summary().replace( QChar('\n'), QChar(' ') ) )
      << delim << pF( event->location().replace( QChar('\n'), QChar(' ') ) )
      << delim << pF( event->description().replace( QChar('\n'), QChar(' ') ) )
      << delim << pF( event->uid() )
      << endl;

  return true;
}
