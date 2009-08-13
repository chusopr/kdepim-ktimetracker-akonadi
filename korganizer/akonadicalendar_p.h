/*
    This file is part of Akonadi.

    Copyright (c) 2009 Sebastian Sauer <sebsauer@kdab.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef AKONADICALENDAR_P_H
#define AKONADICALENDAR_P_H

#include "akonadicalendar.h"

#include <QObject>
#include <QCoreApplication>
#include <QDBusInterface>

#include <akonadi/entity.h>
#include <akonadi/collection.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectiondialog.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agenttype.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/monitor.h>
#include <akonadi/session.h>

#include <KCal/Incidence>

using namespace KCal;

class AkonadiCalendarCollection : public QObject
{
    Q_OBJECT
  public:
    AkonadiCalendar *m_calendar;
    Akonadi::Collection m_collection;

    AkonadiCalendarCollection(AkonadiCalendar *calendar, const Akonadi::Collection &collection)
      : QObject()
      , m_calendar(calendar)
      , m_collection(collection)
    {
    }

    ~AkonadiCalendarCollection()
    {
    }
};

class AkonadiCalendarItem : public QObject
{
    Q_OBJECT
  public:
    AkonadiCalendar *m_calendar;
    Akonadi::Item m_item; //needed to keep an instance to increment shared_ptr ref-counter

    AkonadiCalendarItem(AkonadiCalendar *calendar, const Akonadi::Item &item)
      : QObject()
      , m_calendar(calendar)
      , m_item(item)
    {
    }

    ~AkonadiCalendarItem()
    {
    }

    KCal::Incidence::Ptr incidence() const
    {
      Q_ASSERT( m_item.hasPayload() );
      return m_item.payload<KCal::Incidence::Ptr>();
    }

};

class KCal::AkonadiCalendar::Private : public QObject
{
    Q_OBJECT
  public:
    explicit Private(AkonadiCalendar *q)
      : q(q)
      , m_monitor( new Akonadi::Monitor() )
      , m_session( new Akonadi::Session( QCoreApplication::instance()->applicationName().toUtf8() + QByteArray("-AkonadiCal-") + QByteArray::number(qrand()) ) )
      , m_incidenceBeingChanged()
    {
      m_monitor->itemFetchScope().fetchFullPayload();
      m_monitor->ignoreSession( m_session );

      connect( m_monitor, SIGNAL(itemChanged( const Akonadi::Item&, const QSet<QByteArray>& )),
               this, SLOT(itemChanged( const Akonadi::Item&, const QSet<QByteArray>& )) );
      connect( m_monitor, SIGNAL(itemMoved( const Akonadi::Item&, const Akonadi::Collection&, const Akonadi::Collection& )),
               this, SLOT(itemMoved( const Akonadi::Item&, const Akonadi::Collection&, const Akonadi::Collection& ) ) );
      connect( m_monitor, SIGNAL(itemAdded( const Akonadi::Item&, const Akonadi::Collection& )),
               this, SLOT(itemAdded( const Akonadi::Item&, const Akonadi::Collection& )) );
      connect( m_monitor, SIGNAL(itemRemoved( const Akonadi::Item&, const Akonadi::Collection& )),
               this, SLOT(itemRemoved( const Akonadi::Item&, const Akonadi::Collection& )) );
      /*
      connect( m_monitor, SIGNAL(itemLinked(const Akonadi::Item&, const Akonadi::Collection&)),
               this, SLOT(itemAdded(const Akonadi::Item&, const Akonadi::Collection&)) );
      connect( m_monitor, SIGNAL(itemUnlinked( const Akonadi::Item&, const Akonadi::Collection& )),
               this, SLOT(itemRemoved( const Akonadi::Item&, const Akonadi::Collection& )) );
      */
    }

    ~Private()
    {
      delete m_monitor;
      delete m_session;
    }

    /*
    void clear()
    {
      kDebug();
      mEvents.clear();
      mTodos.clear();
      mJournals.clear();
      m_map.clear();
      qDeleteAll(m_itemMap);
      qDeleteAll(m_collectionMap);
    }
    */

    bool addIncidence( Incidence *incidence )
    {
      kDebug();
      Akonadi::CollectionDialog dlg( 0 );
      dlg.setMimeTypeFilter( QStringList() << QString::fromLatin1( "text/calendar" ) );
      if ( ! dlg.exec() ) {
        return false;
      }
      const Akonadi::Collection collection = dlg.selectedCollection();
      Q_ASSERT( collection.isValid() );
      //Q_ASSERT( m_collectionMap.contains( collection.id() ) ); //we can add items to collections we don't show yet
      Q_ASSERT( ! m_uidToItemId.contains( incidence->uid() ) ); //but we can not have the same incidence in 2 collections

      Akonadi::Item item;
      //the sub-mimetype of text/calendar as defined at kdepim/akonadi/kcal/kcalmimetypevisitor.cpp
      item.setMimeType( QString("application/x-vnd.akonadi.calendar.%1").arg(QString(incidence->type().toLower())) );
      KCal::Incidence::Ptr incidencePtr( incidence ); //no clone() needed
      item.setPayload<KCal::Incidence::Ptr>( incidencePtr );
      Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection, m_session );
      connect( job, SIGNAL( result( KJob* ) ), this, SLOT( createDone( KJob* ) ) );
      return true;
    }

    bool deleteIncidence( Incidence *incidence )
    {
      kDebug();
      m_changes.removeAll( incidence->uid() ); //abort changes to this incidence cause we will just delete it
      Q_ASSERT( m_uidToItemId.contains( incidence->uid() ) );
      Akonadi::Item item = itemForUid( incidence->uid() )->m_item;
      Q_ASSERT( item.isValid() );
      Q_ASSERT( item.hasPayload() );
      Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( item, m_session );
      connect( job, SIGNAL( result( KJob* ) ), this, SLOT( deleteDone( KJob* ) ) );
      return true;
    }

    AkonadiCalendarItem* itemForUid( const QString& uid )
    {
      if ( m_uidToItemId.contains( uid ) ) {
        const Akonadi::Item::Id id = m_uidToItemId[uid];
        Q_ASSERT( m_itemMap.contains( id ) );
        return m_itemMap[id];
      }
      return 0;
    }

    void removeIncidenceFromMultiHashByUID(const Incidence::Ptr &incidence, const QString &key)
    {
      const QList<KCal::Incidence::Ptr> values = m_incidenceForDate.values( key );
      QListIterator<KCal::Incidence::Ptr> it(values);
      while( it.hasNext() ) {
        KCal::Incidence::Ptr inc = it.next();
        if( inc->uid() == incidence->uid() ) {
          m_incidenceForDate.remove( key, inc );
        }
      }
    }

    AkonadiCalendar *q;
    Akonadi::Monitor *m_monitor;
    Akonadi::Session *m_session;
    QHash<Akonadi::Entity::Id, AkonadiCalendarCollection*> m_collectionMap;
    QHash<Akonadi::Item::Id, AkonadiCalendarItem*> m_itemMap; // akonadi uid to calendar items
    QMap<QString, Akonadi::Item::Id> m_uidToItemId;
    QList<QString> m_changes; //list of Incidence->uid() that are modified atm
    KCal::Incidence::Ptr m_incidenceBeingChanged; // clone of the incidence currently being modified, for rollback and to check if something actually changed

    //CalFormat *mFormat;                    // calendar format
    //QHash<QString, Event *>mEvents;        // hash on uids of all Events
    QMultiHash<QString, KCal::Incidence::Ptr> m_incidenceForDate;// on start dates of non-recurring, single-day Incidences
