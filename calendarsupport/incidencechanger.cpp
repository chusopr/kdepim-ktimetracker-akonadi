/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "incidencechanger.h"
#include "incidencechanger_p.h"
#include "groupware.h"

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/calendaradaptor.h>
#include <akonadi/kcal/groupware.h>
#include <akonadi/kcal/mailscheduler.h>
#include <akonadi/kcal/utils.h>
#include <akonadi/kcal/dndfactory.h>

#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Collection>

#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>


#include <KCal/AssignmentVisitor>
#include <KCal/FreeBusy>
#include <KCal/Incidence>
#include <kcal/comparisonvisitor.h>

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

using namespace KCal;
using namespace Akonadi;

bool IncidenceChanger::Private::myAttendeeStatusChanged( const Incidence* newInc,
                                                         const Incidence* oldInc )
{
  Attendee *oldMe = oldInc->attendeeByMails( KCalPrefs::instance()->allEmails() );
  Attendee *newMe = newInc->attendeeByMails( KCalPrefs::instance()->allEmails() );
  if ( oldMe && newMe && ( oldMe->status() != newMe->status() ) ) {
    return true;
  }

  return false;
}

void IncidenceChanger::Private::queueChange( Change *change )
{
  // If there's already a change queued we just discard it
  // and send the newer change, which already includes
  // previous modifications
  const Item::Id id = change->newItem.id();
  if ( mQueuedChanges.contains( id ) ) {
    delete mQueuedChanges.take( id );
  }

  mQueuedChanges[id] = change;
}

void IncidenceChanger::Private::cancelChanges( Item::Id id )
{
  delete mQueuedChanges.take( id );
  delete mCurrentChanges.take( id );
}

void IncidenceChanger::Private::performNextChange( Item::Id id )
{
  delete mCurrentChanges.take( id );

  if ( mQueuedChanges.contains( id ) ) {
    performChange( mQueuedChanges.take( id ) );
  }
}

bool IncidenceChanger::Private::performChange( Change *change )
{
  Item newItem = change->newItem;
  const Incidence::Ptr oldinc =  change->oldInc;
  const Incidence::Ptr newinc = Akonadi::incidence( newItem );

  kDebug() << "id="                  << newItem.id()         <<
              "uid="                 << newinc->uid()        <<
              "version="             << newItem.revision()   <<
              "summary="             << newinc->summary()    <<
              "old summary"          << oldinc->summary()    <<
              "type="                << newinc->type()       <<
              "storageCollectionId=" << newItem.storageCollectionId();

  // There not any job modifying this item, so mCurrentChanges[item.id] can't exist
  Q_ASSERT( !mCurrentChanges.contains( newItem.id() ) );

  if ( incidencesEqual( newinc.get(), oldinc.get() ) ) {
    // Don't do anything
    kDebug() << "Incidence not changed";
    return true;
  } else {

    if ( mLatestRevisionByItemId.contains( newItem.id() ) &&
         mLatestRevisionByItemId[newItem.id()] > newItem.revision() ) {
      /* When a ItemModifyJob ends, the application can still modify the old items if the user
       * is quick because the ETM wasn't updated yet, and we'll get a STORE error, because
       * we are not modifying the latest revision.
       *
       * When a job ends, we keep the new revision in m_latestVersionByItemId
       * so we can update the item's revision
       */
      newItem.setRevision( mLatestRevisionByItemId[newItem.id()] );
    }

    kDebug() << "Changing incidence";
    const bool attendeeStatusChanged = myAttendeeStatusChanged( oldinc.get(), newinc.get() );
    const int revision = newinc->revision();
    newinc->setRevision( revision + 1 );
    // FIXME: Use a generic method for this! Ideally, have an interface class
    //        for group cheduling. Each implementation could then just do what
    //        it wants with the event. If no groupware is used,use the null
    //        pattern...
    bool success = true;
    if ( KCalPrefs::instance()->mUseGroupwareCommunication ) {
      if ( !mGroupware ) {
          kError() << "Groupware communication enabled but no groupware instance set";
      } else {
        success = mGroupware->sendICalMessage( change->parent,
                                               KCal::iTIPRequest,
                                               newinc.get(),
                                               INCIDENCEEDITED,
                                               attendeeStatusChanged );
      }
    }

    if ( !success ) {
      kDebug() << "Changing incidence failed. Reverting changes.";
      assignIncidence( newinc.get(), oldinc.get() );
      return false;
    }
  }

  // FIXME: if that's a groupware incidence, and I'm not the organizer,
  // send out a mail to the organizer with a counterproposal instead
  // of actually changing the incidence. Then no locking is needed.
  // FIXME: if that's a groupware incidence, and the incidence was
  // never locked, we can't unlock it with endChange().

  mCurrentChanges[newItem.id()] = change;

  // Don't write back remote revision since we can't make sure it is the current one
  // fixes problems with DAV resource
  newItem.setRemoteRevision( QString() );

  ItemModifyJob *job = new ItemModifyJob( newItem );
  connect( job, SIGNAL(result( KJob*)), this, SLOT(changeIncidenceFinished(KJob*)) );
  return true;
}

