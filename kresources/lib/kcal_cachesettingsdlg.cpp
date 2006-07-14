/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kcal_cachesettingsdlg.h"

#include "kcal_resourcegroupwarebase.h"
#include "kresources_groupwareprefs.h"

#include <kcal/resourcecachedconfig.h>

#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kdialog.h>

#include <QLabel>
#include <QLayout>
#include <QGridLayout>


using namespace KCal;

CacheSettingsDialog::CacheSettingsDialog( QWidget* parent )
    : KDialog( parent )
{
  setCaption( i18n("Resource Cache Settings") );
  setButtons( Close );

  QWidget *mainWidget = new QWidget( this );
  setMainWidget( mainWidget );

  QGridLayout *mainLayout = new QGridLayout( mainWidget );
  mainLayout->setSpacing( KDialog::spacingHint() );

  mReloadConfig = new KCal::ResourceCachedReloadConfig( mainWidget );
  mainLayout->addWidget( mReloadConfig, 1, 2, 3, 1);

  mSaveConfig = new KCal::ResourceCachedSaveConfig( mainWidget );
  mainLayout->addWidget( mSaveConfig, 4, 2, 1, 1);
}

void CacheSettingsDialog::loadSettings( KRES::Resource *resource )
{
  kDebug(7000) << "KCal::CacheSettingsDialog::loadSettings()" << endl;

  ResourceGroupwareBase *res = static_cast<ResourceGroupwareBase *>( resource );
  if ( res ) {
    if ( !res->prefs() ) {
      kError() << "No PREF" << endl;
      return;
    }
    
    mReloadConfig->loadSettings( res );
    mSaveConfig->loadSettings( res );
    
  } else {
    kError(5700) << "CacheSettingsDialog::loadSettings(): "
                     "no ResourceGroupwareBase, cast failed" << endl;
  }
}

void CacheSettingsDialog::saveSettings( KRES::Resource *resource )
{
  ResourceGroupwareBase *res = static_cast<ResourceGroupwareBase*>( resource );
  if ( res ) {
    mReloadConfig->saveSettings( res );
    mSaveConfig->saveSettings( res );
  } else {
    kError(5700) << "CacheSettingsDialog::saveSettings(): "
                     "no ResourceGroupwareBase, cast failed" << endl;
  }
}

#include "kcal_cachesettingsdlg.moc"