//QMultiHash<QString, Event *>mEventsForDate;// on start dates of non-recurring, single-day Events
    //QHash<QString, Todo *>mTodos;          // hash on uids of all Todos
//QMultiHash<QString, Todo*>mTodosForDate;// on due dates for all Todos
    //QHash<QString, Journal *>mJournals;    // hash on uids of all Journals
//QMultiHash<QString, Journal *>mJournalsForDate; // on dates of all Journals
    //Incidence::List mDeletedIncidences;    // list of all deleted Incidences

  public Q_SLOTS:
  
    void listingDone( KJob *job )
    {
        kDebug();
        Akonadi::ItemFetchJob *fetchjob = static_cast<Akonadi::ItemFetchJob*>( job );
        if ( job->error() ) {
            kWarning( 5250 ) << "Item query failed:" << job->errorString();
            emit q->signalErrorMessage( job->errorString() );
            return;
        }
        itemsAdded( fetchjob->items(), fetchjob->collection() );
    }

    void agentCreated( KJob *job )
    {
        kDebug();
        Akonadi::AgentInstanceCreateJob *createjob = dynamic_cast<Akonadi::AgentInstanceCreateJob*>( job );
        if ( createjob->error() ) {
            kWarning( 5250 ) << "Agent create failed:" << createjob->errorString();
            emit q->signalErrorMessage( createjob->errorString() );
            return;
        }
        Akonadi::AgentInstance instance = createjob->instance();
        //instance.setName( CalendarName );
        QDBusInterface iface("org.freedesktop.Akonadi.Resource." + instance.identifier(), "/Settings");
        if( ! iface.isValid() ) {
            kWarning( 5250 ) << "Failed to obtain D-Bus interface for remote configuration.";
            emit q->signalErrorMessage( "Failed to obtain D-Bus interface for remote configuration." );
            return;
        }
        QString path = createjob->property("path").toString();
        Q_ASSERT( ! path.isEmpty() );
        iface.call("setPath", path);
        instance.reconfigure();
    }

    void createDone( KJob *job )
    {
        kDebug();
        if ( job->error() ) {
            kWarning( 5250 ) << "Item create failed:" << job->errorString();
            emit q->signalErrorMessage( job->errorString() );
            return;
        }
        Akonadi::ItemCreateJob *createjob = static_cast<Akonadi::ItemCreateJob*>( job );
        itemAdded( createjob->item(), createjob->collection() );
    }

    void deleteDone( KJob *job )
    {
        kDebug();
        if ( job->error() ) {
            kWarning( 5250 ) << "Item delete failed:" << job->errorString();
            emit q->signalErrorMessage( job->errorString() );
            return;
        }
        Akonadi::ItemDeleteJob *deletejob = static_cast<Akonadi::ItemDeleteJob*>( job );
        itemsRemoved( deletejob->items(), deletejob->collection() );
    }

    void modifyDone( KJob *job )
    {
        Akonadi::ItemModifyJob *modifyjob = static_cast<Akonadi::ItemModifyJob*>( job );
        if ( modifyjob->error() ) {
            kWarning( 5250 ) << "Item modify failed:" << job->errorString();
            emit q->signalErrorMessage( job->errorString() );
            return;
        }
        Akonadi::Item item = modifyjob->item();
        Q_ASSERT( item.hasPayload() );
        Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>() );
        const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
        Q_ASSERT( incidence );
        const Akonadi::Item::Id uid = item.id();
        //kDebug()<<"Old storageCollectionId="<<m_itemMap[uid]->m_item.storageCollectionId();
        kDebug() << "Item modify done uid=" << uid << "storageCollectionId=" << item.storageCollectionId();
        Q_ASSERT( m_itemMap.contains(uid) );
        Q_ASSERT( item.storageCollectionId() == m_itemMap[uid]->m_item.storageCollectionId() ); // there was once a bug that resulted in items forget there collectionId...
        m_itemMap[uid]->m_item = item;
        q->notifyIncidenceChanged( incidence.get() );
        q->setModified( true );
        emit q->calendarChanged();
    }

    void itemChanged( const Akonadi::Item& item, const QSet<QByteArray>& )
    {
        Q_ASSERT( item.isValid() );
        Q_ASSERT( item.hasPayload() );
        Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>() );
        const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
        Q_ASSERT( incidence );
        const Akonadi::Item::Id uid = item.id();
        kDebug() << "Item changed uid=" << uid << "summary=" << incidence->summary() << "type=" << incidence->type() << "storageCollectionId=" << item.storageCollectionId();
        Q_ASSERT( m_itemMap.contains(uid) );
        m_itemMap[uid]->m_item = item;
        q->notifyIncidenceChanged( incidence.get() );
        q->setModified( true );
        emit q->calendarChanged();
    }

    void itemMoved( const Akonadi::Item &item, const Akonadi::Collection& colSrc, const Akonadi::Collection& colDst )
    {
        kDebug();
        if( m_collectionMap.contains(colSrc.id()) && ! m_collectionMap.contains(colDst.id()) )
            itemRemoved( item, colSrc );
        else if( m_collectionMap.contains(colDst.id()) && ! m_collectionMap.contains(colSrc.id()) )
            itemAdded( item, colDst );
    }

    void itemsAdded( const Akonadi::Item::List &items, const Akonadi::Collection &collection )
    {
        kDebug();
        Q_ASSERT( collection.isValid() );
        foreach( const Akonadi::Item &item, items ) {
            Q_ASSERT( item.isValid() );
            Q_ASSERT( item.hasPayload() );
            const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
            kDebug() << "Add uid=" << incidence->uid() << "summary=" << incidence->summary() << "type=" << incidence->type();
            const Akonadi::Item::Id uid = item.id();
            Q_ASSERT( ! m_itemMap.contains( uid ) ); //uh, 2 incidences with the same uid?
            
            if( Event *e = dynamic_cast<Event*>(incidence.get()) ) {
              if ( !e->recurs() && !e->isMultiDay() )
                m_incidenceForDate.insert( e->dtStart().date().toString(), incidence );
            } else if( Todo *t = dynamic_cast<Todo*>(incidence.get()) ) {
              if ( t->hasDueDate() )
                m_incidenceForDate.insert( t->dtDue().date().toString(), incidence );
            } else if( Journal *j = dynamic_cast<Journal*>(incidence.get()) ) {
                m_incidenceForDate.insert( j->dtStart().date().toString(), incidence );
            } else {
              Q_ASSERT(false);
              continue;
            }
    
            m_itemMap[ uid ] = new AkonadiCalendarItem(q, item);
            m_incidenceForDate.insert( incidence->dtStart().date().toString(), incidence );
            m_uidToItemId.insert( incidence->uid(), uid );
            incidence->registerObserver( q );
            q->notifyIncidenceAdded( incidence.get() );
        }
        q->setModified( true );
        emit q->calendarChanged();
    }

    void itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
    {
        kDebug();
        Q_ASSERT( item.isValid() );
        Q_ASSERT( item.hasPayload() );
        if( ! m_itemMap.contains( item.id() ) ) {
          itemsAdded( Akonadi::Item::List() << item, collection );
        }
    }

    void itemsRemoved( const Akonadi::Item::List &items, const Akonadi::Collection &collection ) {
        Q_UNUSED(collection);
        //kDebug()<<items.count();
        foreach(const Akonadi::Item& item, items) {
            Q_ASSERT( item.isValid() );
            std::auto_ptr<AkonadiCalendarItem> ci( m_itemMap.take( item.id() ) );
            Q_ASSERT( ci->m_item.hasPayload<KCal::Incidence::Ptr>() );
            const KCal::Incidence::Ptr incidence = ci->m_item.payload<KCal::Incidence::Ptr>();
            kDebug() << "Remove uid=" << incidence->uid() << "summary=" << incidence->summary() << "type=" << incidence->type();

            if( Event *e = dynamic_cast<Event*>(incidence.get()) ) {
              if ( !e->recurs() )
                removeIncidenceFromMultiHashByUID( incidence, e->dtStart().date().toString() );
            } else if( Todo *t = dynamic_cast<Todo*>(incidence.get()) ) {
              if ( t->hasDueDate() )
                removeIncidenceFromMultiHashByUID( incidence, t->dtDue().date().toString() );
            } else if( Journal *j = dynamic_cast<Journal*>(incidence.get()) ) {
              removeIncidenceFromMultiHashByUID( incidence, j->dtStart().date().toString() );
            } else {
              Q_ASSERT(false);
              continue;
            }

            //incidence->unregisterObserver( q );
            q->notifyIncidenceDeleted( incidence.get() );
            m_uidToItemId.take( incidence->uid() );
        }
        q->setModified( true );
        emit q->calendarChanged();
    }

    void itemRemoved( const Akonadi::Item &item, const Akonadi::Collection &collection )
    {
        kDebug();
        itemsRemoved( Akonadi::Item::List() << item, collection );
    }
  
};

#endif
