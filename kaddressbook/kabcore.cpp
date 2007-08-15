/*
    This file is part of KAddressbook.
    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QLayout>
#include <QRegExp>
#include <q3vbox.h>
#include <QSplitter>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QList>
#include <QHBoxLayout>

#include <kabc/addresseelist.h>
#include <kabc/errorhandler.h>
#include <kabc/resource.h>
#include <kabc/stdaddressbook.h>
#include <kabc/vcardconverter.h>
#include <kaboutdata.h>
#include <kacceleratormanager.h>
#include <kapplication.h>

#include <kcmdlineargs.h>
#include <kcmultidialog.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kimproxy.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <kprotocolinfo.h>
#include <kresources/selectdialog.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <KStandardGuiItem>
#include <kundostack.h>
#include <kxmlguiclient.h>
#include <ktoolbar.h>
#include <libkdepim/addresseeview.h>
#include <libkdepim/categoryeditdialog.h>
#include <libkdepim/categoryselectdialog.h>
#include <ktoolinvocation.h>
#include <krandom.h>
#include <QtDBus>
#include <ktoggleaction.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kicon.h>

#include "addresseeutil.h"
#include "addresseeeditordialog.h"
#include "extensionmanager.h"
#include "filterselectionwidget.h"
#include "incsearchwidget.h"
#include "jumpbuttonbar.h"
#include "kablock.h"
#include "kabprefs.h"
#include "kabtools.h"
#include "kaddressbookservice.h"
#include "ldapsearchdialog.h"
#include "locationmap.h"
#include "printing/printingwizard.h"
#include "searchmanager.h"
#include "undocmds.h"
#include "viewmanager.h"
#include "xxportmanager.h"

#include "kabcore.h"
#include "kaddressbookcore_interface.h"

KABCore::KABCore( KXMLGUIClient *client, bool readWrite, QWidget *parent,
                  const QString &file, const char *name )
  : KAB::Core( client, parent, name ), mStatusBar( 0 ), mViewManager( 0 ),
    mExtensionManager( 0 ), mJumpButtonBar( 0 ), mCategorySelectDialog( 0 ),
    mCategoryEditDialog( 0 ), mLdapSearchDialog( 0 ), mReadWrite( readWrite ),
    mModified( false )
{
  mWidget = new QWidget( parent );
  mWidget->setObjectName( name );
//
  mIsPart = (strcmp(parent->metaObject()->className(), "KAddressBookMain") );

  mAddressBookChangedTimer = new QTimer( this );
  connect( mAddressBookChangedTimer, SIGNAL( timeout() ),
           this, SLOT( addressBookChanged() ) );

  if ( file.isEmpty() ) {
    mAddressBook = KABC::StdAddressBook::self( true );
  } else {
    kDebug(5720) <<"KABCore(): document '" << file <<"'";
    mAddressBook = KABC::StdAddressBook::self( true );
    /*
    mAddressBook = new KABC::AddressBook;
    mAddressBook->addResource( new KABC::ResourceFile( file ) );
    if ( !mAddressBook->load() ) {
      KMessageBox::error( parent, i18n( "Unable to load '%1'.", file ) );
    }
    */
  }
  mAddressBook->setErrorHandler( new KABC::GuiErrorHandler( mWidget ) );

  mAddressBook->addCustomField( i18n( "Department" ), KABC::Field::Organization,
                                "X-Department", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Profession" ), KABC::Field::Organization,
                                "X-Profession", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Assistant's Name" ), KABC::Field::Organization,
                                "X-AssistantsName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Manager's Name" ), KABC::Field::Organization,
                                "X-ManagersName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Partner's Name" ), KABC::Field::Personal,
                                "X-SpousesName", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Office" ), KABC::Field::Personal,
                                "X-Office", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "IM Address" ), KABC::Field::Personal,
                                "X-IMAddress", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Anniversary" ), KABC::Field::Personal,
                                "X-Anniversary", "KADDRESSBOOK" );
  mAddressBook->addCustomField( i18n( "Blog" ), KABC::Field::Personal,
                                "BlogFeed", "KADDRESSBOOK" );

  mSearchManager = new KAB::SearchManager( mAddressBook, parent );

  connect( mSearchManager, SIGNAL( contactsUpdated() ),
           this, SLOT( slotContactsUpdated() ) );

  initGUI();

  connect( mAddressBook, SIGNAL( addressBookChanged( AddressBook* ) ),
           SLOT( delayedAddressBookChanged() ) );
  connect( mAddressBook, SIGNAL( loadingFinished( Resource* ) ),
           SLOT( delayedAddressBookChanged() ) );

  mIncSearchWidget->setFocus();

  connect( mViewManager, SIGNAL( selected( const QString& ) ),
           SLOT( setContactSelected( const QString& ) ) );
  connect( mViewManager, SIGNAL( executed( const QString& ) ),
           SLOT( editContact( const QString& ) ) );
  connect( mViewManager, SIGNAL( modified() ),
           SLOT( setModified() ) );
  connect( mViewManager, SIGNAL( urlDropped( const KUrl& ) ),
           mXXPortManager, SLOT( importVCard( const KUrl& ) ) );
  connect( mViewManager, SIGNAL( viewFieldsChanged() ),
           SLOT( updateIncSearchWidget() ) );
  connect( mExtensionManager, SIGNAL( modified( const KABC::Addressee::List& ) ),
           this, SLOT( extensionModified( const KABC::Addressee::List& ) ) );
  connect( mExtensionManager, SIGNAL( deleted( const QStringList& ) ),
           this, SLOT( extensionDeleted( const QStringList& ) ) );

  connect( mXXPortManager, SIGNAL( modified() ),
           SLOT( setModified() ) );

  connect( mDetails, SIGNAL( highlightedMessage( const QString& ) ),
           SLOT( detailsHighlighted( const QString& ) ) );

  connect( mIncSearchWidget, SIGNAL( scrollUp() ),
           mViewManager, SLOT( scrollUp() ) );
  connect( mIncSearchWidget, SIGNAL( scrollDown() ),
           mViewManager, SLOT( scrollDown() ) );

  mAddressBookService = new KAddressBookService( this );

  mCommandHistory = new KUndoStack( this );
  mCommandHistory->createUndoAction( actionCollection() );
  mCommandHistory->createRedoAction( actionCollection() );
  connect( mCommandHistory, SIGNAL( indexChanged( int ) ),
           mSearchManager, SLOT( reload() ) );

  mSearchManager->reload();

  setModified( false );

  KAcceleratorManager::manage( mWidget );

  mKIMProxy = ::KIMProxy::instance();
}

