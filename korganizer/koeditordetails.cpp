/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "koeditordetails.h"

#include <qtooltip.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>
#include <qdragobject.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qvaluevector.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#ifndef KORG_NOKABC
#include <kabc/addresseedialog.h>
#include <kabc/vcardconverter.h>
#include <libkdepim/addressesdialog.h>
#include <libkdepim/addresseelineedit.h>
#include <kabc/distributionlist.h>
#include <kabc/stdaddressbook.h>
#endif
#include <libkdepim/kvcarddrag.h>
#include <libkdepim/email.h>
#include <libkdepim/distributionlist.h>

#include <libkcal/incidence.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koeditorfreebusy.h"

#include "kocore.h"

template <>
CustomListViewItem<class Attendee *>::~CustomListViewItem()
{
  delete mData;
}

template <>
void CustomListViewItem<class Attendee *>::updateItem()
{
  setText(0,mData->name());
  setText(1,mData->email());
  setText(2,mData->roleStr());
  setText(3,mData->statusStr());
  if (mData->RSVP() && !mData->email().isEmpty())
    setPixmap(4,KOGlobals::self()->smallIcon("mailappt"));
  else
    setPixmap(4,KOGlobals::self()->smallIcon("nomailappt"));
}

KOAttendeeListView::KOAttendeeListView ( QWidget *parent, const char *name )
  : KListView(parent, name)
{
  setAcceptDrops( true );
  setAllColumnsShowFocus( true );
  setSorting( -1 );
}

/** KOAttendeeListView is a child class of KListView  which supports
 *  dropping of attendees (e.g. from kaddressbook) onto it. If an attendeee
 *  was dropped, the signal dropped(Attendee*)  is emitted. Valid drop classes
 *   are KVCardDrag and QTextDrag.
 */
KOAttendeeListView::~KOAttendeeListView()
{
}

void KOAttendeeListView::contentsDragEnterEvent( QDragEnterEvent *e )
{
  dragEnterEvent(e);
}

void KOAttendeeListView::contentsDragMoveEvent( QDragMoveEvent *e )
{
#ifndef KORG_NODND
  if ( KVCardDrag::canDecode( e ) || QTextDrag::canDecode( e ) ) {
    e->accept();
  } else {
    e->ignore();
  }
#endif
}

void KOAttendeeListView::dragEnterEvent( QDragEnterEvent *e )
{
#ifndef KORG_NODND
  if ( KVCardDrag::canDecode( e ) || QTextDrag::canDecode( e ) ) {
    e->accept();
  } else {
    e->ignore();
  }
#endif
}

void KOAttendeeListView::addAttendee( const QString &newAttendee )
{
  kdDebug(5850) << " Email: " << newAttendee << endl;
  QString name;
  QString email;
  KPIM::getNameAndMail( newAttendee, name, email );
  emit dropped( new Attendee( name, email, true ) );
}

void KOAttendeeListView::contentsDropEvent( QDropEvent *e )
{
  dropEvent(e);
}

void KOAttendeeListView::dropEvent( QDropEvent *e )
{
#ifndef KORG_NODND
  QString text;
  QString vcards;

#ifndef KORG_NOKABC
  if ( KVCardDrag::decode( e, vcards ) ) {
    KABC::VCardConverter converter;

    KABC::Addressee::List list = converter.parseVCards( vcards );
    KABC::Addressee::List::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      QString em( (*it).fullEmail() );
      if (em.isEmpty()) {
        em=(*it).realName();
      }
      addAttendee( em );
    }
  } else
#endif // KORG_NOKABC
  if (QTextDrag::decode(e,text)) {
    kdDebug(5850) << "Dropped : " << text << endl;
    QStringList emails = QStringList::split(",",text);
    for(QStringList::ConstIterator it = emails.begin();it!=emails.end();++it) {
      addAttendee(*it);
    }
  }
#endif //KORG_NODND
}


