
#include <qdir.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kservice.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kgenericfactory.h>
#include <kparts/componentfactory.h>


#include "kdevice.h"
#include "konnectorplugin.h"
#include "konnector.h"
#include "kapabilities.h"
#include "koperations.h"


class Konnector::KonnectorPrivate{
public:
  KonnectorPrivate(){

  }
  QMap<QString, KonnectorPlugin*> m_konnectors;
  KDeviceList m_devices;
};

Konnector::Konnector( QObject *object, const char *name ) : QObject( object, name )
{
  // initialize
  d = new KonnectorPrivate;
  kdDebug(5201) << "c'tor" << endl;
}
Konnector::~Konnector()
{
    QMap<QString,  KonnectorPlugin*>::Iterator it;
    for ( it = d->m_konnectors.begin(); it != d->m_konnectors.end(); ++it ) {
        delete it.data();
    }
  delete d;
};
KDeviceList Konnector::query(const QString &category )
{
  // lets find
    kdDebug(5201) << "query " << category << endl;
  allDevices();
  if(category.isEmpty() ){
      kdDebug(5201) << "no devices found" << endl;
    return d->m_devices;
  }
  KDeviceList dev;
  for(KDeviceList::Iterator it = d->m_devices.begin(); it != d->m_devices.end(); ++it ){
    if( (*it).group() == category){
      dev.append( (*it) );
      kdDebug(5201) << "searching" << endl;
    }
  }
  return dev;
}
QString Konnector::registerKonnector(const QString &Device )
{
    kdDebug(5201) << "registerKonnector " << Device << endl;
  for(KDeviceList::Iterator it = d->m_devices.begin(); it != d->m_devices.end(); ++it ){
    if((*it).identify() == Device ){ // ok found
      // now load the lib
      QString randStr;
      do{
	randStr = kapp->randomString(8);
	kdDebug(5201) << "randStr :" << randStr << endl;
      }while( d->m_konnectors.contains( randStr ) );
      KonnectorPlugin* plugin =  KParts::ComponentFactory::
	createInstanceFromLibrary<KonnectorPlugin>( (*it).library(), this );
      if(!plugin){
	return QString::null;
      }
      plugin->setUDI( randStr );
      connect(plugin, SIGNAL(sync(const QString&, KSyncEntryList ) ),
              this, SLOT(slotSync(const QString&,  KSyncEntryList ) ) );

      connect(plugin, SIGNAL(errorKonnector(const QString&, int, const QString&) ),
              this, SLOT(slotError(const QString&,int, const QString&)) );
      connect(plugin, SIGNAL(stateChanged(const QString&,  bool ) ),
              this, SLOT(slotChanged( const QString&,  bool ) ) );
      d->m_konnectors.insert(randStr, plugin  );
      return randStr;
    }
  }
  return QString::null;
}
QString Konnector::registerKonnector(const KDevice &Device )
{
    kdDebug(5201) << "registerKonnector lib:" << Device.library() << endl;
  QString randStr;
  do{
    randStr = kapp->randomString(8);
  }while( d->m_konnectors.contains( randStr ) );
  KonnectorPlugin* plugin =  KParts::ComponentFactory::
    createInstanceFromLibrary<KonnectorPlugin>( Device.library(), this );
  if(!plugin){
      kdDebug(5201) << "failed to load"<<  endl;
    return QString::null;
  }
  plugin->setUDI(randStr);
  connect(plugin, SIGNAL(sync(const QString&, KSyncEntryList ) ),
              this, SLOT(slotSync(const QString&,  KSyncEntryList ) ) );

  connect(plugin, SIGNAL(errorKonnector(const QString&, int, const QString&) ),
              this, SLOT(slotError(const QString&,int, const QString&)) );
  connect(plugin, SIGNAL(stateChanged(const QString&,  bool ) ),
              this, SLOT(slotChanged( const QString&,  bool ) ) );
  d->m_konnectors.insert(randStr, plugin  );
  return randStr;
}
Kapabilities Konnector::capabilities (const QString&  udi )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return Kapabilities();

  return plugin->capabilities();
}
void Konnector::setCapabilities(const QString &udi, const Kapabilities& cap )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  return plugin->setCapabilities( cap);
}
QByteArray Konnector::file(const QString &udi, const QString &path )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return QByteArray();

  return plugin->retrFile( path );
}
KSyncEntry* Konnector::fileAsEntry(const QString& udi,  const QString &path )
{
    KSyncEntry* entry = 0l;
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return entry;

    return plugin->retrEntry( path );
}
void Konnector::retrieveFile(const QString &udi, const QString &file )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  plugin->insertFile(file );
}
void Konnector::write( const QString &udi, KSyncEntryList entry)
{
    kdDebug() << "write " << endl;
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  plugin->slotWrite( entry );
}
void Konnector::write( const QString &udi, KOperationsList operations)
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  plugin->slotWrite(operations );
}
void Konnector::write( const QString &udi, const QString &dest, const QByteArray &array )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return;

  return plugin->slotWrite(dest, array );
}
bool Konnector::isConnected(const QString &udi ){
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return false;

  return plugin->isConnected();
}
bool Konnector::startSync(const QString &udi )
{
  KonnectorPlugin *plugin = pluginByUDI( udi );
  if( plugin == 0)
    return false;

  return plugin->startSync();
}
void Konnector::allDevices()
{
    kdDebug(5201) << "searching for devices" << endl;
  d->m_devices.clear();
  QStringList list = KGlobal::dirs()->findDirs("data", "kitchensync" );

  if(list.isEmpty() )
      kdDebug(5201) << "no dirs found" << endl;

  for(QStringList::Iterator it = list.begin(); it != list.end(); ++it ){
    QDir dir( (*it), "*.desktop" );
    kdDebug(5201) << "searching for devices in " << (*it) << endl;
    QStringList list2 = dir.entryList();
    QStringList::Iterator it2;
    for(it2 = list2.begin(); it2 != list2.end(); ++it2 ){
      kdDebug(5201) << (*it) << " " << (*it2);
      KService service( (*it) + (*it2) );
      QString name = service.name();
      QString library = service.library();
      kdDebug(5201) << "library library " << library << endl;
      QString group = (service.property( QString::fromLatin1("Group")  )).toString();
      QString vendor = (service.property("Vendor" )).toString();
      kdDebug(5201) << "group : " << group << "  vendor : " << vendor << endl;
      QString id = (service.property("Id")).toString();
      KDevice device( name, group, vendor, library, id );
      d->m_devices.append(device );
// debug
      //QStringList lis = service.propertyNames();
      //for(QStringList::Iterator props = lis.begin(); props != lis.end(); ++props ){
      //  kdDebug(5201) << "Key " << (*props) << endl;
      //}
//end debug

    }
  }
}
KonnectorPlugin* Konnector::pluginByUDI(const QString &udi )
{
  KonnectorPlugin* plugin=0l;
  if( d->m_konnectors.contains(udi ) ){
    QMap<QString, KonnectorPlugin*>::Iterator it;
    it = d->m_konnectors.find( udi );
    plugin = it.data();
    kdDebug(5201) << "UDIS " << udi << " " << plugin->udi() << endl;
   }
  return plugin;
}
KonnectorPlugin* Konnector::pluginByUDI(const QString &udi )const
{
  KonnectorPlugin* plugin=0l;
  if( d->m_konnectors.contains(udi ) ){
    QMap<QString, KonnectorPlugin*>::ConstIterator it;
    it = d->m_konnectors.find( udi );
    plugin = it.data();
    kdDebug(5201) << "UDIS " << udi << " " << plugin->udi() << endl;
   }
  return plugin;
}
void Konnector::slotSync(const QString &udi,  KSyncEntryList entry)
{
  emit wantsToSync(udi, entry );
}

void Konnector::slotError(const QString &udi, int mode, const QString &
                          info)
{
  emit konnectorError(udi, mode, info );

}
bool Konnector::connectDevice( const QString &udi )
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return false;

    return plugin->connectDevice();
}
void Konnector::disconnectDevice( const QString &udi )
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return;

    return plugin->disconnectDevice();
}
QIconSet Konnector::iconSet(const QString& udi )const
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return QIconSet();

    return plugin->iconSet();
}
QString Konnector::id(const QString& udi )const
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return QString::null;

    return plugin->id();
}
void Konnector::slotChanged(const QString& i,  bool b)
{
    kdDebug(5201) << "Konnector state changed" << endl;
    emit stateChanged( i,  b );
}
QString Konnector::metaId( const QString& udi ) const
{
    KonnectorPlugin *plugin = pluginByUDI( udi );
    if( plugin == 0l)
        return QString::null;

    return plugin->metaId();
}
#include "konnector.moc"