KABCore::~KABCore()
{
  mAddressBook->disconnect();

  mAddressBook = 0;
  // NB For KDE 4 this could probably be tied to KInstance instead
  KABC::StdAddressBook::close();
  mKIMProxy = 0;
}

void KABCore::restoreSettings()
{
  bool state = KABPrefs::instance()->jumpButtonBarVisible();
  mActionJumpBar->setChecked( state );
  setJumpButtonBarVisible( state );

  state = KABPrefs::instance()->detailsPageVisible();
  mActionDetails->setChecked( state );
  setDetailsVisible( state );

  mViewManager->restoreSettings();
  mExtensionManager->restoreSettings();

  updateIncSearchWidget();
  mIncSearchWidget->setCurrentItem( KABPrefs::instance()->currentIncSearchField() );

  QList<int> splitterSize = KABPrefs::instance()->extensionsSplitter();
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( mDetailsSplitter->height() / 2 );
    splitterSize.append( mDetailsSplitter->height() / 2 );
  }
  mExtensionBarSplitter->setSizes( splitterSize );

  splitterSize = KABPrefs::instance()->detailsSplitter();
  if ( splitterSize.count() == 0 ) {
    splitterSize.append( 360 );
    splitterSize.append( 260 );
  }
  mDetailsSplitter->setSizes( splitterSize );

}

void KABCore::saveSettings()
{
  KABPrefs::instance()->setJumpButtonBarVisible( mActionJumpBar->isChecked() );
  KABPrefs::instance()->setDetailsPageVisible( mActionDetails->isChecked() );

  KABPrefs::instance()->setExtensionsSplitter( mExtensionBarSplitter->sizes() );
  KABPrefs::instance()->setDetailsSplitter( mDetailsSplitter->sizes() );

  mExtensionManager->saveSettings();
  mViewManager->saveSettings();

  KABPrefs::instance()->setCurrentIncSearchField( mIncSearchWidget->currentItem() );
}

KABC::AddressBook *KABCore::addressBook() const
{
  return mAddressBook;
}

KConfig *KABCore::config() const
{
  return KABPrefs::instance()->config();
}

KActionCollection *KABCore::actionCollection() const
{
  return guiClient()->actionCollection();
}

KABC::Field *KABCore::currentSortField() const
{
  return mViewManager->currentSortField();
}

QStringList KABCore::selectedUIDs() const
{
  return mViewManager->selectedUids();
}

KABC::Resource *KABCore::requestResource( QWidget *parent )
{
  const QList<KABC::Resource*> kabcResources = addressBook()->resources();

  QList<KRES::Resource*> kresResources;
  QList<KABC::Resource*>::const_iterator resIt;
  for ( resIt = kabcResources.begin(); resIt != kabcResources.end(); ++resIt) {
    if ( !(*resIt)->readOnly() ) {
      KRES::Resource *res = static_cast<KRES::Resource*>( *resIt );
      if ( res )
        kresResources.append( res );
    }
  }

  KRES::Resource *res = KRES::SelectDialog::getResource( kresResources, parent );
  return static_cast<KABC::Resource*>( res );
}

QWidget *KABCore::widget() const
{
  return mWidget;
}

KAboutData *KABCore::createAboutData()
{
  KAboutData *about = new KAboutData( "kaddressbook", 0, ki18n( "KAddressBook" ),
                                      "3.5", ki18n( "The KDE Address Book" ),
                                      KAboutData::License_GPL_V2,
                                      ki18n( "(c) 1997-2005, The KDE PIM Team" ) );
  about->addAuthor( ki18n("Tobias Koenig"), ki18n( "Current maintainer" ), "tokoe@kde.org" );
  about->addAuthor( ki18n("Don Sanders"), ki18n( "Original author" ) );
  about->addAuthor( ki18n("Cornelius Schumacher"),
                    ki18n( "Co-maintainer, libkabc port, CSV import/export" ),
                    "schumacher@kde.org" );
  about->addAuthor( ki18n("Mike Pilone"), ki18n( "GUI and framework redesign" ),
                    "mpilone@slac.com" );
  about->addAuthor( ki18n("Greg Stern"), ki18n( "DCOP interface" ) );
  about->addAuthor( ki18n("Mark Westcott"), ki18n( "Contact pinning" ) );
  about->addAuthor( ki18n("Mischel Boyer de la Giroday"), ki18n( "LDAP Lookup" ),
                    "michel@klaralvdalens-datakonsult.se" );
  about->addAuthor( ki18n("Steffen Hansen"), ki18n( "LDAP Lookup" ),
                    "hansen@kde.org" );

  return about;
}

void KABCore::setStatusBar( KStatusBar *statusBar )
{
  mStatusBar = statusBar;
}

KStatusBar *KABCore::statusBar() const
{
  return mStatusBar;
}

void KABCore::setContactSelected( const QString &uid )
{
  KABC::Addressee addr = mAddressBook->findByUid( uid );
  if ( !mDetails->isHidden() )
    mDetails->setAddressee( addr );

  mExtensionManager->setSelectionChanged();

  // update the actions
  bool selected = !uid.isEmpty();

  if ( mReadWrite ) {
    mActionCut->setEnabled( selected );

    QClipboard *cb = QApplication::clipboard();
    const QMimeData *data = cb->mimeData( QClipboard::Clipboard );
    KABC::Addressee::List list = AddresseeUtil::clipboardToAddressees( data->data( "text/directory" ) );
    mActionPaste->setEnabled( !list.isEmpty() );
  }

  mActionCopy->setEnabled( selected );
  mActionDelete->setEnabled( selected );
  mActionEditAddressee->setEnabled( selected );
  mActionStoreAddresseeIn->setEnabled( selected );
  mActionMail->setEnabled( selected );
  mActionMailVCard->setEnabled( selected );
  mActionChat->setEnabled( selected && mKIMProxy && mKIMProxy->initialize() );
  mActionWhoAmI->setEnabled( selected );
  mActionCategories->setEnabled( selected );
  mActionMerge->setEnabled( selected );
}