KOEditorDetails::KOEditorDetails( int spacing, QWidget *parent,
                                  const char *name )
  : QWidget( parent, name), mDisableItemUpdate( false ), mFreeBusy( 0 )
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( spacing );

  mOrganizerHBox = new QHBox( this );
  // If creating a new event, then the user is the organizer -> show the
  // identity combo
  // readEvent will delete it and set another label text instead, if the user
  // isn't the organizer.
  // Note that the i18n text below is duplicated in readEvent
  mOrganizerLabel = new QLabel( i18n( "Identity as organizer:" ),
                                mOrganizerHBox );
  mOrganizerCombo = new QComboBox( mOrganizerHBox );
  fillOrganizerCombo();
  mOrganizerHBox->setStretchFactor( mOrganizerCombo, 100 );

  mListView = new KOAttendeeListView( this, "mListView" );
  mListView->addColumn( i18n("Name"), 200 );
  mListView->addColumn( i18n("Email"), 200 );
  mListView->addColumn( i18n("Role"), 60 );
  mListView->addColumn( i18n("Status"), 100 );
  mListView->addColumn( i18n("RSVP"), 35 );
  mListView->setResizeMode( QListView::LastColumn );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    mListView->setFixedHeight( 78 );
  }

  connect( mListView, SIGNAL( selectionChanged( QListViewItem * ) ),
           SLOT( updateAttendeeInput() ) );
#ifndef KORG_NODND
  connect( mListView, SIGNAL( dropped( Attendee * ) ),
           SLOT( insertAttendee( Attendee * ) ) );
#endif

  QLabel *attendeeLabel = new QLabel( this );
  attendeeLabel->setText( i18n("Na&me:") );

  mNameEdit = new KPIM::AddresseeLineEdit( this );
  mNameEdit->setClickMessage( i18n("Click to add a new attendee") );
  attendeeLabel->setBuddy( mNameEdit );
  mNameEdit->installEventFilter( this );
  connect( mNameEdit, SIGNAL( textChanged( const QString & ) ),
           SLOT( updateAttendeeItem() ) );

  mUidEdit = new QLineEdit( 0 );
  mUidEdit->setText( "" );

  QLabel *attendeeRoleLabel = new QLabel( this );
  attendeeRoleLabel->setText( i18n("Ro&le:") );

  mRoleCombo = new QComboBox( false, this );
  mRoleCombo->insertStringList( Attendee::roleList() );
  attendeeRoleLabel->setBuddy( mRoleCombo );
  connect( mRoleCombo, SIGNAL( activated( int ) ),
           SLOT( updateAttendeeItem() ) );

  QLabel *statusLabel = new QLabel( this );
  statusLabel->setText( i18n("Stat&us:") );

  mStatusCombo = new QComboBox( false, this );
  mStatusCombo->insertStringList( Attendee::statusList() );
  statusLabel->setBuddy( mStatusCombo );
  connect( mStatusCombo, SIGNAL( activated( int ) ),
           SLOT( updateAttendeeItem() ) );

  mRsvpButton = new QCheckBox( this );
  mRsvpButton->setText( i18n("Re&quest response") );
  connect( mRsvpButton, SIGNAL( clicked() ), SLOT( updateAttendeeItem() ) );

  QWidget *buttonBox = new QWidget( this );
  QVBoxLayout *buttonLayout = new QVBoxLayout( buttonBox );

  QPushButton *newButton = new QPushButton( i18n("&New"), buttonBox );
  buttonLayout->addWidget( newButton );
  connect( newButton, SIGNAL( clicked() ), SLOT( addNewAttendee() ) );

  mRemoveButton = new QPushButton( i18n("&Remove"), buttonBox );
  buttonLayout->addWidget( mRemoveButton );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeAttendee() ) );

  mAddressBookButton = new QPushButton( i18n("Select Addressee..."),
                                        buttonBox );
  buttonLayout->addWidget( mAddressBookButton );
  connect( mAddressBookButton, SIGNAL( clicked() ), SLOT( openAddressBook() ) );

  topLayout->addMultiCellWidget( mOrganizerHBox, 0, 0, 0, 5 );
  topLayout->addMultiCellWidget( mListView, 1, 1, 0, 5 );
  topLayout->addWidget( attendeeLabel, 2, 0 );
  topLayout->addMultiCellWidget( mNameEdit, 2, 2, 1, 1 );
//  topLayout->addWidget( emailLabel, 3, 0 );
  topLayout->addWidget( attendeeRoleLabel, 3, 0 );
  topLayout->addWidget( mRoleCombo, 3, 1 );
#if 0
  topLayout->setColStretch( 2, 1 );
  topLayout->addWidget( statusLabel, 3, 3 );
  topLayout->addWidget( mStatusCombo, 3, 4 );