void IncidenceChanger::Private::changeIncidenceFinished( KJob* j )
{
  //AKONADI_PORT this is from the respective method in the old Akonadi::Calendar, so I leave it here: --Frank
  kDebug();

  // we should probably update the revision number here,or internally in the Event
  // itself when certain things change. need to verify with ical documentation.
  const ItemModifyJob* job = qobject_cast<const ItemModifyJob*>( j );
  Q_ASSERT( job );

  const Item newItem = job->item();

  const Private::Change *change = mCurrentChanges[newItem.id()];
  const Incidence::Ptr oldInc = change->oldInc;
  
  Item oldItem;
  oldItem.setPayload<Incidence::Ptr>( oldInc );
  oldItem.setMimeType( QString::fromLatin1( "application/x-vnd.akonadi.calendar.%1" )
                       .arg( QLatin1String( oldInc->type().toLower() ) ) );
  oldItem.setId( newItem.id() );

  if ( !mCurrentChanges.contains( newItem.id() ) ) {
    kDebug() << "Item was deleted? Great.";
    cancelChanges( newItem.id() );
    emit incidenceChangeFinished( oldItem, newItem, change->action, true );
    return;
  }

  if ( job->error() ) {
    kWarning( 5250 ) << "Item modify failed:" << job->errorString();

    const Incidence::Ptr newInc = Akonadi::incidence( newItem );
    KMessageBox::sorry( change->parent,
                        i18n( "Unable to save changes for incidence %1 \"%2\": %3",
                              i18n( newInc->type() ),
                              newInc->summary(),
                              job->errorString( )) );
    emit incidenceChangeFinished( oldItem, newItem, change->action, false );
  } else {
    emit incidenceChangeFinished( oldItem, newItem, change->action, true );
  }

  mLatestRevisionByItemId[newItem.id()] = newItem.revision();

  // execute any other modification if it exists
  qRegisterMetaType<Akonadi::Item::Id>( "Akonadi::Item::Id" );
  QMetaObject::invokeMethod( this, "performNextChange",
                             Qt::QueuedConnection,
                             Q_ARG( Akonadi::Item::Id, newItem.id() ) );
}

IncidenceChanger::IncidenceChanger( Akonadi::Calendar *cal,
                                    QObject *parent,
                                    Entity::Id defaultCollectionId )
  : QObject( parent ), mCalendar( cal ), d( new Private( defaultCollectionId ) )
{
  connect( d, SIGNAL(incidenceChangeFinished(Akonadi::Item,Akonadi::Item,Akonadi::IncidenceChanger::WhatChanged,bool)),
           SIGNAL(incidenceChangeFinished(Akonadi::Item,Akonadi::Item,Akonadi::IncidenceChanger::WhatChanged,bool)) );
}

IncidenceChanger::~IncidenceChanger()
{
  delete d;
}

void IncidenceChanger::setGroupware( Groupware *groupware )
{
  d->mGroupware = groupware;
}

bool IncidenceChanger::sendGroupwareMessage( const Item &aitem,
                                             KCal::iTIPMethod method,
                                             HowChanged action,
                                             QWidget *parent )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  if ( !incidence ) {
    return false;
  }
  if ( KCalPrefs::instance()->thatIsMe( incidence->organizer().email() ) &&
       incidence->attendeeCount() > 0 &&
       !KCalPrefs::instance()->mUseGroupwareCommunication ) {
    emit schedule( method, aitem );
    return true;
  } else if ( KCalPrefs::instance()->mUseGroupwareCommunication ) {
    if ( !d->mGroupware ) {
      kError() << "Groupware communication enabled but no groupware instance set";
      return false;
    }
    return d->mGroupware->sendICalMessage( parent, method, incidence.get(), action, false );
  }
  return true;
}

