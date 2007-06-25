/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

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

#include "kmailconnectionadaptor.h"
#include "resourcekolabbase.h"

#include <kdebug.h>
#include <kdbusservicestarter.h>
#include <kmail_groupwareinterface.h>
#include <kmail_util.h>
#include <klocale.h>
#include <QDBusAbstractInterface>
#include <QDBusError>

using namespace Kolab;

Q_DECLARE_METATYPE(KMail::SubResource )
Q_DECLARE_METATYPE(QList<KMail::SubResource> )

const QDBusArgument &operator<<(QDBusArgument &arg, const KMail::SubResource &subResource)
{
    arg.beginStructure();
    arg << subResource.location << subResource.label << subResource.writable << subResource.alarmRelevant;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, KMail::SubResource &subResource)
{
    arg.beginStructure();
    arg >> subResource.location >> subResource.label >> subResource.writable >> subResource.alarmRelevant;
    arg.endStructure();
    return arg;
}

static void registerTypes()
{
    static bool registered = false;
    if (!registered) {
      qDBusRegisterMetaType<KMail::SubResource>();
      qDBusRegisterMetaType< QList<KMail::SubResource> >();
      registered = true;
    }
}

KMailConnectionAdaptor::KMailConnectionAdaptor( ResourceKolabBase* resource, const QString &uniq )
  : mResource( resource )
  , mKmailGroupwareInterface( 0 )
{
  // Make the connection to KMail ready
#if 0
  // Do we need to kill the interface when kmail exits, to restart kmail next time?
  connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
           SLOT(dbusServiceOwnerChanged(QString,QString,QString)) );
#endif
  // TODO connect to the dbus signals from kmail here
  QDBusConnection::sessionBus().registerObject( uniq,this,QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals );
}


KMailConnectionAdaptor::~KMailConnectionAdaptor()
{
  delete mKmailGroupwareInterface;
  mKmailGroupwareInterface = 0;
}

bool KMailConnectionAdaptor::connectToKMail()
{
   kDebug()<<" bool KMailConnectionAdaptor::connectToKMail()\n";
  if ( !mKmailGroupwareInterface ) {
    QString error;
    QString dbusService;
    int result = KDBusServiceStarter::self()->
      findServiceFor( "DBUS/ResourceBackend/IMAP", QString(),
                      &error, &dbusService );
    if ( result != 0 ) {
      kError(5650) << "Couldn't connect to the IMAP resource backend\n";
      // TODO: You might want to show "error" (if not empty) here,
      // using e.g. KMessageBox
      return false;
    }
//TODO verify interface
    qDebug()<<" dbusService :"<<dbusService<<endl;
    mKmailGroupwareInterface = new QDBusInterface( dbusService, "org.kde.kmail", "/Groupware", QDBusConnection::sessionBus());
    //typedef QMap<quint32, QString> Quint32StringMap;
   //Q_DECLARE_METATYPE(Quint32StringMap)
    //qDBusRegisterMetaType<Quint32StringMap>();
  //TODO verify if it connected.

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "incidenceAdded", this, SLOT(fromKMailAddIncidence(QString,QString,quint32,int,QString) ) );
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "incidenceDeleted", this, SLOT( fromKMailDelIncidence(QString,QString,QString) ) );
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "signalRefresh", this, SLOT( fromKMailRefresh(QString,QString) ) );
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "subresourceAdded", this, SLOT(fromKMailAddSubresource( QString, QString, QString, bool, bool ) ) );
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "subresourceDeleted", this, SLOT(fromKMailDelSubresource(QString,QString) ) );
    dbus.connect( DBUS_KMAIL, "/GroupWare", "org.kde.kmail.groupware", "asyncLoadResult", this, SLOT( fromKMailAsyncLoadResult(QMap<quint32, QString>, QString, QString) ) );

