/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kcalmigrator.h"

#include "icalsettings.h"

#include <akonadi/agentinstance.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agenttype.h>

#include <KDebug>
#include <KGlobal>

using namespace Akonadi;

KCalMigrator::KCalMigrator() :
    KResMigrator<KCal::ResourceCalendar>( "calendar" )
{
}

void KCalMigrator::migrateResource( KCal::ResourceCalendar* res)
{
  kDebug() << res->identifier() << res->type();
  if ( res->type() == "file" && !mBridgeOnly )
    migrateFileResource( res );
  else if ( migrationState( res ) == None )
    migrateToBridge( res, "akonadi_kcal_resource" );
  else
    migrateNext();
}

void KCalMigrator::migrateFileResource(KCal::ResourceCalendar * res)
{
  const KConfigGroup kresCfg = kresConfig( res );
 if ( kresCfg.readEntry( "Format", "" ) != "ical" ) {
    kDebug() << "Unsupported file format found!";
    return;
  }
  const AgentType type = AgentManager::self()->type( "akonadi_ical_resource" );
  if ( !type.isValid() ) {
    kDebug() << "Unable to obtain ical resource type!";
    return;
  }
  AgentInstanceCreateJob *job = new AgentInstanceCreateJob( type, this );
  connect( job, SIGNAL(result(KJob*)), SLOT(fileResourceCreated(KJob*)) );
  setResourceForJob( job, res );
  job->start();
}

void KCalMigrator::fileResourceCreated(KJob * job)
{
  if ( job->error() ) {
    kDebug() << "Failed to create ical resource!";
    return;
  }
  KCal::ResourceCalendar *res = resourceForJob( job );
  AgentInstance instance = static_cast<AgentInstanceCreateJob*>( job )->instance();
  const KConfigGroup kresCfg = kresConfig( res );
  instance.setName( kresCfg.readEntry( "ResourceName", "Migrated Calendar" ) );

  OrgKdeAkonadiICalSettingsInterface *iface = new OrgKdeAkonadiICalSettingsInterface( "org.freedesktop.Akonadi.Resource." + instance.identifier(),
      "/Settings", QDBusConnection::sessionBus(), this );
  if ( !iface->isValid() ) {
    kDebug() << "Failed to obtain dbus interface for resource configuration!";
    return;
  }
  // TODO: the akonadi ical resource doesn't support remote files yet...
  iface->setPath( kresCfg.readPathEntry( "CalendarURL", "" ) );
  iface->setReadOnly( res->readOnly() );
  instance.reconfigure();
  setMigrationState( res, Complete, instance.identifier() );
  migrateNext();
}

#include "kcalmigrator.moc"
