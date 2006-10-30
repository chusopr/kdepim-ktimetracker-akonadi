/*
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

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

#include "notificationcollector.h"
#include "storage/datastore.h"
#include "storage/entity.h"

#include <QtCore/QDebug>

Akonadi::NotificationItem::NotificationItem(const PimItem &item,
                                            const Location& collection,
                                            const QByteArray & mimeType,
                                            const QByteArray & resource) :
  mType( Item ),
  mItem( item ),
  mCollection( collection ),
  mMimeType( mimeType ),
  mResource( resource )
{
}

Akonadi::NotificationItem::NotificationItem(const QString& collection,
                                            const QByteArray & resource) :
  mType( Collection ),
  mCollectionName( collection ),
  mResource( resource )
{
}

Akonadi::NotificationItem::NotificationItem(const Location & collection,
                                            const QByteArray & resource) :
    mType( Collection ),
    mCollection( collection ),
    mResource( resource )
{
}

bool Akonadi::NotificationItem::isComplete() const
{
  if ( !mCollection.isValid() || mResource.isEmpty() )
    return false;

  if ( mType == Item && mMimeType.isEmpty() )
    return false;

  return true;
}

bool Akonadi::NotificationItem::operator ==(const NotificationItem & item) const
{
  if ( mType != item.mType )
    return false;
  if ( mType == Item )
    return mItem.id() == item.mItem.id();
  if ( mType == Collection )
    return mCollection.id() == item.mCollection.id();
  return false;
}

QString Akonadi::NotificationItem::collectionName() const
{
  if ( !mCollectionName.isEmpty() )
    return mCollectionName;
  return mCollection.name();
}



Akonadi::NotificationCollector::NotificationCollector(DataStore * db) :
  QObject( db ),
  mDb( db )
{
  connect( db, SIGNAL(transactionCommitted()), SLOT(transactionCommitted()) );
  connect( db, SIGNAL(transactionRolledBack()), SLOT(transactionRolledBack()) );
}

Akonadi::NotificationCollector::~NotificationCollector()
{
}

void Akonadi::NotificationCollector::itemAdded( const PimItem &item,
                                                const Location &collection,
                                                const QByteArray & mimeType,
                                                const QByteArray & resource )
{
  NotificationItem ni( item, collection, mimeType, resource );
  if ( mDb->inTransaction() )
    mAddedItems.append( ni );
  else {
    completeItem( ni );
    emit itemAddedNotification( ni.uid(), ni.collectionName(), ni.mimeType(), ni.resource() );
  }
}

void Akonadi::NotificationCollector::itemChanged( const PimItem &item,
                                                  const Location &collection,
                                                  const QByteArray & mimeType,
                                                  const QByteArray & resource )
{
  NotificationItem ni( item, collection, mimeType, resource );
  if ( mDb->inTransaction() )
    mChangedItems.append( ni );
  else {
    completeItem( ni );
    emit itemChangedNotification( ni.uid(), ni.collectionName(), ni.mimeType(), ni.resource() );
  }
}

void Akonadi::NotificationCollector::itemRemoved( const PimItem &item,
                                                  const Location &collection,
                                                  const QByteArray & mimeType,
                                                  const QByteArray & resource )
{
  NotificationItem ni( item, collection, mimeType, resource );
  completeItem( ni );
  if ( mDb->inTransaction() )
    mRemovedItems.append( ni );
  else
    emit itemRemovedNotification( ni.uid(), ni.collectionName(), ni.mimeType(), ni.resource() );
}

void Akonadi::NotificationCollector::collectionAdded( const QString &collection,
                                                      const QByteArray &resource )
{
  NotificationItem ni( collection, resource );
  if ( mDb->inTransaction() )
    mAddedCollections.append( ni );
  else {
    completeItem( ni );
    emit collectionAddedNotification( ni.collectionName(), ni.resource() );
  }
}

void Akonadi::NotificationCollector::collectionChanged( const Location &collection,
                                                        const QByteArray &resource )
{
  NotificationItem ni( collection, resource );
  if ( mDb->inTransaction() )
    mChangedCollections.append( ni );
  else {
    completeItem( ni );
    emit collectionChangedNotification( ni.collectionName(), ni.resource() );
  }
}

void Akonadi::NotificationCollector::collectionRemoved( const Location &collection,
                                                        const QByteArray &resource )
{
  NotificationItem ni( collection, resource );
  completeItem( ni );
  if ( mDb->inTransaction() )
    mRemovedCollections.append( ni );
  else
    emit collectionRemovedNotification( ni.collectionName(), ni.resource() );
}

void Akonadi::NotificationCollector::completeItem(NotificationItem & item)
{
  if ( item.isComplete() )
    return;

  if ( item.type() == NotificationItem::Collection ) {
    if ( !item.collection().isValid() && !item.collectionName().isEmpty() )
      item.setCollection( Location::retrieveByName( item.collectionName() ) );
    item.setResource( item.collection().resource().name().toLatin1() );
  }

  if ( item.type() == NotificationItem::Item ) {
    PimItem pi = item.pimItem();
    if ( !pi.isValid() )
      return;
    Location loc;
    if ( !item.collection().isValid() ) {
      loc = pi.location();
      item.setCollection( loc );
    }
    if ( item.mimeType().isEmpty() ) {
      item.setMimeType( pi.mimeType().name().toLatin1() );
    }
    if ( item.resource().isEmpty() ) {
      if ( !loc.isValid() )
        loc = pi.location();
      item.setResource( loc.resource().name().toLatin1() );
    }
  }
}

void Akonadi::NotificationCollector::transactionCommitted()
{
  // TODO: remove duplicates from the lists

  foreach ( NotificationItem ni, mAddedCollections ) {
    completeItem( ni );
    emit collectionAddedNotification( ni.collectionName(), ni.resource() );
    // no change notifications for new collections
    mChangedCollections.removeAll( ni );
  }

  foreach ( NotificationItem ni, mRemovedCollections ) {
    emit collectionRemovedNotification( ni.collectionName(), ni.resource() );
    // no change notifications for removed collections
    mChangedCollections.removeAll( ni );
  }

  foreach ( NotificationItem ni, mChangedCollections ) {
    completeItem( ni );
    emit collectionChangedNotification( ni.collectionName(), ni.resource() );
  }


  foreach ( NotificationItem ni, mAddedItems ) {
    completeItem( ni );
    emit itemAddedNotification( ni.uid(), ni.collectionName(), ni.mimeType(), ni.resource() );
    // no change notifications for new items
    mChangedItems.removeAll( ni );
  }

  foreach ( NotificationItem ni, mRemovedItems ) {
    emit itemRemovedNotification( ni.uid(), ni.collectionName(), ni.mimeType(), ni.resource() );
    // no change notifications for removed items
    mChangedItems.removeAll( ni );
  }

  foreach ( NotificationItem ni, mChangedItems ) {
    completeItem( ni );
    emit itemChangedNotification( ni.uid(), ni.collectionName(), ni.mimeType(), ni.resource() );
  }

  clear();
}

void Akonadi::NotificationCollector::transactionRolledBack()
{
  clear();
}

void Akonadi::NotificationCollector::clear()
{
  mAddedItems.clear();
  mChangedItems.clear();
  mRemovedItems.clear();
  mAddedCollections.clear();
  mChangedCollections.clear();
  mRemovedCollections.clear();
}

#include "notificationcollector.moc"