void KABCore::sendMail()
{
  sendMail( mViewManager->selectedEmails().join( ", " ) );
}

void KABCore::sendMail( const QString& email )
{
  KToolInvocation::invokeMailer( email, "" );
}

void KABCore::mailVCard()
{
  QStringList uids = mViewManager->selectedUids();
  if ( !uids.isEmpty() )
    mailVCard( uids );
}

void KABCore::mailVCard( const QStringList &uids )
{
  KABTools::mailVCards( uids, mAddressBook );
}

void KABCore::startChat()
{
  QStringList uids = mViewManager->selectedUids();
#ifdef __GNUC__
#warning Port KIMProxy usage!
#endif
//  if ( !uids.isEmpty() )
//    mKIMProxy->chatWithContact( uids.first() );
}

void KABCore::browse( const QString& url )
{
  KToolInvocation::invokeBrowser( url );
}

void KABCore::selectAllContacts()
{
  mViewManager->setSelected( QString(), true );
}

void KABCore::deleteContacts()
{
  QStringList uidList = mViewManager->selectedUids();

  deleteContacts( uidList );
}

void KABCore::deleteContacts( const QStringList &uids )
{
  if ( uids.count() > 0 ) {
    QStringList names;
    QStringList::ConstIterator it = uids.begin();
    const QStringList::ConstIterator endIt( uids.end() );
    while ( it != endIt ) {
      KABC::Addressee addr = mAddressBook->findByUid( *it );
      names.append( addr.realName().isEmpty() ? addr.preferredEmail() : addr.realName() );
      ++it;
    }

    if ( KMessageBox::warningContinueCancelList( mWidget, i18np( "Do you really want to delete this contact?",
                                                 "Do you really want to delete these %1 contacts?", uids.count() ),
                                                 names, QString(), KStandardGuiItem::del() ) == KMessageBox::Cancel )
      return;

    DeleteCommand *command = new DeleteCommand( mAddressBook, uids );
    mCommandHistory->push( command );

    // now if we deleted anything, refresh
    setContactSelected( QString() );
    setModified( true );
  }
}

void KABCore::copyContacts()
{
  KABC::Addressee::List addrList = mViewManager->selectedAddressees();

  const QByteArray vcards = AddresseeUtil::addresseesToClipboard( addrList );

  QClipboard *cb = QApplication::clipboard();
  QMimeData *data = new QMimeData();
  data->setData( "text/directory", vcards );
  cb->setMimeData( data );
}

void KABCore::cutContacts()
{
  QStringList uidList = mViewManager->selectedUids();

  if ( uidList.size() > 0 ) {
    CutCommand *command = new CutCommand( mAddressBook, uidList );
    mCommandHistory->push( command );

    setModified( true );
  }
}

void KABCore::pasteContacts()
{
  QClipboard *cb = QApplication::clipboard();
  const QMimeData *data = cb->mimeData( QClipboard::Clipboard );
  KABC::Addressee::List list = AddresseeUtil::clipboardToAddressees( data->data( "text/directory" ) );

  pasteContacts( list );
}

void KABCore::pasteContacts( KABC::Addressee::List &list )
{
  KABC::Resource *resource = requestResource( mWidget );
  if ( !resource )
    return;

  KABC::Addressee::List::Iterator it;
  const KABC::Addressee::List::Iterator endIt( list.end() );
  for ( it = list.begin(); it != endIt; ++it )
    (*it).setResource( resource );

  PasteCommand *command = new PasteCommand( this, list );
  mCommandHistory->push( command );

  setModified( true );
}

void KABCore::mergeContacts()
{
  KABC::Addressee::List list = mViewManager->selectedAddressees();
  if ( list.count() < 2 )
    return;

  KABC::Addressee addr = KABTools::mergeContacts( list );

  KABC::Addressee::List::Iterator it = list.begin();
  const KABC::Addressee::List::Iterator endIt( list.end() );
  KABC::Addressee origAddr = *it;
  QStringList uids;
  ++it;
  while ( it != endIt ) {
    uids.append( (*it).uid() );
    ++it;
  }

  DeleteCommand *command = new DeleteCommand( mAddressBook, uids );
  mCommandHistory->push( command );

  EditCommand *editCommand = new EditCommand( mAddressBook, origAddr, addr );
  mCommandHistory->push( editCommand );

  mSearchManager->reload();
}

void KABCore::setWhoAmI()
{
  KABC::Addressee::List addrList = mViewManager->selectedAddressees();

  if ( addrList.count() > 1 ) {
    KMessageBox::sorry( mWidget, i18n( "Please select only one contact." ) );
    return;
  }

  QString text( i18n( "<qt>Do you really want to use <b>%1</b> as your new personal contact?</qt>", addrList[ 0 ].assembledName() ) );
  if ( KMessageBox::questionYesNo( mWidget, text, QString(), KGuiItem(i18n( "Use" )), KGuiItem(i18n( "Do Not Use" )) ) == KMessageBox::Yes )
    static_cast<KABC::StdAddressBook*>( KABC::StdAddressBook::self( true ) )->setWhoAmI( addrList[ 0 ] );
}

void KABCore::incrementalTextSearch( const QString& text )
{
  setContactSelected( QString() );
  mSearchManager->search( text, mIncSearchWidget->currentFields() );
}

void KABCore::incrementalJumpButtonSearch( const QString& character )
{
  mViewManager->setSelected( QString(), false );

  KABC::AddresseeList list;
  // FIXME this conversion is only temporarily necessary, until
  // KABC::AddresseeList has been ported to QList
  KABC::Addressee::List::ConstIterator tmpIt( mSearchManager->contacts().constBegin() );
  for ( ; tmpIt != mSearchManager->contacts().constEnd(); ++tmpIt ) {
    list.append( *tmpIt );
  }
  KABC::Field *field = mViewManager->currentSortField();
  if ( field ) {
    list.sortByField( field );
    KABC::AddresseeList::ConstIterator it;
    const KABC::AddresseeList::ConstIterator endIt( list.end() );
    for ( it = list.begin(); it != endIt; ++it ) {
      if ( field->value( *it ).startsWith( character, Qt::CaseInsensitive ) ) {
        mViewManager->setSelected( (*it).uid(), true );
        return;
      }
    }
  }
}

