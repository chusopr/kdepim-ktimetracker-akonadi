/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "incidenceformatter.h"

#include <libkcal/attachment.h>
#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/journal.h>
#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/freebusy.h>
#include <libkcal/recurrence.h>

#include <libkdepim/email.h>

#include <ktnef/ktnefparser.h>
#include <ktnef/ktnefmessage.h>
#include <ktnef/ktnefdefs.h>
#include <kabc/phonenumber.h>
#include <kabc/vcardconverter.h>
#include <kabc/stdaddressbook.h>

#include <kapplication.h>
// #include <kdebug.h>

#include <klocale.h>
#include <kiconloader.h>

#include <qbuffer.h>

#include <time.h>


using namespace KCal;

static const struct {
   const char* day;
} daysOfWeek[] = {
	{ I18N_NOOP("Mondays") },
	{ I18N_NOOP("Tuesdays") },
	{ I18N_NOOP("Wednesdays") },
	{ I18N_NOOP("Thursdays") },
	{ I18N_NOOP("Fridays") },
	{ I18N_NOOP("Saturdays") },
	{ I18N_NOOP("Sundays") },
};

const int sizeOfDaysOfWeek = sizeof(daysOfWeek) / sizeof(*daysOfWeek);

static const struct {
   const int dayNo;
   const char *date;
} daysOfMonth[] = {
       { 1, I18N_NOOP("1st") },
       { 2, I18N_NOOP("2nd") },
       { 3, I18N_NOOP("3rd") },
       { 4, I18N_NOOP("4th") },
       { 5, I18N_NOOP("5th") },
       { 6, I18N_NOOP("6th") },
       { 7, I18N_NOOP("7th") },
       { 8, I18N_NOOP("8th") },
       { 9, I18N_NOOP("9th") },
       { 10, I18N_NOOP("10th") },
       { 11, I18N_NOOP("11th") },
       { 12, I18N_NOOP("12th") },
       { 13, I18N_NOOP("13th") },
       { 14, I18N_NOOP("14th") },
       { 15, I18N_NOOP("15th") },
       { 16, I18N_NOOP("16th") },
       { 17, I18N_NOOP("17th") },
       { 18, I18N_NOOP("18th") },
       { 19, I18N_NOOP("19th") },
       { 20, I18N_NOOP("20th") },
       { 21, I18N_NOOP("21st") },
       { 22, I18N_NOOP("22nd") },
       { 23, I18N_NOOP("23rd") },
       { 24, I18N_NOOP("24th") },
       { 25, I18N_NOOP("25th") },
       { 26, I18N_NOOP("26th") },
       { 27, I18N_NOOP("27th") },
       { 28, I18N_NOOP("28th") },
       { 29, I18N_NOOP("29th") },
       { 30, I18N_NOOP("30th") },
       { 31, I18N_NOOP("31st") },
       { -1, I18N_NOOP("last day") },
       { -2, I18N_NOOP("2nd last day") },
       { -3, I18N_NOOP("3rd last day") },
       { -4, I18N_NOOP("4th last day") },
       { -5, I18N_NOOP("5th last day") },
};

const int sizeOfDaysOfMonth = sizeof (daysOfMonth) / sizeof (*daysOfMonth);

static const struct {
   const char* month;
} monthsOfYear[] = {
	{ I18N_NOOP("January") },
	{ I18N_NOOP("February") },
	{ I18N_NOOP("March") },
	{ I18N_NOOP("April") },
	{ I18N_NOOP("May") },
	{ I18N_NOOP("June") },
	{ I18N_NOOP("July") },
	{ I18N_NOOP("August") },
	{ I18N_NOOP("September") },
	{ I18N_NOOP("October") },
	{ I18N_NOOP("November") },
	{ I18N_NOOP("December") }
};

const int sizeOfMonthsofYear = sizeof(monthsOfYear) / sizeof(*monthsOfYear);