#else
  topLayout->addWidget( statusLabel, 4, 0 );
  topLayout->addWidget( mStatusCombo, 4, 1 );
#endif
  topLayout->addMultiCellWidget( mRsvpButton, 5, 5, 0, 1 );
  topLayout->addMultiCellWidget( buttonBox, 2, 4, 5, 5 );

#ifdef KORG_NOKABC
  mAddressBookButton->hide();
#endif

  updateAttendeeInput();
}

KOEditorDetails::~KOEditorDetails()
{
}

bool KOEditorDetails::eventFilter( QObject *watched, QEvent *ev)
{
  if ( watched && watched == mNameEdit && ev->type() == QEvent::FocusIn &&
       mListView->childCount() == 0 ) {
    addNewAttendee();
  }

  return QWidget::eventFilter( watched, ev );
}

void KOEditorDetails::removeAttendee()
{
  AttendeeListItem *aItem =
      static_cast<AttendeeListItem *>( mListView->selectedItem() );
  if ( !aItem ) return;

  Attendee *delA = new Attendee( aItem->data()->name(), aItem->data()->email(),
                                 aItem->data()->RSVP(), aItem->data()->status(),
                                 aItem->data()->role(), aItem->data()->uid() );
  mdelAttendees.append( delA );

  if ( mFreeBusy ) mFreeBusy->removeAttendee( aItem->data() );
  delete aItem;

  updateAttendeeInput();
}


void KOEditorDetails::openAddressBook()
{
#ifndef KORG_NOKABC
  KPIM::AddressesDialog *dia = new KPIM::AddressesDialog( this, "adddialog" );
  dia->setShowCC( false );
  dia->setShowBCC( false );
  if ( dia->exec() ) {
    KABC::Addressee::List aList = dia->allToAddressesNoDuplicates();
    for ( KABC::Addressee::List::iterator itr = aList.begin();
          itr != aList.end(); ++itr ) {
      insertAttendeeFromAddressee( (*itr) );
    }
  }
  delete dia;
  return;
#if 0
    // old code
    KABC::Addressee a = KABC::AddresseeDialog::getAddressee(this);
    if (!a.isEmpty()) {
        // If this is myself, I don't want to get a response but instead
        // assume I will be available
        bool myself = KOPrefs::instance()->thatIsMe( a.preferredEmail() );
        KCal::Attendee::PartStat partStat =
            myself ? KCal::Attendee::Accepted : KCal::Attendee::NeedsAction;
        insertAttendee( new Attendee( a.realName(), a.preferredEmail(),
                                      !myself, partStat,
                                      KCal::Attendee::ReqParticipant, a.uid() ) );
    }
#endif
#endif
}


void KOEditorDetails::addNewAttendee()
{
  Attendee *a = new Attendee( i18n("Firstname Lastname"),
                              i18n("name@domain.com"), true );
  insertAttendee( a, false );
  // We don't want the hint again
  mNameEdit->setClickMessage( "" );
  mNameEdit->setFocus();
  QTimer::singleShot( 0, mNameEdit, SLOT( selectAll() ) );
}


void KOEditorDetails::insertAttendee( Attendee *a )
{
  insertAttendee( a, true );
}

void KOEditorDetails::insertAttendee( Attendee *a, bool goodEmailAddress )
{
  // lastItem() is O(n), but for n very small that should be fine
  AttendeeListItem *item = new AttendeeListItem( a, mListView,
      static_cast<KListViewItem*>( mListView->lastItem() ) );
  mListView->setSelected( item, true );
  if( mFreeBusy ) mFreeBusy->insertAttendee( a, goodEmailAddress );
}

void KOEditorDetails::setDefaults()
{
  mRsvpButton->setChecked( true );
}

