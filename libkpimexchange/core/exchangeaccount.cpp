/*
    This file is part of libkpimexchange
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include <qstring.h>
#include <qapplication.h>
#include <qdom.h>
#include <qwidgetlist.h>
#include <qwidget.h>

#include <kurl.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kcursor.h>

#include <kio/authinfo.h>
#include <kio/davjob.h>
#include <kio/job.h>

#include "exchangeaccount.h"
#include "utils.h"

using namespace KPIM;

ExchangeAccount::ExchangeAccount( QString host, QString account, QString password )
{
  mHost = host;
  mAccount = account;
  mMailbox = account;
  mPassword = password;

  mCalendarURL = 0;
}

ExchangeAccount::ExchangeAccount( QString host, QString account, QString mailbox, QString password )
{
  mHost = host;
  mAccount = account;
  if ( mailbox.isNull() ) 
    mMailbox = account;
  else 
    mMailbox = mailbox;
  mPassword = password;

  mCalendarURL = 0;
}

ExchangeAccount::ExchangeAccount( QString group )
{
  load( group );
}

ExchangeAccount::~ExchangeAccount()
{
}

QString endecryptStr( const QString &aStr ) 
{
  QString result;
  for (uint i = 0; i < aStr.length(); i++)
    result += (aStr[i].unicode() < 0x20) ? aStr[i] :
      QChar(0x1001F - aStr[i].unicode());
  return result;
}

void ExchangeAccount::save( QString const& group )
{
  kapp->config()->setGroup( group );
  kapp->config()->writeEntry( "host", mHost );
  kapp->config()->writeEntry( "user", mAccount );
  kapp->config()->writeEntry( "mailbox", mMailbox );
  kapp->config()->writeEntry( "MS-ID", endecryptStr( mPassword ) );
  kapp->config()->sync();
}

void ExchangeAccount::load( QString const& group )
{
  kapp->config()->setGroup( group );

  QString host = kapp->config()->readEntry( "host" );
  if ( ! host.isNull() ) {
    mHost = host;
  }

  QString user = kapp->config()->readEntry( "user" );
  if ( ! user.isNull() ) {
    mAccount = user;
  }

  QString mailbox = kapp->config()->readEntry( "mailbox" );
  if ( ! mailbox.isNull() ) {
    mMailbox = mailbox;
  } else {
    mMailbox = mAccount;
  }

  QString password = endecryptStr( kapp->config()->readEntry( "MS-ID" ) );
  if ( ! password.isNull() ) {
    mPassword = password;
  }
}

KURL ExchangeAccount::baseURL()
{
  KURL url = KURL( "webdav://" + mHost + "/exchange/" + mMailbox );
  return url;
}

KURL ExchangeAccount::calendarURL()
{
  if ( mCalendarURL ) {
    return *mCalendarURL;
  } else {
    KURL url = baseURL();
    url.addPath( "Calendar" );
    return url;
  }
}

void ExchangeAccount::authenticate( QWidget* window )
{
  if ( window )
    authenticate( window->winId() );
  else
    authenticate();
}

void ExchangeAccount::authenticate()
{

  long windowId;
  QWidgetList* widgets = QApplication::topLevelWidgets();
  if ( widgets->isEmpty() )
    windowId = 0;
  else
    windowId = widgets->first()->winId();
  delete widgets;

  authenticate( windowId );
}

void ExchangeAccount::authenticate( int windowId )
{
  kdDebug() << "Entering ExchangeAccount::authenticate( windowId=" << windowId << " )" << endl;

  KIO::AuthInfo info;
  info.url = baseURL();
  info.username = mAccount;
  info.password = mPassword;
  info.realmValue = mHost;
  info.digestInfo = "Basic";

  DCOPClient *dcopClient = new DCOPClient();
  dcopClient->attach();

  QByteArray params;
  QDataStream stream(params, IO_WriteOnly);
  stream << info << windowId;

  dcopClient->send( "kded", "kpasswdserver", "addAuthInfo(KIO::AuthInfo, long int)", params );

  dcopClient->detach();
  delete dcopClient;

  mCalendarURL = 0;

  calcFolderURLs();

  QApplication::setOverrideCursor( KCursor::waitCursor() );
  do {
    qApp->processEvents();
  } while ( !mCalendarURL );
  QApplication::restoreOverrideCursor();  
}

void ExchangeAccount::calcFolderURLs()
{
  kdDebug() << "Calculating folder URLs" << endl;
  QDomDocument doc;
  QDomElement root = addElement( doc, doc, "DAV:", "propfind" );
  QDomElement prop = addElement( doc, root, "DAV:", "prop" );
  addElement( doc, prop, "urn:schemas:httpmail:", "calendar" );
// For later use:
// urn:schemas:httpmail:contacts Contacts 
// urn:schemas:httpmail:deleteditems Deleted Items 
// urn:schemas:httpmail:drafts Drafts 
// urn:schemas:httpmail:inbox Inbox 
// urn:schemas:httpmail:journal Journal 
// urn:schemas:httpmail:notes Notes 
// urn:schemas:httpmail:outbox Outbox 
// urn:schemas:httpmail:sentitems Sent Items 
// urn:schemas:httpmail:tasks Tasks 
// urn:schemas:httpmail:sendmsg Exchange Mail Submission URI 
// urn:schemas:httpmail:msgfolderroot Mailbox folder (root) 

  KIO::DavJob* job = KIO::davPropFind( baseURL(), doc, "0", false );
  job->addMetaData( "errorPage", "false" );
  connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotFolderResult( KIO::Job * ) ) );
}

void ExchangeAccount::slotFolderResult( KIO::Job * job ) 
{
  kdDebug() << "ExchangeAccount::slotFolderResult()" << endl;
  if ( job->error() ) {
    kdError() << "Error: Cannot get well-know folder names; " << job->error() << endl;
    job->showErrorDialog( 0L );
    return;
  }
  QDomDocument& response = static_cast<KIO::DavJob *>( job )->response();

  QDomElement prop = response.documentElement().namedItem( "response" ).namedItem( "propstat" ).namedItem( "prop" ).toElement();
 
  QDomElement calElement = prop.namedItem( "calendar" ).toElement();
  if ( calElement.isNull() ) {
    kdError() << "Error: no calendar URL in Exchange server reply" << endl;
    return;
  }
  QString calendar = calElement.text();
  mCalendarURL = new KURL( calendar );
  mCalendarURL->setProtocol("webdav");
  kdDebug() << "Calendar URL: " << mCalendarURL->url() << endl;
}

#include "exchangeaccount.moc"