static QString stringProp( KTNEFMessage* tnefMsg, const Q_UINT32& key,
                           const QString& fallback = QString::null)
{
  return tnefMsg->findProp( key < 0x10000 ? key & 0xFFFF : key >> 16,
                            fallback );
}

static QString sNamedProp( KTNEFMessage* tnefMsg, const QString& name,
                           const QString& fallback = QString::null )
{
  return tnefMsg->findNamedProp( name, fallback );
}

struct save_tz { char* old_tz; char* tz_env_str; };

/* temporarily go to a different timezone */
static struct save_tz set_tz( const char* _tc )
{
  const char *tc = _tc?_tc:"UTC";

  struct save_tz rv;

  rv.old_tz = 0;
  rv.tz_env_str = 0;

  //kdDebug(5006) << "set_tz(), timezone before = " << timezone << endl;

  char* tz_env = 0;
  if( getenv( "TZ" ) ) {
    tz_env = strdup( getenv( "TZ" ) );
    rv.old_tz = tz_env;
  }
  char* tmp_env = (char*)malloc( strlen( tc ) + 4 );
  strcpy( tmp_env, "TZ=" );
  strcpy( tmp_env+3, tc );
  putenv( tmp_env );

  rv.tz_env_str = tmp_env;

  /* tmp_env is not free'ed -- it is part of the environment */

  tzset();
  //kdDebug(5006) << "set_tz(), timezone after = " << timezone << endl;

  return rv;
}

/* restore previous timezone */
static void unset_tz( struct save_tz old_tz )
{
  if( old_tz.old_tz ) {
    char* tmp_env = (char*)malloc( strlen( old_tz.old_tz ) + 4 );
    strcpy( tmp_env, "TZ=" );
    strcpy( tmp_env+3, old_tz.old_tz );
    putenv( tmp_env );
    /* tmp_env is not free'ed -- it is part of the environment */
    free( old_tz.old_tz );
  } else {
    /* clear TZ from env */
    putenv( strdup("TZ") );
  }
  tzset();

  /* is this OK? */
  if( old_tz.tz_env_str ) free( old_tz.tz_env_str );
}

static QDateTime utc2Local( const QDateTime& utcdt )
{
  struct tm tmL;

  save_tz tmp_tz = set_tz("UTC");
  time_t utc = utcdt.toTime_t();
  unset_tz( tmp_tz );

  localtime_r( &utc, &tmL );
  return QDateTime( QDate( tmL.tm_year+1900, tmL.tm_mon+1, tmL.tm_mday ),
                    QTime( tmL.tm_hour, tmL.tm_min, tmL.tm_sec ) );
}


static QDateTime pureISOToLocalQDateTime( const QString& dtStr,
                                          bool bDateOnly = false )
{
  QDate tmpDate;
  QTime tmpTime;
  int year, month, day, hour, minute, second;

  if( bDateOnly ) {
    year = dtStr.left( 4 ).toInt();
    month = dtStr.mid( 4, 2 ).toInt();
    day = dtStr.mid( 6, 2 ).toInt();
    hour = 0;
    minute = 0;
    second = 0;
  } else {
    year = dtStr.left( 4 ).toInt();
    month = dtStr.mid( 4, 2 ).toInt();
    day = dtStr.mid( 6, 2 ).toInt();
    hour = dtStr.mid( 9, 2 ).toInt();
    minute = dtStr.mid( 11, 2 ).toInt();
    second = dtStr.mid( 13, 2 ).toInt();
  }
  tmpDate.setYMD( year, month, day );
  tmpTime.setHMS( hour, minute, second );

  if( tmpDate.isValid() && tmpTime.isValid() ) {
    QDateTime dT = QDateTime( tmpDate, tmpTime );

    if( !bDateOnly ) {
      // correct for GMT ( == Zulu time == UTC )
      if (dtStr.at(dtStr.length()-1) == 'Z') {
        //dT = dT.addSecs( 60 * KRFCDate::localUTCOffset() );
        //localUTCOffset( dT ) );
        dT = utc2Local( dT );
      }
    }
    return dT;
  } else
    return QDateTime();
}