void KOEditorDetails::readEvent( Incidence *event )
{
  // Stop flickering in the free/busy view (not sure if this is necessary)
  bool block = false;
  if( mFreeBusy ) {
    block = mFreeBusy->updateEnabled();
    mFreeBusy->setUpdateEnabled( false );
    mFreeBusy->clearAttendees();
  }

  mListView->clear();
  mdelAttendees.clear();
  Attendee::List al = event->attendees();
  Attendee::List::ConstIterator it;
  for( it = al.begin(); it != al.end(); ++it )
    insertAttendee( new Attendee( **it ), true );

  mListView->setSelected( mListView->firstChild(), true );

  if ( KOPrefs::instance()->thatIsMe( event->organizer().email() ) ) {
    if ( !mOrganizerCombo ) {
      mOrganizerCombo = new QComboBox( mOrganizerHBox );
      fillOrganizerCombo();
    }
    mOrganizerLabel->setText( i18n( "Identity as organizer:" ) );

    // This might not be enough, if the combo as a full name too, hence the loop below
    // mOrganizerCombo->setCurrentText( event->organizer().fullName() );
    for ( int i = 0 ; i < mOrganizerCombo->count(); ++i ) {
      QString itemTxt = KPIM::getEmailAddr( mOrganizerCombo->text( i ) );
      if ( KPIM::compareEmail( event->organizer().email(), itemTxt, false ) ) {
        // Make sure we match the organizer setting completely
        mOrganizerCombo->changeItem( event->organizer().fullName(), i );
        mOrganizerCombo->setCurrentItem( i );
        break;
      }
    }
  } else { // someone else is the organizer
    if ( mOrganizerCombo ) {
      delete mOrganizerCombo;
      mOrganizerCombo = 0;
    }
    mOrganizerLabel->setText( i18n( "Organizer: %1" ).arg( event->organizer().fullName() ) );
  }

  // Reinstate free/busy view updates
  if( mFreeBusy ) mFreeBusy->setUpdateEnabled( block );
}

void KOEditorDetails::writeEvent(Incidence *event)
{
  event->clearAttendees();
  QValueVector<QListViewItem*> toBeDeleted;
  QListViewItem *item;
  AttendeeListItem *a;
  for (item = mListView->firstChild(); item;
       item = item->nextSibling()) {
    a = (AttendeeListItem *)item;
    Attendee *attendee = a->data();
    Q_ASSERT( attendee );
    /* Check if the attendee is a distribution list and expand it */
    if ( attendee->email().isEmpty() ) {
      KPIM::DistributionList list = 
        KPIM::DistributionList::findByName( KABC::StdAddressBook::self(), attendee->name() );
      if ( !list.isEmpty() ) {
        toBeDeleted.push_back( item ); // remove it once we are done expanding
        KPIM::DistributionList::Entry::List entries = list.entries( KABC::StdAddressBook::self() );
        KPIM::DistributionList::Entry::List::Iterator it( entries.begin() );
        while ( it != entries.end() ) {
          KPIM::DistributionList::Entry &e = ( *it );
          ++it;
          // this calls insertAttendee, which appends
          insertAttendeeFromAddressee( e.addressee ); 
          // TODO: duplicate check, in case it was already added manually
        }
      }
    } else {
      event->addAttendee( new Attendee( *attendee ) );
    }
  }
  if ( mOrganizerCombo ) {
    // TODO: Don't take a string and split it up... Is there a better way?
    event->setOrganizer( mOrganizerCombo->currentText() );
  }
  // cleanup
  QValueVector<QListViewItem*>::iterator it;
  for( it = toBeDeleted.begin(); it != toBeDeleted.end(); ++it ) {
    delete *it;
  }
}

void KOEditorDetails::cancelAttendeeEvent(Incidence *event)
{
  event->clearAttendees();
  Attendee * att;
  for (att=mdelAttendees.first();att;att=mdelAttendees.next()) {
    event->addAttendee(new Attendee(*att));
  }
  mdelAttendees.clear();
}

bool KOEditorDetails::validateInput()
{
  return true;
}

void KOEditorDetails::updateAttendeeInput()
{

  setEnableAttendeeInput(!mNameEdit->text().isEmpty());
  QListViewItem *item = mListView->selectedItem();
  AttendeeListItem *aItem = static_cast<AttendeeListItem *>( item );
  if (aItem) {
    fillAttendeeInput( aItem );
  } else {
    clearAttendeeInput();
  }
}

void KOEditorDetails::clearAttendeeInput()
{
  mNameEdit->setText("");
  mUidEdit->setText("");
  mRoleCombo->setCurrentItem(0);
  mStatusCombo->setCurrentItem(0);
  mRsvpButton->setChecked(true);
  setEnableAttendeeInput( false );
}

