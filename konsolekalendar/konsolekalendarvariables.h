#ifndef _KONSOLEKALENDARVARIABLES_H_
#define _KONSOLEKALENDARVARIABLES_H_

/***************************************************************************
        konsolekalendarvariables.h  -  description
           -------------------
    begin                : Sun Jan 6 2002
    copyright            : (C) 2002-2003 by Tuukka Pasanen
    copyright            : (C) 2003 by Allen Winter
    email                : illuusio@mailcity.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatetime.h>
#include <qstring.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/event.h>


   /*
    * Our export types
    */
  #define   NONE                     0
  #define   TEXT_KONSOLEKALENDAR     1
  #define   TEXT_KORGANIZER          2
  #define   HTML                     3
  #define   XHTML                    4
  #define   XML                      5
  #define   CSV                      6
  #define   VCARD                    7


namespace KCal {

class KonsoleKalendarVariables
{
  public:
    KonsoleKalendarVariables();
    ~KonsoleKalendarVariables();

    void setStartDateTime( QDateTime start );
    QDateTime getStartDateTime();
    bool isStartDateTime();

    void setEndDateTime( QDateTime end );
    QDateTime getEndDateTime();
    bool isEndDateTime();

    void setNext( bool next );
    bool isNext();

    void setVerbose( bool verbose );
    bool isVerbose();

    void setDryRun( bool dryrun );
    bool isDryRun();

    void setCalendarFile( QString calendar );
    QString getCalendarFile();

    void setImportFile( QString calendar );
    QString getImportFile();

    void setDescription( QString description );
    QString getDescription();
    bool isDescription();

    void setSummary( QString description );
    QString getSummary();
    bool isSummary();

    void setAll( bool all );
    bool getAll();
    bool isAll();

    void setFloating( bool floating );
    bool getFloating();

    void printSpecs( QString mode );

    QDate parseDate( QString string );
    QTime parseTime( QString str );

    /*
     * Set calendar file for global use
     */

   void setCalendar( CalendarLocal *calendar );

   /*
    * Get global calendar
    */

   CalendarLocal *getCalendar();

    /*
     * Set calendar file for global use
     */

   void setExportType( int export_type );

   /*
    * Get global calendar
    */

   int getExportType();


   
   

  private:
    int findNumber( const QString &str, int &pos, int &startpos );
    char findSeparator( const QString &str, int &pos, int &seppos );
    void skipWhiteSpace( const QString &str, int &pos );

    QDateTime m_startDateTime;
    QDateTime m_endDateTime;
    bool m_bIsStartDateTime;
    bool m_bIsEndDateTime;
    QString m_calendar;
    QString m_import;
    QString m_description;
    QString m_summary;
    bool m_bSummary;
    bool m_bNext;
    bool m_bVerbose;
    bool m_bDryRun;
    bool m_bAll;
    bool m_bDescription;
    bool m_bFloating;
    int str_length;
    int m_export_type;
    // We can use this from everywhere
    CalendarLocal *m_caledarLocal;
   
 

};

}

#endif