QString IncidenceFormatter::msTNEFToVPart( const QByteArray& tnef )
{
  bool bOk = false;

  KTNEFParser parser;
  QBuffer buf( tnef );
  CalendarLocal cal;
  KABC::Addressee addressee;
  KABC::VCardConverter cardConv;
  ICalFormat calFormat;
  Event* event = new Event();

  if( parser.openDevice( &buf ) ) {
    KTNEFMessage* tnefMsg = parser.message();
    //QMap<int,KTNEFProperty*> props = parser.message()->properties();

    // Everything depends from property PR_MESSAGE_CLASS
    // (this is added by KTNEFParser):
    QString msgClass = tnefMsg->findProp( 0x001A, QString::null, true )
      .upper();
    if( !msgClass.isEmpty() ) {
      // Match the old class names that might be used by Outlook for
      // compatibility with Microsoft Mail for Windows for Workgroups 3.1.
      bool bCompatClassAppointment = false;
      bool bCompatMethodRequest = false;
      bool bCompatMethodCancled = false;
      bool bCompatMethodAccepted = false;
      bool bCompatMethodAcceptedCond = false;
      bool bCompatMethodDeclined = false;
      if( msgClass.startsWith( "IPM.MICROSOFT SCHEDULE." ) ) {
        bCompatClassAppointment = true;
        if( msgClass.endsWith( ".MTGREQ" ) )
          bCompatMethodRequest = true;
        if( msgClass.endsWith( ".MTGCNCL" ) )
          bCompatMethodCancled = true;
        if( msgClass.endsWith( ".MTGRESPP" ) )
          bCompatMethodAccepted = true;
        if( msgClass.endsWith( ".MTGRESPA" ) )
          bCompatMethodAcceptedCond = true;
        if( msgClass.endsWith( ".MTGRESPN" ) )
          bCompatMethodDeclined = true;
      }
      bool bCompatClassNote = ( msgClass == "IPM.MICROSOFT MAIL.NOTE" );

      if( bCompatClassAppointment || "IPM.APPOINTMENT" == msgClass ) {
        // Compose a vCal
        bool bIsReply = false;
        QString prodID = "-//Microsoft Corporation//Outlook ";
        prodID += tnefMsg->findNamedProp( "0x8554", "9.0" );
        prodID += "MIMEDIR/EN\n";
        prodID += "VERSION:2.0\n";
        calFormat.setApplication( "Outlook", prodID );

        Scheduler::Method method;
        if( bCompatMethodRequest )
          method = Scheduler::Request;
        else if( bCompatMethodCancled )
          method = Scheduler::Cancel;
        else if( bCompatMethodAccepted || bCompatMethodAcceptedCond ||
                 bCompatMethodDeclined ) {
          method = Scheduler::Reply;
          bIsReply = true;
        } else {
          // pending(khz): verify whether "0x0c17" is the right tag ???
          //
          // at the moment we think there are REQUESTS and UPDATES
          //
          // but WHAT ABOUT REPLIES ???
          //
          //

          if( tnefMsg->findProp(0x0c17) == "1" )
            bIsReply = true;
          method = Scheduler::Request;
        }

        /// ###  FIXME Need to get this attribute written
        ScheduleMessage schedMsg(event, method, ScheduleMessage::Unknown );

        QString sSenderSearchKeyEmail( tnefMsg->findProp( 0x0C1D ) );

        if( !sSenderSearchKeyEmail.isEmpty() ) {
          int colon = sSenderSearchKeyEmail.find( ':' );
          // May be e.g. "SMTP:KHZ@KDE.ORG"
          if( sSenderSearchKeyEmail.find( ':' ) == -1 )
            sSenderSearchKeyEmail.remove( 0, colon+1 );
        }

        QString s( tnefMsg->findProp( 0x0e04 ) );
        QStringList attendees = QStringList::split( ';', s );
        if( attendees.count() ) {
          for( QStringList::Iterator it = attendees.begin();
               it != attendees.end(); ++it ) {
            // Skip all entries that have no '@' since these are
            // no mail addresses
            if( (*it).find('@') == -1 ) {
              s = (*it).stripWhiteSpace();

              Attendee *attendee = new Attendee( s, s, true );
              if( bIsReply ) {
                if( bCompatMethodAccepted )
                  attendee->setStatus( Attendee::Accepted );
                if( bCompatMethodDeclined )
                  attendee->setStatus( Attendee::Declined );
                if( bCompatMethodAcceptedCond )
                  attendee->setStatus(Attendee::Tentative);
              } else {
                attendee->setStatus( Attendee::NeedsAction );
                attendee->setRole( Attendee::ReqParticipant );
              }
              event->addAttendee(attendee);
            }
          }
        } else {
          // Oops, no attendees?
          // This must be old style, let us use the PR_SENDER_SEARCH_KEY.
          s = sSenderSearchKeyEmail;
          if( !s.isEmpty() ) {
            Attendee *attendee = new Attendee( QString::null, QString::null,
                                               true );
            if( bIsReply ) {
              if( bCompatMethodAccepted )
                attendee->setStatus( Attendee::Accepted );
              if( bCompatMethodAcceptedCond )
                attendee->setStatus( Attendee::Declined );
              if( bCompatMethodDeclined )
                attendee->setStatus( Attendee::Tentative );
            } else {
              attendee->setStatus(Attendee::NeedsAction);
              attendee->setRole(Attendee::ReqParticipant);
            }
            event->addAttendee(attendee);
          }
        }
        s = tnefMsg->findProp( 0x0c1f ); // look for organizer property
        if( s.isEmpty() && !bIsReply )
          s = sSenderSearchKeyEmail;
        // TODO: Use the common name?
        if( !s.isEmpty() )
          event->setOrganizer( s );

        s = tnefMsg->findProp( 0x8516 ).replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        event->setDtStart( QDateTime::fromString( s ) ); // ## Format??

        s = tnefMsg->findProp( 0x8517 ).replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        event->setDtEnd( QDateTime::fromString( s ) );

        s = tnefMsg->findProp( 0x8208 );
        event->setLocation( s );

        // is it OK to set this to OPAQUE always ??
        //vPart += "TRANSP:OPAQUE\n"; ###FIXME, portme!
        //vPart += "SEQUENCE:0\n";

        // is "0x0023" OK  -  or should we look for "0x0003" ??
        s = tnefMsg->findProp( 0x0023 );
        event->setUid( s );

        // PENDING(khz): is this value in local timezone? Must it be
        // adjusted? Most likely this is a bug in the server or in
        // Outlook - we ignore it for now.
        s = tnefMsg->findProp( 0x8202 ).replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        // ### libkcal always uses currentDateTime()
        // event->setDtStamp(QDateTime::fromString(s));

        s = tnefMsg->findNamedProp( "Keywords" );
        event->setCategories( s );

        s = tnefMsg->findProp( 0x1000 );
        event->setDescription( s );

        s = tnefMsg->findProp( 0x0070 );
        event->setSummary( s );

        s = tnefMsg->findProp( 0x0026 );
        event->setPriority( s.toInt() );

        // is reminder flag set ?
        if(!tnefMsg->findProp(0x8503).isEmpty()) {
          Alarm *alarm = new Alarm(event);
          QDateTime highNoonTime =
            pureISOToLocalQDateTime( tnefMsg->findProp( 0x8502 )
                                     .replace( QChar( '-' ), "" )
                                     .replace( QChar( ':' ), "" ) );
          QDateTime wakeMeUpTime =
            pureISOToLocalQDateTime( tnefMsg->findProp( 0x8560, "" )
                                     .replace( QChar( '-' ), "" )
                                     .replace( QChar( ':' ), "" ) );
          alarm->setTime(wakeMeUpTime);

          if( highNoonTime.isValid() && wakeMeUpTime.isValid() )
            alarm->setStartOffset( Duration( highNoonTime, wakeMeUpTime ) );
          else
            // default: wake them up 15 minutes before the appointment
            alarm->setStartOffset( Duration( 15*60 ) );
          alarm->setDisplayAlarm( i18n( "Reminder" ) );

          // Sorry: the different action types are not known (yet)
          //        so we always set 'DISPLAY' (no sounds, no images...)
          event->addAlarm( alarm );
        }
        cal.addEvent( event );
        bOk = true;
        // we finished composing a vCal
      } else if( bCompatClassNote || "IPM.CONTACT" == msgClass ) {
        addressee.setUid( stringProp( tnefMsg, attMSGID ) );
        addressee.setFormattedName( stringProp( tnefMsg, MAPI_TAG_PR_DISPLAY_NAME ) );
        addressee.insertEmail( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_EMAIL1EMAILADDRESS ), true );
        addressee.insertEmail( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_EMAIL2EMAILADDRESS ), false );
        addressee.insertEmail( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_EMAIL3EMAILADDRESS ), false );
        addressee.insertCustom( "KADDRESSBOOK", "X-IMAddress", sNamedProp( tnefMsg, MAPI_TAG_CONTACT_IMADDRESS ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-SpousesName", stringProp( tnefMsg, MAPI_TAG_PR_SPOUSE_NAME ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-ManagersName", stringProp( tnefMsg, MAPI_TAG_PR_MANAGER_NAME ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-AssistantsName", stringProp( tnefMsg, MAPI_TAG_PR_ASSISTANT ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-Department", stringProp( tnefMsg, MAPI_TAG_PR_DEPARTMENT_NAME ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-Office", stringProp( tnefMsg, MAPI_TAG_PR_OFFICE_LOCATION ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-Profession", stringProp( tnefMsg, MAPI_TAG_PR_PROFESSION ) );

        QString s = tnefMsg->findProp( MAPI_TAG_PR_WEDDING_ANNIVERSARY )
          .replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        if( !s.isEmpty() )
          addressee.insertCustom( "KADDRESSBOOK", "X-Anniversary", s );

        addressee.setUrl( KURL( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_WEBPAGE )  ) );

        // collect parts of Name entry
        addressee.setFamilyName( stringProp( tnefMsg, MAPI_TAG_PR_SURNAME ) );
        addressee.setGivenName( stringProp( tnefMsg, MAPI_TAG_PR_GIVEN_NAME ) );
        addressee.setAdditionalName( stringProp( tnefMsg, MAPI_TAG_PR_MIDDLE_NAME ) );
        addressee.setPrefix( stringProp( tnefMsg, MAPI_TAG_PR_DISPLAY_NAME_PREFIX ) );
        addressee.setSuffix( stringProp( tnefMsg, MAPI_TAG_PR_GENERATION ) );

        addressee.setNickName( stringProp( tnefMsg, MAPI_TAG_PR_NICKNAME ) );
        addressee.setRole( stringProp( tnefMsg, MAPI_TAG_PR_TITLE ) );
        addressee.setOrganization( stringProp( tnefMsg, MAPI_TAG_PR_COMPANY_NAME ) );
        /*
        the MAPI property ID of this (multiline) )field is unknown:
        vPart += stringProp(tnefMsg, "\n","NOTE", ... , "" );
        */

        KABC::Address adr;
        adr.setPostOfficeBox( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_PO_BOX ) );
        adr.setStreet( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_STREET ) );
        adr.setLocality( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_CITY ) );
        adr.setRegion( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_STATE_OR_PROVINCE ) );
        adr.setPostalCode( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_POSTAL_CODE ) );
        adr.setCountry( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_COUNTRY ) );
        adr.setType(KABC::Address::Home);
        addressee.insertAddress(adr);

        adr.setPostOfficeBox( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSPOBOX ) );
        adr.setStreet( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSSTREET ) );
        adr.setLocality( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSCITY ) );
        adr.setRegion( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSSTATE ) );
        adr.setPostalCode( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSPOSTALCODE ) );
        adr.setCountry( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSCOUNTRY ) );
        adr.setType( KABC::Address::Work );
        addressee.insertAddress( adr );

        adr.setPostOfficeBox( stringProp( tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_PO_BOX ) );
        adr.setStreet( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_STREET ) );
        adr.setLocality( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_CITY ) );
        adr.setRegion( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_STATE_OR_PROVINCE ) );
        adr.setPostalCode( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_POSTAL_CODE ) );
        adr.setCountry( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_COUNTRY ) );
        adr.setType( KABC::Address::Dom );
        addressee.insertAddress(adr);

        // problem: the 'other' address was stored by KOrganizer in
        //          a line looking like the following one:
        // vPart += "\nADR;TYPE=dom;TYPE=intl;TYPE=parcel;TYPE=postal;TYPE=work;TYPE=home:other_pobox;;other_str1\nother_str2;other_loc;other_region;other_pocode;other_country

        QString nr;
        nr = stringProp( tnefMsg, MAPI_TAG_PR_HOME_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Home ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_BUSINESS_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Work ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_MOBILE_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Cell ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_HOME_FAX_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_BUSINESS_FAX_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work ) );

        s = tnefMsg->findProp( MAPI_TAG_PR_BIRTHDAY )
          .replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        if( !s.isEmpty() )
          addressee.setBirthday( QDateTime::fromString( s ) );

        bOk = ( !addressee.isEmpty() );
      } else if( "IPM.NOTE" == msgClass ) {

      } // else if ... and so on ...
    }
  }

  // Compose return string
  QString iCal = calFormat.toString( &cal );
  if( !iCal.isEmpty() )
    // This was an iCal
    return iCal;

  // Not an iCal - try a vCard
  KABC::VCardConverter converter;
  return converter.createVCard( addressee );
}

