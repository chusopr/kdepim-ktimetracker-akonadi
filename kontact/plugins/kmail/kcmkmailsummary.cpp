/*
    This file is part of Kontact.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qcheckbox.h>
#include <qlayout.h>

#include <dcopref.h>

#include <kaboutdata.h>
#include <kaccelmanager.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klistview.h>
#include <klocale.h>

#include "kcmkmailsummary.h"

extern "C"
{
  KCModule *create_kmailsummary( QWidget *parent, const char * )
  {
    return new KCMKMailSummary( parent, "kcmkmailsummary" );
  }
}

KCMKMailSummary::KCMKMailSummary( QWidget *parent, const char *name )
  : KCModule( parent, name )
{
  initGUI();

  connect( mFolderView, SIGNAL( clicked( QListViewItem* ) ), SLOT( modified() ) );
  connect( mFullPath, SIGNAL( toggled( bool ) ), SLOT( modified() ) );

  KAcceleratorManager::manage( this );

  load();
}

void KCMKMailSummary::modified()
{
  emit changed( true );
}

void KCMKMailSummary::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mFolderView = new KListView( this );
  mFolderView->setRootIsDecorated( true );
  mFolderView->setFullWidth( true );

  mFolderView->addColumn( i18n( "Summary" ) );

  mFullPath = new QCheckBox( i18n( "Show full path for folders" ), this );

  layout->addWidget( mFolderView );
  layout->addWidget( mFullPath );
}

void KCMKMailSummary::initFolders()
{
  DCOPRef kmail( "kmail", "KMailIface" );

  QStringList folderList;
  kmail.call( "folderList" ).get( folderList );

  mFolderView->clear();
  mFolderMap.clear();

  QStringList::Iterator it;
  for ( it = folderList.begin(); it != folderList.end(); ++it ) {
    if ( (*it).contains( '/' ) == 1 ) {
      if ( mFolderMap.find( *it ) == mFolderMap.end() )
        mFolderMap.insert( *it,
                           new QCheckListItem( mFolderView, (*it).mid( 1 ), QCheckListItem::CheckBox ) );
    } else {
      int pos = (*it).findRev( '/' );
      QString parentFolder = (*it).left( pos );
      QString name = (*it).mid( pos + 1 );
      mFolderMap.insert( *it,
                         new QCheckListItem( mFolderMap[ parentFolder ], name, QCheckListItem::CheckBox ) );
    }
  }
}

void KCMKMailSummary::loadFolders()
{
  KConfig config( "kcmkmailsummaryrc" );
  config.setGroup( "General" );

  QStringList folders;
  if ( !config.hasKey( "ActiveFolders" ) )
    folders << "/inbox";
  else
    folders = config.readListEntry( "ActiveFolders" );

  QMap<QString, QCheckListItem*>::Iterator it;
  for ( it = mFolderMap.begin(); it != mFolderMap.end(); ++it ) {
    if ( folders.contains( it.key() ) ) {
      it.data()->setOn( true );
      mFolderView->ensureItemVisible( it.data() );
    } else
      it.data()->setOn( false );
  }

  mFullPath->setChecked( config.readBoolEntry( "ShowFullPath", false ) );
}

void KCMKMailSummary::storeFolders()
{
  KConfig config( "kcmkmailsummaryrc" );
  config.setGroup( "General" );

  QStringList folders;

  QMap<QString, QCheckListItem*>::Iterator it;
  for ( it = mFolderMap.begin(); it != mFolderMap.end(); ++it )
    if ( it.data()->isOn() )
      folders.append( it.key() );

  config.writeEntry( "ActiveFolders", folders );
  config.writeEntry( "ShowFullPath", mFullPath->isChecked() );

  config.sync();
}

void KCMKMailSummary::load()
{
  initFolders();
  loadFolders();

  emit changed( false );
}

void KCMKMailSummary::save()
{
  storeFolders();

  emit changed( false );
}

void KCMKMailSummary::defaults()
{
}

const KAboutData* KCMKMailSummary::aboutData() const
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkmailsummary" ),
                                      I18N_NOOP( "Mail Summary Configuration Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c) 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  return about;
}

#include "kcmkmailsummary.moc"
