/***************************************************************************
 *   Copyright (C) 2006 by Tobias Koenig <tokoe@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QSettings>

#include <config-prefix.h> // for AKONADIDIR

#include "pluginmanager.h"

PluginManager::PluginManager( QObject *parent )
  : QObject( parent )
{
  mTracer = new org::kde::Akonadi::Tracer( "org.kde.Akonadi", "/tracing", QDBus::sessionBus(), this );

  QFileSystemWatcher *watcher = new QFileSystemWatcher( this );
  watcher->addPath( pluginInfoPath() );

  connect( watcher, SIGNAL( directoryChanged( const QString& ) ),
           this, SLOT( updatePluginInfos() ) );

  updatePluginInfos();
}

PluginManager::~PluginManager()
{
}

QStringList PluginManager::agentTypes() const
{
  return mPluginInfos.keys();
}

QString PluginManager::agentName( const QString &identifier ) const
{
  if ( !mPluginInfos.contains( identifier ) ) {
    mTracer->warning( QLatin1String( "akonadi_control::agentName" ),
                     QString( "Agent type with identifier '%1' does not exist" ).arg( identifier ) );
    return QString();
  }

  return mPluginInfos[ identifier ].name;
}

QString PluginManager::agentComment( const QString &identifier ) const
{
  if ( !mPluginInfos.contains( identifier ) ) {
    mTracer->warning( QLatin1String( "akonadi_control::agentComment" ),
                     QString( "Agent type with identifier '%1' does not exist" ).arg( identifier ) );
    return QString();
  }

  return mPluginInfos[ identifier ].comment;
}

QString PluginManager::agentIcon( const QString &identifier ) const
{
  if ( !mPluginInfos.contains( identifier ) ) {
    mTracer->warning( QLatin1String( "akonadi_control::agentIcon" ),
                     QString( "Agent type with identifier '%1' does not exist" ).arg( identifier ) );
    return QString();
  }

  return mPluginInfos[ identifier ].icon;
}

QStringList PluginManager::agentMimeTypes( const QString &identifier ) const
{
  if ( !mPluginInfos.contains( identifier ) ) {
    mTracer->warning( QLatin1String( "akonadi_control::agentMimeTypes" ),
                     QString( "Agent type with identifier '%1' does not exist" ).arg( identifier ) );
    return QStringList();
  }

  return mPluginInfos[ identifier ].mimeTypes;
}

QStringList PluginManager::agentCapabilities( const QString &identifier ) const
{
  if ( !mPluginInfos.contains( identifier ) ) {
    mTracer->warning( QLatin1String( "akonadi_control::agentCapabilities" ),
                     QString( "Agent type with identifier '%1' does not exist" ).arg( identifier ) );
    return QStringList();
  }

  return mPluginInfos[ identifier ].capabilities;
}


QString PluginManager::createAgentInstance( const QString &identifier )
{
  return QString();
}

void PluginManager::removeAgentInstance( const QString &identifier )
{
}

void PluginManager::updatePluginInfos()
{
  mPluginInfos.clear();

  QDir directory( pluginInfoPath(), "*.desktop" );

  QStringList files = directory.entryList();
  for ( int i = 0; i < files.count(); ++i ) {
    const QString fileName = directory.absoluteFilePath( files[ i ] );

    QSettings file( fileName, QSettings::IniFormat );
    file.beginGroup( "Desktop Entry" );

    PluginInfo info;
    info.name = file.value( "Name" ).toString();
    info.comment = file.value( "Comment" ).toString();
    info.icon = file.value( "Icon" ).toString();
    info.mimeTypes = file.value( "X-Akonadi-MimeTypes" ).toStringList();
    info.capabilities = file.value( "X-Akonadi-Capabilities" ).toStringList();
    info.exec = file.value( "Exec" ).toString();
    const QString identifier = file.value( "X-Akonadi-Identifier" ).toString();

    file.endGroup();

    if ( identifier.isEmpty() ) {
      mTracer->error( QLatin1String( "akonadi_control::updatePluginInfos" ),
                     QString( "Agent desktop file '%1' contains empty identifier" ).arg( fileName ) );
      continue;
    }

    if ( mPluginInfos.contains( identifier ) ) {
      mTracer->error( QLatin1String( "akonadi_control::updatePluginInfos" ),
                     QString( "Duplicated agent identifier '%1' from file '%2'" ).arg( fileName, identifier ) );
      continue;
    }

    mPluginInfos.insert( identifier, info );
  }
}

QString PluginManager::pluginInfoPath()
{
  return QString( "%1/share/apps/akonadi/agents" ).arg( AKONADIDIR );
}

#include "pluginmanager.moc"