void KABCore::setModified()
{
  setModified( true );
}

void KABCore::setModified( bool modified )
{
  mModified = modified;
  mActionSave->setEnabled( mModified );

  mSearchManager->reload();
}

bool KABCore::modified() const
{
  return mModified;
}

void KABCore::contactModified( const KABC::Addressee &addr )
{
  Command *command = 0;

  // check if it exists already
  KABC::Addressee origAddr = mAddressBook->findByUid( addr.uid() );
  if ( origAddr.isEmpty() ) {
    KABC::Addressee::List addressees;
    addressees.append( addr );
    command = new NewCommand( mAddressBook, addressees );
  } else {
    command = new EditCommand( mAddressBook, origAddr, addr );
  }

  mCommandHistory->push( command );

  setContactSelected( addr.uid() );
  setModified( true );
}

void KABCore::newContact()
{
  AddresseeEditorDialog *dialog = 0;

  KABC::Resource* resource = requestResource( mWidget );

  if ( resource ) {
    KABC::Addressee addr;
    addr.setResource( resource );

    if ( !KABLock::self( mAddressBook )->lock( addr.resource() ) )
      return;

    dialog = createAddresseeEditorDialog( mWidget );
    dialog->setAddressee( addr );
  } else
    return;

  mEditorDict.insert( dialog->addressee().uid(), dialog );

  dialog->show();
}

void KABCore::addEmail( const QString &aStr )
{
  QString fullName, email;

  KABC::Addressee::parseEmailAddress( aStr, fullName, email );

  // Try to lookup the addressee matching the email address
  bool found = false;
  QStringList emailList;
  KABC::AddressBook::Iterator it;
  const KABC::AddressBook::Iterator endIt( mAddressBook->end() );
  for ( it = mAddressBook->begin(); !found && (it != endIt); ++it ) {
    emailList = (*it).emails();
    if ( emailList.contains( email ) > 0 ) {
      found = true;
      (*it).setNameFromString( fullName );
      editContact( (*it).uid() );
    }
  }

  if ( !found ) {
    KABC::Addressee addr;
    addr.setNameFromString( fullName );
    addr.insertEmail( email, true );

    mAddressBook->insertAddressee( addr );
    mViewManager->refreshView( addr.uid() );
    editContact( addr.uid() );
  }
}

void KABCore::importVCard( const KUrl &url )
{
  mXXPortManager->importVCard( url );
}

void KABCore::importVCard( const QString &vCardURL )
{
  mXXPortManager->importVCard( vCardURL );
}

void KABCore::editContact( const QString &uid )
{
  if ( mExtensionManager->isQuickEditVisible() )
    return;

  // First, locate the contact entry
  QString localUID = uid;
  if ( localUID.isNull() ) {
    QStringList uidList = mViewManager->selectedUids();
    if ( uidList.count() > 0 )
      localUID = uidList.at( 0 );
  }

  KABC::Addressee addr = mAddressBook->findByUid( localUID );
  if ( !addr.isEmpty() ) {
    AddresseeEditorDialog *dialog = mEditorDict.find( addr.uid() );
    if ( !dialog ) {

      if ( addr.resource() && !addr.resource()->readOnly() )
        if ( !KABLock::self( mAddressBook )->lock( addr.resource() ) ) {
          return;
        }

      dialog = createAddresseeEditorDialog( mWidget );

      mEditorDict.insert( addr.uid(), dialog );

      dialog->setAddressee( addr );
    }

    dialog->raise();
    dialog->show();
  }
}

void KABCore::storeContactIn( const QString &uid )
{
  // First, locate the contact entry
  QStringList uidList;
  if ( uid.isNull() ) {
    uidList = mViewManager->selectedUids();
  } else {
    uidList << uid;
  }
  KABC::Resource *resource = requestResource( mWidget );
  if ( !resource )
    return;

  KABLock::self( mAddressBook )->lock( resource );
  QStringList::Iterator it( uidList.begin() );
  const QStringList::Iterator endIt( uidList.end() );
  while ( it != endIt ) {
    KABC::Addressee addr = mAddressBook->findByUid( *it++ );
    if ( !addr.isEmpty() ) {
      KABC::Addressee newAddr( addr );
      // We need to set a new uid, otherwise the insert below is
      // ignored. This is bad for syncing, but unavoidable, afaiks
      newAddr.setUid( KRandom::randomString( 10 ) );
      newAddr.setResource( resource );
      addressBook()->insertAddressee( newAddr );
      KABLock::self( mAddressBook )->lock( addr.resource() );
      addressBook()->removeAddressee( addr );
      KABLock::self( mAddressBook )->unlock( addr.resource() );
    }
  }
  KABLock::self( mAddressBook )->unlock( resource );
}

void KABCore::save()
{
  const QList<KABC::Resource*> resources = mAddressBook->resources();
  QList<KABC::Resource*>::const_iterator it;

  for(it = resources.begin(); it != resources.end(); ++it) {
    KABC::Ticket *ticket = mAddressBook->requestSaveTicket( *it );
    if ( ticket ) {
      if ( !mAddressBook->save( ticket ) ) {
        KMessageBox::error( mWidget,
                            i18n( "<qt>Unable to save address book <b>%1</b>.</qt>", (*it)->resourceName() ) );
        mAddressBook->releaseSaveTicket( ticket );
      } else {
        setModified( false );
      }
    } else {
      KMessageBox::error( mWidget,
                          i18n( "<qt>Unable to get access for saving the address book <b>%1</b>.</qt>" ,
                            (*it)->resourceName() ) );
    }
  }
}

