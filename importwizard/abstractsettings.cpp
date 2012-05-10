/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "abstractsettings.h"
#include "importwizard.h"
#include "importsettingpage.h"

#include "mailcommon/filter/filteractionmissingargumentdialog.h"

#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <mailtransport/transportmanager.h>

#include <KLocale>
#include <KDebug>
#include <KSharedConfig>

#include <akonadi/agenttype.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>

#include <QDBusReply>
#include <QDBusInterface>
#include <QMetaMethod>

using namespace Akonadi;

AbstractSettings::AbstractSettings(ImportWizard *parent)
  :mImportWizard(parent)
{
  mManager = new KPIMIdentities::IdentityManager( false, this, "mIdentityManager" );
  mKmailConfig = KSharedConfig::openConfig( QLatin1String( "kmail2rc" ) );
}

AbstractSettings::~AbstractSettings()
{
  delete mManager;
}

KPIMIdentities::Identity* AbstractSettings::createIdentity()
{
  KPIMIdentities::Identity* identity = &mManager->newFromScratch( QString() );
  addImportInfo(i18n("Setting up identity..."));
  return identity;
}

void AbstractSettings::storeIdentity(KPIMIdentities::Identity* identity)
{
  mManager->setAsDefault( identity->uoid() );
  mManager->commit();
  addImportInfo(i18n("Identity set up."));
}


MailTransport::Transport *AbstractSettings::createTransport()
{
  MailTransport::Transport* mt = MailTransport::TransportManager::self()->createTransport();
  addImportInfo(i18n("Setting up transport..."));
  return mt;
}

void AbstractSettings::storeTransport(MailTransport::Transport * mt, bool isDefault )
{
  mt->forceUniqueName();
  mt->writeConfig();
  MailTransport::TransportManager::self()->addTransport( mt );
  if ( isDefault )
    MailTransport::TransportManager::self()->setDefaultTransport( mt->id() );
  addImportInfo(i18n("Transport set up."));
}




void AbstractSettings::addImportInfo( const QString& log )
{
  mImportWizard->importSettingPage()->addImportInfo( log );
}

void AbstractSettings::addImportError( const QString& log )
{
  mImportWizard->importSettingPage()->addImportError( log );
}

Akonadi::Collection::Id AbstractSettings::adaptFolderId( const QString& folder)
{
  Akonadi::Collection::Id newFolderId=-1;
  bool exactPath = false;
  Akonadi::Collection::List lst = FilterActionMissingCollectionDialog::potentialCorrectFolders( folder, exactPath );
  if ( lst.count() == 1 && exactPath )
    newFolderId = lst.at( 0 ).id();
  else {
    FilterActionMissingCollectionDialog *dlg = new FilterActionMissingCollectionDialog( lst, QString(), folder );
    if ( dlg->exec() ) {
      newFolderId = dlg->selectedCollection().id();
    }
    delete dlg;
  }
  return newFolderId;
}

QString AbstractSettings::adaptFolder( const QString& folder)
{
  Akonadi::Collection::Id newFolderId= adaptFolderId(folder);
  if(newFolderId == -1 )
    return QString();
  return QString::number(newFolderId);
}

void AbstractSettings::addCheckMailOnStartup(const QString& agentIdentifyName,bool loginAtStartup)
{
  if(agentIdentifyName.isEmpty())
    return;
  const QString groupName = QString::fromLatin1("Resource %1").arg(agentIdentifyName);
  addKmailConfig(groupName,QLatin1String("CheckOnStartup"), loginAtStartup);
}


void AbstractSettings::addKmailConfig( const QString& groupName, const QString& key, const QString& value)
{
  KConfigGroup group = mKmailConfig->group(groupName);
  group.writeEntry(key,value);
  group.sync();
}

void AbstractSettings::addKmailConfig( const QString& groupName, const QString& key, bool value)
{
  KConfigGroup group = mKmailConfig->group(groupName);
  group.writeEntry(key,value);
  group.sync();
}

void AbstractSettings::addKmailConfig( const QString& groupName, const QString& key, int value)
{
  KConfigGroup group = mKmailConfig->group(groupName);
  group.writeEntry(key,value);
  group.sync();
}


void AbstractSettings::addKNodeConfig(const QString& groupName, const QString& key, bool value)
{
  //TODO
}

void AbstractSettings::addAkregatorConfig(const QString& groupName, const QString& key, bool value)
{
  //TODO
}


#include "abstractsettings.moc"
