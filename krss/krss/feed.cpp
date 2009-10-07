/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "feed.h"
#include "feed_p.h"
#include "resource.h"
#include "item_p.h"
#include "itemjobs.h"

#include <akonadi/itemfetchjob.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionstatistics.h>
#include <akonadi/collectionstatisticsjob.h>
#include <akonadi/item.h>
#include <akonadi/itemfetchscope.h>
#include <KIcon>
#include <KDebug>
#include <KLocale>

#include <cassert>

using Akonadi::Collection;
using Akonadi::ItemFetchJob;
using Akonadi::CollectionFetchJob;
using boost::shared_ptr;

using namespace KRss;

void FeedPrivate::updateFromStatistics( const Akonadi::CollectionStatistics& stats ) {
    const qint64 unread = stats.unreadCount();
    const qint64 total = stats.count();
    if ( unread != -1 && m_unreadCount != unread ) {
        m_unreadCount = unread;
        emit q->unreadCountChanged( m_feedCollection.feedId(), m_unreadCount );
    }
    if ( total != -1 && m_totalCount != total ) {
        m_totalCount = total;
        emit q->totalCountChanged( m_feedCollection.feedId(), m_totalCount );
    }
}

void FeedPrivate::slotCollectionLoadDone( KJob *job )
{
    if ( job->error() ) {
        kWarning() << "Collection load failed:" << job->errorString();
        return;
    }

    m_feedCollection = static_cast<CollectionFetchJob*>( job )->collections().first();
    emit q->changed( m_feedCollection.feedId() );
}

void FeedPrivate::slotStatisticsFetchDone( KJob* j )
{
    const Akonadi::CollectionStatisticsJob* const job = qobject_cast<Akonadi::CollectionStatisticsJob*>( j );
    assert( job );
    if ( job->error() ) {
        kWarning() << "Fetching statistics failed. Error:" << job->error() << job->errorText();
        return;
    }
    updateFromStatistics( job->statistics() );
}

bool Feed::isFetching() const
{
    return d->m_fetching;
}

Feed::Feed( const FeedCollection& feedCollection, const shared_ptr<Resource>& resource, QObject* parent )
    : QObject( parent ), d( new FeedPrivate( feedCollection, resource, this ) )
{
    resource->registerListeningFeed( this );
    Akonadi::CollectionStatisticsJob* job = new Akonadi::CollectionStatisticsJob( feedCollection );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(slotStatisticsFetchDone(KJob*)) );
    job->start();
}

Feed::FetchError Feed::error() const
{
    return d->m_error;
}

bool Feed::hasError() const
{
    return d->m_error != NoError;
}

QString Feed::errorString() const
{
    return d->m_errorString;
}

Feed::~Feed()
{
    delete d;
}

Feed::Id Feed::id() const
{
    return d->m_feedCollection.feedId();
}

QString Feed::title() const
{
    return d->m_feedCollection.title();
}

void Feed::setTitle( const QString& title )
{
    d->m_feedCollection.setTitle( title );
}

QString Feed::description() const
{
    return d->m_feedCollection.description();
}

void Feed::setDescription( const QString& description )
{
    d->m_feedCollection.setDescription( description );
}

QIcon Feed::icon() const
{
    return d->m_icon;
}

void Feed::setIcon( const QIcon& icon )
{
    d->m_icon = icon;
}

QList<TagId> Feed::tags() const
{
    return d->m_feedCollection.tags();
}

void Feed::setTags( const QList<TagId>& tags )
{
    d->m_feedCollection.setTags( tags );
}

void Feed::addTag( const TagId& tag )
{
    d->m_feedCollection.addTag( tag );
}

void Feed::removeTag( const TagId& tag )
{
    d->m_feedCollection.removeTag( tag );
}

QPixmap Feed::image() const
{
    return QPixmap();
}

void Feed::setImage( const QPixmap& image )
{
    Q_UNUSED( image )
}

QStringList Feed::subscriptionLabels() const
{
    return d->m_feedCollection.subscriptionLabels();
}

void Feed::setSubscriptionLabels( const QStringList& subscriptionLabels )
{
    d->m_feedCollection.setSubscriptionLabels( subscriptionLabels );
}

void Feed::addSubscriptionLabel( const QString& subscriptionLabel )
{
    d->m_feedCollection.addSubscriptionLabel( subscriptionLabel );
}

void Feed::removeSubscriptionLabel( const QString& subscriptionLabel )
{
    d->m_feedCollection.removeSubscriptionLabel( subscriptionLabel );
}

int Feed::total() const
{
    return d->m_totalCount;
}

int Feed::unread() const
{
    return d->m_unreadCount;
}

int Feed::fetchInterval() const
{
    return ( d->m_feedCollection.fetchInterval() > 0 ? d->m_feedCollection.fetchInterval() : 0 );
}

void Feed::setFetchInterval( int minutes )
{
    d->m_feedCollection.setFetchInterval( minutes > 0 ? minutes : -1 );
}

ItemListJob* Feed::itemListJob() {
    return new ItemListJobImpl( this );
}

StatusModifyJob* Feed::statusModifyJob()
{
    return new StatusModifyJobImpl( this );
}

void Feed::fetch() const
{
    if ( d->m_resource )
        d->m_resource->fetchFeed( d->m_feedCollection.feedId() );
}

void Feed::abortFetch() const
{
    if ( d->m_resource )
        d->m_resource->abortFetch( d->m_feedCollection.feedId() );
}