static QString weeklyRecurrenceAsString( Recurrence * recur )
{
  Q_ASSERT( recur->doesRecur() == Recurrence::rWeekly );

  //For Weekly, setting up the weekdays of recurrence with 
  //comma as a separator and "and" separating last two
  //days 
  QStringList dayList;
  for ( int i = 0; i < sizeOfDaysOfWeek; i++ ) {
    if( recur->days().testBit(i) ) {
      dayList+= i18n( daysOfWeek[i].day );
    } 
  }

  QString strDays;
  if( dayList.count() > 1 ) {
    strDays = dayList.join(", ");
    strDays.replace( strDays.findRev(','), 1, i18n(" and") );
  } else if( dayList.count() == 1 ) {
    strDays = dayList.first();
  }
  return strDays;
}

static QString monthlyPosRecurrenceAsString( Recurrence * recur )
{
  Q_ASSERT( recur->doesRecur() == Recurrence::rMonthlyPos );

  QString strDays;

  Recurrence::rMonthPos *tmpDay;
  /*struct rMonthlyPos {
    QBitArray rDays;
    short rPos;
    bool negative;
    } 
   */

  //Recurrence::rMonthPos tmpDay;
  QPtrList<Recurrence::rMonthPos> tmpDays = recur->monthPositions();
  for ( tmpDay = tmpDays.first();
      tmpDay;
      tmpDay = tmpDays.next() ) {

    QString day;
    for ( int i = 0; i < sizeOfDaysOfWeek; i++ ) {
      if( tmpDay->rDays.testBit(i) ) {
        day = daysOfWeek[i].day;
        break; 
      } 
    }

    strDays += i18n( "%1 %2").arg( daysOfMonth[ tmpDay->rPos ].date ).arg( day );
  }

  return strDays;

}