void KABCore::setJumpButtonBarVisible( bool visible )
{
  if ( visible ) {
    if ( !mJumpButtonBar )
      createJumpButtonBar();
    mJumpButtonBar->show();
  } else
    if ( mJumpButtonBar )
      mJumpButtonBar->hide();
}

void KABCore::setDetailsVisible( bool visible )
{
  if ( visible )
    mDetailsPage->show();
  else
    mDetailsPage->hide();
}

void KABCore::extensionModified( const KABC::Addressee::List &list )
{
  if ( list.count() != 0 ) {
    KABC::Addressee::List::ConstIterator it;
    const KABC::Addressee::List::ConstIterator endIt( list.end() );
    for ( it = list.begin(); it != endIt; ++it ) {
      Command *command = 0;

      // check if it exists already
      KABC::Addressee origAddr = mAddressBook->findByUid( (*it).uid() );
      if ( origAddr.isEmpty() ) {
        KABC::Addressee::List addressees;
        addressees.append( *it );
        command = new NewCommand( mAddressBook, addressees );
      } else
        command = new EditCommand( mAddressBook, origAddr, *it );

      mCommandHistory->blockSignals( true );
      mCommandHistory->push( command );
      mCommandHistory->blockSignals( false );
    }

    setModified(true);
  }
}

void KABCore::extensionDeleted( const QStringList &uidList )
{
  DeleteCommand *command = new DeleteCommand( mAddressBook, uidList );
  mCommandHistory->push( command );

  // now if we deleted anything, refresh
  setContactSelected( QString() );
  setModified( true );
}

QString KABCore::getNameByPhone( const QString &phone )
{
  QRegExp r( "[/*/-/ ]" );
  QString localPhone( phone );

  bool found = false;
  QString ownerName = "";
  KABC::PhoneNumber::List phoneList;

  KABC::AddressBook::ConstIterator iter;
  const KABC::AddressBook::ConstIterator endIter( mAddressBook->end() );

  for ( iter = mAddressBook->begin(); !found && ( iter != endIter ); ++iter ) {
    phoneList = (*iter).phoneNumbers();
    KABC::PhoneNumber::List::Iterator phoneIter( phoneList.begin() );
    const KABC::PhoneNumber::List::Iterator phoneEndIter( phoneList.end() );
    for ( ; !found && ( phoneIter != phoneEndIter ); ++phoneIter) {
      // Get rid of separator chars so just the numbers are compared.
      if ( (*phoneIter).number().replace( r, "" ) == localPhone.replace( r, "" ) ) {
        ownerName = (*iter).realName();
        found = true;
      }
    }
  }

  return ownerName;
}

void KABCore::openLDAPDialog()
{
  if ( !KProtocolInfo::isKnownProtocol( KUrl( "ldap://localhost" ) ) ) {
    KMessageBox::error( mWidget, i18n( "Your KDE installation is missing LDAP "
                                       "support, please ask your administrator or distributor for more information." ),
                        i18n( "No LDAP IO Slave Available" ) );
    return;
  }

  if ( !mLdapSearchDialog ) {
    mLdapSearchDialog = new LDAPSearchDialog( mAddressBook, this, mWidget );
    connect( mLdapSearchDialog, SIGNAL( addresseesAdded() ),
            SLOT( addressBookChanged() ) );
    connect( mLdapSearchDialog, SIGNAL( addresseesAdded() ),
            SLOT( setModified() ) );
  } else
    mLdapSearchDialog->restoreSettings();

  if ( mLdapSearchDialog->isOK() )
    mLdapSearchDialog->exec();
}

void KABCore::configure()
{
  // Save the current config so we do not loose anything if the user accepts
  saveSettings();

  KCMultiDialog dlg( mWidget );
  dlg.setModal( true );
  connect( &dlg, SIGNAL( configCommitted() ),
           this, SLOT( configurationChanged() ) );

  dlg.addModule( "kabconfig.desktop" );
  dlg.addModule( "kabldapconfig.desktop" );
  dlg.addModule( "kabcustomfields.desktop" );

  dlg.exec();
}

void KABCore::print()
{
  KPrinter printer;
  printer.setDocName( i18n( "Address Book" ) );
  printer.setDocFileName( "addressbook" );

  if ( !printer.setup( mWidget, i18n( "Print Addresses" ) ) )
    return;

  KABPrinting::PrintingWizard wizard( &printer, mAddressBook,
                                      mViewManager->selectedUids(), mWidget );

  wizard.exec();
}

void KABCore::detailsHighlighted( const QString &msg )
{
  if ( mStatusBar ) {
    if ( !mStatusBar->hasItem( 2 ) )
      mStatusBar->insertItem( msg, 2 );
    else
      mStatusBar->changeItem( msg, 2 );
  }
}

void KABCore::showContactsAddress( const QString &addrUid )
{
  QStringList uidList = mViewManager->selectedUids();
  if ( uidList.isEmpty() )
    return;

  KABC::Addressee addr = mAddressBook->findByUid( uidList.first() );
  if ( addr.isEmpty() )
    return;

  const KABC::Address::List list = addr.addresses();
  KABC::Address::List::ConstIterator it;
  const KABC::Address::List::ConstIterator endIt( list.end() );
  for ( it = list.begin(); it != endIt; ++it )
    if ( (*it).id() == addrUid ) {
      LocationMap::instance()->showAddress( *it );
      break;
    }
}

void KABCore::configurationChanged()
{
  mExtensionManager->reconfigure();
  mViewManager->refreshView();
}

bool KABCore::queryClose()
{
  saveSettings();
  KABPrefs::instance()->writeConfig();

  QList<KABC::Resource*> resources = mAddressBook->resources();
  QList<KABC::Resource*>::iterator it;
  for (it = resources.begin(); it != resources.end(); ++it ) {
    (*it)->close();
  }

  return true;
}

void KABCore::reinitXMLGUI()
{
  mExtensionManager->createActions();
}
void KABCore::delayedAddressBookChanged()
{
  mAddressBookChangedTimer->start( 1000 );
}

void KABCore::addressBookChanged()
{
  mAddressBookChangedTimer->stop();

  if ( mJumpButtonBar )
    mJumpButtonBar->updateButtons();

  mSearchManager->reload();
  mViewManager->setSelected( QString(), false );
  setContactSelected( QString() );

  updateCategories();
}