/*

    // Attach to the KMail signals
    if ( !connectKMailSignal( "incidenceAdded(QString,QString,quint32,int,QString)",
                              "fromKMailAddIncidence(QString,QString,quint32,int,QString)" ) )
      kError(5650) << "DCOP connection to incidenceAdded failed" << endl;
    if ( !connectKMailSignal( "incidenceDeleted(QString,QString,QString)",
                              "fromKMailDelIncidence(QString,QString,QString)" ) )
      kError(5650) << "DCOP connection to incidenceDeleted failed" << endl;
    if ( !connectKMailSignal( "signalRefresh(QString,QString)",
                              "fromKMailRefresh(QString,QString)" ) )
      kError(5650) << "DCOP connection to signalRefresh failed" << endl;
    if ( !connectKMailSignal( "subresourceAdded( QString, QString, QString, bool, bool )",
                              "fromKMailAddSubresource( QString, QString, QString, bool, bool )" ) )
      kError(5650) << "DCOP connection to subresourceAdded failed" << endl;
    if ( !connectKMailSignal( "subresourceDeleted(QString,QString)",
                              "fromKMailDelSubresource(QString,QString)" ) )
      kError(5650) << "DCOP connection to subresourceDeleted failed" << endl;
    if ( !connectKMailSignal( "asyncLoadResult(QMap<quint32, QString>, QString, QString)",
                              "fromKMailAsyncLoadResult(QMap<quint32, QString>, QString, QString)" ) )
      kError(5650) << "DCOP connection to asyncLoadResult failed" << endl;
*/
  }
  kDebug()<<" mKmailGroupwareInterface != 0 :"<<(  mKmailGroupwareInterface != 0 )<<endl;
  return ( mKmailGroupwareInterface != 0 );
}

bool KMailConnectionAdaptor::fromKMailAddIncidence( const QString& type,
                                             const QString& folder,
                                             quint32 sernum,
                                             int format,
                                             const QString& data )
{
  if ( format != KMail::StorageXML
      && format != KMail::StorageIcalVcard )
    return false;
//   kDebug(5650) << "KMailConnectionAdaptor::fromKMailAddIncidence( " << type << ", "
//                 << folder << " ). iCal:\n" << ical << endl;
  return mResource->fromKMailAddIncidence( type, folder, sernum, format, data );
}

void KMailConnectionAdaptor::fromKMailDelIncidence( const QString& type,
                                             const QString& folder,
                                             const QString& xml )
{
//   kDebug(5650) << "KMailConnectionAdaptor::fromKMailDelIncidence( " << type << ", "
//                 << folder << ", " << uid << " )\n";
  mResource->fromKMailDelIncidence( type, folder, xml );
}

void KMailConnectionAdaptor::fromKMailRefresh( const QString& type, const QString& folder )
{
//   kDebug(5650) << "KMailConnectionAdaptor::fromKMailRefresh( " << type << ", "
//                 << folder << " )\n";
  mResource->fromKMailRefresh( type, folder );
}

void KMailConnectionAdaptor::fromKMailAddSubresource( const QString& type,
                                               const QString& resource,
                                               const QString& label,
                                               bool writable,
                                               bool alarmRelevant )
{
//   kDebug(5650) << "KMailConnectionAdaptor::fromKMailAddSubresource( " << type << ", "
//                 << resource << " )\n";
  mResource->fromKMailAddSubresource( type, resource, label,
                                      writable, alarmRelevant );
}

void KMailConnectionAdaptor::fromKMailDelSubresource( const QString& type,
                                               const QString& resource )
{
//   kDebug(5650) << "KMailConnectionAdaptor::fromKMailDelSubresource( " << type << ", "
//                 << resource << " )\n";
  mResource->fromKMailDelSubresource( type, resource );
}

void KMailConnectionAdaptor::fromKMailAsyncLoadResult( const QMap<quint32, QString>& map,
                                                const QString& type,
                                                const QString& folder )
{
  mResource->fromKMailAsyncLoadResult( map, type, folder );
}

bool KMailConnectionAdaptor::connectKMailSignal( const QByteArray& signal,
                                          const QByteArray& method )
{
#if 0 // TODO
  return connectDCOPSignal( "kmail", dcopObjectId, signal, method, false )
    && connectDCOPSignal( "kontact", dcopObjectId, signal, method, false );
#else
  return false;
#endif
}