void IncidenceChanger::cancelAttendees( const Item &aitem )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  Q_ASSERT( incidence );
  if ( KCalPrefs::instance()->mUseGroupwareCommunication ) {
    if ( KMessageBox::questionYesNo(
           0,
           i18n( "Some attendees were removed from the incidence. "
                 "Shall cancel messages be sent to these attendees?" ),
           i18n( "Attendees Removed" ), KGuiItem( i18n( "Send Messages" ) ),
           KGuiItem( i18n( "Do Not Send" ) ) ) == KMessageBox::Yes ) {
      // don't use Akonadi::Groupware::sendICalMessage here, because that asks just
      // a very general question "Other people are involved, send message to
      // them?", which isn't helpful at all in this situation. Afterwards, it
      // would only call the Akonadi::MailScheduler::performTransaction, so do this
      // manually.
      // FIXME: Groupware scheduling should be factored out to it's own class
      //        anyway
      Akonadi::MailScheduler scheduler( static_cast<Akonadi::Calendar*>(mCalendar) );
      scheduler.performTransaction( incidence.get(), iTIPCancel );
    }
  }
}

bool IncidenceChanger::deleteIncidence( const Item &aitem, QWidget *parent )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  if ( !incidence ) {
    return false;
  }

  if ( !( aitem.parentCollection().rights() & Collection::CanDeleteItem ) ) {
    kWarning() << "insufficient rights to delete incidence";
    return false;
  }

  bool doDelete = sendGroupwareMessage( aitem, KCal::iTIPCancel,
                                        INCIDENCEDELETED, parent );
  if( !doDelete ) {
    return false;
  }

  d->mDeletedItemIds.append( aitem.id() );

  emit incidenceToBeDeleted( aitem );
  d->cancelChanges( aitem.id() ); //abort changes to this incidence cause we will just delete it
  ItemDeleteJob* job = new ItemDeleteJob( aitem );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(deleteIncidenceFinished(KJob*)) );
  return true;
}

void IncidenceChanger::deleteIncidenceFinished( KJob* j )
{
  // todo, cancel changes?
  kDebug();
  const ItemDeleteJob* job = qobject_cast<const ItemDeleteJob*>( j );
  Q_ASSERT( job );
  const Item::List items = job->deletedItems();
  Q_ASSERT( items.count() == 1 );
  Incidence::Ptr tmp = Akonadi::incidence( items.first() );
  Q_ASSERT( tmp );
  if ( job->error() ) {
    KMessageBox::sorry( 0, //PENDING(AKONADI_PORT) set parent
                        i18n( "Unable to delete incidence %1 \"%2\": %3",
                              i18n( tmp->type() ),
                              tmp->summary(),
                              job->errorString( )) );
    d->mDeletedItemIds.removeOne( items.first().id() );
    emit incidenceDeleteFinished( items.first(), false );
    return;
  }
  if ( !KCalPrefs::instance()->thatIsMe( tmp->organizer().email() ) ) {
    const QStringList myEmails = KCalPrefs::instance()->allEmails();
    bool notifyOrganizer = false;
    for ( QStringList::ConstIterator it = myEmails.begin(); it != myEmails.end(); ++it ) {
      QString email = *it;
      Attendee *me = tmp->attendeeByMail( email );
      if ( me ) {
        if ( me->status() == KCal::Attendee::Accepted ||
             me->status() == KCal::Attendee::Delegated ) {
          notifyOrganizer = true;
        }
        Attendee *newMe = new Attendee( *me );
        newMe->setStatus( KCal::Attendee::Declined );
        tmp->clearAttendees();
        tmp->addAttendee( newMe );
        break;
      }
    }

    if ( d->mGroupware ) {
      if ( !d->mGroupware->doNotNotify() && notifyOrganizer ) {
        Akonadi::MailScheduler scheduler( static_cast<Akonadi::Calendar*>(mCalendar) );
        scheduler.performTransaction( tmp.get(), KCal::iTIPReply );
      }
      //reset the doNotNotify flag
      d->mGroupware->setDoNotNotify( false );
    }
  }
  d->mLatestRevisionByItemId.remove( items.first().id() );
  emit incidenceDeleteFinished( items.first(), true );
}

