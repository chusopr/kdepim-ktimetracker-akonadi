/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "editordetails.h"
#include "editorconfig.h"

#include <Akonadi/Contact/ContactGroupExpandJob>
#include <Akonadi/Contact/ContactGroupSearchJob>

#include <KABC/VCardDrag>

#include <kcalutils/stringify.h>
#include <kcalcore/incidence.h>

#include <KPIMUtils/Email>

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include <QCheckBox>
#include <QMimeData>
#include <QPushButton>
#include <QVBoxLayout>
#ifndef KORG_NODND
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#endif

using namespace KCalCore;
using namespace KCalUtils;

namespace IncidenceEditors {
template <>
CustomListViewItem<KCalCore::Attendee::Ptr >::~CustomListViewItem()
{
}

template <>
void CustomListViewItem<KCalCore::Attendee::Ptr >::updateItem()
{
  setText( 0, mData->name() );
  setText( 1, mData->email() );
  setText( 2, Stringify::attendeeRole( mData->role() ) );
  setText( 3, Stringify::attendeeStatus( mData->status() ) );
  if ( mData->RSVP() && !mData->email().isEmpty() ) {
    setPixmap( 4, SmallIcon( "mail-flag" ) );
  } else {
    setPixmap( 4, SmallIcon( "mail-queue" ) );
  }

  setText( 5, mData->delegate() );
  setText( 6, mData->delegator() );
}

}

using namespace IncidenceEditors;

AttendeeListView::AttendeeListView ( QWidget *parent )
  : K3ListView( parent )
{
  setAcceptDrops( true );
  setAllColumnsShowFocus( true );
  setSorting( -1 );
}

/** AttendeeListView is a child class of K3ListView  which supports
 *  dropping of attendees (e.g. from kcontactmanager) onto it. If an attendee
 *  was dropped, the signal dropped(Attendee*)  is emitted.
 */
AttendeeListView::~AttendeeListView()
{
}

#ifndef KORG_NODND
void AttendeeListView::contentsDragEnterEvent( QDragEnterEvent *e )
{
  dragEnterEvent( e );
}
#endif

#ifndef KORG_NODND
void AttendeeListView::contentsDragMoveEvent( QDragMoveEvent *e )
{
  const QMimeData *md = e->mimeData();
  if ( KABC::VCardDrag::canDecode( md ) || md->hasText() ) {
    e->accept();
  } else {
    e->ignore();
  }
}
#endif

#ifndef KORG_NODND
void AttendeeListView::dragEnterEvent( QDragEnterEvent *e )
{
  const QMimeData *md = e->mimeData();
  if ( KABC::VCardDrag::canDecode( md ) || md->hasText() ) {
    e->accept();
  } else {
    e->ignore();
  }
}
#endif

void AttendeeListView::addAttendee( const QString &newAttendee )
{
  kDebug() << " Email:" << newAttendee;
  QString name;
  QString email;
  KPIMUtils::extractEmailAddressAndName( newAttendee, email, name );
#ifndef KORG_NODND
  emit dropped( Attendee::Ptr( new Attendee( name, email, true ) ) );
#endif
}

#ifndef KORG_NODND
void AttendeeListView::contentsDropEvent( QDropEvent *e )
{
  dropEvent( e );
}
#endif

#ifndef KORG_NODND
void AttendeeListView::dropEvent( QDropEvent *e )
{
  const QMimeData *md = e->mimeData();

  if ( KABC::VCardDrag::canDecode( md ) ) {
    KABC::Addressee::List list;
    KABC::VCardDrag::fromMimeData( md, list );

    KABC::Addressee::List::ConstIterator it;
    for ( it = list.constBegin(); it != list.constEnd(); ++it ) {
      QString em( (*it).fullEmail() );
      if ( em.isEmpty() ) {
        em = (*it).realName();
      }
      addAttendee( em );
    }
  }

  if ( md->hasText() ) {
    QString text = md->text();
    kDebug() << "Dropped :" << text;
    QStringList emails = text.split( ',', QString::SkipEmptyParts );
    for ( QStringList::ConstIterator it = emails.constBegin(); it != emails.constEnd(); ++it ) {
      addAttendee( *it );
    }
  }
}
#endif

