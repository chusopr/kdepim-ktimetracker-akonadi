/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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

#include "exchangeglobals.h"
#include <webdavhandler.h>
#include <groupwaredataadaptor.h>
#include "exchangeconvertercalendar.h"
#include "exchangeconvertercontact.h"
#include <calendaradaptor.h>
#include <addressbookadaptor.h>

#include <libkcal/resourcecached.h>
#include <libemailfunctions/idmapper.h>
// #include <kio/job.h>
#include <kio/davjob.h>
#include <kdebug.h>

// #include <qdom.h>

KPIM::GroupwareJob::ContentType ExchangeGlobals::getContentType( const QDomElement &prop )
{
  const QString &contentclass = prop.namedItem("contentclass").toElement().text();
kdDebug()<<"contentclass: "<<contentclass<<endl;
  return getContentType( contentclass );
}

KPIM::GroupwareJob::ContentType ExchangeGlobals::getContentType( const QString &contentclass )
{
  if ( contentclass == "urn:content-classes:appointment" )
    return KPIM::GroupwareJob::Appointment;
  if ( contentclass == "urn:content-classes:task" )
    return KPIM::GroupwareJob::Task;
  if ( contentclass == "urn:content-classes:message" )
    return KPIM::GroupwareJob::Message;
  if ( contentclass == "urn:content-classes:person" )
    return KPIM::GroupwareJob::Contact;
  return KPIM::GroupwareJob::Unknown;
}


KPIM::FolderLister::FolderType ExchangeGlobals::getFolderType( const QDomNode &folderNode )
{
kdDebug()<<"ExchangeGlobals::getFolderType(...)"<<endl;
  QDomNode n4;
  for( n4 = folderNode.firstChild(); !n4.isNull(); n4 = n4.nextSibling() ) {
    QDomElement e = n4.toElement();

    if ( e.tagName() == "contentclass" ) {
      QString contentclass( e.text() );
      if ( contentclass == "urn:content-classes:contactfolder" )
        return KPIM::FolderLister::ContactsFolder;
      if ( contentclass == "urn:content-classes:calendarfolder" )
        return KPIM::FolderLister::CalendarFolder;
      if ( contentclass == "urn:content-classes:taskfolder" )
        return KPIM::FolderLister::TasksFolder;
      if ( contentclass == "urn:content-classes:journalfolder" )
        return KPIM::FolderLister::JournalsFolder;
      if ( contentclass == "urn:content-classes:folder" )
        return KPIM::FolderLister::Folder;
    }
  }
  return KPIM::FolderLister::Unknown;
}

bool ExchangeGlobals::getFolderHasSubs( const QDomNode &folderNode )
{
  QString hassubs = folderNode.namedItem( "hassubs" ).toElement().text();
  return hassubs == "1";
}




KIO::Job *ExchangeGlobals::createListFoldersJob( const KURL &url )
{
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "d:propfind" );
  QDomElement prop = WebdavHandler::addElement(  doc, root, "d:prop" );
  WebdavHandler::addElement( doc, prop, "d:displayname" );
  WebdavHandler::addElement( doc, prop, "d:contentclass" );
  WebdavHandler::addElement( doc, prop, "d:hassubs" );

  kdDebug(7000) << "props: " << doc.toString() << endl;
  return KIO::davPropFind( url, doc, "1", false );
}


KIO::TransferJob *ExchangeGlobals::createListItemsJob( const KURL &url )
{
  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement(  doc, doc, "d:propfind" );
  QDomElement prop = WebdavHandler::addElement(  doc, root, "d:prop" );
  WebdavHandler::addElement( doc, prop, "d:getetag" );
  WebdavHandler::addElement( doc, prop, "d:contentclass" );
  kdDebug(5800) << "props = "<< doc.toString() << endl;
  return KIO::davPropFind( url, doc, "1", false );
}


KIO::TransferJob *ExchangeGlobals::createDownloadJob( KPIM::GroupwareDataAdaptor */*adaptor*/,
                        const KURL &url, KPIM::GroupwareJob::ContentType ctype )
{
kdDebug()<<"ExchangeGlobals::createDownloadJob()"<<endl;
kdDebug()<<"ctype="<<ctype<<endl;
kdDebug()<<"Person="<<KPIM::GroupwareJob::Contact << ", Appointment="<<KPIM::GroupwareJob::Appointment<<", Task="<<KPIM::GroupwareJob::Task<<", Journal="<<KPIM::GroupwareJob::Journal<<", Message="<<KPIM::GroupwareJob::Message<<endl;
  // Don't use an <allprop/> request!

  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "d:propfind" );
  QDomElement prop = WebdavHandler::addElement( doc, root, "d:prop" );
  QDomAttr att_h = doc.createAttribute( "xmlns:h" );
  att_h.setValue( "urn:schemas:mailheader:" );
  root.setAttributeNode( att_h );

  QDomAttr att_m = doc.createAttribute( "xmlns:m" );
  att_m.setValue( "urn:schemas:httpmail:" );
  root.setAttributeNode( att_m );

  switch ( ctype ) {
    case KPIM::GroupwareJob::Appointment:
        KCal::ExchangeConverterCalendar::createRequestAppointment( doc, prop );
        break;
    case KPIM::GroupwareJob::Task:
        KCal::ExchangeConverterCalendar::createRequestTask( doc, prop );
        break;
    case KPIM::GroupwareJob::Journal:
    case KPIM::GroupwareJob::Message:
        KCal::ExchangeConverterCalendar::createRequestJournal( doc, prop );
        break;
    case KPIM::GroupwareJob::Contact:
        KABC::ExchangeConverterContact::createRequest( doc, prop );
        break;
    default:
        break;
  }

  kdDebug(7000) << "doc: " << doc.toString() << endl;
  KIO::DavJob *job = KIO::davPropFind( url, doc, "0", false );
  return job;
}