static QString monthlyDayRecurrenceAsString( Recurrence * recur )
{
  Q_ASSERT( recur->doesRecur() == Recurrence::rMonthlyDay );

  QString strDays;

  int *tmpDay;
  QPtrList<int> tmpDays = recur->monthDays();
  for ( tmpDay = tmpDays.first();
      tmpDay;
      tmpDay = tmpDays.next() ) {

    strDays += i18n( "%1 ").arg( daysOfMonth[ *tmpDay ].date );
  }

  return strDays;

}

//rYearlyMonth = 0x0007, 
static QString yearlMonthRecurrenceAsString( Recurrence * recur )
{
   Q_ASSERT( recur->doesRecur() == Recurrence::rYearlyMonth );

  QString strDays;
 

  return strDays;

}

//rYearlyDay = 0x0008, 
static QString yearlyDayRecurrenceAsString( Recurrence * recur )
{
   Q_ASSERT( recur->doesRecur() == Recurrence::rYearlyDay );

  QString strDays;
 

  return strDays;

}

//rYearlyPos
static QString yearlyPosRecurrenceAsString( Recurrence * recur )
{
   Q_ASSERT( recur->doesRecur() == Recurrence::rYearlyPos );

  QString strDays;
 

  return strDays;

}

QString IncidenceFormatter::recurrenceAsHTML( Incidence * incidence )
{
  QString result;

  if ( incidence->doesRecur() ) {
    Recurrence *recur = incidence->recurrence();
    switch (recur->doesRecur())
    {
        case Recurrence::rDaily:
          result += i18n("Recurs every day. ", "Recurs every %n days.",recur->frequency());
          break;
        case Recurrence::rWeekly:
          result += i18n("Recurs every week on %1. ", "Recurs every %n weeks on %1.",recur->frequency())
                              .arg( weeklyRecurrenceAsString( recur ) );
          break;
        case Recurrence::rMonthlyPos:
          result += i18n("Recurs on the %1 of every month.", "Recurs on the %1 every %n months.", recur->frequency())
                              .arg( monthlyPosRecurrenceAsString( recur ) );
          break;
        case Recurrence::rMonthlyDay:
          result += i18n("Recurs on the %1 of every month.", "Recurs on the %1 every %n months.", recur->frequency())
                              .arg( monthlyDayRecurrenceAsString( recur ) );
          break;
        case Recurrence::rYearlyMonth:
           result += i18n("Recurrence::rYearlyMonth");
          //result += i18n("Recurs on %1 of every month.", "Recurs on %1 every %n months.", recur->frequency())
          //                    .arg( monthlyDayRecurrenceAsString( recur ) );
          break;
        case Recurrence::rYearlyDay:
           result += i18n("Recurrence::rYearlyDay");
          //result += i18n("Recurs on %1 day of every month.", "Recurs on %1 day every %n months.", recur->frequency())
          //                    .arg( monthlyDayRecurrenceAsString( recur ) );
          break;
        case Recurrence::rYearlyPos:
           result += i18n("Recurrence::rYearlyPos");
          //result += i18n("Recurs on %1 of every month.", "Recurs on %1 every %n months.", recur->frequency())
          //                    .arg( monthlyDayRecurrenceAsString( recur ) );
          break;
    }

    if ( recur->duration() > 0 ) {
      result += i18n (" Repeats once.", " Repeats %n times.", recur->duration());
      result += '\n';
    } else {
      if ( recur->duration() != -1 ) {
// TODO_Recurrence: What to do with floating
        QString endstr;
        if ( incidence->doesFloat() ) {
          endstr = KGlobal::locale()->formatDate( recur->endDate() );
        } else {
          endstr = KGlobal::locale()->formatDateTime( recur->endDateTime() );
        }
        result += i18n(" Repeats until: %1.<br>").arg( endstr );
      } else {
        result += i18n(" Repeats forever.<br>");
      }
    }
    const DateList exdates = incidence->exDates();
    if ( !exdates.isEmpty() )
      result += i18n("Except: ");
    DateList::ConstIterator it( exdates.begin() );
    while ( it != exdates.end() ) {
        if ( it != exdates.begin() )
          result += " ,";
        QDate d(*it++);
        result += KGlobal::locale()->formatDateTime( d );
    }
     if ( !exdates.isEmpty() )
      result += "<br>";

  }
  return result;
}