EditorDetails::EditorDetails( int spacing, QWidget *parent )
  : AttendeeEditor( parent ), mDisableItemUpdate( false )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  initOrganizerWidgets( this, topLayout );

  mListView = new AttendeeListView( this );
  mListView->setObjectName( "mListView" );
  mListView->setWhatsThis( i18nc( "@info:whatsthis",
                                  "Displays information about current attendees. "
                                  "To edit an attendee, select it in this list "
                                  "and modify the values in the area below. "
                                  "Clicking on a column title will sort the list "
                                  "according to that column. The RSVP column "
                                  "indicates whether or not a response is "
                                  "requested from the attendee." ) );
  mListView->addColumn( i18nc( "@title:column attendee name", "Name" ), 200 );
  mListView->addColumn( i18nc( "@title:column attendee email", "Email" ), 200 );
  mListView->addColumn( i18nc( "@title:column attendee role", "Role" ), 80 );
  mListView->addColumn( i18nc( "@title:column attendee status", "Status" ), 100 );
  mListView->addColumn( i18nc( "@title:column attendee has RSVPed?", "RSVP" ), 55 );
  mListView->addColumn( i18nc( "@title:column attendee delegated to", "Delegated To" ), 120 );
  mListView->addColumn( i18nc( "@title:column attendee delegated from", "Delegated From" ), 120 );
  mListView->setResizeMode( Q3ListView::LastColumn );

  connect( mListView, SIGNAL(selectionChanged(Q3ListViewItem*)),
           SLOT(updateAttendeeInput()) );
#ifndef KORG_NODND
  connect( mListView, SIGNAL( dropped( KCalCore::Attendee::Ptr  ) ),
           SLOT( slotInsertAttendee( KCalCore::Attendee::Ptr ) ) );
#endif
  topLayout->addWidget( mListView );

  initEditWidgets( this, topLayout );

  connect( mRemoveButton, SIGNAL(clicked()), SLOT(removeAttendee()) );

  updateAttendeeInput();
}

EditorDetails::~EditorDetails()
{
}

bool EditorDetails::hasAttendees()
{
  return mListView->childCount() > 0;
}

void EditorDetails::removeAttendee()
{
  AttendeeListItem *aItem = static_cast<AttendeeListItem *>( mListView->selectedItem() );
  if ( !aItem ) {
    return;
  }

  AttendeeListItem *nextSelectedItem = static_cast<AttendeeListItem*>( aItem->nextSibling() );
  if ( mListView->childCount() == 1 ) {
    nextSelectedItem = 0;
  }
  if ( mListView->childCount() > 1 && aItem == mListView->lastItem() ) {
    nextSelectedItem = static_cast<AttendeeListItem*>( mListView->firstChild() );
  }

  Attendee::Ptr delA = Attendee::Ptr( new Attendee( aItem->data()->name(), aItem->data()->email(),
                                                    aItem->data()->RSVP(), aItem->data()->status(),
                                                    aItem->data()->role(), aItem->data()->uid() ) );
  mDelAttendees.append( delA );

  delete aItem;

  if ( nextSelectedItem ) {
    mListView->setSelected( nextSelectedItem, true );
  }
  updateAttendeeInput();
  emit updateAttendeeSummary( mListView->childCount() );
}

void EditorDetails::insertAttendee( const Attendee::Ptr &a, bool goodEmailAddress )
{
  Q_UNUSED( goodEmailAddress );

  // lastItem() is O(n), but for n very small that should be fine
  AttendeeListItem *item = new AttendeeListItem( a, mListView,
      static_cast<K3ListViewItem*>( mListView->lastItem() ) );
  mListView->setSelected( item, true );
  emit updateAttendeeSummary( mListView->childCount() );
}

void EditorDetails::setDefaults()
{
  mRsvpButton->setChecked( true );
}

void EditorDetails::readIncidence( const Incidence::Ptr &event )
{
  mListView->clear();
  AttendeeEditor::readIncidence( event );

  mListView->setSelected( mListView->firstChild(), true );

  emit updateAttendeeSummary( mListView->childCount() );
}