KIO::Job *ExchangeGlobals::createRemoveJob( const KURL &uploadurl,
       KPIM::GroupwareUploadItem::List deletedItems )
{
  QStringList urls;
  KPIM::GroupwareUploadItem::List::iterator it;
  kdDebug(5800) << " ExchangeGlobals::createRemoveJob: , URL="<<uploadurl.url()<<endl;
  for ( it = deletedItems.begin(); it != deletedItems.end(); ++it ) {
    //kdDebug(7000) << "Delete: " << endl << format.toICalString(*it) << endl;
    kdDebug(7000) << "Delete: " <<   (*it)->url().url() << endl;
    KURL url( uploadurl );
    url.setPath( (*it)->url().url() );
    if ( !(*it)->url().isEmpty() )
      urls << url.url();
    kdDebug(5700) << "Delete (Mod) : " <<   url.url() << endl;
  }
//  return KIO::del( urls, false, false );
  // TODO
  return 0;
}




// FIXME: This is exactly the same code as in the OGo resource, except that
// it calls getContentType of the ExchangeGlobals class, instead of the one
// from OGoGlobals!!!!
bool ExchangeGlobals::interpretListItemsJob( KPIM::GroupwareDataAdaptor *adaptor,
    KIO::Job *job, const QString &/*jobData*/ )
{
kdDebug()<<"ExchangeGlobals::interpretListItemsJob"<<endl;
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob *>(job);

  if ( !davjob ) {
    return false;
  }
  QDomDocument doc = davjob->response();

  kdDebug(7000) << " Doc: " << doc.toString() << endl;
  kdDebug(7000) << " IdMapper: " << adaptor->idMapper()->asString() << endl;

  QDomElement docElem = doc.documentElement();
  QDomNode n = docElem.firstChild();
  while( !n.isNull() ) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    n = n.nextSibling();
    if ( e.isNull() )
      continue;

    const QString &entry = e.namedItem("href").toElement().text();
    QDomElement propstat = e.namedItem("propstat").toElement();
    if ( propstat.isNull() )
      continue;
    QDomElement prop = propstat.namedItem( "prop" ).toElement();
    if ( prop.isNull() )
      continue;
    QDomElement elem = prop.namedItem("getetag").toElement();
    const QString &newFingerprint = elem.text();
    if ( elem.isNull() || newFingerprint.isEmpty() )
      continue;

    KPIM::GroupwareJob::ContentType type = getContentType( prop );

    adaptor->processDownloadListItem( entry, newFingerprint, type );
  }

  return true;
}


bool ExchangeGlobals::interpretCalendarDownloadItemsJob( KCal::CalendarAdaptor *adaptor,
                                         KIO::Job *job, const QString &/*jobData*/ )
{
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob*>(job);
  if ( !davjob || !adaptor ) return false;

kdDebug() << "ExchangeGlobals::interpretCalendarDownloadItemsJob(): QDomDocument="
          << endl << davjob->response().toString() << endl;
  KCal::ExchangeConverterCalendar conv;
  conv.setTimeZone( adaptor->resource()->timeZoneId() );
  KCal::Incidence::List incidences = conv.parseWebDAV( davjob->response() );

  bool res = false;
  KCal::Incidence::List::Iterator it = incidences.begin();
  for ( ; it != incidences.end(); ++it ) {
    QString fpr = (*it)->customProperty( "KDEPIM-Exchange-Resource", "fingerprint" );
    QString href = (*it)->customProperty( "KDEPIM-Exchange-Resource", "href" );
    KURL u( href );
    adaptor->calendarItemDownloaded( (*it), (*it)->uid(), u.path(), fpr, u.prettyURL() );
    res = true;
  }
  return res;
}


bool ExchangeGlobals::interpretAddressBookDownloadItemsJob(
      KABC::AddressBookAdaptor *adaptor, KIO::Job *job, const QString &/*jobData*/ )
{
  KIO::DavJob *davjob = dynamic_cast<KIO::DavJob*>(job);
  if ( !davjob || !adaptor ) return false;

kdDebug() << "ExchangeGlobals::interpretAddressBookDownloadItemsJob(): QDomDocument="
          << endl << davjob->response().toString() << endl;
  KABC::ExchangeConverterContact conv;
  KABC::Addressee::List addressees = conv.parseWebDAV( davjob->response() );

  bool res = false;
  KABC::Addressee::List::Iterator it = addressees.begin();
  for ( ; it != addressees.end(); ++it ) {
    QString fpr = (*it).custom( "KDEPIM-Exchange-Resource", "fingerprint" );
    QString href = (*it).custom( "KDEPIM-Exchange-Resource", "href" );
    KURL u( href );
    adaptor->addressbookItemDownloaded( (*it), (*it).uid(), u.path(), fpr, u.prettyURL() );
    res = true;
  }
  return res;
}