void Feed::triggerChanged()
{
    CollectionFetchJob* const job = new CollectionFetchJob( d->m_feedCollection, CollectionFetchJob::Base, this );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionLoadDone( KJob* ) ) );
    job->start();
}

void Feed::triggerRemoved()
{
    emit removed( d->m_feedCollection.feedId() );
}

void Feed::triggerStatisticsChanged( const Akonadi::CollectionStatistics& stats )
{
    d->updateFromStatistics( stats );
}

void Feed::triggerFetchStarted()
{
    d->m_fetching = true;
    emit fetchStarted( d->m_feedCollection.feedId() );
}

void Feed::triggerFetchPercent( uint percentage )
{
    emit fetchPercent( d->m_feedCollection.feedId(), percentage );
}

void Feed::triggerFetchFinished()
{
    d->m_error = NoError;
    d->m_errorString.clear();
    d->m_fetching = false;
    emit fetchFinished( d->m_feedCollection.feedId() );
}

void Feed::triggerFetchFailed( FetchError error, const QString& errorMessage )
{
    d->m_error = error;
    d->m_errorString = errorMessage;
    d->m_fetching = false;
    emit fetchFailed( d->m_feedCollection.feedId(), errorMessage );
}

void Feed::triggerFetchAborted()
{
    d->m_error = NoError;
    d->m_errorString.clear();
    d->m_fetching = false;
    emit fetchAborted( d->m_feedCollection.feedId() );
}


void ItemListJobImpl::slotItemsReceived( const Akonadi::Item::List& items )
{
    if ( m_feed->total() > 0 ) {
        emitPercent( items.count(), m_feed->total() );
    }
}

void ItemListJobImpl::jobDone( KJob* j ) {
    assert( m_job == j );
    setError( m_job->error() );
    setErrorText( m_job->errorString() );
    if ( m_job->error() ) {
        emitResult();
        return;
    }

    Q_FOREACH( const Akonadi::Item &loadedItem, m_job->items() ) {
        Item item;
        item.d->akonadiItem = loadedItem;
        m_items.append( item );
    }

    emitResult();
}

void ItemListJobImpl::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void ItemListJobImpl::doStart() {
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( m_feed->d->m_feedCollection, this );
    if ( !m_fetchScope.isEmpty() )
        job->setFetchScope( m_fetchScope );
    connect( job, SIGNAL(itemsReceived(Akonadi::Item::List)),
             this, SLOT(slotItemsReceived(Akonadi::Item::List)) );
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(jobDone(KJob*)) );
    m_job = job;
    job->start();
}

bool ItemListJobImpl::doKill() {
     delete m_job;
    return true;
}

StatusModifyJobImpl::StatusModifyJobImpl( Feed *parent )
    : StatusModifyJob( parent ), m_feed( parent ), m_jobs( 0 ), m_totalJobs( 0 )
{
}

void StatusModifyJobImpl::clearFlags( const QList<KRss::Item::StatusFlag>& flags )
{
    m_clearFlags = flags;
}

void StatusModifyJobImpl::setFlags( const QList<KRss::Item::StatusFlag>& flags )
{
    m_setFlags = flags;
}

void StatusModifyJobImpl::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

QString StatusModifyJobImpl::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        // simply pass through the error text from the underlying jobs
        case StatusModifyJobImpl::CouldNotLoadItems:
        case StatusModifyJobImpl::CouldNotModifyItem:
            result = errorText();
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void StatusModifyJobImpl::doStart()
{
    ItemListJob * const job = m_feed->itemListJob();
    Akonadi::ItemFetchScope scope;
    scope.fetchFullPayload( false );
    scope.fetchAllAttributes();
    job->setFetchScope( scope );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotItemsLoaded( KJob* ) ) );
    job->start();
}

void StatusModifyJobImpl::slotItemsLoaded( KJob *job )
{
    if ( job->error() ) {
        setError( StatusModifyJobImpl::CouldNotLoadItems );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    const ItemListJob * const ljob = qobject_cast<const ItemListJob*>( job );
    Q_ASSERT( ljob );
    const QList<Item> items = ljob->items();
    Q_FOREACH( Item item, items ) {
        bool updated = false;

        Q_FOREACH( const KRss::Item::StatusFlag& flag, m_setFlags ) {
            if ( !item.status().testFlag( flag ) ) {
                item.setStatus( item.status() | flag );
                updated = true;
            }
        }

        Q_FOREACH( const KRss::Item::StatusFlag& flag, m_clearFlags ) {
            if ( item.status().testFlag( flag ) ) {
                item.setStatus( item.status() & ~flag );
                updated = true;
            }
        }

        if ( updated ) {
            ++m_jobs;
            ++m_totalJobs;
            ItemModifyJob * const mjob = new ItemModifyJob();
            mjob->setItem( item );
            mjob->setIgnorePayload( true );
            connect( mjob, SIGNAL( result( KJob* ) ), this, SLOT( slotItemModified( KJob* ) ) );
            mjob->start();
        }
    }

    if ( m_jobs > 0 )
        return;

    emitResult();
}

void StatusModifyJobImpl::slotItemModified( KJob *job )
{
    if ( job->error() ) {
        setError( StatusModifyJobImpl::CouldNotModifyItem );
        setErrorText( job->errorString() );
    }

    --m_jobs;
    if ( m_jobs > 0 ) {
        emitPercent( m_totalJobs - m_jobs, m_totalJobs );
        return;
    }

    emitResult();
}

#include "feed.moc"
#include "feed_p.moc"