void EditorDetails::fillIncidence( Incidence::Ptr &incidence )
{
  incidence->clearAttendees();
  QVector<Q3ListViewItem*> toBeDeleted;
  Q3ListViewItem *item;
  AttendeeListItem *a;
  for ( item = mListView->firstChild(); item; item = item->nextSibling() ) {
    a = (AttendeeListItem *)item;
    Attendee::Ptr attendee = a->data();
    Q_ASSERT( attendee );
    /* Check if the attendee is a distribution list and expand it */
    if ( attendee->email().isEmpty() ) {
      Akonadi::ContactGroupSearchJob *job = new Akonadi::ContactGroupSearchJob();
      job->setQuery( Akonadi::ContactGroupSearchJob::Name, attendee->name() );
      job->exec();

      const KABC::ContactGroup::List groups = job->contactGroups();
      if ( !groups.isEmpty() ) {
        toBeDeleted.push_back( item ); // remove it once we are done expanding
        Akonadi::ContactGroupExpandJob *expandJob =
          new Akonadi::ContactGroupExpandJob( groups.first() );
        expandJob->exec();

        const KABC::Addressee::List contacts = expandJob->contacts();
        foreach ( const KABC::Addressee &contact, contacts ) {
          // this calls insertAttendee, which appends
          insertAttendeeFromAddressee( contact, attendee );
          // TODO: duplicate check, in case it was already added manually
        }
      }
    } else {
      bool skip = false;
      if ( attendee->email().endsWith( QLatin1String( "example.net" ) ) ) {
        if ( KMessageBox::warningYesNo(
              this,
              i18nc( "@info",
                "%1 does not look like a valid email address. "
                "Are you sure you want to invite this participant?",
                attendee->email() ),
              i18nc( "@title", "Invalid Email Address" ) ) != KMessageBox::Yes ) {
          skip = true;
        }
      }
      if ( !skip ) {
        incidence->addAttendee( Attendee::Ptr( new Attendee( *attendee ) ) );
      }
    }
  }

  AttendeeEditor::fillIncidence( incidence );

  // cleanup
  qDeleteAll( toBeDeleted );
  toBeDeleted.clear();
}

bool EditorDetails::validateInput()
{
  return true;
}

KCalCore::Attendee::Ptr EditorDetails::currentAttendee() const
{
  Q3ListViewItem *item = mListView->selectedItem();
  AttendeeListItem *aItem = static_cast<AttendeeListItem *>( item );
  if ( !aItem ) {
    return Attendee::Ptr();
  }
  return aItem->data();
}

void EditorDetails::updateCurrentItem()
{
  AttendeeListItem *item = static_cast<AttendeeListItem*>( mListView->selectedItem() );
  if ( item ) {
    item->updateItem();
  }
}

void EditorDetails::slotInsertAttendee( const Attendee::Ptr &a )
{
  insertAttendee( a );
  mNewAttendees.append( a );
}

void EditorDetails::changeStatusForMe( Attendee::PartStat status )
{
  const QStringList myEmails = EditorConfig::instance()->allEmails();
  for ( Q3ListViewItemIterator it( mListView ); it.current(); ++it ) {
    AttendeeListItem *item = static_cast<AttendeeListItem*>( it.current() );
    for ( QStringList::ConstIterator it2( myEmails.begin() ), end( myEmails.end() );
          it2 != end; ++it2 ) {
      if ( item->data()->email() == *it2 ) {
        item->data()->setStatus( status );
        item->updateItem();
      }
    }
  }
}

Q3ListViewItem *EditorDetails::hasExampleAttendee() const
{
  for ( Q3ListViewItemIterator it( mListView ); it.current(); ++it ) {
    AttendeeListItem *item = static_cast<AttendeeListItem*>( it.current() );
    Attendee::Ptr attendee = item->data();
    Q_ASSERT( attendee );
    if ( isExampleAttendee( attendee ) ) {
      return item;
    }
  }
  return 0;
}

#include "editordetails.moc"
