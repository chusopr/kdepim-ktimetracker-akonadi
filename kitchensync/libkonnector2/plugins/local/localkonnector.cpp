/*
    This file is part of KitchenSync.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "localkonnector.h"

#include "localkonnectorconfig.h"

#include <calendarsyncee.h>
#include <addressbooksyncee.h>
#include <bookmarksyncee.h>
#include <synchistory.h>

#include <libkdepim/kpimprefs.h>

#include <kabc/resourcefile.h>

#include <konnectorinfo.h>
#include <kapabilities.h>

#include <kconfig.h>
#include <kgenericfactory.h>

using namespace KSync;

extern "C"
{
  void *init_liblocalkonnector()
  {
    return new KRES::PluginFactory<LocalKonnector,LocalKonnectorConfig>();
  }
}


LocalKonnector::LocalKonnector( const KConfig *config )
    : Konnector( config ), mConfigWidget( 0 ), 
    mCalendar( KPimPrefs::timezone() )
{
  if ( config ) {
    mCalendarFile = config->readPathEntry( "CalendarFile" );
    mAddressBookFile = config->readPathEntry( "AddressBookFile" );
    mBookmarkFile = config->readPathEntry( "BookmarkFile" );
  }

  mMd5sumCal = generateMD5Sum( mCalendarFile ) + "_localkonnector_cal.log";
  mMd5sumAbk = generateMD5Sum( mAddressBookFile ) + "_localkonnector_abk.log";
  mMd5sumBkm = generateMD5Sum( mBookmarkFile ) + "_localkonnector_bkm.log";

  mAddressBookSyncee = new AddressBookSyncee( &mAddressBook );
  mAddressBookSyncee->setTitle( i18n( "Local" ) );

  mCalendarSyncee = new CalendarSyncee( &mCalendar );
  mCalendarSyncee->setTitle( i18n( "Local" ) );

  mSyncees.append( mCalendarSyncee );
  mSyncees.append( mAddressBookSyncee );
  mSyncees.append( new BookmarkSyncee( &mBookmarkManager ) );

  mAddressBookResourceFile = new KABC::ResourceFile( mAddressBookFile );
  mAddressBook.addResource( mAddressBookResourceFile );
}

LocalKonnector::~LocalKonnector()
{
}

void LocalKonnector::writeConfig( KConfig *config )
{
  Konnector::writeConfig( config );

  config->writePathEntry( "CalendarFile", mCalendarFile );
  config->writeEntry( "AddressBookFile", mAddressBookFile );
  config->writeEntry( "BookmarkFile", mAddressBookFile );
}

KSync::Kapabilities LocalKonnector::capabilities()
{
  KSync::Kapabilities caps;

  caps.setSupportMetaSyncing( false ); // we can meta sync
  caps.setSupportsPushSync( false ); // we can initialize the sync from here
  caps.setNeedsConnection( false ); // we need to have pppd running
  caps.setSupportsListDir( false ); // we will support that once there is API for it...
  caps.setNeedsIPs( false ); // we need the IP
  caps.setNeedsSrcIP( false ); // we do not bind to any address...
  caps.setNeedsDestIP( false ); // we need to know where to connect
  caps.setAutoHandle( false ); // we currently do not support auto handling
  caps.setNeedAuthentication( false ); // HennevL says we do not need that
  caps.setNeedsModelName( false ); // we need a name for our meta path!

  return caps;
}

void LocalKonnector::setCapabilities( const KSync::Kapabilities& )
{
}

bool LocalKonnector::readSyncees()
{
  kdDebug() << "LocalKonnector::readSyncee()" << endl;

  if ( !mCalendarFile.isEmpty() ) {
    kdDebug() << "LocalKonnector::readSyncee(): calendar: " << mCalendarFile
              << endl;
    mCalendar.close();
    if ( mCalendar.load( mCalendarFile ) ) {
      kdDebug() << "Read succeeded." << endl;
      mCalendarSyncee->reset();
      mCalendarSyncee->setIdentifier( mCalendarFile );
      kdDebug() << "IDENTIFIER: " << mCalendarSyncee->identifier() << endl;

      /* apply SyncInformation here this will also create the SyncEntries */
      CalendarSyncHistory cHelper(  mCalendarSyncee, storagePath() + "/"+mMd5sumCal );
      cHelper.load();
    } else {
      kdDebug() << "Read failed." << endl;
      return false;
    }
  }

    if ( !mAddressBookFile.isEmpty() ) {
      kdDebug() << "LocalKonnector::readSyncee(): addressbook: "
                << mAddressBookFile << endl;

      mAddressBookResourceFile->setFileName( mAddressBookFile );
      if ( !mAddressBook.load() ) {
        kdDebug() << "Read failed." << endl;
        return false;
      }

      kdDebug() << "Read succeeded." << endl;

      mAddressBookSyncee->reset();
      mAddressBookSyncee->setIdentifier( mAddressBook.identifier() );

      KABC::AddressBook::Iterator it;
      for ( it = mAddressBook.begin(); it != mAddressBook.end(); ++it ) {
        KSync::AddressBookSyncEntry entry( *it, mAddressBookSyncee );
        mAddressBookSyncee->addEntry( entry.clone() );
      }

      /* let us apply Sync Information */
      AddressBookSyncHistory aHelper( mAddressBookSyncee, storagePath() + "/"+mMd5sumAbk );
      aHelper.load();
    }

  // TODO: Read Bookmarks

  emit synceesRead( this );

  return true;
}

bool LocalKonnector::connectDevice()
{
  return true;
}

bool LocalKonnector::disconnectDevice()
{
  return true;
}

KSync::KonnectorInfo LocalKonnector::info() const
{
  return KonnectorInfo( i18n("Dummy Konnector"),
                        QIconSet(),
                        QString::fromLatin1("LocalKonnector"),  // same as the .desktop file
                        "Dummy Konnector",
                        "agenda", // icon name
                        false );
}

void LocalKonnector::download( const QString& )
{
  error( StdError::downloadNotSupported() );
}

bool LocalKonnector::writeSyncees()
{
  if ( !mCalendarFile.isEmpty() ) {
    purgeRemovedEntries( mCalendarSyncee );

    if ( !mCalendar.save( mCalendarFile ) ) return false;
    CalendarSyncHistory cHelper(  mCalendarSyncee, storagePath() + "/"+mMd5sumCal );
    cHelper.save();
  }

  if ( !mAddressBookFile.isEmpty() ) {
    purgeRemovedEntries( mAddressBookSyncee );
    KABC::Ticket *ticket;
    ticket = mAddressBook.requestSaveTicket( mAddressBookResourceFile );
    if ( !ticket ) {
      kdWarning() << "LocalKonnector::writeSyncees(). Couldn't get ticket for "
                  << "addressbook." << endl;
      return false;
    }
    if ( !mAddressBook.save( ticket ) ) return false;
    AddressBookSyncHistory aHelper( mAddressBookSyncee, storagePath() + "/"+mMd5sumAbk );
    aHelper.save();
  }

  // TODO: Write Bookmarks

  emit synceesWritten( this );

  return true;
}


#include "localkonnector.moc"
