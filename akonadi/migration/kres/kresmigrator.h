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

#ifndef KRESMIGRATOR_H
#define KRESMIGRATOR_H

#include "kresmigratorbase.h"

#include <kresources/manager.h>
#include <kresources/resource.h>

#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>

template <typename T> class KResMigrator : public KResMigratorBase
{
  public:
    KResMigrator( const QString &type ) :
      KResMigratorBase( type ),
      mConfig( 0 ),
      mManager( 0 ),
      mBridgeManager( 0 )
    {
    }

    virtual ~KResMigrator()
    {
      delete mConfig;
      delete mManager;
    }

    void migrate()
    {
      mManager = new KRES::Manager<T>( mType );
      mManager->readConfig();
      const QString kresCfgFile = KStandardDirs::locateLocal( "config", QString( "kresources/%1/stdrc" ).arg( mType ) );
      mConfig = new KConfig( kresCfgFile );
      mIt = mManager->begin();
      migrateNext();
    }

    void migrateNext()
    {
      while ( mIt != mManager->end() ) {
        KConfigGroup cfg( KGlobal::config(), "Resource " + (*mIt)->identifier() );
        if ( migrationState( *mIt ) == None ) {
          mPendingBridgedResources.removeAll( (*mIt)->identifier() );
          T* res = *mIt;
          ++mIt;
          migrateResource( res );
          return;
        }
        if ( migrationState( *mIt ) == Bridged && !mPendingBridgedResources.contains( (*mIt)->identifier() ) )
          mPendingBridgedResources << (*mIt)->identifier();
        ++mIt;
      }
      if ( mIt == mManager->end() ) {
        delete mConfig;
        mConfig = 0;
        migrateBridged();
      }
    }

    void migrateBridged()
    {
      delete mBridgeManager;
      mBridgeManager = 0;

      if ( mPendingBridgedResources.isEmpty() ) {
        deleteLater();
        return;
      }
      const QString resId = mPendingBridgedResources.takeFirst();
      KConfigGroup resMigrationCfg( KGlobal::config(), "Resource " + resId );
      const QString akoResId = resMigrationCfg.readEntry( "ResourceIdentifier", "" );
      if ( akoResId.isEmpty() ) {
        kWarning() << "No Akonadi agent identifier specified for previously bridged resource!";
        migrateNext();
        return;
      }

      const QString bridgedCfgFile = KStandardDirs::locateLocal( "config", QString( "%1rc" ).arg( akoResId ) );
      kDebug() << bridgedCfgFile;
      mConfig = new KConfig( bridgedCfgFile );

      KRES::Manager<T> *mBridgeManager = new KRES::Manager<T>( mType );
      mBridgeManager->readConfig( mConfig );
      if ( !mBridgeManager->standardResource() ) {
        kWarning() << "Bridged resource" << resId << "has no standard kresource!";
        migrateNext();
        return;
      }

      T *res = mBridgeManager->standardResource();
      migrateResource( res );
    }

    virtual void migrateResource( T *res ) = 0;

    KConfigGroup kresConfig( KRES::Resource* res ) const
    {
      return KConfigGroup( mConfig, "Resource_" + res->identifier() );
    }

    T* resourceForJob( KJob *job )
    {
      if ( mJobResMap.contains( job ) )
        return static_cast<T*>( mJobResMap.value( job ) );
      Q_ASSERT( false );
      return 0;
    }

  private:
    KConfig *mConfig;
    KRES::Manager<T> *mManager, *mBridgeManager;
    typedef typename KRES::Manager<T>::Iterator ResourceIterator;
    ResourceIterator mIt;
};

#endif
