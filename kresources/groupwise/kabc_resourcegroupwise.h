/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KABC_RESOURCEGROUPWISE_H
#define KABC_RESOURCEGROUPWISE_H

#include "soap/groupwiseserver.h"

#include <kabcresourcecached.h>
#include <kdepimmacros.h>

#include <libkdepim/progressmanager.h>

#include <kio/job.h>

class KConfig;

class GroupwiseServer;

namespace KABC {

class GroupwisePrefs;

class KDE_EXPORT ResourceGroupwise : public ResourceCached
{
  friend class ResourceGroupwiseConfig;

  Q_OBJECT

  public:
    ResourceGroupwise( const KConfig * );
    ResourceGroupwise( const KUrl &url,
                       const QString &user, const QString &password,
                       const QStringList &readAddressBooks,
                       const QString &writeAddressBook );
    ~ResourceGroupwise();

    void readConfig( const KConfig * );
    void writeConfig( KConfig * );

    void readAddressBooks();
    void writeAddressBooks();

    void retrieveAddressBooks();

    GroupwisePrefs *prefs() const { return mPrefs; }

    GroupWise::AddressBook::List addressBooks() const { return mAddressBooks; }

    bool doOpen();
    void doClose();

    Ticket *requestSaveTicket();
    void releaseSaveTicket( Ticket* );

    bool load();
    bool asyncLoad();
    bool save( Ticket * );
    bool asyncSave( Ticket * );
    bool updateAddressBooks();

    /**
     * Clears the cached data, in memory and on disk
     */
    void clearCache();
  protected:
    void init();
    void initGroupwise();

  private slots:
    void slotFetchJobResult( KIO::Job * );
    void slotUpdateJobResult( KIO::Job * );
    void slotReadJobData( KIO::Job *, const QByteArray & );
    void slotUpdateJobData( KIO::Job *, const QByteArray & );
    void slotJobPercent( KIO::Job *job, unsigned long percent );

    void cancelLoad();

  private:
    GroupwisePrefs *mPrefs;
    GroupWise::AddressBook::List mAddressBooks;

    GroupwiseServer *mServer;

    KIO::TransferJob *mDownloadJob;
    KPIM::ProgressItem *mProgress;
    QString mJobData;
    bool mUpdateSystemAddressBook;
};

}

#endif
