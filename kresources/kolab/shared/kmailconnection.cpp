/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "kmailconnection.h"
#include "resourcekolabbase.h"

#include <kdebug.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdcopservicestarter.h>
#include <klocale.h>

#include "kmailicalIface_stub.h"


using namespace Kolab;


KMailConnection::KMailConnection( ResourceKolabBase* resource,
                                  const QCString& objId )
  : DCOPObject( objId ), mResource( resource ), mKMailIcalIfaceStub( 0 )
{
  // Make the connection to KMail ready
  mDCOPClient = new DCOPClient();
  mDCOPClient->attach();
  mDCOPClient->registerAs( objId, true );

  kapp->dcopClient()->setNotifications( true );
  connect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QCString& ) ),
           this, SLOT( unregisteredFromDCOP( const QCString& ) ) );
}

KMailConnection::~KMailConnection()
{
  kapp->dcopClient()->setNotifications( false );
  delete mKMailIcalIfaceStub;
  mKMailIcalIfaceStub = 0;
  delete mDCOPClient;
  mDCOPClient = 0;
}

static const QCString dcopObjectId = "KMailICalIface";
bool KMailConnection::connectToKMail()
{
  if ( !mKMailIcalIfaceStub ) {
    QString error;
    QCString dcopService;
    int result = KDCOPServiceStarter::self()->
      findServiceFor( "DCOP/ResourceBackend/IMAP", QString::null,
                      QString::null, &error, &dcopService );
    if ( result != 0 ) {
      kdError(5650) << "Couldn't connect to the IMAP resource backend\n";
      // TODO: You might want to show "error" (if not empty) here,
      // using e.g. KMessageBox
      return false;
    }

    mKMailIcalIfaceStub = new KMailICalIface_stub( kapp->dcopClient(),
                                                   dcopService, dcopObjectId );

    // Attach to the KMail signals
    if ( !connectKMailSignal( "incidenceAdded(QString,QString,Q_UINT32,QString)",
                              "fromKMailAddIncidence(QString,QString,Q_UINT32,QString)" ) )
      kdError(5650) << "DCOP connection to incidenceAdded failed" << endl;
    if ( !connectKMailSignal( "incidenceDeleted(QString,QString,QString)",
                              "fromKMailDelIncidence(QString,QString,QString)" ) )
      kdError(5650) << "DCOP connection to incidenceDeleted failed" << endl;
    if ( !connectKMailSignal( "signalRefresh(QString,QString)",
                              "slotRefresh(QString,QString)" ) )
      kdError(5650) << "DCOP connection to signalRefresh failed" << endl;
    if ( !connectKMailSignal( "subresourceAdded(QString,QString)",
                              "fromKMailAddSubresource(QString,QString)" ) )
      kdError(5650) << "DCOP connection to signalRefresh failed" << endl;
    if ( !connectKMailSignal( "subresourceDeleted(QString,QString)",
                              "fromKMailDelSubresource(QString,QString)" ) )
      kdError(5650) << "DCOP connection to signalRefresh failed" << endl;
  }

  return ( mKMailIcalIfaceStub != 0 );
}

bool KMailConnection::fromKMailAddIncidence( const QString& type,
                                             const QString& folder,
                                             Q_UINT32 sernum,
                                             const QString& xml )
{
//   kdDebug(5650) << "KMailConnection::fromKMailAddIncidence( " << type << ", "
//                 << folder << " ). iCal:\n" << ical << endl;
  return mResource->fromKMailAddIncidence( type, folder, sernum, xml );
}

void KMailConnection::fromKMailDelIncidence( const QString& type,
                                             const QString& folder,
                                             const QString& xml )
{
//   kdDebug(5650) << "KMailConnection::fromKMailDelIncidence( " << type << ", "
//                 << folder << ", " << uid << " )\n";
  mResource->fromKMailDelIncidence( type, folder, xml );
}

void KMailConnection::slotRefresh( const QString& type, const QString& folder )
{
//   kdDebug(5650) << "KMailConnection::slotRefresh( " << type << ", "
//                 << folder << " )\n";
  mResource->slotRefresh( type, folder );
}

void KMailConnection::fromKMailAddSubresource( const QString& type,
                                               const QString& resource )
{
//   kdDebug(5650) << "KMailConnection::fromKMailAddSubresource( " << type << ", "
//                 << resource << " )\n";
  bool writable = true;

  // TODO: This should be told by KMail right away
  if ( connectToKMail() )
    writable = mKMailIcalIfaceStub->isWritableFolder( type, resource );

  mResource->fromKMailAddSubresource( type, resource, writable );
}

void KMailConnection::fromKMailDelSubresource( const QString& type,
                                               const QString& resource )
{
//   kdDebug(5650) << "KMailConnection::fromKMailDelSubresource( " << type << ", "
//                 << resource << " )\n";
  mResource->fromKMailDelSubresource( type, resource );
}

bool KMailConnection::connectKMailSignal( const QCString& signal,
                                          const QCString& method )
{
  return connectDCOPSignal( "kmail", dcopObjectId, signal, method, false )
    && connectDCOPSignal( "kontact", dcopObjectId, signal, method, false );
}

bool KMailConnection::kmailSubresources( QValueList<KMailICalIface::SubResource>& lst,
                                         const QString& contentsType )
{
  if ( !connectToKMail() )
    return false;

  lst = mKMailIcalIfaceStub->subresourcesKolab( contentsType );
  return mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailIncidences( QMap<Q_UINT32, QString>& lst,
                                       const QString& mimetype,
                                       const QString& resource )
{
  if ( !connectToKMail() )
    return false;

  lst = mKMailIcalIfaceStub->incidencesKolab( mimetype, resource );
  return mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailGetAttachment( KURL& url,
                                          const QString& resource,
                                          Q_UINT32 sernum,
                                          const QString& filename )
{
  if ( !connectToKMail() )
    return false;

  url = mKMailIcalIfaceStub->getAttachment( resource, sernum, filename );
  return mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailDeleteIncidence( const QString& resource,
                                            Q_UINT32 sernum )
{
  return connectToKMail()
    && mKMailIcalIfaceStub->deleteIncidenceKolab( resource, sernum )
    && mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailUpdate( const QString& resource,
                                   Q_UINT32& sernum,
                                   const QString& subject,
                                   const QStringList& attachmentURLs,
                                   const QStringList& attachmentMimetypes,
                                   const QStringList& attachmentNames,
                                   const QStringList& deletedAttachments )
{
  if ( connectToKMail() ) {
    sernum = mKMailIcalIfaceStub->update( resource, sernum, subject,
                                          attachmentURLs, attachmentMimetypes, attachmentNames,
                                          deletedAttachments );
    return sernum && mKMailIcalIfaceStub->ok();
  } else
    return false;
}


void KMailConnection::unregisteredFromDCOP( const QCString& appId )
{
  if ( mKMailIcalIfaceStub && mKMailIcalIfaceStub->app() == appId ) {
    // Delete the stub so that the next time we need to talk to kmail,
    // we'll know that we need to start a new one.
    delete mKMailIcalIfaceStub;
    mKMailIcalIfaceStub = 0;
  }
}

#include "kmailconnection.moc"