void KOEditorDetails::fillAttendeeInput( AttendeeListItem *aItem )
{
  Attendee *a = aItem->data();
  mDisableItemUpdate = true;
  QString name = a->name();
  if (!a->email().isEmpty()) {
    // Taken from KABC::Addressee::fullEmail
    QRegExp needQuotes( "[^ 0-9A-Za-z\\x0080-\\xFFFF]" );
    if ( name.find( needQuotes ) != -1 )
      name = "\"" + name + "\" <" + a->email() + ">";
    else
      name += " <" + a->email() + ">";
  }
  mNameEdit->setText(name);
  mUidEdit->setText(a->uid());
  mRoleCombo->setCurrentItem(a->role());
  mStatusCombo->setCurrentItem(a->status());
  mRsvpButton->setChecked(a->RSVP());

  mDisableItemUpdate = false;

  setEnableAttendeeInput( true );
}

void KOEditorDetails::setEnableAttendeeInput( bool enabled )
{
  //mNameEdit->setEnabled( enabled );
  mRoleCombo->setEnabled( enabled );
  mStatusCombo->setEnabled( enabled );
  mRsvpButton->setEnabled( enabled );

  mRemoveButton->setEnabled( enabled );
}

void KOEditorDetails::updateAttendeeItem()
{
  if (mDisableItemUpdate) return;

  QListViewItem *item = mListView->selectedItem();
  AttendeeListItem *aItem = static_cast<AttendeeListItem *>( item );
  if ( !aItem ) return;

  Attendee *a = aItem->data();
  QString name;
  QString email;
  KPIM::getNameAndMail(mNameEdit->text(), name, email);

  bool iAmTheOrganizer = mOrganizerCombo &&
    KOPrefs::instance()->thatIsMe( mOrganizerCombo->currentText() );
  if ( iAmTheOrganizer ) {
    bool myself =
      KPIM::compareEmail( email, mOrganizerCombo->currentText(), false );
    bool wasMyself =
      KPIM::compareEmail( a->email(), mOrganizerCombo->currentText(), false );
    if ( myself ) {
      mStatusCombo->setCurrentItem( KCal::Attendee::Accepted );
      mRsvpButton->setChecked( false );
      mRsvpButton->setEnabled( false );
    } else if ( wasMyself ) {
      // this was me, but is no longer, reset
      mStatusCombo->setCurrentItem( KCal::Attendee::NeedsAction );
      mRsvpButton->setChecked( true );
      mRsvpButton->setEnabled( true );
    }
  }
  a->setName( name );
  a->setUid( mUidEdit->text() );
  a->setEmail( email );
  a->setRole( Attendee::Role( mRoleCombo->currentItem() ) );
  a->setStatus( Attendee::PartStat( mStatusCombo->currentItem() ) );
  a->setRSVP( mRsvpButton->isChecked() );
  aItem->updateItem();
  if ( mFreeBusy ) mFreeBusy->updateAttendee( a );
}

void KOEditorDetails::setFreeBusyWidget( KOEditorFreeBusy *v )
{
  mFreeBusy = v;
}

void KOEditorDetails::fillOrganizerCombo()
{
  Q_ASSERT( mOrganizerCombo );
  // Get all emails from KOPrefs (coming from various places),
  // and insert them - removing duplicates
  const QStringList lst = KOPrefs::instance()->fullEmails();
  QStringList uniqueList;
  for( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    if ( uniqueList.find( *it ) == uniqueList.end() )
      uniqueList << *it;
  }
  mOrganizerCombo->insertStringList( uniqueList );
}

void KOEditorDetails::insertAttendeeFromAddressee( const KABC::Addressee& a )
{
  bool myself = KOPrefs::instance()->thatIsMe( a.preferredEmail() );
  bool sameAsOrganizer = mOrganizerCombo &&
    KPIM::compareEmail( a.preferredEmail(), mOrganizerCombo->currentText(), false );
  KCal::Attendee::PartStat partStat;
  if ( myself && sameAsOrganizer )
    partStat = KCal::Attendee::Accepted;
  else
    partStat = KCal::Attendee::NeedsAction;
  Attendee *at = new Attendee( a.realName(),
                               a.preferredEmail(),
                               !myself, partStat,
                               KCal::Attendee::ReqParticipant,
                               a.uid() );
  at->setRSVP( partStat != KCal::Attendee::Accepted );
  insertAttendee( at, true );
}
#include "koeditordetails.moc"