AddresseeEditorDialog *KABCore::createAddresseeEditorDialog( QWidget *parent,
                                                             const char *name )
{
  AddresseeEditorDialog *dialog = new AddresseeEditorDialog( this, parent,
                                                 name ? name : "editorDialog" );
  connect( dialog, SIGNAL( contactModified( const KABC::Addressee& ) ),
           SLOT( contactModified( const KABC::Addressee& ) ) );
  connect( dialog, SIGNAL( editorDestroyed( const QString& ) ),
           SLOT( slotEditorDestroyed( const QString& ) ) );

  return dialog;
}

void KABCore::slotEditorDestroyed( const QString &uid )
{
  AddresseeEditorDialog *dialog = mEditorDict.take( uid );

  KABC::Addressee addr = dialog->addressee();

  if ( addr.resource() && !addr.resource()->readOnly() ) {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    KABLock::self( mAddressBook )->unlock( addr.resource() );
    QApplication::restoreOverrideCursor();
  }
}

void KABCore::initGUI()
{
  QVBoxLayout *topLayout = new QVBoxLayout( mWidget );
  topLayout->setSpacing( 0 );
  topLayout->setMargin( 0 );
  KToolBar* searchTB = new KToolBar( mWidget, "search toolbar" );
  searchTB->layout()->setSpacing( KDialog::spacingHint() );
  mIncSearchWidget = new IncSearchWidget( searchTB, "kde toolbar widget" );
  searchTB->addWidget( mIncSearchWidget );
  connect( mIncSearchWidget, SIGNAL( doSearch( const QString& ) ),
           SLOT( incrementalTextSearch( const QString& ) ) );

  mFilterSelectionWidget = new FilterSelectionWidget( searchTB , "kde toolbar widget" );
  searchTB->addWidget( mFilterSelectionWidget );

  mDetailsSplitter = new QSplitter( mWidget );
  topLayout->addWidget( searchTB );
  topLayout->addWidget( mDetailsSplitter );

  mExtensionBarSplitter = new QSplitter( mDetailsSplitter );
  mExtensionBarSplitter->setOrientation( Qt::Vertical );

  mDetailsWidget = new QWidget( mDetailsSplitter );
  mDetailsLayout = new QHBoxLayout( mDetailsWidget );
  mDetailsLayout->setSpacing( 0 );
  mDetailsLayout->setMargin( 0 );

  mDetailsPage = new QWidget( mDetailsWidget );
  mDetailsLayout->addWidget( mDetailsPage );

  QHBoxLayout *detailsPageLayout = new QHBoxLayout( mDetailsPage );
  detailsPageLayout->setSpacing( 0 );
  detailsPageLayout->setMargin( 0 );
  mDetails = new KPIM::AddresseeView( mDetailsPage );
  mDetails->setVerticalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
  detailsPageLayout->addWidget( mDetails );

  connect( mDetails, SIGNAL( addressClicked( const QString&) ),
           this, SLOT( showContactsAddress( const QString& ) ) );

  mViewManager = new ViewManager( this, mExtensionBarSplitter );
  mViewManager->setFilterSelectionWidget( mFilterSelectionWidget );

  connect( mFilterSelectionWidget, SIGNAL( filterActivated( int ) ),
           mViewManager, SLOT( setActiveFilter( int ) ) );

  mExtensionManager = new ExtensionManager( this, mExtensionBarSplitter );

  topLayout->setStretchFactor( mDetailsSplitter, 1 );

  mXXPortManager = new XXPortManager( this, mWidget );

  initActions();
}

void KABCore::createJumpButtonBar()
{
  mJumpButtonBar = new JumpButtonBar( this, mDetailsWidget );
  mDetailsLayout->addWidget( mJumpButtonBar );
  mDetailsLayout->setStretchFactor( mJumpButtonBar, 1 );

  connect( mJumpButtonBar, SIGNAL( jumpToLetter( const QString& ) ),
           SLOT( incrementalJumpButtonSearch( const QString& ) ) );
  connect( mViewManager, SIGNAL( sortFieldChanged() ),
           mJumpButtonBar, SLOT( updateButtons() ) );
}

void KABCore::initActions()
{
  connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
           SLOT( clipboardDataChanged() ) );

  QAction *action;
  KActionCollection *coll = actionCollection();

  // file menu
  mActionMail = coll->addAction( "file_mail" );
  mActionMail->setIcon( KIcon("mail-send") );
  mActionMail->setText( i18n( "&Send Email to Contact..." ) );
  connect(mActionMail, SIGNAL(triggered(bool)), SLOT( sendMail() ));
  action = KStandardAction::print( this, SLOT( print() ), actionCollection() );
  mActionMail->setWhatsThis( i18n( "Send a mail to all selected contacts." ) );
  action->setWhatsThis( i18n( "Print a special number of contacts." ) );

  mActionSave = KStandardAction::save( this, SLOT( save() ), actionCollection() );
  actionCollection()->addAction( "file_sync", mActionSave );
  mActionSave->setWhatsThis( i18n( "Save all changes of the address book to the storage backend." ) );

  action = coll->addAction( "file_new_contact" );
  action->setIcon( KIcon("identity") );
  action->setText( i18n( "&New Contact..." ) );
  connect(action, SIGNAL(triggered(bool)), SLOT( newContact() ));
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_N));
  action->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers." ) );

  mActionMailVCard = coll->addAction( "file_mail_vcard" );
  mActionMailVCard->setIcon( KIcon("mail_post_to") );
  mActionMailVCard->setText( i18n( "Send &Contact..." ) );
  connect(mActionMailVCard, SIGNAL(triggered(bool) ), SLOT( mailVCard() ));
  mActionMailVCard->setWhatsThis( i18n( "Send a mail with the selected contact as attachment." ) );

  mActionChat = coll->addAction( "file_chat" );
  mActionChat->setText( i18n( "Chat &With..." ) );
  connect(mActionChat, SIGNAL(triggered(bool) ), SLOT( startChat() ));
  mActionChat->setWhatsThis( i18n( "Start a chat with the selected contact." ) );

  mActionEditAddressee = coll->addAction( "file_properties" );
  mActionEditAddressee->setIcon( KIcon("edit") );
  mActionEditAddressee->setText( i18n( "&Edit Contact..." ) );
  connect(mActionEditAddressee, SIGNAL(triggered(bool) ), SLOT( editContact() ));
  mActionEditAddressee->setWhatsThis( i18n( "Edit a contact<p>You will be presented with a dialog where you can change all data about a person, including addresses and phone numbers." ) );

  mActionMerge = coll->addAction( "edit_merge" );
  mActionMerge->setText( i18n( "&Merge Contacts" ) );
  connect(mActionMerge, SIGNAL(triggered(bool) ), SLOT( mergeContacts() ));

  // edit menu
  mActionCopy = KStandardAction::copy( this, SLOT( copyContacts() ), actionCollection() );
  mActionCut = KStandardAction::cut( this, SLOT( cutContacts() ), actionCollection() );
  mActionPaste = KStandardAction::paste( this, SLOT( pasteContacts() ), actionCollection() );
  action = KStandardAction::selectAll( this, SLOT( selectAllContacts() ), actionCollection() );
  mActionCopy->setWhatsThis( i18n( "Copy the currently selected contact(s) to system clipboard in vCard format." ) );
  mActionCut->setWhatsThis( i18n( "Cuts the currently selected contact(s) to system clipboard in vCard format." ) );
  mActionPaste->setWhatsThis( i18n( "Paste the previously cut or copied contacts from clipboard." ) );
  action->setWhatsThis( i18n( "Selects all visible contacts from current view." ) );
