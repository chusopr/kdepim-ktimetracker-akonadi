/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "koeventviewer.h"
#include "urihandler.h"
#include "korganizerinterface.h"
#include "koglobals.h"

#include <libkdepim/kdepimprotocols.h>
#include <libkdepim/kpimprefs.h>

#include <akonadi/kcal/utils.h>

#include <kcal/incidence.h>
#include <kcal/incidenceformatter.h>

#include <kapplication.h>
#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kconfiggroup.h>
#include <ksystemtimezone.h>

#include <QRegExp>

using namespace KCal;
using namespace Akonadi;

KOEventViewer::KOEventViewer( QWidget *parent )
  : KTextBrowser( parent ), mDefaultText( "" )
{
  setNotifyClick( true );
  setMinimumHeight( 1 );
}

KOEventViewer::~KOEventViewer()
{
}

void KOEventViewer::readSettings( KConfig *config )
{
  if ( config ) {
// With each restart of KOrganizer the font site gets halfed. What should this
// be good for?
#if 0
    config->setGroup( QString( "EventViewer-%1" ).arg( name() ) );
    int zoomFactor = config->readEntry( "ZoomFactor", fontPointSize() );
    zoomTo( zoomFactor / 2 );
    kDebug(5850) << " KOEventViewer: restoring the fontPointSize:" << fontPointSize()
                 << ", zoomFactor: " << zoomFactor;
#endif
  }
}

void KOEventViewer::writeSettings( KConfig *config )
{
  if ( config ) {
    kDebug() << "saving the zoomFactor:" << fontPointSize();
    KConfigGroup configGroup( config, QString( "EventViewer-%1" ).arg( objectName() ) );
    configGroup.writeEntry( "ZoomFactor", fontPointSize() );
  }
}

void KOEventViewer::setSource( const QUrl &name )
{
  QString uri = name.toString();
  // QTextBrowser for some reason insists on putting // or / in links,
  // this is a crude workaround
  if ( uri.startsWith( KDEPIMPROTOCOL_CONTACT ) ||
       uri.startsWith( KDEPIMPROTOCOL_EMAIL ) ||
       uri.startsWith( QString( KDEPIMPROTOCOL_INCIDENCE ).section( ':', 0, 0 ) ) ||
       uri.startsWith( KDEPIMPROTOCOL_NEWSARTICLE ) ||
       uri.startsWith( QLatin1String( "mailto:" ) ) ) {
    uri.replace( QRegExp( "^([^:]+:)/+" ), "\\1" );
  }

  UriHandler::process( uri );
}

bool KOEventViewer::appendIncidence( const Item &item, const QDate &date )
{
  if ( !Akonadi::hasIncidence( item ) )
    return false;
  //PENDING(AKONADI_PORT): replace QString() by incidence location (was: mCalendar)
  addText( IncidenceFormatter::extensiveDisplayStr(
      QString(), Akonadi::incidence( item ).get(), date, KSystemTimeZones::local() ) );
  return true;
}

void KOEventViewer::setIncidence( const Item &item, const QDate &date )
{
  clearEvents();
  if ( Akonadi::hasIncidence( item ) ) {
    appendIncidence( item, date );
    mIncidence = item;
  } else {
    clearEvents( true );
    mIncidence = Item();
  }

}

void KOEventViewer::clearEvents( bool now )
{
  mText.clear();
  if ( now ) {
    setText( mDefaultText );
  }
}

void KOEventViewer::addText( const QString &text )
{
  mText.append( text );
  setText( mText );
}

void KOEventViewer::setDefaultText( const QString &text )
{
  mDefaultText = text;
}

void KOEventViewer::changeIncidenceDisplay( const Item &item, const QDate &date, int action )
{
  if ( item.id() == mIncidence.id() ) {
    switch ( action ) {
    case KOGlobals::INCIDENCEEDITED:
      setIncidence( item, date );
      break;
    case KOGlobals::INCIDENCEDELETED:
      setIncidence( Item(), date );
      break;
    }
  }
}

void KOEventViewer::editIncidence()
{
  if ( Akonadi::hasIncidence( mIncidence ) ) {
    // make sure korganizer is running or the part is shown
    KToolInvocation::startServiceByDesktopPath( "korganizer" );

    OrgKdeKorganizerKorganizerInterface korganizerIface(
      "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
    korganizerIface.editIncidence( Akonadi::incidence( mIncidence )->uid() );
  }
}

void KOEventViewer::showIncidenceContext()
{
  if ( Akonadi::hasIncidence( mIncidence ) ) {
    // make sure korganizer is running or the part is shown
    KToolInvocation::startServiceByDesktopPath( "korganizer" );

    OrgKdeKorganizerKorganizerInterface korganizerIface(
      "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
    korganizerIface.showIncidenceContext( Akonadi::incidence( mIncidence )->uid() );
  }
}

#include "koeventviewer.moc"
