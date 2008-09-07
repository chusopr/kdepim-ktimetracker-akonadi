/*
    This file is part of libkcal.
    Copyright (c) 2008 Kevin Krammer <kevin.krammer@gmx.at>

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

#include "resourceakonadi.h"
#include "resourceakonadiconfig.h"

#include "kcal/calendarlocal.h"
#include "kabc/locknull.h"

#include <akonadi/collection.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/control.h>
#include <akonadi/monitor.h>
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemsync.h>
#include <akonadi/kcal/kcalmimetypevisitor.h>

#include <kconfiggroup.h>
#include <kdebug.h>
#include <kmimetype.h>

#include <QHash>
#include <QTimer>

#include <boost/shared_ptr.hpp>

using namespace Akonadi;
using namespace KCal;

using namespace Akonadi;
using namespace KCal;

typedef boost::shared_ptr<Incidence> IncidencePtr;

typedef QMap<Item::Id, Item> ItemMap;
typedef QHash<QString, Item::Id> IdHash;

static bool isCalendarCollection( const Akonadi::Collection &collection )
{
  const QStringList collectionMimeTypes = collection.contentMimeTypes();
  foreach ( const QString &collectionMimeType, collectionMimeTypes ) {
    KMimeType::Ptr mimeType = KMimeType::mimeType( collectionMimeType, KMimeType::ResolveAliases );
    if ( mimeType.isNull() )
      continue;

    // check if the collection content MIME type is or inherits from the
    // wanted one, e.g.
    // if the collection offers application/x-vnd.akonadi.calendar.todo
    // this will be true, because it is a subclass of text/calendar
    if ( mimeType->is( QLatin1String( "text/calendar" ) ) )
      return true;
  }

  return false;
}

class SubResource
{
  public:
    SubResource( const Collection &collection )
        : mCollection( collection ), mLabel( collection.name() ),
          mActive(true)
    {
    }

    // TODO: need to use KConfigGroup instead
    // or probably a collection attribute?
    void setActive( bool active )
    {
      mActive = active;
    }

    bool isActive() const { return mActive; }

  public:
    Collection mCollection;
    QString mLabel;

    bool mActive;
};

typedef QHash<QString, SubResource*> SubResourceMap;
typedef QMap<QString, QString>  UidResourceMap;
typedef QMap<Item::Id, QString> ItemIdResourceMap;

class ResourceAkonadi::Private : public KCal::Calendar::CalendarObserver
{
  public:
    Private( ResourceAkonadi *parent )
      : mParent( parent ), mMonitor( 0 ), mCalendar( QLatin1String( "UTC" ) ),
        mLock( true ), mInternalCalendarModification( false ),
        mMimeVisitor( new KCalMimeTypeVisitor() ),
        mCollectionModel( 0 ), mCollectionFilterModel( 0 )
    {
      mCalendar.registerObserver( this );
    }

    ~Private()
    {
      delete mMimeVisitor;
    }

  public:
    ResourceAkonadi *mParent;

    Monitor *mMonitor;

    CalendarLocal mCalendar;

    KABC::LockNull mLock;

    Collection mStoreCollection;

    ItemMap    mItems;
    IdHash     mIdMapping;

    bool mInternalCalendarModification;

    QTimer mAutoSaveOnDeleteTimer;

    KCalMimeTypeVisitor *mMimeVisitor;

    CollectionModel *mCollectionModel;
    CollectionFilterProxyModel *mCollectionFilterModel;

    SubResourceMap mSubResources;
    QSet<QString> mSubResourceIds;

    UidResourceMap    mUidToResourceMap;
    ItemIdResourceMap mItemIdToResourceMap;

    QHash<Akonadi::Job*, QString> mJobToResourceMap;

    enum ChangeType
    {
      Added,
      Changed,
      Removed
    };

    typedef QMap<QString, ChangeType> ChangeMap;
    ChangeMap mChanges;

  public:
    void subResourceLoadResult( KJob *job );

    void itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection );
    void itemChanged( const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers );
    void itemRemoved( const Akonadi::Item &item );

    void collectionRowsInserted( const QModelIndex &parent, int start, int end );
    void collectionRowsRemoved( const QModelIndex &parent, int start, int end );
    void collectionDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight );

    void addCollectionsRecursively( const QModelIndex &parent, int start, int end );
    bool removeCollectionsRecursively( const QModelIndex &parent, int start, int end );

    void delayedAutoSaveOnDelete();

    bool prepareSaving();
    KJob *createSaveSequence();

    // from the CalendarObserver interface
    virtual void calendarIncidenceAdded( Incidence *incidence );
    virtual void calendarIncidenceModified( Incidence *incidence );
    virtual void calendarIncidenceDeleted( Incidence *incidence );

    bool reloadSubResource( SubResource *subResource );
};

ResourceAkonadi::ResourceAkonadi()
  : ResourceCalendar(), d( new Private( this ) )
{
  init();
}

ResourceAkonadi::ResourceAkonadi( const KConfigGroup &group )
  : ResourceCalendar( group ), d( new Private( this ) )
{
  KUrl url = group.readEntry( QLatin1String( "CollectionUrl" ), KUrl() );

  if ( !url.isValid() ) {
    // TODO error handling
  } else {
    d->mStoreCollection = Collection::fromUrl( url );
  }

  init();
}

ResourceAkonadi::~ResourceAkonadi()
{
  delete d;
}

void ResourceAkonadi::writeConfig( KConfigGroup &group )
{
  ResourceCalendar::writeConfig( group );

  group.writeEntry( QLatin1String( "CollectionUrl" ), d->mStoreCollection.url() );
}

void ResourceAkonadi::setStoreCollection( const Collection &collection )
{
  if ( collection == d->mStoreCollection )
    return;

  d->mStoreCollection = collection;
}

Collection ResourceAkonadi::storeCollection() const
{
  return d->mStoreCollection;
}

KABC::Lock *ResourceAkonadi::lock()
{
  return &(d->mLock);
}

bool ResourceAkonadi::addEvent( Event *event )
{
  return d->mCalendar.addEvent( event );
}

bool ResourceAkonadi::deleteEvent( Event *event )
{
  return d->mCalendar.deleteEvent( event );
}

void ResourceAkonadi::deleteAllEvents()
{
  d->mCalendar.deleteAllEvents();
}

Event *ResourceAkonadi::event( const QString &uid )
{
  return d->mCalendar.event( uid );
}

Event::List ResourceAkonadi::rawEvents( EventSortField sortField,
                                        SortDirection sortDirection )
{
  return d->mCalendar.rawEvents( sortField, sortDirection );
}

Event::List ResourceAkonadi::rawEventsForDate( const QDate &date,
                                               const KDateTime::Spec &timespec,
                                               EventSortField sortField,
                                               SortDirection sortDirection )
{
  return d->mCalendar.rawEventsForDate( date, timespec, sortField, sortDirection );
}

Event::List ResourceAkonadi::rawEventsForDate( const KDateTime &date )
{
  return d->mCalendar.rawEventsForDate( date );
}

Event::List ResourceAkonadi::rawEvents( const QDate &start, const QDate &end,
                                        const KDateTime::Spec &timespec,
                                        bool inclusive )
{
  return d->mCalendar.rawEvents( start, end, timespec, inclusive );
}

bool ResourceAkonadi::addTodo( Todo *todo )
{
  return d->mCalendar.addTodo( todo );
}

bool ResourceAkonadi::deleteTodo( Todo *todo )
{
  return d->mCalendar.deleteTodo( todo );
}

void ResourceAkonadi::deleteAllTodos()
{
  d->mCalendar.deleteAllTodos();
}

Todo *ResourceAkonadi::todo( const QString &uid )
{
  return d->mCalendar.todo( uid );
}

Todo::List ResourceAkonadi::rawTodos( TodoSortField sortField,
                                      SortDirection sortDirection )
{
  return d->mCalendar.rawTodos( sortField, sortDirection );
}

Todo::List ResourceAkonadi::rawTodosForDate( const QDate &date )
{
  return d->mCalendar.rawTodosForDate( date );
}

bool ResourceAkonadi::addJournal( Journal *journal )
{
  return d->mCalendar.addJournal( journal );
}

bool ResourceAkonadi::deleteJournal( Journal *journal )
{
  return d->mCalendar.deleteJournal( journal );
}

void ResourceAkonadi::deleteAllJournals()
{
  d->mCalendar.deleteAllJournals();
}

Journal *ResourceAkonadi::journal( const QString &uid )
{
  return d->mCalendar.journal( uid );
}

Journal::List ResourceAkonadi::rawJournals( JournalSortField sortField,
                                            SortDirection sortDirection )
{
  return d->mCalendar.rawJournals( sortField, sortDirection );
}

Journal::List ResourceAkonadi::rawJournalsForDate( const QDate &date )
{
  return d->mCalendar.rawJournalsForDate( date );
}

Alarm::List ResourceAkonadi::alarms( const KDateTime &from, const KDateTime &to )
{
  return d->mCalendar.alarms( from, to );
}

Alarm::List ResourceAkonadi::alarmsTo( const KDateTime &to )
{
  return d->mCalendar.alarmsTo( to );
}

void ResourceAkonadi::setTimeSpec( const KDateTime::Spec &timeSpec )
{
  d->mCalendar.setTimeSpec( timeSpec );
}

KDateTime::Spec ResourceAkonadi::timeSpec() const
{
  return d->mCalendar.timeSpec();
}

void ResourceAkonadi::setTimeZoneId( const QString &timeZoneId )
{
  d->mCalendar.setTimeZoneId( timeZoneId );
}

QString ResourceAkonadi::timeZoneId() const
{
  return d->mCalendar.timeZoneId();
}

void ResourceAkonadi::shiftTimes( const KDateTime::Spec &oldSpec,
                                  const KDateTime::Spec &newSpec )
{
  d->mCalendar.shiftTimes( oldSpec, newSpec );
}

void ResourceAkonadi::setSubresourceActive( const QString &subResource, bool active )
{
  kDebug(5700) << "subResource" << subResource << ", active" << active;

  SubResource *resource = d->mSubResources[ subResource ];
  if ( resource != 0 ) {
    resource->setActive( active );

    if ( !active ) {
      bool changed = false;

      bool internalModification = d->mInternalCalendarModification;
      d->mInternalCalendarModification = true;

      UidResourceMap::iterator it = d->mUidToResourceMap.begin();
      while ( it != d->mUidToResourceMap.end() ) {
        if ( it.value() == subResource ) {
          changed = true;

          Incidence *incidence = d->mCalendar.incidence( it.key() );
          Q_ASSERT( incidence != 0 );
          Q_ASSERT( d->mCalendar.deleteIncidence( incidence ) );
          it = d->mUidToResourceMap.erase( it );
        }
        else
          ++it;
      }

      d->mInternalCalendarModification = internalModification;

      if ( changed )
        emit resourceChanged( this );

    } else {
      if ( d->reloadSubResource( resource ) )
        emit resourceChanged( this );
    }
  }
}

bool ResourceAkonadi::subresourceActive( const QString &resource ) const
{
  SubResource *subResource = d->mSubResources[ resource ];
  if ( subResource == 0 )
    return false;

  return subResource->isActive();
}

QString ResourceAkonadi::subresourceIdentifier( Incidence *incidence )
{
  return d->mUidToResourceMap[ incidence->uid() ];
}

QStringList ResourceAkonadi::subresources() const
{
  return d->mSubResourceIds.toList();
}

bool ResourceAkonadi::doLoad( bool syncCache )
{
  kDebug(5800) << "syncCache=" << syncCache;

  // TODO since Akonadi resources can set a MIME type depending on incidence type
  // it should be enough to just "list" the items and fetch the payloads
  // when the class' getter methods are called or do a full fetch in the
  // unfortunate case where all items only have text/calendar as their MIME type

  // save the list of collections we potentially already have
  Collection::List collections;
  SubResourceMap::const_iterator it    = d->mSubResources.begin();
  SubResourceMap::const_iterator endIt = d->mSubResources.end();
  for ( ; it != endIt; ++it ) {
    collections << it.value()->mCollection;
  }

  // clear local caches
  d->mInternalCalendarModification = true;
  d->mCalendar.close();
  d->mInternalCalendarModification = false;

  d->mItems.clear();
  d->mIdMapping.clear();
  d->mUidToResourceMap.clear();
  d->mItemIdToResourceMap.clear();
  d->mJobToResourceMap.clear();
  d->mChanges.clear();

  // if we do not have any collection yet, fetch them explicitly
  if ( collections.isEmpty() ) {
    CollectionFetchJob *colJob =
      new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive );
    if ( !colJob->exec() ) {
      loadError( colJob->errorString() );
      return false;
    }

    // TODO: should probably add the data to the model right here
    collections = colJob->collections();
  }

  bool result = true;
  foreach ( const Collection &collection, collections ) {
     if ( !isCalendarCollection( collection ) )
       continue;

    const QString collectionUrl = collection.url().url();
    SubResource *subResource = d->mSubResources[ collectionUrl ];
    if ( subResource == 0 ) {
      subResource = new SubResource( collection );
      d->mSubResources.insert( collectionUrl, subResource );

      emit signalSubresourceAdded( this, QLatin1String( "calendar" ), collectionUrl, subResource->mLabel );
    }

    // TODO should check whether the model signal handling has already fetched the
    // collection's items
    if ( !d->reloadSubResource( subResource ) )
      result = false;
  }

  return result;
}

bool ResourceAkonadi::doSave( bool syncCache )
{
  kDebug(5800) << "syncCache=" << syncCache;

  if ( !d->prepareSaving() )
    return false;

  KJob *job = d->createSaveSequence();
  if ( job == 0 )
    return false;

  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( saveResult( KJob* ) ) );

  job->start();
  d->mChanges.clear();

  return true;
}

bool ResourceAkonadi::doSave( bool syncCache, Incidence *incidence )
{
  kDebug(5800) << "syncCache=" << syncCache
               << ", incidence" << incidence->uid();

  UidResourceMap::const_iterator findIt = d->mUidToResourceMap.find( incidence->uid() );
  while ( findIt == d->mUidToResourceMap.end() ) {
    if ( !d->mStoreCollection.isValid() ||
          d->mSubResources[ d->mStoreCollection.url().url() ] == 0 ) {

      ResourceAkonadiConfigDialog dialog( this );
      if ( dialog.exec() != QDialog::Accepted )
        return false;
    } else
      d->mUidToResourceMap.insert( incidence->uid(), d->mStoreCollection.url().url() );

    findIt = d->mUidToResourceMap.find( incidence->uid() );
  }

  KJob *job = 0;

  ItemMap::const_iterator itemIt = d->mItems.end();

  IdHash::const_iterator idIt = d->mIdMapping.find( incidence->uid() );
  if ( idIt != d->mIdMapping.end() ) {
    itemIt = d->mItems.find( idIt.value() );
  }

  if ( itemIt == d->mItems.end() ) {
    kDebug(5800) << "No item yet, using ItemCreateJob";

    Item item( d->mMimeVisitor->mimeType( incidence ) );
    item.setPayload<IncidencePtr>( IncidencePtr( incidence->clone() ) );

    job = new ItemCreateJob( item, d->mStoreCollection, this );
  } else {
    kDebug(5800) << "Item already exists, using ItemModifyJob";
    Item item = itemIt.value();
    item.setPayload<IncidencePtr>( IncidencePtr( incidence->clone() ) );

    ItemModifyJob *modifyJob = new ItemModifyJob( item, this );

    job = modifyJob;
  }

  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( saveResult( KJob* ) ) );

  job->start();
  d->mChanges.remove( incidence->uid() );

  return true;
}

bool ResourceAkonadi::doOpen()
{
  // if already connected, ignore
  if ( d->mCollectionFilterModel != 0 )
    return true;

  // TODO: probably check if Akonadi is running

  d->mCollectionModel = new CollectionModel( this );

  d->mCollectionFilterModel = new CollectionFilterProxyModel( this );
  d->mCollectionFilterModel->addMimeTypeFilter( QLatin1String( "text/calendar" ) );

  connect( d->mCollectionFilterModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
           this, SLOT( collectionRowsInserted( const QModelIndex&, int, int ) ) );
  connect( d->mCollectionFilterModel, SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ),
           this, SLOT( collectionRowsRemoved( const QModelIndex&, int, int ) ) );
  connect( d->mCollectionFilterModel, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( collectionDataChanged( const QModelIndex&, const QModelIndex& ) ) );

  d->mCollectionFilterModel->setSourceModel( d->mCollectionModel );

  d->mMonitor = new Monitor( this );

  d->mMonitor->setMimeTypeMonitored( QLatin1String( "text/calendar" ) );
  d->mMonitor->itemFetchScope().fetchFullPayload();

  connect( d->mMonitor,
           SIGNAL( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ),
           this,
           SLOT( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ) );

  connect( d->mMonitor,
           SIGNAL( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ),
           this,
           SLOT( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ) );

  connect( d->mMonitor,
           SIGNAL( itemRemoved( const Akonadi::Item& ) ),
           this,
           SLOT( itemRemoved( const Akonadi::Item& ) ) );

  return true;
}

void ResourceAkonadi::doClose()
{
  delete d->mMonitor;
  d->mMonitor = 0;
  delete d->mCollectionFilterModel;
  d->mCollectionFilterModel = 0;
  delete d->mCollectionModel;
  d->mCollectionModel = 0;

  // clear local caches
  d->mInternalCalendarModification = true;
  d->mCalendar.close();
  d->mInternalCalendarModification = false;

  d->mItems.clear();
  d->mIdMapping.clear();
  d->mUidToResourceMap.clear();
  d->mItemIdToResourceMap.clear();
  d->mJobToResourceMap.clear();
  d->mChanges.clear();
}

void ResourceAkonadi::loadResult( KJob *job )
{
  kDebug(5800) << job->errorString();

  if ( job->error() != 0 ) {
    emit resourceLoadError( this, job->errorString() );
    return;
  }

  ItemFetchJob *fetchJob = dynamic_cast<ItemFetchJob*>( job );
  Q_ASSERT( fetchJob != 0 );

  Item::List items = fetchJob->items();

  kDebug(5800) << "Item fetch produced" << items.count() << "items";

  bool internalModification = d->mInternalCalendarModification;
  d->mInternalCalendarModification = true;

  foreach ( const Item& item, items ) {
    if ( item.hasPayload<IncidencePtr>() ) {
      IncidencePtr incidence = item.payload<IncidencePtr>();

      const Item::Id id = item.id();
      d->mIdMapping.insert( incidence->uid(), id );

      d->mCalendar.addIncidence( incidence->clone() );
      d->mItems.insert( id, item );

      // TODO add resource mappings
    }
  }

  d->mInternalCalendarModification = internalModification;

  emit resourceLoaded( this );
}

void ResourceAkonadi::saveResult( KJob *job )
{
  kDebug(5800) << job->errorString();

  if ( job->error() != 0 ) {
    emit resourceSaveError( this, job->errorString() );
  } else {
    emit resourceSaved( this );
  }
}

void ResourceAkonadi::init()
{
  // TODO: might be better to do this already in the resource factory
  Akonadi::Control::start();

  d->mMonitor = new Monitor( this );

  // deactivate reacting to changes, will be enabled in doOpen()
  d->mMonitor->blockSignals( true );

  d->mMonitor->setMimeTypeMonitored( QLatin1String( "text/calendar" ) );
  d->mMonitor->itemFetchScope().fetchFullPayload();

  connect( d->mMonitor,
           SIGNAL( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ),
           this,
           SLOT( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ) );

  connect( d->mMonitor,
           SIGNAL( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ),
           this,
           SLOT( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ) );

  connect( d->mMonitor,
           SIGNAL( itemRemoved( const Akonadi::Item&) ),
           this,
           SLOT( itemRemoved( const Akonadi::Item& ) ) );

  connect( &d->mAutoSaveOnDeleteTimer, SIGNAL( timeout() ),
           this, SLOT( delayedAutoSaveOnDelete() ) );
}

void ResourceAkonadi::Private::subResourceLoadResult( KJob *job )
{
  if ( job->error() != 0 ) {
    emit mParent->loadError( job->errorString() );
    return;
  }

  ItemFetchJob *fetchJob = dynamic_cast<ItemFetchJob*>( job );
  Q_ASSERT( fetchJob != 0 );

  const QString collectionUrl = mJobToResourceMap.take( fetchJob );
  Q_ASSERT( !collectionUrl.isEmpty() );

  Item::List items = fetchJob->items();

  kDebug(5700) << "Item fetch for sub resource " << collectionUrl
               << "produced" << items.count() << "items";

  bool internalModification = mInternalCalendarModification;
  mInternalCalendarModification = true;

  foreach ( const Item& item, items ) {
    if ( item.hasPayload<IncidencePtr>() ) {
      IncidencePtr incidencePtr = item.payload<IncidencePtr>();

      const Item::Id id = item.id();
      mIdMapping.insert( incidencePtr->uid(), id );

      Incidence *incidence = incidencePtr->clone();
      Q_ASSERT( mCalendar.addIncidence( incidence ) );
      mUidToResourceMap.insert( incidence->uid(), collectionUrl );
      mItemIdToResourceMap.insert( id, collectionUrl );
    }

    // always update the item
    mItems.insert( item.id(), item );
  }

  mInternalCalendarModification = internalModification;

  emit mParent->resourceChanged( mParent );

  SubResource *subResource = mSubResources[ collectionUrl ];
  Q_ASSERT( subResource != 0 );
  emit mParent->signalSubresourceAdded( mParent, QLatin1String( "calendar" ), collectionUrl, subResource->mLabel );
}

void ResourceAkonadi::Private::itemAdded( const Akonadi::Item &item,
                                          const Akonadi::Collection &collection )
{
  kDebug(5800);

  const QString collectionUrl = collection.url().url();
  SubResource *subResource = mSubResources[ collectionUrl ];
  if ( subResource == 0 )
    return;

  const Item::Id id = item.id();
  mItems.insert( id, item );
  mItemIdToResourceMap.insert( id, collectionUrl );

  if ( !item.hasPayload<IncidencePtr>() ) {
    kError(5800) << "Item does not have IncidencePtr payload";
    return;
  }

  IncidencePtr incidence = item.payload<IncidencePtr>();

  kDebug(5800) << "Incidence" << incidence->uid();

  mIdMapping.insert( incidence->uid(), id );
  mUidToResourceMap.insert( incidence->uid(), collectionUrl );

  mChanges.remove( incidence->uid() );

  // might be the result of our own saving
  if ( mCalendar.incidence( incidence->uid() ) == 0 ) {
    bool internalModification = mInternalCalendarModification;
    mInternalCalendarModification = true;

    mCalendar.addIncidence( incidence->clone() );

    mInternalCalendarModification = internalModification;

    emit mParent->resourceChanged( mParent );
  }
}

void ResourceAkonadi::Private::itemChanged( const Akonadi::Item &item,
                                            const QSet<QByteArray> &partIdentifiers )
{
  kDebug(5800) << partIdentifiers;

  ItemMap::iterator itemIt = mItems.find( item.id() );
  if ( itemIt == mItems.end() || !( itemIt.value() == item ) ) {
    kWarning(5800) << "No matching local item for item: id="
                   << item.id() << ", remoteId="
                   << item.remoteId();
    return;
  }

  itemIt.value() = item;

  if ( !partIdentifiers.contains( Akonadi::Item::FullPayload ) ) {
    kDebug(5800) << "No update to the item body";
    // FIXME find out why payload updates do not contain PartBody?
    //return;
  }

  if ( !item.hasPayload<IncidencePtr>() ) {
    kError(5800) << "Item does not have IncidencePtr payload";
    return;
  }

  IncidencePtr incidence = item.payload<IncidencePtr>();

  kDebug(5800) << "Incidence" << incidence->uid();

  Incidence *cachedIncidence = mCalendar.incidence( incidence->uid() );
  if ( cachedIncidence == 0 ) {
    kWarning(5800) << "Incidence" << incidence->uid()
                   << "changed but no longer in local list";
    return;
  }

  bool internalModification = mInternalCalendarModification;
  mInternalCalendarModification = true;

  mCalendar.deleteIncidence( cachedIncidence );
  mCalendar.addIncidence( incidence.get()->clone() );

  mInternalCalendarModification = internalModification;

  mChanges.remove( incidence->uid() );

  emit mParent->resourceChanged( mParent );
}

void ResourceAkonadi::Private::itemRemoved( const Akonadi::Item &reference )
{
  kDebug(5800);

  const Item::Id id = reference.id();

  ItemMap::iterator itemIt = mItems.find( id );
  if ( itemIt == mItems.end() )
    return;

  const Item item = itemIt.value();
  if ( item != reference )
    return;

  QString uid;
  if ( item.hasPayload<IncidencePtr>() ) {
    uid = item.payload<IncidencePtr>()->uid();
  } else {
    // since we always fetch the payload this should not happen
    // but we really do not want stale entries
    kWarning(5700) << "No IncidencePtr in local item: id=" << id
                   << ", remoteId=" << item.remoteId();

    IdHash::const_iterator idIt    = mIdMapping.begin();
    IdHash::const_iterator idEndIt = mIdMapping.end();
    for ( ; idIt != idEndIt; ++idIt ) {
      if ( idIt.value() == id ) {
        uid = idIt.key();
        break;
      }
    }

    // if there is no mapping we already removed it locally
    if ( uid.isEmpty() )
      return;
  }

  mItems.erase( itemIt );
  mUidToResourceMap.remove( uid );
  mItemIdToResourceMap.remove( id );
  mChanges.remove( uid );

  Incidence *cachedIncidence = mCalendar.incidence( uid );

  // if it does not exist as an addressee we already removed it locally
  if ( cachedIncidence == 0 )
    return;

  kDebug(5800) << "Incidence" << cachedIncidence->uid();

  bool internalModification = mInternalCalendarModification;
  mInternalCalendarModification = true;

  mCalendar.deleteIncidence( cachedIncidence );

  mInternalCalendarModification = internalModification;

  emit mParent->resourceChanged( mParent );
}

void ResourceAkonadi::Private::collectionRowsInserted( const QModelIndex &parent, int start, int end )
{
  kDebug(5700) << "start" << start << ", end" << end;

  addCollectionsRecursively( parent, start, end );
}

void ResourceAkonadi::Private::collectionRowsRemoved( const QModelIndex &parent, int start, int end )
{
  kDebug(5700) << "start" << start << ", end" << end;

  if ( removeCollectionsRecursively( parent, start, end ) )
    emit mParent->resourceChanged( mParent );
}

void ResourceAkonadi::Private::collectionDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
  const int start = topLeft.row();
  const int end   = bottomRight.row();
  kDebug(5700) << "start=" << start << ", end=" << end;

  bool changed = false;
  for ( int row = start; row <= end; ++row ) {
    QModelIndex index = mCollectionFilterModel->index( row, 0, topLeft.parent() );
    if ( index.isValid() ) {
      QVariant data = mCollectionFilterModel->data( index, CollectionModel::CollectionRole );
      if ( data.isValid() ) {
        Collection collection = data.value<Collection>();
        if ( collection.isValid() ) {
          const QString collectionUrl = collection.url().url();
          SubResource* subResource = mSubResources[ collectionUrl ];
          if ( subResource != 0 ) {
            if ( subResource->mLabel != collection.name() ) {
              kDebug(5700) << "Renaming subResource" << collectionUrl
                           << "from" << subResource->mLabel
                           << "to"   << collection.name();
              subResource->mLabel = collection.name();
              changed = true;
              // TODO probably need to add this to ResourceCalendar as well
              //emit mParent->signalSubresourceChanged( mParent, QLatin1String( "calendar" ), collectionUrl );
            }
          }
        }
      }
    }
  }

  if ( changed )
    emit mParent->resourceChanged( mParent );
}

void ResourceAkonadi::Private::addCollectionsRecursively( const QModelIndex &parent, int start, int end )
{
  kDebug(5700) << "start=" << start << ", end=" << end;
  for ( int row = start; row <= end; ++row ) {
    QModelIndex index = mCollectionFilterModel->index( row, 0, parent );
    if ( index.isValid() ) {
      QVariant data = mCollectionFilterModel->data( index, CollectionModel::CollectionRole );
      if ( data.isValid() ) {
        // TODO: ignore virtual collections, since they break Uid to Resource mapping
        // and the application can't handle them anyway
        Collection collection = data.value<Collection>();
        if ( collection.isValid() ) {
          const QString collectionUrl = collection.url().url();

          SubResource *subResource = mSubResources[ collectionUrl ];
          if ( subResource == 0 ) {
            subResource = new SubResource( collection );
            mSubResources.insert( collectionUrl, subResource );
            mSubResourceIds.insert( collectionUrl );
            kDebug(5700) << "Adding subResource" << subResource->mLabel
                         << "for collection" << collection.url();

            ItemFetchJob *job = new ItemFetchJob( collection );
            job->fetchScope().fetchFullPayload();

            connect( job, SIGNAL( result( KJob* ) ),
                     mParent, SLOT( subResourceLoadResult( KJob* ) ) );

            mJobToResourceMap.insert( job, collectionUrl );

            // check if this item has children
            const int rowCount = mCollectionFilterModel->rowCount( index );
            if ( rowCount > 0 )
              addCollectionsRecursively( index, 0, rowCount - 1 );
          }
        }
      }
    }
  }
}

bool ResourceAkonadi::Private::removeCollectionsRecursively( const QModelIndex &parent, int start, int end )
{
  kDebug(5700) << "start=" << start << ", end=" << end;

  bool changed = false;
  for ( int row = start; row <= end; ++row ) {
    QModelIndex index = mCollectionFilterModel->index( row, 0, parent );
    if ( index.isValid() ) {
      QVariant data = mCollectionFilterModel->data( index, CollectionModel::CollectionRole );
      if ( data.isValid() ) {
        Collection collection = data.value<Collection>();
        if ( collection.isValid() ) {
          const QString collectionUrl = collection.url().url();
          mSubResourceIds.remove( collectionUrl );
          SubResource* subResource = mSubResources.take( collectionUrl );
          if ( subResource != 0 ) {
            bool internalModification = mInternalCalendarModification;
            mInternalCalendarModification = true;

            UidResourceMap::iterator uidIt = mUidToResourceMap.begin();
            while ( uidIt != mUidToResourceMap.end() ) {
              if ( uidIt.value() == collectionUrl ) {
                changed = true;

                Incidence *incidence = mCalendar.incidence( uidIt.key() );
                Q_ASSERT( incidence != 0 );
                Q_ASSERT( mCalendar.deleteIncidence( incidence ) );
                uidIt = mUidToResourceMap.erase( uidIt );
              }
              else
                ++uidIt;
            }

            mInternalCalendarModification = internalModification;

            ItemIdResourceMap::iterator idIt = mItemIdToResourceMap.begin();
            while ( idIt != mItemIdToResourceMap.end() ) {
              if ( idIt.value() == collectionUrl ) {
                idIt = mItemIdToResourceMap.erase( idIt );
              }
              else
                ++idIt;
            }

            emit mParent->signalSubresourceRemoved( mParent, QLatin1String( "calendar" ), collectionUrl );

            delete subResource;

            const int rowCount = mCollectionFilterModel->rowCount( index );
            if ( rowCount > 0 )
              removeCollectionsRecursively( index, 0, rowCount - 1 );
          }
        }
      }
    }
  }

  return changed;
}

void ResourceAkonadi::Private::delayedAutoSaveOnDelete()
{
  kDebug(5800);

  mParent->doSave( false );
}

bool ResourceAkonadi::Private::prepareSaving()
{
  // if an incidence is not yet mapped to one of the sub resources we put it into
  // our store collection.
  // if the store collection is no or nor longer valid, ask for a new one
  Incidence::List incidenceList = mCalendar.rawIncidences();
  Incidence::List::const_iterator it    = incidenceList.begin();
  Incidence::List::const_iterator endIt = incidenceList.end();
  while ( it != endIt ) {
    UidResourceMap::const_iterator findIt = mUidToResourceMap.find( (*it)->uid() );
    if ( findIt == mUidToResourceMap.end() ) {
      if ( !mStoreCollection.isValid() ||
           mSubResources[ mStoreCollection.url().url() ] == 0 ) {

        ResourceAkonadiConfigDialog dialog( mParent );
        if ( dialog.exec() != QDialog::Accepted )
          return false;

        // if accepted, use the same iterator position again to re-check
      } else {
        mUidToResourceMap.insert( (*it)->uid(), mStoreCollection.url().url() );
        ++it;
      }
    } else
      ++it;
  }

  return true;
}

KJob *ResourceAkonadi::Private::createSaveSequence()
{
  kDebug(5800);
  mAutoSaveOnDeleteTimer.stop();

  Item::List items;

  Incidence::List incidences = mCalendar.rawIncidences();

  Incidence::List::const_iterator incidenceIt    = incidences.begin();
  Incidence::List::const_iterator incidenceEndIt = incidences.end();
  for ( ; incidenceIt != incidenceEndIt; ++incidenceIt ) {
    Incidence *incidence = *incidenceIt;
    ItemMap::const_iterator itemIt = mItems.end();

    IdHash::const_iterator idIt = mIdMapping.find( incidence->uid() );
    if ( idIt != mIdMapping.end() ){
      itemIt = mItems.find( idIt.value() );
    }

    if ( itemIt == mItems.end() ) {
      incidence->accept( *mMimeVisitor );
      Item item( mMimeVisitor->mimeType() );
      item.setPayload<IncidencePtr>( IncidencePtr( incidence->clone() ) );

      items << item;
    } else {
      Item item = itemIt.value();
      item.setPayload<IncidencePtr>( IncidencePtr( incidence->clone() ) );

      items << item;
    }
  }

  ItemSync *job = new ItemSync( mStoreCollection, mParent );
  job->setFullSyncItems( items );

  return job;
}

void ResourceAkonadi::Private::calendarIncidenceAdded( Incidence *incidence )
{
  if ( mInternalCalendarModification )
    return;

  kDebug(5800) << incidence->uid();

  IdHash::iterator idIt = mIdMapping.find( incidence->uid() );
  Q_ASSERT( idIt == mIdMapping.end() );

  mChanges[ incidence->uid() ] = Added;
}

void ResourceAkonadi::Private::calendarIncidenceModified( Incidence *incidence )
{
  if ( mInternalCalendarModification )
    return;

  kDebug(5800) << incidence->uid();

  IdHash::iterator idIt = mIdMapping.find( incidence->uid() );
  if ( idIt == mIdMapping.end() ) {
    Q_ASSERT( mChanges[ incidence->uid() ] == Added );
  } else
    mChanges[ incidence->uid() ] = Changed;
}

void ResourceAkonadi::Private::calendarIncidenceDeleted( Incidence *incidence )
{
  if ( mInternalCalendarModification )
    return;

  kDebug(5800) << incidence->uid();

  // check if we have saved it already, otherwise it is just a local change
  IdHash::iterator idIt = mIdMapping.find( incidence->uid() );
  if ( idIt != mIdMapping.end() ) {
    mIdMapping.erase( idIt );
    mUidToResourceMap.remove( incidence->uid() );

    mChanges[ incidence->uid() ] = Removed;

    mAutoSaveOnDeleteTimer.start( 5000 ); // FIXME: configurable if needed at all
  } else
    mChanges.remove( incidence->uid() );
}

bool ResourceAkonadi::Private::reloadSubResource( SubResource *subResource )
{
  ItemFetchJob *job = new ItemFetchJob( subResource->mCollection );
  job->fetchScope().fetchFullPayload();

  if ( !job->exec() ) {
    mParent->loadError( job->errorString() );
    return false;
  }

  const QString collectionUrl = subResource->mCollection.url().url();

  Item::List items = job->items();

  kDebug(5700) << "Reload for sub resource " << collectionUrl
               << "produced" << items.count() << "items";

  bool internalModification = mInternalCalendarModification;
  mInternalCalendarModification = true;

  bool changed = false;
  foreach ( const Item& item, items ) {
    if ( item.hasPayload<IncidencePtr>() ) {
      changed = true;

      IncidencePtr incidencePtr = item.payload<IncidencePtr>();

      const Item::Id id = item.id();
      mIdMapping.insert( incidencePtr->uid(), id );

      Incidence *incidence = mCalendar.incidence( incidencePtr->uid() );
      if ( incidence != 0 )
        mCalendar.deleteIncidence( incidence );

      mCalendar.addIncidence( incidencePtr->clone() );
      mUidToResourceMap.insert( incidencePtr->uid(), collectionUrl );
      mItemIdToResourceMap.insert( id, collectionUrl );

    }

    // always update the item
    mItems.insert( item.id(), item );
  }

  mInternalCalendarModification = internalModification;

  return changed;
}

#include "resourceakonadi.moc"
// kate: space-indent on; indent-width 2; replace-tabs on;