//  mActionUndo->setWhatsThis( i18n( "Undoes the last <b>Cut</b>, <b>Copy</b> or <b>Paste</b>." ) );
//  mActionRedo->setWhatsThis( i18n( "Redoes the last <b>Cut</b>, <b>Copy</b> or <b>Paste</b>." ) );

  mActionDelete = coll->addAction( "edit_delete" );
  mActionDelete->setIcon( KIcon("edit-delete") );
  mActionDelete->setText( i18n( "&Delete Contact" ) );
  connect(mActionDelete, SIGNAL(triggered(bool) ), SLOT( deleteContacts() ));
  mActionDelete->setShortcut(QKeySequence(Qt::Key_Delete));
  mActionDelete->setWhatsThis( i18n( "Delete all selected contacts." ) );


  mActionStoreAddresseeIn = coll->addAction( "edit_store_in" );
  mActionStoreAddresseeIn->setIcon( KIcon("kaddressbook") );
  mActionStoreAddresseeIn->setText( i18n( "St&ore Contact In..." ) );
  connect(mActionStoreAddresseeIn, SIGNAL(triggered(bool) ), SLOT( storeContactIn() ));
  mActionStoreAddresseeIn->setWhatsThis( i18n( "Store a contact in a different Addressbook<p>You will be presented with a dialog where you can select a new storage place for this contact." ) );

  // settings menu
  mActionJumpBar = coll->add<KToggleAction>( "options_show_jump_bar" );
  mActionJumpBar->setText( i18n( "Show Jump Bar" ) );
  mActionJumpBar->setIcon( KIcon( "find-next" ) );
  mActionJumpBar->setWhatsThis( i18n( "Toggle whether the jump button bar shall be visible." ) );
  mActionJumpBar->setCheckedState( KGuiItem(i18n( "Hide Jump Bar" )) );
  connect( mActionJumpBar, SIGNAL( toggled( bool ) ), SLOT( setJumpButtonBarVisible( bool ) ) );

  mActionDetails = coll->add<KToggleAction>( "options_show_details" );
  mActionDetails->setText( i18n( "Show Details" ) );
  mActionDetails->setWhatsThis( i18n( "Toggle whether the details page shall be visible." ) );
  mActionDetails->setCheckedState( KGuiItem(i18n( "Hide Details" )) );
  connect( mActionDetails, SIGNAL( toggled( bool ) ), SLOT( setDetailsVisible( bool ) ) );

  if ( mIsPart )
  {
    action = coll->addAction("kaddressbook_configure");
    action->setIcon(KIcon("configure"));
    action->setText(i18n("&Configure Address Book..."));
    connect(action, SIGNAL(triggered(bool) ), SLOT( configure() ));
  } else {
    action = KStandardAction::preferences( this, SLOT( configure() ), actionCollection() );
  }

  action->setWhatsThis( i18n( "You will be presented with a dialog, that offers you all possibilities to configure KAddressBook." ) );

  // misc
  action = coll->addAction( "ldap_lookup" );
  action->setIcon( KIcon( "edit-find" ) );
  action->setText( i18n( "&Lookup Addresses in LDAP Directory..." ) );
  connect(action, SIGNAL(triggered(bool)), SLOT( openLDAPDialog() ));
  action->setWhatsThis( i18n( "Search for contacts on a LDAP server<p>You will be presented with a dialog, where you can search for contacts and select the ones you want to add to your local address book." ) );

  mActionWhoAmI = coll->addAction( "edit_set_personal" );
  mActionWhoAmI->setIcon( KIcon("personal") );
  mActionWhoAmI->setText( i18n( "Set as Personal Contact Data" ) );
  connect(mActionWhoAmI, SIGNAL(triggered(bool) ), SLOT( setWhoAmI() ));
  mActionWhoAmI->setWhatsThis( i18n( "Set the personal contact<p>The data of this contact will be used in many other KDE applications, so you do not have to input your personal data several times." ) );

  mActionCategories = coll->addAction( "edit_set_categories" );
  mActionCategories->setText( i18n( "Select Categories..." ) );
  connect(mActionCategories, SIGNAL(triggered(bool) ), SLOT( setCategories() ));
  mActionCategories->setWhatsThis( i18n( "Set the categories for all selected contacts." ) );

  QAction *clearLocation = coll->addAction( "clear_search" );
  clearLocation->setIcon( KIcon(QApplication::isRightToLeft() ? "clear-left" : "locationbar-erase") );
  clearLocation->setText( i18n( "Clear Search Bar" ) );
  connect(clearLocation, SIGNAL(triggered(bool) ), SLOT( slotClearSearchBar() ));
  clearLocation->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_L));
  clearLocation->setWhatsThis( i18n( "Clear Search Bar<p>"
				     "Clears the content of the quick search bar." ) );

  clipboardDataChanged();
}

