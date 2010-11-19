/*  This file is part of the KDE project
    Copyright (C) 2009,2010 Kevin Krammer <kevin.krammer@gmx.at>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef AKONADI_FILESTORE_STORECOMPACTJOB_H
#define AKONADI_FILESTORE_STORECOMPACTJOB_H

#include "job.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

namespace Akonadi
{

namespace FileStore
{

/**
 */
class AKONADI_FILESTORE_EXPORT StoreCompactJob : public Job
{
  friend class AbstractJobSession;

  Q_OBJECT

  public:
    explicit StoreCompactJob( AbstractJobSession *session = 0 );

    virtual ~StoreCompactJob();

    virtual bool accept( Visitor *visitor );

    Item::List changedItems() const;

    Collection::List changedCollections() const;

  Q_SIGNALS:
    void collectionsChanged( const Akonadi::Collection::List &collections );
    void itemsChanged( const Akonadi::Item::List &items );

  private:
    void handleCollectionsChanged( const Collection::List &collections );
    void handleItemsChanged( const Item::List &items );

  private:
    class Private;
    Private *d;
};

}
}

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
