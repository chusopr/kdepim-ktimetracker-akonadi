
#include <qvaluelist.h>
#include <qpair.h>
#include <qptrlist.h>
#include <qhostaddress.h>
#include <kapabilities.h>
#include <koperations.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include "opiesocket.h"
#include "opiekonnector.h"

typedef KGenericFactory<OpiePlugin, QObject>  OpieKonnectorPlugin;
K_EXPORT_COMPONENT_FACTORY( libopiekonnector,  OpieKonnectorPlugin );

class OpiePlugin::OpiePluginPrivate{
 public:
  OpiePluginPrivate(){
    connection = false;
    socket = 0;
  }
  QString udi;
  bool connection;
  QStringList retrFiles;
  OpieSocket *socket;
};

OpiePlugin::OpiePlugin(QObject *obj, const char *name, const QStringList )
  : KonnectorPlugin(obj, name )
{
    kdDebug() << "OpiePlugin ";
    d = new OpiePluginPrivate;
    d->socket = new OpieSocket(this, "opiesocket");
    connect(d->socket, SIGNAL(sync(QPtrList<KSyncEntry> ) ),
	    this, SLOT(slotSync(QPtrList<KSyncEntry> ) ) );
    connect(d->socket, SIGNAL(errorKonnnector(int, QString ) ),
	    this, SLOT(slotErrorKonnector(int, QString) ) );
}
OpiePlugin::~OpiePlugin()
{
  delete d;
}
void OpiePlugin::setUDI(const QString &udi )
{
  d->udi = udi;
}

QString OpiePlugin::udi()const
{
  return d->udi;
}


Kapabilities OpiePlugin::capabilities( )
{
  // create the capabilities Apply
  kdDebug() << "OpiePlugin capabilities" << endl;
  Kapabilities caps;
  caps.setSupportsPushSync( true );
  caps.setNeedsConnection( true );
  caps.setSupportsListDir( false );
  caps.setNeedsIPs( true );
  caps.setNeedsSrcIP( false );
  caps.setNeedsDestIP( true );
  caps.setAutoHandle( false );
  caps.setNeedAuthentication( true );
  QValueList<QPair<QString,QString> > user;
  user.append(qMakePair(QString::fromLatin1("root"),  QString::fromLatin1("test123") ) );
  user.append(qMakePair(QString::fromLatin1("ich"), QString::fromLatin1("test321") ) );
  caps.setUserProposals( user );
  QValueList< QPair<QHostAddress, QHostAddress> > ips;
  QHostAddress src;
  QHostAddress dest;
  src.setAddress("192.168.1.100" );
  dest.setAddress("192.168.1.201");
  ips.append( qMakePair(src, dest ) );
  src.setAddress("127.0.0.1" );
  dest.setAddress("127.0.0.1");
  ips.append( qMakePair(src, dest ) );
  return caps;
}

void OpiePlugin::setCapabilities( const Kapabilities &kaps )
{
  // create 
  d->socket->setSrcIP( kaps.srcIP() );
  d->socket->setDestIP( kaps.destIP() );
  d->socket->setUser( kaps.user() );
  d->socket->setPassword( kaps.password() );
  d->socket->startUp();
}
bool OpiePlugin::startSync()
{
  return d->socket->startSync();
}
bool OpiePlugin::isConnected()
{
  return d->socket->isConnected();
}
bool OpiePlugin::insertFile(const QString &fileName )
{
    return d->socket->insertFile(fileName );
}
QByteArray OpiePlugin::retrFile(const QString &path )
{
    return d->socket->retrFile(path );
}
void OpiePlugin::slotWrite(const QString &path, const QByteArray &array )
{
    d->socket->write(path, array );
}
void OpiePlugin::slotWrite(QPtrList<KSyncEntry> entry)
{
    d->socket->write(entry );
};
void OpiePlugin::slotSync(QPtrList<KSyncEntry> entry )
{
    emit sync( d->udi, entry );
}
void OpiePlugin::slotErrorKonnector( int mode, QString error )
{
    emit errorKonnector(d->udi, mode, error );
} 
void OpiePlugin::slotWrite(QValueList<KOperations> operations )
{
    d->socket->write(operations );
}




