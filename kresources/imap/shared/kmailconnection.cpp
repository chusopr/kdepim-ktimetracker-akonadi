/*
    This file is part of the IMAP resources.

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
*/

#include <kdebug.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdcopservicestarter.h>
#include <klocale.h>

#include "kmailicalIface_stub.h"

#include "resourceimapshared.h"
#include "kmailconnection.h"

using namespace ResourceIMAPBase;


KMailConnection::KMailConnection( ResourceIMAPShared* resource,
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
      kdDebug() << "Couldn't connect to the IMAP resource backend\n";
      // TODO: You might want to show "error" (if not empty) here,
      // using e.g. KMessageBox
      return false;
    }

    mKMailIcalIfaceStub = new KMailICalIface_stub( kapp->dcopClient(),
                                                   dcopService, dcopObjectId );

    // Attach to the KMail signals
    if ( !connectKMailSignal( "incidenceAdded(QString,QString,QString)",
                              "addIncidence(QString,QString,QString)" ) )
      kdError() << "DCOP connection to incidenceAdded failed" << endl;
    if ( !connectKMailSignal( "incidenceDeleted(QString,QString,QString)",
                              "deleteIncidence(QString,QString,QString)" ) )
      kdError() << "DCOP connection to incidenceDeleted failed" << endl;
    if ( !connectKMailSignal( "signalRefresh(QString,QString)",
                              "slotRefresh(QString,QString)" ) )
      kdError() << "DCOP connection to signalRefresh failed" << endl;
    if ( !connectKMailSignal( "subresourceAdded(QString,QString,QString)",
                              "subresourceAdded(QString,QString,QString)" ) )
      kdError() << "DCOP connection to signalRefresh failed" << endl;
    if ( !connectKMailSignal( "subresourceDeleted(QString,QString,QString)",
                              "subresourceDeleted(QString,QString,QString)" ) )
      kdError() << "DCOP connection to signalRefresh failed" << endl;
  }

  return ( mKMailIcalIfaceStub != 0 );
}

bool KMailConnection::addIncidence( const QString& type, const QString& folder,
                                    const QString& ical )
{
  return mResource->addIncidence( type, ical );
}

void KMailConnection::deleteIncidence( const QString& type,
                                       const QString& folder,
                                       const QString& uid )
{
  mResource->deleteIncidence( type, uid );
}

void KMailConnection::slotRefresh( const QString& type, const QString& folder )
{
  mResource->slotRefresh( type );
}

void KMailConnection::subresourceAdded( const QString& type,
                                        const QString& resource,
                                        const QString& id )
{
  mResource->subresourceAdded( type, id );
}

void KMailConnection::subresourceDeleted( const QString& type,
                                          const QString& resource,
                                          const QString& id )
{
  mResource->subresourceDeleted( type, id );
}

bool KMailConnection::connectKMailSignal( const QCString& signal,
                                          const QCString& method )
{
  return connectDCOPSignal( "kmail", dcopObjectId, signal, method, false );
}

bool KMailConnection::kmailIncidences( QStringList& lst, const QString& type,
                                       const QString& resource )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error: Can't connect to KMail\n";
    return false;
  }

  lst = mKMailIcalIfaceStub->incidences( type, resource );
  return mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailAddIncidence( const QString& type,
                                         const QString& resource,
                                         const QString& uid,
                                         const QString& incidence )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error: Can't connect to KMail\n";
    return false;
  }

  return mKMailIcalIfaceStub->addIncidence( type, resource, uid, incidence )
    && mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailDeleteIncidence( const QString& type,
                                            const QString& resource,
                                            const QString& uid )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error: Can't connect to KMail\n";
    return false;
  }

  return mKMailIcalIfaceStub->deleteIncidence( type, resource, uid )
    && mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailUpdate( const QString& type,
                                   const QString& resource,
                                   const QStringList& lst )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error: Can't connect to KMail\n";
    return false;
  }

  return mKMailIcalIfaceStub->update( type, resource, lst )
    && mKMailIcalIfaceStub->ok();
}

bool KMailConnection::kmailUpdate( const QString& type,
                                   const QString& resource, const QString& uid,
                                   const QString& incidence )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error: Can't connect to KMail\n";
    return false;
  }

  return mKMailIcalIfaceStub->update( type, resource, uid, incidence )
    && mKMailIcalIfaceStub->ok();
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