bool KMailConnectionAdaptor::kmailSubresources( QList<KMail::SubResource>& lst,
                                         const QString& contentsType )
{
  if ( !connectToKMail() )
    return false;
  registerTypes();

  QDBusReply<QVariantList> r = mKmailGroupwareInterface->call( "subresourcesKolab", contentsType );
  if ( r.isValid() )
  {
    //  QVariantList args = r;
    //QVariantList::ConstIterator it = r.constBegin();
    //QVariantList::ConstIterator end = r.constEnd();
    //for ( ; it != end; ++it)
    {
      //lst<< *it,
    }
  }
  return (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}

bool KMailConnectionAdaptor::kmailIncidencesCount( int& count,
                                            const QString& mimetype,
                                            const QString& resource )
{
  if ( !connectToKMail() )
    return false;

  QDBusReply<int> val= mKmailGroupwareInterface->call( "incidencesKolabCount", mimetype, resource );
  count = val;
  return (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}

bool KMailConnectionAdaptor::kmailIncidences( QMap<quint32, QString>& lst,
                                       const QString& mimetype,
                                       const QString& resource,
                                       int startIndex,
                                       int nbMessages )
{
  if ( !connectToKMail() )
    return false;
  //TODO port it
#if 0
  if ( !connectToKMail() )
    return false;

  lst = mKmailGroupwareInterface->call( "incidencesKolab",  mimetype, resource, startIndex, nbMessages );
  return (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
#endif
  //Registered dbus type.
  QDBusReply<QVariantMap> r = mKmailGroupwareInterface->call( "incidencesKolab",  mimetype, resource, startIndex, nbMessages );
  if (r.isValid()) {
    // convert <QVariant,QString> to <quint32, QString>
    const QVariantMap val = r.value();
    for( QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it ) {
      //lst.insert(it.key().toUint(), it.value().toString());
    }
  }
  return (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}


bool KMailConnectionAdaptor::kmailGetAttachment( KUrl& url,
                                          const QString& resource,
                                          quint32 sernum,
                                          const QString& filename )
{
  if ( !connectToKMail() )
    return false;

  QDBusReply<QString> val = mKmailGroupwareInterface->call( "getAttachment", resource, sernum, filename );
  QString tmp = val;
  url = KUrl( val );
  return (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}

bool KMailConnectionAdaptor::kmailDeleteIncidence( const QString& resource,
                                            quint32 sernum )
{
  QDBusReply<bool> deleteIncidence = mKmailGroupwareInterface->call( "deleteIncidenceKolab", resource, sernum );
  bool b = deleteIncidence;
  return connectToKMail()
    && b
    && (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
}

bool KMailConnectionAdaptor::kmailUpdate( const QString& resource,
                                   quint32& sernum,
                                   const QString& subject,
                                   const QString& plainTextBody,
                                   const QMap<QByteArray, QString>& customHeaders,
                                   const QStringList& attachmentURLs,
                                   const QStringList& attachmentMimetypes,
                                   const QStringList& attachmentNames,
                                   const QStringList& deletedAttachments )
{
  //PORT it
#if 0
  kDebug(5006) << kBacktrace() << endl;
  if ( connectToKMail() ) {
    sernum = mKmailGroupwareInterface->update( resource, sernum, subject, plainTextBody, customHeaders,
                                          attachmentURLs, attachmentMimetypes, attachmentNames,
                                          deletedAttachments );
    return sernum && (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
  } else
    return false;
#endif
  return false;
}

bool KMailConnectionAdaptor::kmailStorageFormat( KMail::StorageFormat& type,
                                          const QString& folder )
{
  //TODO port it
#if 0
  bool ok = connectToKMail();
  type = mKmailGroupwareInterface->storageFormat( folder );
  return ok && (mKmailGroupwareInterface->lastError().type()==QDBusError::NoError);
#endif
}


bool KMailConnectionAdaptor::kmailTriggerSync( const QString &contentsType )
{
  bool ok = connectToKMail();
  QDBusReply<bool> val =  mKmailGroupwareInterface->call( "triggerSync", contentsType );
  bool ret = val;
  return ok && ret;
}

void KMailConnectionAdaptor::dbusServiceOwnerChanged(const QString & service, const QString & oldOwner, const QString & newOwner)
{
#if 0 // TODO
  if ( mKmailGroupwareInterface && mKmailGroupwareInterface->app() == appId ) {
    // Delete the stub so that the next time we need to talk to kmail,
    // we'll know that we need to start a new one.
    delete mKmailGroupwareInterface;
    mKmailGroupwareInterface = 0;
  }
#endif
}

#include "kmailconnectionadaptor.moc"
