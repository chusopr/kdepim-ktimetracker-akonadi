/*
    Copyright (c) 2008 Bertjan Broeksema <b.broeksema@kdemail.net>
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

#ifndef AKONADI_SINGLEFILERESOURCE_H
#define AKONADI_SINGLEFILERESOURCE_H

#include "singlefileresourcebase.h"

#include <KLocale>

namespace Akonadi
{

/**
 * Base class for single file based resources.
 */
template <typename Settings>
class SingleFileResource : public SingleFileResourceBase
{
  public:
    SingleFileResource( const QString &id ) : SingleFileResourceBase( id )
    {
    }

    /**
     * Indicate that there are pending changes.
     */
    void fileDirty()
    {
      if( Settings::self()->autosave() && !mDirtyTimer.isActive() ) {
        mDirtyTimer.setInterval( Settings::self()->autosaveInterval() * 60 * 1000 );
        mDirtyTimer.start();
      }
    }

    /**
     * Read changes from the backend file.
     */
    void readFile()
    {
      // if we have something loaded already, make sure we write that back in case the settings changed
      if ( !mCurrentUrl.isEmpty() )
        writeFile();

      if ( Settings::self()->path().isEmpty() ) {
        emit status( Broken, i18n( "No file selected." ) );
        return;
      }

      // FIXME: Make this asynchronous by using a KIO file job.
      // See: http://api.kde.org/4.x-api/kdelibs-apidocs/kio/html/namespaceKIO.html
      // NOTE: Test what happens with remotefile -> save, close before save is finished.

      mCurrentUrl = KUrl( Settings::self()->path() );
      if ( !readFromFile( mCurrentUrl.path() ) ) {
        mCurrentUrl = KUrl();
        return;
      }

      emit status( Idle, i18n( "Data loaded from '%1'.", mCurrentUrl.url() ) );
    }

    /**
     * Write changes to the backend file.
     */
    void writeFile()
    {
      if ( Settings::self()->readOnly() ) {
        emit error( i18n( "Trying to write to a read-only file: '%1'", Settings::self()->path() ) );
        return;
      }

      mDirtyTimer.stop();

      // FIXME: Make asynchronous.

      // We don't use the Settings::self()->path() here as that might have changed
      // and in that case it would probably cause data lose.
      if ( mCurrentUrl.isEmpty() ) {
        emit status( Broken, i18n( "No file specified." ) );
        return;
      }

      if ( !writeToFile( mCurrentUrl.path() ) )
        return;

      emit status( Idle, i18n( "Data successfully saved to '%1'.", mCurrentUrl.url() ) );
    }
};

}

#endif