bool IncidenceChanger::cutIncidences( const Item::List &list, QWidget *parent )
{
  Item::List::ConstIterator it;
  bool doDelete = true;
  Item::List itemsToCut;
  for ( it = list.constBegin(); it != list.constEnd(); ++it ) {
    if ( Akonadi::hasIncidence( ( *it ) ) ) {
      doDelete = sendGroupwareMessage( *it, KCal::iTIPCancel,
                                       INCIDENCEDELETED, parent );
      if ( doDelete ) {
        emit incidenceToBeDeleted( *it );
        itemsToCut.append( *it );
      }
    }
   }
  Akonadi::CalendarAdaptor *cal = new Akonadi::CalendarAdaptor( mCalendar, parent );
  Akonadi::DndFactory factory( cal, true /*delete calendarAdaptor*/ );

  if ( factory.cutIncidences( itemsToCut ) ) {
    for ( it = itemsToCut.constBegin(); it != itemsToCut.constEnd(); ++it ) {
      emit incidenceDeleteFinished( *it, true );
    }
    return !itemsToCut.isEmpty();
  } else {
    return false;
  }
}

bool IncidenceChanger::cutIncidence( const Item &item, QWidget *parent )
{
  Item::List items;
  items.append( item );
  return cutIncidences( items, parent );
}

void IncidenceChanger::setDefaultCollectionId( Entity::Id defaultCollectionId )
{
  d->mDefaultCollectionId = defaultCollectionId;
}

namespace {
class YetAnotherComparisonVisitor : public IncidenceBase::Visitor
{
  public:
    YetAnotherComparisonVisitor() {}
    bool act( IncidenceBase *incidence, IncidenceBase *inc2 )
    {
      mIncidence2 = inc2;
      if ( incidence ) {
        return incidence->accept( *this );
      } else {
        return inc2 == 0;
      }
    }

  protected:
    bool visit( Event *event )
    {
      Event *ev2 = dynamic_cast<Event*>( mIncidence2 );
      if ( event && ev2 ) {
        return *event == *ev2;
      } else {
        // either both 0, or return false;
        return ev2 == event;
      }
    }
    bool visit( Todo *todo )
    {
      Todo *to2 = dynamic_cast<Todo*>( mIncidence2 );
      if ( todo && to2 ) {
        return *todo == *to2;
      } else {
        // either both 0, or return false;
        return todo == to2;
      }
    }
    bool visit( Journal *journal )
    {
      Journal *j2 = dynamic_cast<Journal*>( mIncidence2 );
      if ( journal && j2 ) {
        return *journal == *j2;
      } else {
        // either both 0, or return false;
        return journal == j2;
      }
    }
    bool visit( FreeBusy *fb )
    {
      FreeBusy *fb2 = dynamic_cast<FreeBusy*>( mIncidence2 );
      if ( fb && fb2 ) {
        return *fb == *fb2;
      } else {
        // either both 0, or return false;
        return fb2 == fb;
      }
    }

  protected:
    IncidenceBase *mIncidence2;
};
}

bool IncidenceChanger::incidencesEqual( Incidence *inc1, Incidence *inc2 )
{
  YetAnotherComparisonVisitor v;
  return ( v.act( inc1, inc2 ) );
}

bool IncidenceChanger::assignIncidence( Incidence *inc1, Incidence *inc2 )
{
  if ( !inc1 || !inc2 ) {
    return false;
  }
  // PENDING(AKONADI_PORT) review
  AssignmentVisitor v;
  return v.assign( inc1, inc2 );
}

bool IncidenceChanger::changeIncidence( const KCal::Incidence::Ptr &oldinc,
                                        const Item &newItem,
                                        WhatChanged action,
                                        QWidget *parent )
{
  if ( !Akonadi::hasIncidence( newItem ) ||
       !newItem.isValid() ) {
    kDebug() << "Skipping invalid item id=" << newItem.id();
    return false;
  }

  if ( !( newItem.parentCollection().rights() & Collection::CanChangeItem ) ) {
    kWarning() << "insufficient rights to change incidence";
    return false;
  }

  Private::Change *change = new Private::Change();
  change->action = action;
  change->newItem = newItem;
  change->oldInc = oldinc;
  change->parent = parent;

  if ( d->mCurrentChanges.contains( newItem.id() ) ) {
    d->queueChange( change );
  } else {
    d->performChange( change );
  }

  return true;
}


