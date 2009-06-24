/*
    Copyright 2008 Ingo Klöcker <kloecker@kde.org>
    Copyright 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "maildispatcheragent.h"

//#include "configdialog.h"
//#include "maildispatcheradaptor.h"
#include "outboxqueue.h"
#include "sendjob.h"
#include "settings.h"
#include "settingsadaptor.h"

#include <QtDBus/QDBusConnection>
#include <QTimer>

#include <KDebug>
#include <KLocalizedString>
#include <KWindowSystem>

#include <Akonadi/ItemFetchScope>

using namespace Akonadi;


class MailDispatcherAgent::Private
{
  public:
    Private( MailDispatcherAgent *parent )
        : q( parent )
        , currentJob( 0 )
        , currentItem()
        , aborting( false )
        , sentAnything( false )
    {
    }

    ~Private()
    {
    }

    MailDispatcherAgent * const q;

    OutboxQueue *queue;
    SendJob *currentJob;
    Item currentItem;
    bool aborting;
    bool sentAnything;

    // slots:
    void abort();
    void dispatch();
    void itemFetched( Item &item );
    void sendResult( KJob *job );
    void emitStatusReady();

};


void MailDispatcherAgent::Private::abort()
{
  if( aborting ) {
    kDebug() << "Already aborting.";
    return;
  }

  if( !currentJob ) {
    kDebug() << "MDA is idle.";
    Q_ASSERT( q->status() == AgentBase::Idle );
    Q_ASSERT( queue->isEmpty() );
  } else {
    kDebug() << "Aborting...";
    aborting = true;
    currentJob->abort();
    // Further SendJobs will mark remaining items in the queue as 'aborted'.
  }
}

void MailDispatcherAgent::Private::dispatch()
{
  Q_ASSERT( queue );

  if( !q->isOnline() ) {
    kDebug() << "Offline. See you later.";
    return;
  }

  if( currentJob ) {
    kDebug() << "Another job is active. See you later.";
    return;
  }

  if( !queue->isEmpty() ) {
    sentAnything = true;
    // TODO Sending message X of Y: <subject>
    emit q->status( AgentBase::Running,
        i18np( "Sending messages (1 item in queue)...",
               "Sending messages (%1 items in queue)...", queue->count() ) );
    kDebug() << "Attempting to dispatch the next message.";
    queue->fetchOne(); // will trigger itemFetched
  } else {
    kDebug() << "Empty queue.";
    if( aborting ) {
      // Finished marking messages as 'aborted'.
      aborting = false;
      emit q->status( AgentBase::Idle, i18n( "Sending cancelled." ) );
      QTimer::singleShot( 3000, q, SLOT(emitStatusReady()) );
    } else {
      if( sentAnything ) {
        // Finished sending messages in queue.
        sentAnything = false;
        emit q->status( AgentBase::Idle, i18n( "Finished sending messages." ) );
      } else {
        // Empty queue.
        emit q->status( AgentBase::Idle, i18n( "No items in queue." ) );
      }
      QTimer::singleShot( 3000, q, SLOT(emitStatusReady()) );
    }
  }
}


MailDispatcherAgent::MailDispatcherAgent( const QString &id )
  : AgentBase( id ),
    d( new Private( this ) )
{
  kDebug() << "maildispatcheragent: At your service, sir!";

  new SettingsAdaptor( Settings::self() );
  //new MailDispatcherAdaptor( this );
  QDBusConnection::sessionBus().registerObject( QLatin1String( "/Settings" ),
                              Settings::self(), QDBusConnection::ExportAdaptors );

  d->queue = new OutboxQueue( this );
  connect( d->queue, SIGNAL( newItems() ), this, SLOT( dispatch() ) );
  connect( d->queue, SIGNAL( itemReady( Akonadi::Item& ) ),
      this, SLOT( itemFetched( Akonadi::Item& ) ) );
  connect( this, SIGNAL(itemProcessed(Akonadi::Item,bool)),
      d->queue, SLOT(itemProcessed(Akonadi::Item,bool)) );
  connect( this, SIGNAL(abortRequested()), this, SLOT(abort()) );
}

MailDispatcherAgent::~MailDispatcherAgent()
{
  delete d;
}

void MailDispatcherAgent::configure( WId windowId )
{
  Q_UNUSED( windowId );
  kDebug() << "I have no options; you can't break me.";
}

void MailDispatcherAgent::doSetOnline( bool online )
{
  Q_ASSERT( d->queue );
  if( online ) {
    kDebug() << "Online. Dispatching messages.";
    emit status( AgentBase::Idle, i18n( "Online, sending messages in queue." ) );
    QTimer::singleShot( 0, this, SLOT( dispatch() ) );
  } else {
    kDebug() << "Offline.";
    emit status( AgentBase::Idle, i18n( "Offline, message sending suspended." ) );

    // TODO: This way, the OutboxQueue will continue to react to changes in
    // the outbox, but the MDA will just not send anything.  Is this what we
    // want?
  }
  
  AgentBase::doSetOnline( online );
}

void MailDispatcherAgent::Private::itemFetched( Item &item )
{
  kDebug() << "Fetched item" << item.id() << "; creating SendJob.";
  Q_ASSERT( !currentItem.isValid() );
  currentItem = item;
  Q_ASSERT( currentJob == 0 );
  currentJob = new SendJob( item, q );
  if( aborting ) {
    currentJob->setMarkAborted();
  }
  connect( currentJob, SIGNAL( result( KJob* ) ),
      q, SLOT( sendResult( KJob* ) ) );
  currentJob->start();
}

void MailDispatcherAgent::Private::sendResult( KJob *job )
{
  Q_ASSERT( job == currentJob );
  currentJob->disconnect( q );
  currentJob = 0;

  Q_ASSERT( currentItem.isValid() );
  emit q->itemProcessed( currentItem, !job->error() );
  currentItem = Item();

  if( job->error() ) {
    // The SendJob gave the item an ErrorAttribute, so we don't have to
    // do anything.
    kDebug() << "Sending failed. error:" << job->errorString();
  } else {
    kDebug() << "Sending succeeded.";
  }

  // dispatch next message
  QTimer::singleShot( 0, q, SLOT( dispatch() ) );
}

void MailDispatcherAgent::Private::emitStatusReady()
{
  if( q->status() == AgentBase::Idle ) {
    // If still idle after aborting, clear 'aborted' status.
    emit q->status( AgentBase::Idle, i18n( "Ready to dispatch messages." ) );
  }
}


AKONADI_AGENT_MAIN( MailDispatcherAgent )


#include "maildispatcheragent.moc"