void KABCore::clipboardDataChanged()
{
  if ( mReadWrite )
    mActionPaste->setEnabled( !QApplication::clipboard()->text().isEmpty() );
}

void KABCore::updateIncSearchWidget()
{
  mIncSearchWidget->setViewFields( mViewManager->viewFields() );
}

void KABCore::updateCategories()
{
  QStringList categories( allCategories() );
  categories.sort();

  const QStringList customCategories( KABPrefs::instance()->customCategories() );
  QStringList::ConstIterator it;
  const QStringList::ConstIterator endIt( customCategories.end() );
  for ( it = customCategories.begin(); it != endIt; ++it ) {
    if ( !categories.contains( *it )  ) {
      categories.append( *it );
    }
  }

  KABPrefs::instance()->mCustomCategories = categories;
  KABPrefs::instance()->writeConfig();

  if ( mCategoryEditDialog )
    mCategoryEditDialog->reload();
}

QStringList KABCore::allCategories() const
{
  QStringList categories, allCategories;
  QStringList::ConstIterator catIt;

  KABC::AddressBook::ConstIterator it;
  const KABC::AddressBook::ConstIterator endIt( mAddressBook->end() );
  for ( it = mAddressBook->begin(); it != endIt; ++it ) {
    categories = (*it).categories();
    const QStringList::ConstIterator catEndIt( categories.end() );
    for ( catIt = categories.begin(); catIt != catEndIt; ++catIt )
      if ( !allCategories.contains( *catIt ) )
        allCategories.append( *catIt );
  }

  return allCategories;
}

void KABCore::setCategories()
{
  // Show the category dialog
  if ( mCategorySelectDialog == 0 ) {
    mCategorySelectDialog = new KPIM::CategorySelectDialog( KABPrefs::instance(), mWidget );
    connect( mCategorySelectDialog, SIGNAL( categoriesSelected( const QStringList& ) ),
             SLOT( categoriesSelected( const QStringList& ) ) );
    connect( mCategorySelectDialog, SIGNAL( editCategories() ), SLOT( editCategories() ) );
  }

  mCategorySelectDialog->show();
  mCategorySelectDialog->raise();
}

void KABCore::categoriesSelected( const QStringList &categories )
{
  bool merge = false;
  QString msg = i18n( "Merge with existing categories?" );
  if ( KMessageBox::questionYesNo( mWidget, msg, QString(), KGuiItem(i18n( "Merge" )), KGuiItem(i18n( "Do Not Merge" )) ) == KMessageBox::Yes )
    merge = true;

  QStringList uids = mViewManager->selectedUids();
  QStringList::ConstIterator it;
  const QStringList::ConstIterator endIt( uids.end() );
  for ( it = uids.begin(); it != endIt; ++it ) {
    KABC::Addressee addr = mAddressBook->findByUid( *it );
    if ( !addr.isEmpty() ) {
      if ( !merge )
        addr.setCategories( categories );
      else {
        QStringList addrCategories = addr.categories();
        QStringList::ConstIterator catIt;
        const QStringList::ConstIterator catEndIt( categories.end() );
        for ( catIt = categories.begin(); catIt != catEndIt; ++catIt ) {
          if ( !addrCategories.contains( *catIt ) )
            addrCategories.append( *catIt );
        }
        addr.setCategories( addrCategories );
      }

      mAddressBook->insertAddressee( addr );
    }
  }

  if ( uids.count() > 0 )
    setModified( true );
}

void KABCore::editCategories()
{
  if ( mCategoryEditDialog == 0 ) {
    mCategoryEditDialog = new KPIM::CategoryEditDialog( KABPrefs::instance(), mWidget );
    connect( mCategoryEditDialog, SIGNAL( categoryConfigChanged() ),
             mCategorySelectDialog, SLOT( updateCategoryConfig() ) );
  }

  mCategoryEditDialog->show();
  mCategoryEditDialog->raise();
}

void KABCore::slotClearSearchBar()
{
  mIncSearchWidget->clear();
  mIncSearchWidget->setFocus();
}

void KABCore::slotContactsUpdated()
{
  if ( mStatusBar ) {
    QString msg( i18np( "%1 contact matches", "%1 contacts matching", mSearchManager->contacts().count() ) );
    if ( !mStatusBar->hasItem( 1 ) )
      mStatusBar->insertItem( msg, 1 );
    else
      mStatusBar->changeItem( msg, 1 );
  }

  emit contactsUpdated();
}

bool KABCore::handleCommandLine()
{
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString addrStr = args->getOption( "addr" );
  QString uidStr = args->getOption( "uid" );

  OrgKdeKAddressbookCoreInterface interface("org.kde.KAddressbook", "/KAddressbook", QDBusConnection::sessionBus());
  QString addr, uid, vcard;
  if ( !addrStr.isEmpty() )
    addr = addrStr ;
  if ( !uidStr.isEmpty() )
    uid = uidStr;

  bool doneSomething = false;

  // Can not see why anyone would pass both a uid and an email address, so I'll leave it that two contact editors will show if they do
  if ( !addr.isEmpty() ) {
    interface.addEmail( addr );
    doneSomething = true;
  }

  if ( !uid.isEmpty() ) {
    interface.showContactEditor(uid );
    doneSomething = true;
  }

  if ( args->isSet( "new-contact" ) ) {
    interface.newContact();
    doneSomething = true;
  }

  if ( args->count() >= 1 ) {
    for ( int i = 0; i < args->count(); ++i ) {
      interface.importVCard(args->url( i ).url() );
    }
    doneSomething = true;
  }
  return doneSomething;
}

#ifdef KDEPIM_NEW_DISTRLISTS
KPIM::DistributionList::List KABCore::distributionLists() const
{
  return mSearchManager->distributionLists();
}

QStringList KABCore::distributionListNames() const
{
  return mSearchManager->distributionListNames();
}
#endif

#include "kabcore.moc"
