/*
    This file is part of KDE.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "groupwise.h"

#include "groupwiseserver.h"

#include <libkcal/freebusy.h>
#include <libkcal/icalformat.h>
#include <libkcal/scheduler.h>

#include <kinstance.h>
#include <kdebug.h>
#include <klocale.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

extern "C" {
int kdemain( int argc, char **argv );
}

int kdemain( int argc, char **argv )
{
  KInstance instance( "kio_groupwise" );
  
  kdDebug(7000) << "Starting kio_groupwise(pid:  " << getpid() << ")" << endl;
  
  if (argc != 4) {
    fprintf( stderr, "Usage: kio_groupwise protocol domain-socket1 domain-socket2\n");
    exit( -1 );
  }
  
  Groupwise slave( argv[2], argv[3] );
  slave.dispatchLoop();
  
  return 0;
}

Groupwise::Groupwise( const QCString &pool, const QCString &app )
: SlaveBase( "groupwise", pool, app )
{
}

void Groupwise::get( const KURL &url )
{
  kdDebug(7000) << "Groupwise::get()" << endl;
  kdDebug(7000) << " URL: " << url.url() << endl;
  #if 1
  kdDebug(7000) << " Path: " << url.path() << endl;
  kdDebug(7000) << " Query: " << url.query() << endl;
  kdDebug(7000) << " Protocol: " << url.protocol() << endl;
  kdDebug(7000) << " Filename: " << url.filename() << endl;
  #endif

  mimeType( "text/plain" );

  QString path = url.path();
  debugMessage( "Path: " + path );

  if ( !path.startsWith( "/freebusy/" ) ) {
    QString error = i18n("Unknown path. Known path is '/freebusy/'.");
    errorMessage( error );
  } else {
    QString file = url.filename();
    if ( file.right( 4 ) != ".ifb" ) {
      QString error = i18n("Illegal filename. File has to have '.ifb' suffix.");
      errorMessage( error );
    } else {
      QString email = file.left( file.length() - 4 );
      debugMessage( "Email: " + email );

      QString u = url.protocol() + "://" + url.host() + ":";
      if ( url.port() ) u += QString::number( url.port() );
      else if (url.protocol()=="https") 
              u += "8201";
           else
              u += "7181";
      u += "/soap";

      QString user = url.user();
      QString pass = url.pass();

      debugMessage( "URL: " + u );
      debugMessage( "User: " + user );
      debugMessage( "Password: " + pass );

      KCal::FreeBusy *fb = new KCal::FreeBusy;

      if ( user.isEmpty() || pass.isEmpty() ) {
        errorMessage( i18n("Need username and password.") );
      } else {
        GroupwiseServer server( u, user, pass, 0 );

        // FIXME: Read range from configuration or URL parameters.
        QDate start = QDate::currentDate().addDays( -3 );
        QDate end = QDate::currentDate().addDays( 60 );

        fb->setDtStart( start );
        fb->setDtEnd( end );

        kdDebug() << "Login" << endl;

        if ( !server.login() ) {
          errorMessage( i18n("Unable to login.") );
        } else {
          kdDebug() << "Read free/busy" << endl;
          if ( !server.readFreeBusy( email, start, end, fb ) ) {
            errorMessage( i18n("Unable to read free/busy data.") );
          }
          kdDebug() << "Read free/busy" << endl;
          server.logout();
        }
      }

      QDateTime s = QDateTime( QDate( 2004, 9, 27 ), QTime( 10, 0 ) );
      QDateTime e = QDateTime( QDate( 2004, 9, 27 ), QTime( 11, 0 ) );

      fb->addPeriod( s, e );

      KCal::ICalFormat format;

      QString ical = format.createScheduleMessage( fb, KCal::Scheduler::Publish );

      data( ical.utf8() );

      finished();
    }
  }  
  
  kdDebug(7000) << "GroupwiseCgiProtocol::get() done" << endl;
}

void Groupwise::errorMessage( const QString &msg )
{
  error( KIO::ERR_SLAVE_DEFINED, msg );
}

void Groupwise::debugMessage( const QString &msg )
{
#if 0
  data( ( msg + "\n" ).utf8() );
#else
  Q_UNUSED( msg );
#endif
}