bool IncidenceChanger::addIncidence( const KCal::Incidence::Ptr &incidence,
                                     QWidget *parent, Akonadi::Collection &selectedCollection,
                                     int &dialogCode )
{
  const Collection defaultCollection = mCalendar->collection( d->mDefaultCollectionId );

  const QString incidenceMimeType = Akonadi::subMimeTypeForIncidence( incidence.get() );
  const bool defaultCollSupportsMimeType = defaultCollection.contentMimeTypes().contains( incidenceMimeType );

  if ( d->mDestinationPolicy == ASK_DESTINATION ||
       !defaultCollection.isValid() ||
       !defaultCollSupportsMimeType ) {
    QStringList mimeTypes( incidenceMimeType );
    selectedCollection = Akonadi::selectCollection( parent,
                                                    dialogCode,
                                                    mimeTypes,
                                                    defaultCollection );
  } else {
    dialogCode = QDialog::Accepted;
    selectedCollection = defaultCollection;
  }

  if ( selectedCollection.isValid() ) {
    return addIncidence( incidence, selectedCollection, parent );
  } else {
    return false;
  }
}

bool IncidenceChanger::addIncidence( const Incidence::Ptr &incidence,
                                     const Collection &collection, QWidget *parent )
{
  if ( !incidence || !collection.isValid() ) {
    return false;
  }

  if ( !( collection.rights() & Collection::CanCreateItem ) ) {
    kWarning() << "insufficient rights to create incidence";
    return false;
  }

  Item item;
  item.setPayload<KCal::Incidence::Ptr>( incidence );

  item.setMimeType( Akonadi::subMimeTypeForIncidence( incidence.get() ) );
  ItemCreateJob *job = new ItemCreateJob( item, collection );

  // The connection needs to be queued to be sure addIncidenceFinished is called after the kjob finished
  // it's eventloop. That's needed cause Akonadi::Groupware uses synchron job->exec() calls.
  connect( job, SIGNAL(result(KJob*)),
           this, SLOT(addIncidenceFinished(KJob*)), Qt::QueuedConnection );
  return true;
}

void IncidenceChanger::addIncidenceFinished( KJob* j )
{
  kDebug();
  const Akonadi::ItemCreateJob* job = qobject_cast<const Akonadi::ItemCreateJob*>( j );
  Q_ASSERT( job );
  Incidence::Ptr incidence = Akonadi::incidence( job->item() );

  if  ( job->error() ) {
    KMessageBox::sorry(
      0, //PENDING(AKONADI_PORT) set parent, ideally the one passed in addIncidence...
      i18n( "Unable to save %1 \"%2\": %3",
            i18n( incidence->type() ),
            incidence->summary(),
            job->errorString() ) );
    emit incidenceAddFinished( job->item(), false );
    return;
  }

  Q_ASSERT( incidence );
  if ( KCalPrefs::instance()->mUseGroupwareCommunication ) {
    if ( !d->mGroupware ) {
      kError() << "Groupware communication enabled but no groupware instance set";
    } else if ( !d->mGroupware->sendICalMessage(
           0, //PENDING(AKONADI_PORT) set parent, ideally the one passed in addIncidence...
           KCal::iTIPRequest,
           incidence.get(), INCIDENCEADDED, false ) ) {
      kError() << "sendIcalMessage failed.";
    }
  }

  emit incidenceAddFinished( job->item(), true );
}

void IncidenceChanger::setDestinationPolicy( DestinationPolicy destinationPolicy )
{
  d->mDestinationPolicy = destinationPolicy;
}

IncidenceChanger::DestinationPolicy IncidenceChanger::destinationPolicy() const
{
  return d->mDestinationPolicy;
}

bool IncidenceChanger::isNotDeleted( Akonadi::Item::Id id ) const
{
  if ( mCalendar->incidence( id ).isValid() ) {
    // it's inside the calendar, but maybe it's being deleted by a job or was
    // deleted but the ETM doesn't know yet
    return !d->mDeletedItemIds.contains( id );
  } else {
    // not inside the calendar, i don't know it
    return false;
  }
}