/*
 *   kmail: KDE mail client
 *   This file: Copyright (C) 2000 Espen Sand, espen@kde.org
 *              Copyright (C) 2001 Marc Mutz, mutz@kde.org
 *   Contains code segments and ideas from earlier kmail dialog code.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

// This must be first
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// my headers:
#include "configuredialog.h"
#define private public
#include "configuredialog_p.h"
#undef private

// other KMail headers:
#include "simplestringlisteditor.h"
#include "accountdialog.h"
#include "colorlistbox.h"
#include "kbusyptr.h"
#include "kmacctmgr.h"
#include "kmacctseldlg.h"
#include "kmsender.h"
#include "kmtopwidget.h"
#include "kmtransport.h"
#include "kmfoldermgr.h"
#include "signatureconfigurationdialogimpl.h"
#include "encryptionconfigurationdialogimpl.h"
#include "directoryservicesconfigurationdialogimpl.h"
#include "certificatehandlingdialogimpl.h"
#include "kmidentity.h"
#include "identitymanager.h"
#include "identitylistview.h"
using KMail::IdentityListView;
using KMail::IdentityListViewItem;
#include "identitydialog.h"
using KMail::IdentityDialog;
#include "kmkernel.h"
#include "signatureconfigurator.h"
using KMail::SignatureConfigurator;

// other kdenetwork headers:
#include <kpgp.h>
#include <kpgpui.h>


// other KDE headers:
#include <kapplication.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <knuminput.h>
#include <kfontdialog.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#include <kurlrequester.h>
#include <kseparator.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kstringvalidator.h>
#include <kwin.h>

// Qt headers:
#include <qregexp.h>
#include <qtabwidget.h>
#include <qwhatsthis.h>
#include <qwidgetstack.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qvbuttongroup.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtextcodec.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qobjectlist.h>

// added for CRYPTPLUG
#include <qvariant.h>   // first for gcc 2.7.2
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include "cryptplugwrapperlist.h"
#include "cryptplugwrapper.h"
#include "signatureconfigurationdialog.h"
#include "encryptionconfigurationdialog.h"
#include "directoryservicesconfigurationdialogimpl.h"

// other headers:
#include <assert.h>

#ifndef _PATH_SENDMAIL
#define _PATH_SENDMAIL  "/usr/sbin/sendmail"
#endif

// little helper:
static inline QPixmap loadIcon( const char * name ) {
  return KGlobal::instance()->iconLoader()
    ->loadIcon( QString::fromLatin1(name), KIcon::NoGroup, KIcon::SizeMedium );
}


ConfigureDialog::ConfigureDialog( CryptPlugWrapperList* cryptpluglist,
                                  QWidget *parent, const char *name,
                                  bool modal )
  : KDialogBase( IconList, i18n("Configure"), Help|Apply|Ok|Cancel,
                 Ok, parent, name, modal, true ),
    mCryptPlugList( cryptpluglist )
{
  KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );
  // setHelp() not needed, since we override slotHelp() anyway...

  setIconListAllVisible( true );

  connect( this, SIGNAL(cancelClicked()), this, SLOT(slotCancelOrClose()) );

  QWidget *page;
  QVBoxLayout *vlay;

  // Identity Page:
  page = addPage( IdentityPage::iconLabel(), IdentityPage::title(),
		  loadIcon( IdentityPage::iconName() ) );
  vlay = new QVBoxLayout( page, 0, spacingHint() );
  mIdentityPage = new IdentityPage( page );
  vlay->addWidget( mIdentityPage );
  mIdentityPage->setPageIndex( pageIndex( page ) );

  // Network Page:
  page = addPage( NetworkPage::iconLabel(), NetworkPage::title(),
		  loadIcon( NetworkPage::iconName() ) );
  vlay = new QVBoxLayout( page, 0, spacingHint() );
  mNetworkPage = new NetworkPage( page );
  vlay->addWidget( mNetworkPage );
  mNetworkPage->setPageIndex( pageIndex( page ) );

  // ### FIXME: We need a KMTransportCombo...  It's also no good to
  // allow non-applied transports to be presented in the identity
  // settings...
  connect( mNetworkPage, SIGNAL(transportListChanged(const QStringList &)),
	   mIdentityPage, SLOT(slotUpdateTransportCombo(const QStringList &)) );

  // Appearance Page:
  page = addPage( AppearancePage::iconLabel(), AppearancePage::title(),
		  loadIcon( AppearancePage::iconName() ) );
  vlay = new QVBoxLayout( page, 0, spacingHint() );
  mAppearancePage = new AppearancePage( page );
  vlay->addWidget( mAppearancePage );
  mAppearancePage->setPageIndex( pageIndex( page ) );

  connect( mAppearancePage, SIGNAL(profileSelected(KConfig*)),
	   this, SLOT(slotInstallProfile(KConfig*)) );

  // Composer Page:
  page = addPage( ComposerPage::iconLabel(), ComposerPage::title(),
		  loadIcon( ComposerPage::iconName() ) );
  vlay = new QVBoxLayout( page, 0, spacingHint() );
  mComposerPage = new ComposerPage( page );
  vlay->addWidget( mComposerPage );
  mComposerPage->setPageIndex( pageIndex( page ) );

  // Security Page:
  page = addPage( SecurityPage::iconLabel(), SecurityPage::title(),
		  loadIcon( SecurityPage::iconName() ) );
  vlay = new QVBoxLayout( page, 0, spacingHint() );
  mSecurityPage = new SecurityPage( page );
  vlay->addWidget( mSecurityPage );
  mSecurityPage->setPageIndex( pageIndex( page ) );

  // Miscellaneous Page:
  page = addPage( MiscPage::iconLabel(), MiscPage::title(),
		  loadIcon( MiscPage::iconName() ) );
  vlay = new QVBoxLayout( page, 0, spacingHint() );
  mMiscPage = new MiscPage( page );
  vlay->addWidget( mMiscPage );
  mMiscPage->setPageIndex( pageIndex( page ) );

  // Plugin Page:

  page = addPage( PluginPage::iconLabel(), PluginPage::title(),
		  loadIcon( PluginPage::iconName() ) );
  vlay = new QVBoxLayout( page, 0, spacingHint() );
  mPluginPage = new PluginPage( mCryptPlugList, page );
  vlay->addWidget( mPluginPage );
  mPluginPage->setPageIndex( pageIndex( page ) );
}


ConfigureDialog::~ConfigureDialog()
{
}


void ConfigureDialog::show()
{
  // ### try to move the setup into the *Page::show() methods?
  if( !isVisible() )
    setup();
  KDialogBase::show();
}

void ConfigureDialog::slotCancelOrClose()
{
  mIdentityPage->dismiss();
  mNetworkPage->dismiss();
  mAppearancePage->dismiss();
  mComposerPage->dismiss();
  mSecurityPage->dismiss();
  mMiscPage->dismiss();
  mPluginPage->dismiss();
}

void ConfigureDialog::slotOk()
{
  apply( true );
  accept();
}


void ConfigureDialog::slotApply() {
  apply( false );
}

void ConfigureDialog::slotHelp() {
  // ### FIXME: use a list for the pages...
  int activePage = activePageIndex();
  if ( activePage == mIdentityPage->pageIndex() )
    kapp->invokeHelp( mIdentityPage->helpAnchor() );
  else if ( activePage == mNetworkPage->pageIndex() )
    kapp->invokeHelp( mNetworkPage->helpAnchor() );
  else if ( activePage == mAppearancePage->pageIndex() )
    kapp->invokeHelp( mAppearancePage->helpAnchor() );
  else if ( activePage == mComposerPage->pageIndex() )
    kapp->invokeHelp( mComposerPage->helpAnchor() );
  else if ( activePage == mSecurityPage->pageIndex() )
    kapp->invokeHelp( mSecurityPage->helpAnchor() );
  else if ( activePage == mMiscPage->pageIndex() )
    kapp->invokeHelp( mMiscPage->helpAnchor() );
  else if ( activePage == mPluginPage->pageIndex() )
    kapp->invokeHelp( mPluginPage->helpAnchor() );
  else
    kdDebug(5006) << "ConfigureDialog::slotHelp(): no page selected???"
		  << endl;
}

void ConfigureDialog::setup()
{
  mIdentityPage->setup();
  mNetworkPage->setup();
  mAppearancePage->setup();
  mComposerPage->setup();
  mSecurityPage->setup();
  mMiscPage->setup();
  mPluginPage->setup();
}

void ConfigureDialog::slotInstallProfile( KConfig * profile ) {
  mIdentityPage->installProfile( profile );
  mNetworkPage->installProfile( profile );
  mAppearancePage->installProfile( profile );
  mComposerPage->installProfile( profile );
  mSecurityPage->installProfile( profile );
  mMiscPage->installProfile( profile );
  mPluginPage->installProfile( profile );
}

void ConfigureDialog::apply( bool everything ) {
  int activePage = activePageIndex();

  if ( everything || activePage == mAppearancePage->pageIndex() )
    mAppearancePage->apply(); // must be first, since it may install profiles!

  if ( everything || activePage == mIdentityPage->pageIndex() )
    mIdentityPage->apply();

  if ( everything || activePage == mNetworkPage->pageIndex() )
    mNetworkPage->apply();

  if ( everything || activePage == mComposerPage->pageIndex() )
    mComposerPage->apply();

  if ( everything || activePage == mSecurityPage->pageIndex() )
    mSecurityPage->apply();

  if ( everything || activePage == mMiscPage->pageIndex() )
    mMiscPage->apply();

  if ( everything || activePage == mPluginPage->pageIndex() )
    mPluginPage->apply();

  //
  // Make other components read the new settings
  //
  KMMessage::readConfig();
  kernel->kbp()->busy(); // this can take some time when a large folder is open
  QPtrListIterator<KMainWindow> it( *KMainWindow::memberList );
  for ( it.toFirst() ; it.current() ; ++it )
    // ### FIXME: use dynamic_cast.
    if ( (*it)->inherits( "KMTopLevelWidget" ) )
      ((KMTopLevelWidget*)(*it))->readConfig();
  kernel->kbp()->idle();
}



// *************************************************************
// *                                                           *
// *                      IdentityPage                         *
// *                                                           *
// *************************************************************

QString IdentityPage::iconLabel() {
  return i18n("Identities");
}

QString IdentityPage::title() {
  return i18n("Manage Identities");
}

const char * IdentityPage::iconName() {
  return "identity";
}

QString IdentityPage::helpAnchor() {
  return QString::fromLatin1("configure-identity");
}

IdentityPage::IdentityPage( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name ),
    mIdentityDialog( 0 )
{
  QHBoxLayout * hlay = new QHBoxLayout( this, 0, KDialog::spacingHint() );

  mIdentityList = new IdentityListView( this );
  connect( mIdentityList, SIGNAL(selectionChanged(QListViewItem*)),
	   SLOT(slotIdentitySelectionChanged(QListViewItem*)) );
  connect( mIdentityList, SIGNAL(itemRenamed(QListViewItem*,const QString&,int)),
	   SLOT(slotRenameIdentity(QListViewItem*,const QString&,int)) );
  // ### connect dragged(...), doubleClicked(...)

  hlay->addWidget( mIdentityList, 1 );

  QVBoxLayout * vlay = new QVBoxLayout( hlay ); // inherits spacing

  QPushButton * button = new QPushButton( i18n("&New..."), this );
  mModifyButton = new QPushButton( i18n("&Modify..."), this );
  mRemoveButton = new QPushButton( i18n("&Remove..."), this );
  mSetAsDefaultButton = new QPushButton( i18n("Set as &Default"), this );
  button->setAutoDefault( false );
  mModifyButton->setAutoDefault( false );
  mModifyButton->setEnabled( false );
  mRemoveButton->setAutoDefault( false );
  mRemoveButton->setEnabled( false );
  mSetAsDefaultButton->setAutoDefault( false );
  mSetAsDefaultButton->setEnabled( false );
  connect( button, SIGNAL(clicked()),
	   this, SLOT(slotNewIdentity()) );
  connect( mModifyButton, SIGNAL(clicked()),
	   this, SLOT(slotModifyIdentity()) );
  connect( mRemoveButton, SIGNAL(clicked()),
	   this, SLOT(slotRemoveIdentity()) );
  connect( mSetAsDefaultButton, SIGNAL(clicked()),
	   this, SLOT(slotSetAsDefault()) );
  vlay->addWidget( button );
  vlay->addWidget( mModifyButton );
  vlay->addWidget( mRemoveButton );
  vlay->addWidget( mSetAsDefaultButton );
  vlay->addStretch( 1 );
}

void IdentityPage::setup()
{
  kdDebug() << "IdentityPage::setup()" << endl;
  IdentityManager * im = kernel->identityManager();
  mOldNumberOfIdentities = im->shadowIdentities().count();
  // Fill the list:
  mIdentityList->clear();
  // Don't use ConstIterator here - it iterates over the wrong list!
  QListViewItem * item = 0;
  for ( IdentityManager::Iterator it = im->begin() ; it != im->end() ; ++it )
    item = new IdentityListViewItem( mIdentityList, item, *it  );
  mIdentityList->setSelected( mIdentityList->currentItem(), true );
}

void IdentityPage::apply() {
  assert( !mIdentityDialog );

  kernel->identityManager()->sort();
  kernel->identityManager()->commit();

  if( mOldNumberOfIdentities < 2 && mIdentityList->childCount() > 1 ) {
    // have more than one identity, so better show the combo in the
    // composer now:
    KConfigGroup composer( kapp->config(), "Composer" );
    int showHeaders = composer.readNumEntry( "headers", HDR_STANDARD );
    showHeaders |= HDR_IDENTITY;
    composer.writeEntry( "headers", showHeaders );
  }
}

void IdentityPage::dismiss() {
  assert( !mIdentityDialog );
  kernel->identityManager()->rollback();
}

void IdentityPage::slotNewIdentity()
{
  assert( !mIdentityDialog );

  IdentityManager * im = kernel->identityManager();
  NewIdentityDialog dialog( im->shadowIdentities(), this, "new", true );

  if( dialog.exec() == QDialog::Accepted ) {
    QString identityName = dialog.identityName().stripWhiteSpace();
    assert( !identityName.isEmpty() );

    //
    // Construct a new Identity:
    //
    switch ( dialog.duplicateMode() ) {
    case NewIdentityDialog::ExistingEntry:
      {
	KMIdentity & dupThis = im->identityForName( dialog.duplicateIdentity() );
	im->newFromExisting( dupThis, identityName );
	break;
      }
    case NewIdentityDialog::ControlCenter:
      im->newFromControlCenter( identityName );
      break;
    case NewIdentityDialog::Empty:
      im->newFromScratch( identityName );
    default: ;
    }
    
    //
    // Insert into listview:
    //
    KMIdentity & newIdent = im->identityForName( identityName );
    QListViewItem * item = mIdentityList->selectedItem();
    if ( item )
      item = item->itemAbove();
    mIdentityList->setSelected( new IdentityListViewItem( mIdentityList,
							  /*after*/ item,
							  newIdent ), true );
  }
}

void IdentityPage::slotModifyIdentity() {
  assert( !mIdentityDialog );

  IdentityListViewItem * item =
    dynamic_cast<IdentityListViewItem*>( mIdentityList->selectedItem() );
  if ( !item ) return;

  mIdentityDialog = new IdentityDialog( this );
  mIdentityDialog->setIdentity( item->identity() );

  // Hmm, an unmodal dialog would be nicer, but a modal one is easier ;-)
  if ( mIdentityDialog->exec() == QDialog::Accepted ) {
    mIdentityDialog->updateIdentity( item->identity() );
    item->redisplay();
  }

  delete mIdentityDialog;
  mIdentityDialog = 0;
}

void IdentityPage::slotRemoveIdentity()
{
  assert( !mIdentityDialog );
  
  IdentityManager * im = kernel->identityManager();
  kdFatal( im->shadowIdentities().count() < 2 )
    << "Attempted to remove the last identity!" << endl;
  
  IdentityListViewItem * item =
    dynamic_cast<IdentityListViewItem*>( mIdentityList->selectedItem() );
  if ( !item ) return;
  
  QString msg = i18n("<qt>Do you really want to remove the identity named "
		     "<b>%1</b>?</qt>").arg( item->identity().identityName() );
  if( KMessageBox::warningYesNo( this, msg ) == KMessageBox::Yes )
    if ( im->removeIdentity( item->identity().identityName() ) ) {
      delete item;
      mIdentityList->setSelected( mIdentityList->currentItem(), true );
      refreshList();
    }
}

void IdentityPage::slotRenameIdentity( QListViewItem * i,
				       const QString & s, int col ) {
  assert( col == 0 );

  IdentityListViewItem * item = dynamic_cast<IdentityListViewItem*>( i );
  if ( !item ) return;

  QString newName = s.stripWhiteSpace();
  if ( !newName.isEmpty() &&
       !kernel->identityManager()->shadowIdentities().contains( newName ) ) {
    KMIdentity & ident = item->identity();
    ident.setIdentityName( newName );
  }
  item->redisplay();
}
  

void IdentityPage::slotSetAsDefault() {
  assert( !mIdentityDialog );
  
  IdentityListViewItem * item =
    dynamic_cast<IdentityListViewItem*>( mIdentityList->selectedItem() );
  if ( !item ) return;
  
  IdentityManager * im = kernel->identityManager();
  im->setAsDefault( item->identity().identityName() );
  refreshList();
}

void IdentityPage::refreshList() {
  for ( QListViewItemIterator it( mIdentityList ) ; it.current() ; ++it ) {
    IdentityListViewItem * item =
      dynamic_cast<IdentityListViewItem*>(it.current());
    if ( item )
      item->redisplay();
  }
}

void IdentityPage::slotIdentitySelectionChanged( QListViewItem * i ) {
  kdDebug() << "IdentityPage::slotIdentitySelectionChanged( " << i << " )" << endl;

  IdentityListViewItem * item = dynamic_cast<IdentityListViewItem*>( i );

  mRemoveButton->setEnabled( item && mIdentityList->childCount() > 1 );
  mModifyButton->setEnabled( item );
  mSetAsDefaultButton->setEnabled( item && !item->identity().isDefault() );
}

void IdentityPage::slotUpdateTransportCombo( const QStringList & sl )
{
  if ( mIdentityDialog ) mIdentityDialog->slotUpdateTransportCombo( sl );
}



// *************************************************************
// *                                                           *
// *                       NetworkPage                         *
// *                                                           *
// *************************************************************

QString NetworkPage::iconLabel() {
  return i18n("Network");
}

QString NetworkPage::title() {
  return i18n("Setup for Sending and Receiving Messages");
}

const char * NetworkPage::iconName() {
  return "network";
}

QString NetworkPage::helpAnchor() {
  return QString::fromLatin1("configure-network");
}

NetworkPage::NetworkPage( QWidget * parent, const char * name )
  : TabbedConfigurationPage( parent, name )
{
  //
  // "Sending" tab:
  //
  mSendingTab = new SendingTab();
  addTab( mSendingTab, mSendingTab->title() );
  connect( mSendingTab, SIGNAL(transportListChanged(const QStringList&)),
	   this, SIGNAL(transportListChanged(const QStringList&)) );

  //
  // "Receiving" tab:
  //
  mReceivingTab = new ReceivingTab();
  addTab( mReceivingTab, mReceivingTab->title() );

  connect( mReceivingTab, SIGNAL(accountListChanged(const QStringList &)),
	   this, SIGNAL(accountListChanged(const QStringList &)) );
}

void NetworkPage::setup() {
  mSendingTab->setup();
  mReceivingTab->setup();
}

void NetworkPage::installProfile( KConfig * profile ) {
  mSendingTab->installProfile( profile );
  mReceivingTab->installProfile( profile );
}

void NetworkPage::apply() {
  mSendingTab->apply();
  mReceivingTab->apply();
}

void NetworkPage::dismiss() {
  mReceivingTab->dismiss();
}

QString NetworkPage::SendingTab::title() {
  return i18n("&Sending");
}

QString NetworkPage::SendingTab::helpAnchor() {
  return QString::fromLatin1("configure-network-sending");
}

NetworkPageSendingTab::NetworkPageSendingTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name )
{
  mTransportInfoList.setAutoDelete( true );
  // temp. vars:
  QVBoxLayout *vlay;
  QVBoxLayout *btn_vlay;
  QHBoxLayout *hlay;
  QGridLayout *glay;
  QPushButton *button;
  QGroupBox   *group;

  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );
  // label: zero stretch ### FIXME more
  vlay->addWidget( new QLabel( i18n("Outgoing accounts (add at least one):"), this ) );

  // hbox layout: stretch 10, spacing inherited from vlay
  hlay = new QHBoxLayout();
  vlay->addLayout( hlay, 10 ); // high stretch b/c of the groupbox's sizeHint

  // transport list: left widget in hlay; stretch 1
  // ### FIXME: allow inline renaming of the account:
  mTransportList = new ListView( this, "transportList", 5 );
  mTransportList->addColumn( i18n("Name") );
  mTransportList->addColumn( i18n("Type") );
  mTransportList->setAllColumnsShowFocus( true );
  mTransportList->setFrameStyle( QFrame::WinPanel + QFrame::Sunken );
  mTransportList->setSorting( -1 );
  connect( mTransportList, SIGNAL(selectionChanged()),
           this, SLOT(slotTransportSelected()) );
  connect( mTransportList, SIGNAL(doubleClicked( QListViewItem *)),
           this, SLOT(slotModifySelectedTransport()) );
  hlay->addWidget( mTransportList, 1 );

  // a vbox layout for the buttons: zero stretch, spacing inherited from hlay
  btn_vlay = new QVBoxLayout( hlay );

  // "add..." button: stretch 0
  button = new QPushButton( i18n("A&dd..."), this );
  button->setAutoDefault( false );
  connect( button, SIGNAL(clicked()),
	   this, SLOT(slotAddTransport()) );
  btn_vlay->addWidget( button );

  // "modify..." button: stretch 0
  mModifyTransportButton = new QPushButton( i18n("&Modify..."), this );
  mModifyTransportButton->setAutoDefault( false );
  mModifyTransportButton->setEnabled( false ); // b/c no item is selected yet
  connect( mModifyTransportButton, SIGNAL(clicked()),
	   this, SLOT(slotModifySelectedTransport()) );
  btn_vlay->addWidget( mModifyTransportButton );

  // "remove" button: stretch 0
  mRemoveTransportButton = new QPushButton( i18n("R&emove"), this );
  mRemoveTransportButton->setAutoDefault( false );
  mRemoveTransportButton->setEnabled( false ); // b/c no item is selected yet
  connect( mRemoveTransportButton, SIGNAL(clicked()),
	   this, SLOT(slotRemoveSelectedTransport()) );
  btn_vlay->addWidget( mRemoveTransportButton );

  // "up" button: stretch 0
  // ### FIXME: shouldn't this be a QToolButton?
  mTransportUpButton = new QPushButton( QString::null, this );
  mTransportUpButton->setPixmap( BarIcon( "up", KIcon::SizeSmall ) );
  //  mTransportUpButton->setPixmap( BarIcon( "up", KIcon::SizeSmall ) );
  mTransportUpButton->setAutoDefault( false );
  mTransportUpButton->setEnabled( false ); // b/c no item is selected yet
  connect( mTransportUpButton, SIGNAL(clicked()),
           this, SLOT(slotTransportUp()) );
  btn_vlay->addWidget( mTransportUpButton );

  // "down" button: stretch 0
  // ### FIXME: shouldn't this be a QToolButton?
  mTransportDownButton = new QPushButton( QString::null, this );
  mTransportDownButton->setPixmap( BarIcon( "down", KIcon::SizeSmall ) );
  //  mTransportDownButton->setPixmap( BarIcon( "down", KIcon::SizeSmall ) );
  mTransportDownButton->setAutoDefault( false );
  mTransportDownButton->setEnabled( false ); // b/c no item is selected yet
  connect( mTransportDownButton, SIGNAL(clicked()),
           this, SLOT(slotTransportDown()) );
  btn_vlay->addWidget( mTransportDownButton );
  btn_vlay->addStretch( 1 ); // spacer

  // "Common options" groupbox:
  group = new QGroupBox( 0, Qt::Vertical,
			 i18n("Common Options"), this );
  vlay->addWidget(group);

  // a grid layout for the contents of the "common options" group box
  glay = new QGridLayout( group->layout(), 5, 3, KDialog::spacingHint() );
  glay->setColStretch( 2, 10 );

  // "confirm before send" check box:
  mConfirmSendCheck = new QCheckBox( i18n("Confirm &before send"), group );
  glay->addMultiCellWidget( mConfirmSendCheck, 0, 0, 0, 1 );

  // "send messages in outbox on check" check box:
  mSendOutboxCheck =
    new QCheckBox(i18n("Send messages in outbox &folder on check"), group );
  glay->addMultiCellWidget( mSendOutboxCheck, 1, 1, 0, 1 );

  // "default send method" combo:
  mSendMethodCombo = new QComboBox( false, group );
  mSendMethodCombo->insertStringList( QStringList()
				      << i18n("Send Now")
				      << i18n("Send Later") );
  glay->addWidget( mSendMethodCombo, 2, 1 );

  // "message property" combo:
  // ### FIXME: remove completely?
  mMessagePropertyCombo = new QComboBox( false, group );
  mMessagePropertyCombo->insertStringList( QStringList()
		     << i18n("Allow 8-bit")
		     << i18n("MIME Compliant (Quoted Printable)") );
  glay->addWidget( mMessagePropertyCombo, 3, 1 );

  // "default domain" input field:
  mDefaultDomainEdit = new QLineEdit( group );
  glay->addMultiCellWidget( mDefaultDomainEdit, 4, 4, 1, 2 );

  // labels:
  glay->addWidget( new QLabel( mSendMethodCombo, /*buddy*/
			       i18n("Defa&ult send method:"), group ), 2, 0 );
  glay->addWidget( new QLabel( mMessagePropertyCombo, /*buddy*/
			       i18n("Message &property:"), group ), 3, 0 );
  QLabel *l = new QLabel( mDefaultDomainEdit, /*buddy*/
                          i18n("Default domain:"), group );
  glay->addWidget( l, 4, 0 );

  // and now: add QWhatsThis:
  QString msg = i18n( "<qt><p>The default domain is used to complete email "
                      "addresses that only consist of the user's name."
                      "</p></qt>" );
  QWhatsThis::add( l, msg );
  QWhatsThis::add( mDefaultDomainEdit, msg );
};


void NetworkPage::SendingTab::slotTransportSelected()
{
  QListViewItem *cur = mTransportList->currentItem();
  mModifyTransportButton->setEnabled( cur );
  mRemoveTransportButton->setEnabled( cur );
  mTransportDownButton->setEnabled( cur && cur->itemBelow() );
  mTransportUpButton->setEnabled( cur && cur->itemAbove() );
}

// adds a number to @p name to make the name unique
static inline QString uniqueName( const QStringList & list,
				  const QString & name )
{
  int suffix = 1;
  QString result = name;
  while ( list.find( result ) != list.end() ) {
    result = i18n("%1: name; %2: number appended to it to make it unique "
		  "among a list of names", "%1 %2")
      .arg( name ).arg( suffix );
    suffix++;
  }
  return result;
}

void NetworkPage::SendingTab::slotAddTransport()
{
  int transportType;

  { // limit scope of selDialog
    KMTransportSelDlg selDialog( this );
    if ( selDialog.exec() != QDialog::Accepted ) return;
    transportType = selDialog.selected();
  }

  KMTransportInfo *transportInfo = new KMTransportInfo();
  switch ( transportType ) {
  case 0: // smtp
    transportInfo->type = QString::fromLatin1("smtp");
    break;
  case 1: // sendmail
    transportInfo->type = QString::fromLatin1("sendmail");
    transportInfo->name = i18n("Sendmail");
    transportInfo->host = _PATH_SENDMAIL; // ### FIXME: use const, not #define
    break;
  default:
    assert( 0 );
  }

  KMTransportDialog dialog( i18n("Add Transport"), transportInfo, this );

  // create list of names:
  // ### move behind dialog.exec()?
  QStringList transportNames;
  QPtrListIterator<KMTransportInfo> it( mTransportInfoList );
  for ( it.toFirst() ; it.current() ; ++it )
    transportNames << (*it)->name;

  if( dialog.exec() != QDialog::Accepted ) {
    delete transportInfo;
    return;
  }

  // disambiguate the name by appending a number:
  // ### FIXME: don't allow this error to happen in the first place!
  transportInfo->name = uniqueName( transportNames, transportInfo->name );
  // append to names and transportinfo lists:
  transportNames << transportInfo->name;
  mTransportInfoList.append( transportInfo );

  // append to listview:
  // ### FIXME: insert before the selected item, append on empty selection
  QListViewItem *lastItem = mTransportList->firstChild();
  QString typeDisplayName;
  if ( lastItem )
    while ( lastItem->nextSibling() )
      lastItem = lastItem->nextSibling();
  if ( lastItem )
    typeDisplayName = transportInfo->type;
  else
    typeDisplayName = i18n("%1: type of transport. Result used in "
			   "Configure->Network->Sending listview, \"type\" "
			   "column, first row, to indicate that this is the "
			   "default transport", "%1 (Default)")
      .arg( transportInfo->type );
  (void) new QListViewItem( mTransportList, lastItem, transportInfo->name,
			    typeDisplayName );

  // notify anyone who cares:
  emit transportListChanged( transportNames );
}

void NetworkPage::SendingTab::slotModifySelectedTransport()
{
  QListViewItem *item = mTransportList->currentItem();
  if ( !item ) return;

  QPtrListIterator<KMTransportInfo> it( mTransportInfoList );
  for ( it.toFirst() ; it.current() ; ++it )
    if ( (*it)->name == item->text(0) ) break;
  if ( !it.current() ) return;

  KMTransportDialog dialog( i18n("Modify Transport"), (*it), this );

  if ( dialog.exec() != QDialog::Accepted ) return;

  // create the list of names of transports, but leave out the current
  // item:
  QStringList transportNames;
  QPtrListIterator<KMTransportInfo> jt( mTransportInfoList );
  int entryLocation = -1;
  for ( jt.toFirst() ; jt.current() ; ++jt )
    if ( jt != it )
      transportNames << (*jt)->name;
    else
      entryLocation = transportNames.count();
  assert( entryLocation >= 0 );

  // make the new name unique by appending a high enough number:
  (*it)->name = uniqueName( transportNames, (*it)->name );
  // change the list item to the new name
  item->setText( 0, (*it)->name );
  // and insert the new name at the position of the old in the list of
  // strings; then broadcast the new list:
  transportNames.insert( transportNames.at( entryLocation ), (*it)->name );
  emit transportListChanged( transportNames );
}


void NetworkPage::SendingTab::slotRemoveSelectedTransport()
{
  QListViewItem *item = mTransportList->currentItem();
  if ( !item ) return;

  QPtrListIterator<KMTransportInfo> it( mTransportInfoList );
  for ( it.toFirst() ; it.current() ; ++it )
    if ( (*it)->name == item->text(0) ) break;
  if ( !it.current() ) return;

  QListViewItem *newCurrent = item->itemBelow();
  if ( !newCurrent ) newCurrent = item->itemAbove();
  //mTransportList->removeItem( item );
  if ( newCurrent ) {
    mTransportList->setCurrentItem( newCurrent );
    mTransportList->setSelected( newCurrent, true );
  }

  delete item;
  mTransportInfoList.remove( it );

  QStringList transportNames;
  for ( it.toFirst() ; it.current() ; ++it )
    transportNames << (*it)->name;
  emit transportListChanged( transportNames );
}


void NetworkPage::SendingTab::slotTransportUp()
{
  QListViewItem *item = mTransportList->selectedItem();
  if ( !item ) return;
  QListViewItem *above = item->itemAbove();
  if ( !above ) return;

  // swap in the transportInfo list:
  // ### FIXME: use value-based list. This is ugly.
  KMTransportInfo *ti, *ti2 = NULL;
  int i = 0;
  for (ti = mTransportInfoList.first(); ti;
    ti2 = ti, ti = mTransportInfoList.next(), i++)
      if (ti->name == item->text(0)) break;
  if (!ti || !ti2) return;
  ti = mTransportInfoList.take(i);
  mTransportInfoList.insert(i-1, ti);

  // swap in the display
  item->setText(0, ti2->name);
  item->setText(1, ti2->type);
  above->setText(0, ti->name);
  if ( above->itemAbove() )
    // not first:
    above->setText( 1, ti->type );
  else
    // first:
    above->setText( 1, i18n("%1: type of transport. Result used in "
			    "Configure->Network->Sending listview, \"type\" "
			    "column, first row, to indicate that this is the "
			    "default transport", "%1 (Default)")
		    .arg( ti->type ) );

  mTransportList->setCurrentItem( above );
  mTransportList->setSelected( above, true );
}


void NetworkPage::SendingTab::slotTransportDown()
{
  QListViewItem * item = mTransportList->selectedItem();
  if ( !item ) return;
  QListViewItem * below = item->itemBelow();
  if ( !below ) return;

  KMTransportInfo *ti, *ti2 = NULL;
  int i = 0;
  for (ti = mTransportInfoList.first(); ti;
       ti = mTransportInfoList.next(), i++)
    if (ti->name == item->text(0)) break;
  ti2 = mTransportInfoList.next();
  if (!ti || !ti2) return;
  ti = mTransportInfoList.take(i);
  mTransportInfoList.insert(i+1, ti);

  item->setText(0, ti2->name);
  below->setText(0, ti->name);
  below->setText(1, ti->type);
  if ( item->itemAbove() )
    item->setText( 1, ti2->type );
  else
    item->setText( 1, i18n("%1: type of transport. Result used in "
			   "Configure->Network->Sending listview, \"type\" "
			   "column, first row, to indicate that this is the "
			   "default transport", "%1 (Default)")
		   .arg( ti2->type ) );


  mTransportList->setCurrentItem(below);
  mTransportList->setSelected(below, TRUE);
}

void NetworkPage::SendingTab::setup() {
  KConfigGroup general( kapp->config(), "General");
  KConfigGroup composer( kapp->config(), "Composer");

  int numTransports = general.readNumEntry("transports", 0);

  QListViewItem *top = 0;
  mTransportInfoList.clear();
  mTransportList->clear();
  QStringList transportNames;
  for ( int i = 1 ; i <= numTransports ; i++ ) {
    KMTransportInfo *ti = new KMTransportInfo();
    ti->readConfig(i);
    mTransportInfoList.append( ti );
    transportNames << ti->name;
    top = new QListViewItem( mTransportList, top, ti->name, ti->type );
  }
  emit transportListChanged( transportNames );

  QListViewItem *listItem = mTransportList->firstChild();
  if ( listItem ) {
    listItem->setText( 1, i18n("%1: type of transport. Result used in "
			       "Configure->Network->Sending listview, "
			       "\"type\" column, first row, to indicate "
			       "that this is the default transport",
			       "%1 (Default)").arg( listItem->text(1) ) );
    mTransportList->setCurrentItem( listItem );
    mTransportList->setSelected( listItem, true );
  }

  mSendMethodCombo->setCurrentItem(
		kernel->msgSender()->sendImmediate() ? 0 : 1 );
  mMessagePropertyCombo->setCurrentItem(
                kernel->msgSender()->sendQuotedPrintable() ? 1 : 0 );
  mSendOutboxCheck->setChecked( general.readBoolEntry( "sendOnCheck",
						       false ) );

  mConfirmSendCheck->setChecked( composer.readBoolEntry( "confirm-before-send",
							 false ) );
  QString str = general.readEntry( "Default domain", "" );
  if( str.isEmpty() )
  {
    //### FIXME: Use the global convenience function instead of the homebrewed
    //           solution once we can rely on HEAD kdelibs.
    //str = KGlobal::hostname(); ???????
    char buffer[256];
    if ( !gethostname( buffer, 255 ) )
      // buffer need not be NUL-terminated if it has full length
      buffer[255] = 0;
    else
      buffer[0] = 0;
    str = QString::fromLatin1( *buffer ? buffer : "localhost" );
  }
  mDefaultDomainEdit->setText( str );
}


void NetworkPage::SendingTab::apply() {
  KConfigGroup general( kapp->config(), "General" );
  KConfigGroup composer( kapp->config(), "Composer" );

  // Save transports:
  general.writeEntry( "transports", mTransportInfoList.count() );
  QPtrListIterator<KMTransportInfo> it( mTransportInfoList );
  for ( int i = 1 ; it.current() ; ++it, ++i )
    (*it)->writeConfig(i);

  // Save common options:
  general.writeEntry( "sendOnCheck", mSendOutboxCheck->isChecked() );
  kernel->msgSender()->setSendImmediate(
			     mSendMethodCombo->currentItem() == 0 );
  kernel->msgSender()->setSendQuotedPrintable(
			     mMessagePropertyCombo->currentItem() == 1 );
  kernel->msgSender()->writeConfig( false ); // don't sync
  composer.writeEntry("confirm-before-send", mConfirmSendCheck->isChecked() );
  general.writeEntry( "Default domain", mDefaultDomainEdit->text() );
}



QString NetworkPage::ReceivingTab::title() {
  return i18n("&Receiving");
}

QString NetworkPage::ReceivingTab::helpAnchor() {
  return QString::fromLatin1("configure-network-receiving");
}

NetworkPageReceivingTab::NetworkPageReceivingTab( QWidget * parent, const char * name )
  : ConfigurationPage ( parent, name )
{
  // temp. vars:
  QVBoxLayout *vlay;
  QVBoxLayout *btn_vlay;
  QHBoxLayout *hlay;
  QPushButton *button;
  QGroupBox   *group;

  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // label: zero stretch
  vlay->addWidget( new QLabel( i18n("Incoming accounts (add at least one):"), this ) );

  // hbox layout: stretch 10, spacing inherited from vlay
  hlay = new QHBoxLayout();
  vlay->addLayout( hlay, 10 ); // high stretch to suppress groupbox's growing

  // account list: left widget in hlay; stretch 1
  mAccountList = new ListView( this, "accountList", 5 );
  mAccountList->addColumn( i18n("Name") );
  mAccountList->addColumn( i18n("Type") );
  mAccountList->addColumn( i18n("Folder") );
  mAccountList->setAllColumnsShowFocus( true );
  mAccountList->setFrameStyle( QFrame::WinPanel + QFrame::Sunken );
  mAccountList->setSorting( -1 );
  connect( mAccountList, SIGNAL(selectionChanged()),
	   this, SLOT(slotAccountSelected()) );
  connect( mAccountList, SIGNAL(doubleClicked( QListViewItem *)),
	   this, SLOT(slotModifySelectedAccount()) );
  hlay->addWidget( mAccountList, 1 );

  // a vbox layout for the buttons: zero stretch, spacing inherited from hlay
  btn_vlay = new QVBoxLayout( hlay );

  // "add..." button: stretch 0
  button = new QPushButton( i18n("A&dd..."), this );
  button->setAutoDefault( false );
  connect( button, SIGNAL(clicked()),
	   this, SLOT(slotAddAccount()) );
  btn_vlay->addWidget( button );

  // "modify..." button: stretch 0
  mModifyAccountButton = new QPushButton( i18n("&Modify..."), this );
  mModifyAccountButton->setAutoDefault( false );
  mModifyAccountButton->setEnabled( false ); // b/c no item is selected yet
  connect( mModifyAccountButton, SIGNAL(clicked()),
	   this, SLOT(slotModifySelectedAccount()) );
  btn_vlay->addWidget( mModifyAccountButton );

  // "remove..." button: stretch 0
  mRemoveAccountButton = new QPushButton( i18n("R&emove"), this );
  mRemoveAccountButton->setAutoDefault( false );
  mRemoveAccountButton->setEnabled( false ); // b/c no item is selected yet
  connect( mRemoveAccountButton, SIGNAL(clicked()),
	   this, SLOT(slotRemoveSelectedAccount()) );
  btn_vlay->addWidget( mRemoveAccountButton );
  btn_vlay->addStretch( 1 ); // spacer

  // "New Mail Notification" group box: stretch 0
  group = new QVGroupBox( i18n("New Mail Notification"), this );
  vlay->addWidget( group );
  group->layout()->setSpacing( KDialog::spacingHint() );

  // "beep on new mail" check box:
  mBeepNewMailCheck = new QCheckBox(i18n("&Beep"), group );

  // "display message box" check box:
  mShowMessageBoxCheck = new QCheckBox(i18n("D&isplay message box"), group );

  // "Execute command" check box:
  mMailCommandCheck = new QCheckBox( i18n("E&xecute command line"), group );

  // HBox layout for the "specify command" line:
  QHBox *hbox = new QHBox( group );

  // command line requester (stretch 1) and label (stretch 0):
  QLabel *label = new QLabel( i18n("S&pecify command:"), hbox );
  mMailCommandRequester = new KURLRequester( hbox );
  label->setBuddy( mMailCommandRequester );
  mMailCommandRequester->setEnabled( false );
  label->setEnabled( false ); // b/c !mMailCommandCheck->isChecked()

  // Connections that {en,dis}able the "specify command" according to
  // the state of the "Execute command" check box:
  connect( mMailCommandCheck, SIGNAL(toggled(bool)),
	   label, SLOT(setEnabled(bool)) );
  connect( mMailCommandCheck, SIGNAL(toggled(bool)),
	   mMailCommandRequester, SLOT(setEnabled(bool)) );
}


void NetworkPage::ReceivingTab::slotAccountSelected()
{
  QListViewItem * item = mAccountList->selectedItem();
  mModifyAccountButton->setEnabled( item );
  mRemoveAccountButton->setEnabled( item );
}

QStringList NetworkPage::ReceivingTab::occupiedNames()
{
  QStringList accountNames = kernel->acctMgr()->getAccounts();

  QValueList<ModifiedAccountsType*>::Iterator k;
  for (k = mModifiedAccounts.begin(); k != mModifiedAccounts.end(); ++k )
    if ((*k)->oldAccount)
      accountNames.remove( (*k)->oldAccount->name() );

  QValueList< QGuardedPtr<KMAccount> >::Iterator l;
  for (l = mAccountsToDelete.begin(); l != mAccountsToDelete.end(); ++l )
    if (*l)
      accountNames.remove( (*l)->name() );

  QValueList< QGuardedPtr<KMAccount> >::Iterator it;
  for (it = mNewAccounts.begin(); it != mNewAccounts.end(); ++it )
    if (*it)
      accountNames += (*it)->name();

  QValueList<ModifiedAccountsType*>::Iterator j;
  for (j = mModifiedAccounts.begin(); j != mModifiedAccounts.end(); ++j )
    accountNames += (*j)->newAccount->name();

  return accountNames;
}

void NetworkPage::ReceivingTab::slotAddAccount() {
  KMAcctSelDlg accountSelectorDialog( this );
  if( accountSelectorDialog.exec() != QDialog::Accepted ) return;

  const char *accountType = 0;
  switch ( accountSelectorDialog.selected() ) {
    case 0: accountType = "local";   break;
    case 1: accountType = "pop";     break;
    case 2: accountType = "imap";    break;
    case 3: accountType = "maildir"; break;

    default:
      // ### FIXME: How should this happen???
      // replace with assert.
      KMessageBox::sorry( this, i18n("Unknown account type selected") );
      return;
  }

  KMAccount *account
    = kernel->acctMgr()->create( QString::fromLatin1( accountType ),
				 i18n("Unnamed") );
  if ( !account ) {
    // ### FIXME: Give the user more information. Is this error
    // recoverable?
    KMessageBox::sorry( this, i18n("Unable to create account") );
    return;
  }

  account->init(); // fill the account fields with good default values

  AccountDialog dialog( i18n("Add account"), account, this );

  QStringList accountNames = occupiedNames();

  if( dialog.exec() != QDialog::Accepted ) {
    delete account;
    return;
  }

  account->setName( uniqueName( accountNames, account->name() ) );

  QListViewItem *after = mAccountList->firstChild();
  while ( after && after->nextSibling() )
    after = after->nextSibling();

  QListViewItem *listItem =
    new QListViewItem( mAccountList, after, account->name(), account->type() );
  if( account->folder() )
    listItem->setText( 2, account->folder()->label() );

  mNewAccounts.append( account );
}



void NetworkPage::ReceivingTab::slotModifySelectedAccount()
{
  QListViewItem *listItem = mAccountList->selectedItem();
  if( !listItem ) return;

  KMAccount *account = 0;
  QValueList<ModifiedAccountsType*>::Iterator j;
  for (j = mModifiedAccounts.begin(); j != mModifiedAccounts.end(); ++j )
    if ( (*j)->newAccount->name() == listItem->text(0) ) {
      account = (*j)->newAccount;
      break;
    }

  if ( !account ) {
    QValueList< QGuardedPtr<KMAccount> >::Iterator it;
    for ( it = mNewAccounts.begin() ; it != mNewAccounts.end() ; ++it )
      if ( (*it)->name() == listItem->text(0) ) {
	account = *it;
	break;
      }

    if ( !account ) {
      account = kernel->acctMgr()->find( listItem->text(0) );
      if( !account ) {
	// ### FIXME: How should this happen? See above.
        KMessageBox::sorry( this, i18n("Unable to locate account") );
        return;
      }

      ModifiedAccountsType *mod = new ModifiedAccountsType;
      mod->oldAccount = account;
      mod->newAccount = kernel->acctMgr()->create( account->type(),
						   account->name() );
      mod->newAccount->pseudoAssign( account );
      mModifiedAccounts.append( mod );
      account = mod->newAccount;
    }

    if( !account ) {
      // ### FIXME: See above.
      KMessageBox::sorry( this, i18n("Unable to locate account") );
      return;
    }
  }

  QStringList accountNames = occupiedNames();
  accountNames.remove( account->name() );

  AccountDialog dialog( i18n("Modify Account"), account, this );

  if( dialog.exec() != QDialog::Accepted ) return;

  account->setName( uniqueName( accountNames, account->name() ) );

  listItem->setText( 0, account->name() );
  listItem->setText( 1, account->type() );
  if( account->folder() )
    listItem->setText( 2, account->folder()->label() );
}



void NetworkPage::ReceivingTab::slotRemoveSelectedAccount() {
  QListViewItem *listItem = mAccountList->selectedItem();
  if( !listItem ) return;

  KMAccount *acct = 0;
  QValueList<ModifiedAccountsType*>::Iterator j;
  for ( j = mModifiedAccounts.begin() ; j != mModifiedAccounts.end() ; ++j )
    if ( (*j)->newAccount->name() == listItem->text(0) ) {
      acct = (*j)->oldAccount;
      mAccountsToDelete.append( acct );
      mModifiedAccounts.remove( j );
      break;
    }
  if ( !acct ) {
    QValueList< QGuardedPtr<KMAccount> >::Iterator it;
    for ( it = mNewAccounts.begin() ; it != mNewAccounts.end() ; ++it )
      if ( (*it)->name() == listItem->text(0) ) {
	acct = *it;
	mNewAccounts.remove( it );
	break;
      }
  }
  if ( !acct ) {
    acct = kernel->acctMgr()->find( listItem->text(0) );
    if ( acct )
      mAccountsToDelete.append( acct );
  }
  if ( !acct ) {
    // ### FIXME: see above
    KMessageBox::sorry( this, i18n("Unable to locate account %1")
			.arg(listItem->text(0)) );
    return;
  }

  QListViewItem * item = listItem->itemBelow();
  if ( !item ) item = listItem->itemAbove();
  delete listItem;

  if ( item )
    mAccountList->setSelected( item, true );
}


void NetworkPage::ReceivingTab::setup() {
  KConfigGroup general( kapp->config(), "General" );

  mAccountList->clear();
  QListViewItem *top = 0;
  for( KMAccount *a = kernel->acctMgr()->first(); a!=0;
       a = kernel->acctMgr()->next() ) {
    QListViewItem *listItem =
      new QListViewItem( mAccountList, top, a->name(), a->type() );
    if( a->folder() )
      listItem->setText( 2, a->folder()->label() );
    top = listItem;
  }

  QListViewItem *listItem = mAccountList->firstChild();
  if ( listItem ) {
    mAccountList->setCurrentItem( listItem );
    mAccountList->setSelected( listItem, true );
  }

  mBeepNewMailCheck->setChecked( general.readBoolEntry("beep-on-mail", false ) );
  mShowMessageBoxCheck->setChecked( general.readBoolEntry("msgbox-on-mail", false) );
  mMailCommandCheck->setChecked( general.readBoolEntry("exec-on-mail", false) );
  mMailCommandRequester->setURL( general.readEntry("exec-on-mail-cmd", ""));

}

void NetworkPage::ReceivingTab::apply() {
  // Add accounts marked as new
  QValueList< QGuardedPtr<KMAccount> >::Iterator it;
  for (it = mNewAccounts.begin(); it != mNewAccounts.end(); ++it )
    kernel->acctMgr()->add( *it );
  mNewAccounts.clear();

  // Update accounts that have been modified
  QValueList<ModifiedAccountsType*>::Iterator j;
  for ( j = mModifiedAccounts.begin() ; j != mModifiedAccounts.end() ; ++j ) {
    (*j)->oldAccount->pseudoAssign( (*j)->newAccount );
    delete (*j)->newAccount;
    delete (*j);
  }
  mModifiedAccounts.clear();

  // Delete accounts marked for deletion
  for ( it = mAccountsToDelete.begin() ;
	it != mAccountsToDelete.end() ; ++it ) {
    // ### FIXME: KConfig has now deleteGroup()!
    // The old entries will never really disappear, so better at least
    // clear the password:
    (*it)->clearPasswd();
    kernel->acctMgr()->writeConfig( true );
    if ( !(*it) || !kernel->acctMgr()->remove(*it) )
      KMessageBox::sorry( this, i18n("Unable to locate account %1")
			  .arg( (*it)->name() ) );
  }
  mAccountsToDelete.clear();

  // Incoming mail
  kernel->acctMgr()->writeConfig( false );
  kernel->cleanupImapFolders();

  // Save Mail notification settings
  KConfigGroup general( kapp->config(), "General" );
  general.writeEntry( "beep-on-mail", mBeepNewMailCheck->isChecked() );
  general.writeEntry( "msgbox-on-mail", mShowMessageBoxCheck->isChecked() );
  general.writeEntry( "exec-on-mail", mMailCommandCheck->isChecked() );
  general.writeEntry( "exec-on-mail-cmd", mMailCommandRequester->url() );
}

void NetworkPage::ReceivingTab::dismiss() {
  // dismiss new accounts:
  for ( QValueList< QGuardedPtr<KMAccount> >::Iterator
	  it = mNewAccounts.begin() ;
	it != mNewAccounts.end() ; ++it )
    delete *it;

  // dismiss modifications of accounts:
  for ( QValueList< ModifiedAccountsType* >::Iterator
	  it = mModifiedAccounts.begin() ;
	it != mModifiedAccounts.end() ; ++it ) {
    delete (*it)->newAccount;
    delete (*it);
  }

  // cancel deletion of accounts:
  mAccountsToDelete.clear();

  mNewAccounts.clear(); // ### Why that? didn't we just delete all items?
  mModifiedAccounts.clear(); // ### see above...
}

// *************************************************************
// *                                                           *
// *                     AppearancePage                        *
// *                                                           *
// *************************************************************

QString AppearancePage::iconLabel() {
  return i18n("Appearance");
}

QString AppearancePage::title() {
  return i18n("Customize Visual Appearance");
}

const char * AppearancePage::iconName() {
  return "appearance";
}

QString AppearancePage::helpAnchor() {
  return QString::fromLatin1("configure-appearance");
}

AppearancePage::AppearancePage( QWidget * parent, const char * name )
  : TabbedConfigurationPage( parent, name )
{
  //
  // "Fonts" tab:
  //
  mFontsTab = new FontsTab();
  addTab( mFontsTab, mFontsTab->title() );

  //
  // "Colors" tab:
  //
  mColorsTab = new ColorsTab();
  addTab( mColorsTab, mColorsTab->title() );

  //
  // "Layout" tab:
  //
  mLayoutTab = new LayoutTab();
  addTab( mLayoutTab, mLayoutTab->title() );

  //
  // "Profile" tab:
  //
  mProfileTab = new ProfileTab();
  addTab( mProfileTab, mProfileTab->title() );

  connect( mProfileTab, SIGNAL(profileSelected(KConfig*)),
	   this, SIGNAL(profileSelected(KConfig*)) );
}

void AppearancePage::setup() {
  mFontsTab->setup();
  mColorsTab->setup();
  mLayoutTab->setup();
  mProfileTab->setup();
}

void AppearancePage::installProfile( KConfig * profile ) {
  mFontsTab->installProfile( profile );
  mColorsTab->installProfile( profile );
  mLayoutTab->installProfile( profile );
  mProfileTab->installProfile( profile );
}

void AppearancePage::apply() {
  mProfileTab->apply(); // must be first, since it may install profiles!
  mFontsTab->apply();
  mColorsTab->apply();
  mLayoutTab->apply();
}


QString AppearancePage::FontsTab::title() {
  return i18n("&Fonts");
}

QString AppearancePage::FontsTab::helpAnchor() {
  return QString::fromLatin1("configure-appearance-fonts");
}

static const struct {
  const char * configName;
  const char * displayName;
  bool   enableFamilyAndSize;
  bool   onlyFixed;
} fontNames[] = {
  { "body-font", I18N_NOOP("Message Body"), true, false },
  { "list-font", I18N_NOOP("Message List"), true, false },
  { "list-date-font", I18N_NOOP("Message List - Date Field"), true, false },
  { "folder-font", I18N_NOOP("Folder List"), true, false },
  { "quote1-font", I18N_NOOP("Quoted Text - First Level"), false, false },
  { "quote2-font", I18N_NOOP("Quoted Text - Second Level"), false, false },
  { "quote3-font", I18N_NOOP("Quoted Text - Third Level"), false, false },
  { "fixed-font", I18N_NOOP("Fixed Width Font"), true, true },
  { "composer-font", I18N_NOOP("Composer"), true, false },
  { "print-font",  I18N_NOOP("Printing Output"), true, false },
};
static const int numFontNames = sizeof fontNames / sizeof *fontNames;

AppearancePageFontsTab::AppearancePageFontsTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name ), mActiveFontIndex( -1 )
{
  assert( numFontNames == sizeof mFont / sizeof *mFont );
  // tmp. vars:
  QVBoxLayout *vlay;
  QHBoxLayout *hlay;
  QLabel      *label;

  // "Use custom fonts" checkbox, followed by <hr>
  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );
  mCustomFontCheck = new QCheckBox( i18n("&Use custom fonts"), this );
  vlay->addWidget( mCustomFontCheck );
  vlay->addWidget( new KSeparator( KSeparator::HLine, this ) );

  // "font location" combo box and label:
  hlay = new QHBoxLayout( vlay ); // inherites spacing
  mFontLocationCombo = new QComboBox( false, this );
  mFontLocationCombo->setEnabled( false ); // !mCustomFontCheck->isChecked()

  QStringList fontDescriptions;
  for ( int i = 0 ; i < numFontNames ; i++ )
    fontDescriptions << i18n( fontNames[i].displayName );
  mFontLocationCombo->insertStringList( fontDescriptions );

  label = new QLabel( mFontLocationCombo, i18n("Apply &to:"), this );
  label->setEnabled( false ); // since !mCustomFontCheck->isChecked()
  hlay->addWidget( label );

  hlay->addWidget( mFontLocationCombo );
  hlay->addStretch( 10 );
  vlay->addSpacing( KDialog::spacingHint() );
  mFontChooser = new KFontChooser( this, "font", false, QStringList(),
				   false, 4 );
  mFontChooser->setEnabled( false ); // since !mCustomFontCheck->isChecked()
  vlay->addWidget( mFontChooser );

  // {en,dis}able widgets depending on the state of mCustomFontCheck:
  connect( mCustomFontCheck, SIGNAL(toggled(bool)),
	   label, SLOT(setEnabled(bool)) );
  connect( mCustomFontCheck, SIGNAL(toggled(bool)),
	   mFontLocationCombo, SLOT(setEnabled(bool)) );
  connect( mCustomFontCheck, SIGNAL(toggled(bool)),
	   mFontChooser, SLOT(setEnabled(bool)) );
  // load the right font settings into mFontChooser:
  connect( mFontLocationCombo, SIGNAL(activated(int) ),
	   this, SLOT(slotFontSelectorChanged(int)) );
}


void AppearancePage::FontsTab::slotFontSelectorChanged( int index )
{
  kdDebug() << "slotFontSelectorChanged() called" << endl;
  if( index < 0 || index >= mFontLocationCombo->count() )
    return; // Should never happen, but it is better to check.

  // Save current fontselector setting before we install the new:
  if( mActiveFontIndex == 0 ) {
    mFont[0] = mFontChooser->font();
    // hardcode the family and size of "message body" dependant fonts:
    for ( int i = 0 ; i < numFontNames ; i++ )
      if ( !fontNames[i].enableFamilyAndSize ) {
	// ### shall we copy the font and set the save and re-set
	// {regular,italic,bold,bold italic} property or should we
	// copy only family and pointSize?
	mFont[i].setFamily( mFont[0].family() );
	mFont[i].setPointSize/*Float?*/( mFont[0].pointSize/*Float?*/() );
      }
  } else if ( mActiveFontIndex > 0 )
    mFont[ mActiveFontIndex ] = mFontChooser->font();
  mActiveFontIndex = index;


  // Display the new setting:
  mFontChooser->setFont( mFont[index], fontNames[index].onlyFixed );

  // Disable Family and Size list if we have selected a quote font:
  mFontChooser->enableColumn( KFontChooser::FamilyList|KFontChooser::SizeList,
			      fontNames[ index ].enableFamilyAndSize );
}

void AppearancePage::FontsTab::setup() {
  KConfigGroup fonts( kapp->config(), "Fonts" );

  mFont[0] = KGlobalSettings::generalFont();
  QFont fixedFont = KGlobalSettings::fixedFont();
  for ( int i = 0 ; i < numFontNames ; i++ )
    mFont[i] = fonts.readFontEntry( fontNames[i].configName,
      (fontNames[i].onlyFixed) ? &fixedFont : &mFont[0] );

  mCustomFontCheck->setChecked( !fonts.readBoolEntry( "defaultFonts", true ) );
  mFontLocationCombo->setCurrentItem( 0 );
  // ### FIXME: possible Qt bug: setCurrentItem doesn't emit activated(int).
  slotFontSelectorChanged( 0 );
}

void AppearancePage::FontsTab::installProfile( KConfig * profile ) {
  KConfigGroup fonts( profile, "Fonts" );

  // read fonts that are defined in the profile:
  bool needChange = false;
  for ( int i = 0 ; i < numFontNames ; i++ )
    if ( fonts.hasKey( fontNames[i].configName ) ) {
      needChange = true;
      mFont[i] = fonts.readFontEntry( fontNames[i].configName );
      kdDebug() << "got font \"" << fontNames[i].configName
		<< "\" thusly: \"" << mFont[i].toString() << "\"" << endl;
    }
  if ( needChange && mFontLocationCombo->currentItem() > 0 )
    mFontChooser->setFont( mFont[ mFontLocationCombo->currentItem() ],
      fontNames[ mFontLocationCombo->currentItem() ].onlyFixed );

  if ( fonts.hasKey( "defaultFonts" ) )
    mCustomFontCheck->setChecked( !fonts.readBoolEntry( "defaultFonts" ) );
}

void AppearancePage::FontsTab::apply() {
  KConfigGroup fonts( kapp->config(), "Fonts" );

  // read the current font (might have been modified)
  if ( mActiveFontIndex >= 0 )
    mFont[ mActiveFontIndex ] = mFontChooser->font();

  bool customFonts = mCustomFontCheck->isChecked();
  fonts.writeEntry( "defaultFonts", !customFonts );
  for ( int i = 0 ; i < numFontNames ; i++ )
    if ( customFonts || fonts.hasKey( fontNames[i].configName ) )
      // Don't write font info when we use default fonts, but write
      // if it's already there:
      fonts.writeEntry( fontNames[i].configName, mFont[i] );
}


QString AppearancePage::ColorsTab::title() {
  return i18n("Colo&rs");
}

QString AppearancePage::ColorsTab::helpAnchor() {
  return QString::fromLatin1("configure-appearance-colors");
}


static const struct {
  const char * configName;
  const char * displayName;
} colorNames[] = { // adjust setup() if you change this:
  { "BackgroundColor", I18N_NOOP("Composer background") },
  { "AltBackgroundColor", I18N_NOOP("Alternative background color") },
  { "ForegroundColor", I18N_NOOP("Normal text") },
  { "QuotedText1", I18N_NOOP("Quoted text - first level") },
  { "QuotedText2", I18N_NOOP("Quoted text - second level") },
  { "QuotedText3", I18N_NOOP("Quoted text - third level") },
  { "LinkColor", I18N_NOOP("Link") },
  { "FollowedColor", I18N_NOOP("Followed link") },
  { "NewMessage", I18N_NOOP("New message") },
  { "UnreadMessage", I18N_NOOP("Unread message") },
  { "FlagMessage", I18N_NOOP("Important message") },
  { "PGPMessageEncr", I18N_NOOP("OpenPGP message - encrypted") },
  { "PGPMessageOkKeyOk", I18N_NOOP("OpenPGP message - valid signature with trusted key") },
  { "PGPMessageOkKeyBad", I18N_NOOP("OpenPGP message - valid signature with untrusted key") },
  { "PGPMessageWarn", I18N_NOOP("OpenPGP message - unchecked signature") },
  { "PGPMessageErr", I18N_NOOP("OpenPGP message - bad signature") },
  { "ColorbarPGP", I18N_NOOP("Colorbar - OpenPGP message") },
  { "ColorbarPlain", I18N_NOOP("Colorbar - plain text message") },
  { "ColorbarHTML", I18N_NOOP("Colorbar - HTML message") },
};
static const int numColorNames = sizeof colorNames / sizeof *colorNames;

AppearancePageColorsTab::AppearancePageColorsTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name )
{
  // tmp. vars:
  QVBoxLayout *vlay;

  // "use custom colors" check box
  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );
  mCustomColorCheck = new QCheckBox( i18n("&Use custom colors"), this );
  vlay->addWidget( mCustomColorCheck );

  // color list box:
  mColorList = new ColorListBox( this );
  mColorList->setEnabled( false ); // since !mCustomColorCheck->isChecked()
  QStringList modeList;
  for ( int i = 0 ; i < numColorNames ; i++ )
    mColorList->insertItem( new ColorListItem( i18n( colorNames[i].displayName ) ) );
  vlay->addWidget( mColorList, 1 );

  // "recycle colors" check box:
  mRecycleColorCheck =
    new QCheckBox( i18n("Recycle colors on deep &quoting"), this );
  mRecycleColorCheck->setEnabled( false );
  vlay->addWidget( mRecycleColorCheck );

  // {en,dir}able widgets depending on the state of mCustomColorCheck:
  connect( mCustomColorCheck, SIGNAL(toggled(bool)),
	   mColorList, SLOT(setEnabled(bool)) );
  connect( mCustomColorCheck, SIGNAL(toggled(bool)),
	   mRecycleColorCheck, SLOT(setEnabled(bool)) );
}

void AppearancePage::ColorsTab::setup() {
  KConfigGroup reader( kapp->config(), "Reader" );

  mCustomColorCheck->setChecked( !reader.readBoolEntry( "defaultColors", true ) );
  mRecycleColorCheck->setChecked( reader.readBoolEntry( "RecycleQuoteColors", false ) );

  static const QColor defaultColor[ numColorNames ] = {
    kapp->palette().active().base(), // bg
    KGlobalSettings::alternateBackgroundColor(), // alt bg
    kapp->palette().active().text(), // fg
    QColor( 0x00, 0x80, 0x00 ), // quoted l1
    QColor( 0x00, 0x70, 0x00 ), // quoted l2
    QColor( 0x00, 0x60, 0x00 ), // quoted l3
    KGlobalSettings::linkColor(), // link
    KGlobalSettings::visitedLinkColor(), // visited link
    QColor("red"), // new msg
    QColor("blue"), // unread mgs
    QColor( 0x00, 0x7F, 0x00 ), // important msg
    QColor( 0x00, 0x80, 0xFF ), // light blue // pgp encrypted
    QColor( 0x40, 0xFF, 0x40 ), // light green // pgp ok, trusted key
    QColor( 0xA0, 0xFF, 0x40 ), // light yellow // pgp ok, untrusted key
    QColor( 0xFF, 0xFF, 0x40 ), // light yellow // pgp unchk
    QColor( 0xFF, 0x00, 0x00 ), // red // pgp bad
    QColor( 0x80, 0xFF, 0x80 ), // very light green // colorbar pgp
    QColor( 0xFF, 0xFF, 0x80 ), // very light yellow // colorbar plain
    QColor( 0xFF, 0x40, 0x40 ), // light red // colorbar html
  };

  for ( int i = 0 ; i < numColorNames ; i++ )
    mColorList->setColor( i,
      reader.readColorEntry( colorNames[i].configName, &defaultColor[i] ) );
}

void AppearancePage::ColorsTab::installProfile( KConfig * profile ) {
  KConfigGroup reader( profile, "Reader" );

  if ( reader.hasKey( "defaultColors" ) )
    mCustomColorCheck->setChecked( !reader.readBoolEntry( "defaultColors" ) );
  if ( reader.hasKey( "RecycleQuoteColors" ) )
    mRecycleColorCheck->setChecked( reader.readBoolEntry( "RecycleQuoteColors" ) );

  for ( int i = 0 ; i < numColorNames ; i++ )
    if ( reader.hasKey( colorNames[i].configName ) )
      mColorList->setColor( i, reader.readColorEntry( colorNames[i].configName ) );
}

void AppearancePage::ColorsTab::apply() {
  KConfigGroup reader( kapp->config(), "Reader" );

  bool customColors = mCustomColorCheck->isChecked();
  reader.writeEntry( "defaultColors", !customColors );

  for ( int i = 0 ; i < numColorNames ; i++ )
    // Don't write color info when we use default colors, but write
    // if it's already there:
    if ( customColors || reader.hasKey( colorNames[i].configName ) )
      reader.writeEntry( colorNames[i].configName, mColorList->color(i) );

  reader.writeEntry( "RecycleQuoteColors", mRecycleColorCheck->isChecked() );
}

QString AppearancePage::LayoutTab::title() {
  return i18n("&Layout");
}

QString AppearancePage::LayoutTab::helpAnchor() {
  return QString::fromLatin1("configure-appearance-layout");
}

// hrmpf. Needed b/c I18N_NOOP can't take hints.
const AppearancePage::LayoutTab::dateDisplayConfigType
AppearancePage::LayoutTab::dateDisplayConfig[] = {
  { I18N_NOOP("Sta&ndard format (%1)"), KMime::DateFormatter::CTime },
  { I18N_NOOP("Locali&zed format (%1)"), KMime::DateFormatter::Localized },
  { I18N_NOOP("Fanc&y format (%1)"), KMime::DateFormatter::Fancy },
  { I18N_NOOP("&Custom (shift + F1 for help)"), KMime::DateFormatter::Custom }
};

AppearancePageLayoutTab::AppearancePageLayoutTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name ),
    mShowMIMETreeModeLastValue( -1 )
{
  // tmp. vars:
  QVBoxLayout  *vlay;

  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // The general options (were previously standalone, without button group
  QVButtonGroup* generalOptionsVBG = new QVButtonGroup( i18n( "&General Options" ), this );
  vlay->addWidget( generalOptionsVBG );
  generalOptionsVBG->layout()->setSpacing( KDialog::spacingHint() );

  mShowColorbarCheck = new QCheckBox( i18n("Show color &bar"),
                                      generalOptionsVBG );

  mMessageSizeCheck = new QCheckBox( i18n("&Display message sizes"),
                                     generalOptionsVBG );

  mCryptoIconsCheck = new QCheckBox( i18n( "Show crypto icons" ),
                                     generalOptionsVBG );

  mNestedMessagesCheck =
    new QCheckBox( i18n("&Thread list of message headers"),
                   generalOptionsVBG );


  // window layout and MIME tree viewer
  QHBoxLayout* hlayWindowLayoutAndMIMETree = new QHBoxLayout();
  vlay->addLayout( hlayWindowLayoutAndMIMETree );
  hlayWindowLayoutAndMIMETree->setSpacing( KDialog::spacingHint() );

  // The window layout
  QHGroupBox* visibleHGB = new QHGroupBox( i18n( "W&indow Layout" ), this );
  hlayWindowLayoutAndMIMETree->addWidget( visibleHGB );
  visibleHGB->layout()->setSpacing( KDialog::spacingHint() );

  mWindowLayoutBG = new QButtonGroup( this );
  mWindowLayoutBG->hide();
  mWindowLayoutBG->setExclusive( true );

  QHBox* layoutHB = new QHBox( visibleHGB );
  layoutHB->layout()->setSpacing( KDialog::spacingHint() );

  mLayout1PB = new QPushButton( layoutHB );
  mWindowLayoutBG->insert( mLayout1PB, 0 );
  mLayout1PB->setPixmap( UserIcon( "kmailwindowlayout1" ) );
  mLayout1PB->setFixedSize( mLayout1PB->sizeHint() );
  mLayout1PB->setToggleButton( true );

  mLayout2PB = new QPushButton( layoutHB );
  mWindowLayoutBG->insert( mLayout2PB, 1 );
  mLayout2PB->setPixmap( UserIcon( "kmailwindowlayout2" ) );
  mLayout2PB->setFixedSize( mLayout2PB->sizeHint() );
  mLayout2PB->setToggleButton( true );

  mLayout3PB = new QPushButton( layoutHB );
  mWindowLayoutBG->insert( mLayout3PB, 2 );
  mLayout3PB->setPixmap( UserIcon( "kmailwindowlayout3" ) );
  mLayout3PB->setFixedSize( mLayout3PB->sizeHint() );
  mLayout3PB->setToggleButton( true );

  mLayout4PB = new QPushButton( layoutHB );
  mWindowLayoutBG->insert( mLayout4PB, 3 );
  mLayout4PB->setPixmap( UserIcon( "kmailwindowlayout4" ) );
  mLayout4PB->setFixedSize( mLayout4PB->sizeHint() );
  mLayout4PB->setToggleButton( true );

  mLayout5PB = new QPushButton( layoutHB );
  mWindowLayoutBG->insert( mLayout5PB, 4 );
  mLayout5PB->setPixmap( UserIcon( "kmailwindowlayout5" ) );
  mLayout5PB->setFixedSize( mLayout5PB->sizeHint() );
  mLayout5PB->setToggleButton( true );

  // the MIME Tree Viewer
  mShowMIMETreeMode =
    new QVButtonGroup( i18n("Show &MIME Tree"), this );
  hlayWindowLayoutAndMIMETree->addWidget( mShowMIMETreeMode );
  mShowMIMETreeMode->layout()->setSpacing( KDialog::spacingHint() );

  mShowMIMETreeMode->insert(
    new QRadioButton( i18n("Never"),  mShowMIMETreeMode ), 0 );
  mShowMIMETreeMode->insert(
    new QRadioButton( i18n("Smart"),  mShowMIMETreeMode ), 1 );
  mShowMIMETreeMode->insert(
    new QRadioButton( i18n("Always"), mShowMIMETreeMode ), 2 );

  connect( mShowMIMETreeMode, SIGNAL( clicked( int )),
          this, SLOT( showMIMETreeClicked( int )) );

  // a button group for four radiobuttons (by default exclusive):
  mNestingPolicy =
    new QVButtonGroup( i18n("Message Header Threading Options"), this );
  mNestingPolicy->layout()->setSpacing( KDialog::spacingHint() );

  mNestingPolicy->insert(
    new QRadioButton( i18n("Always &keep threads open"),
		      mNestingPolicy ), 0 );
  mNestingPolicy->insert(
    new QRadioButton( i18n("Threads default to op&en"),
		      mNestingPolicy ), 1 );
  mNestingPolicy->insert(
    new QRadioButton( i18n("Threads default to clo&sed"),
		      mNestingPolicy ), 2 );
  mNestingPolicy->insert(
    new QRadioButton( i18n("Open threads that contain new, unread "
			   "or important messages"), mNestingPolicy ), 3 );
  vlay->addWidget( mNestingPolicy );

  // a button group for three radiobuttons:
  mDateDisplay = new QVButtonGroup( i18n( "Displaying of the date", "Date Display" ), this );
  mDateDisplay->layout()->setSpacing( KDialog::spacingHint() );

  for ( int i = 0 ; i < numDateDisplayConfig ; i++ ) {
    if ( dateDisplayConfig[i].dateDisplay == KMime::DateFormatter::Custom ) {
      QRadioButton *b = new QRadioButton( i18n(dateDisplayConfig[i].displayName),
					  mDateDisplay );
      mDateDisplay->insert(b);
      QLineEdit *le = new QLineEdit( mDateDisplay );
      QWhatsThis::add(le, i18n( "<B>These expressions may be used for the date:</B> <BR>"
			        "<UL>"
				"<LI>d - the day as a number without a leading zero (1-31) "
				"<LI>dd - the day as a number with a leading zero (01-31) "
				"<LI>ddd - the abbreviated day name (Mon - Sun) "
				"<LI>dddd - the long day name (Monday - Sunday) "
				"<LI>M - the month as a number without a leading zero (1-12) "
				"<LI>MM - the month as a number with a leading zero (01-12) "
				"<LI>MMM - the abbreviated month name (Jan - Dec) "
				"<LI>MMMM - the long month name (January - December) "
				"<LI>yy - the year as a two digit number (00-99) "
				"<LI>yyyy - the year as a four digit number (0000-9999) "
				"</UL>"
				"<B>These expressions may be used for the time:</B> "
				"<UL>"
				"<LI>h - the hour without a leading zero (0-23 or 1-12 if AM/PM display) "
				"<LI>hh - the hour with a leading zero (00-23 or 01-12 if AM/PM display) "
				"<LI>m - the minutes without a leading zero (0-59) "
				"<LI>mm - the minutes with a leading zero (00-59) "
				"<LI>s - the seconds without a leading zero (0-59) "
				"<LI>ss - the seconds with a leading zero (00-59) "
				"<LI>z - the milliseconds without leading zeroes (0-999) "
	      			"<LI>zzz - the milliseconds with leading zeroes (000-999) "
				"<LI>AP - switch to AM/PM display. AP will be replaced by either \"AM\" or \"PM\". "
				"<LI>ap - switch to AM/PM display. ap will be replaced by either \"am\" or \"pm\". "
				"<LI>Z - time zone in numeric form (-0500) "
				"</UL>"
				"<B>All other input characters will be ignored.</B>") );
      le->setEnabled( false );
      QObject::connect( b, SIGNAL(toggled(bool)), le, SLOT(setEnabled(bool)) );
    } else {
      mDateDisplay->insert( new QRadioButton( i18n(dateDisplayConfig[i].displayName)
					      .arg( KMime::DateFormatter::formatCurrentDate(
                                                         dateDisplayConfig[i].dateDisplay ) ),
					                 mDateDisplay ), i );
      }
  }

  vlay->addWidget( mDateDisplay );
  vlay->addStretch( 10 ); // spacer
}

void AppearancePageLayoutTab::showMIMETreeClicked( int id )
{
  if ( mShowMIMETreeModeLastValue != id ) {
    mShowMIMETreeModeLastValue = id;
    QString postfix;
    switch ( id ) {
    case 0:
      postfix = "_no_mime";
      break;
    case 1:
      postfix = "_smart_mime";
      break;
    case 2:
      postfix = "";
      break;
    }
    mLayout1PB->setPixmap( UserIcon( "kmailwindowlayout1"+postfix ) );
    mLayout2PB->setPixmap( UserIcon( "kmailwindowlayout2"+postfix ) );
    mLayout3PB->setPixmap( UserIcon( "kmailwindowlayout3"+postfix ) );
    mLayout4PB->setPixmap( UserIcon( "kmailwindowlayout4"+postfix ) );
    mLayout5PB->setPixmap( UserIcon( "kmailwindowlayout5"+postfix ) );
  }
}

void AppearancePage::LayoutTab::setup() {
  KConfigGroup reader( kapp->config(), "Reader" );
  KConfigGroup geometry( kapp->config(), "Geometry" );
  KConfigGroup general( kapp->config(), "General" );

  mShowColorbarCheck->setChecked( reader.readBoolEntry( "showColorbar", false ) );
  mNestedMessagesCheck->setChecked( geometry.readBoolEntry( "nestedMessages", false ) );
  mMessageSizeCheck->setChecked( general.readBoolEntry( "showMessageSize", false ) );
  mCryptoIconsCheck->setChecked( general.readBoolEntry( "showCryptoIcons", false ) );


  int windowLayout = geometry.readNumEntry( "windowLayout", 0 );
  if( windowLayout < 0 || windowLayout > 4 )
      windowLayout = 0;
  mWindowLayoutBG->setButton( windowLayout );

  int num = geometry.readNumEntry( "showMIME", 1 );
  if ( num < 0 || num > 2 ) num = 1;
  mShowMIMETreeMode->setButton( num );
  showMIMETreeClicked( num );


  num = geometry.readNumEntry( "nestingPolicy", 3 );
  if ( num < 0 || num > 3 ) num = 3;
  mNestingPolicy->setButton( num );

  KMime::DateFormatter::FormatType dateDisplay = static_cast<KMime::DateFormatter::FormatType>( general.readNumEntry( "dateFormat", KMime::DateFormatter::Fancy ) );
  int i;
  for ( i = 0 ; i < numDateDisplayConfig ; i++ )
    if ( dateDisplay == dateDisplayConfig[i].dateDisplay ) {
      if ( dateDisplay == KMime::DateFormatter::Custom ) {
	QObjectListIt it( *mDateDisplay->queryList( "QLineEdit" ) );
	static_cast<QLineEdit*>(it.current())->setText( general.readEntry( "customDateFormat", QString::null) );
      }
      mDateDisplay->setButton( i );
      break;
    }
  if ( i >= numDateDisplayConfig )
    mDateDisplay->setButton( numDateDisplayConfig - 2 ); // default
}

void AppearancePage::LayoutTab::installProfile( KConfig * profile ) {
  KConfigGroup reader( profile, "Reader" );
  KConfigGroup geometry( profile, "Geometry" );
  KConfigGroup general( profile, "General" );

  if ( reader.hasKey( "showColorbar" ) )
    mShowColorbarCheck->setChecked( reader.readBoolEntry( "showColorbar" ) );
  if ( geometry.hasKey( "nestedMessages" ) )
    mNestedMessagesCheck->setChecked( geometry.readBoolEntry( "nestedMessages" ) );
  if ( general.hasKey( "showMessageSize" ) )
    mMessageSizeCheck->setChecked( general.readBoolEntry( "showMessageSize" ) );

  if( general.hasKey( "showCryptoIcons" ) )
      mCryptoIconsCheck->setChecked( general.readBoolEntry( "showCryptoIcons" ) );

  if( geometry.hasKey( "windowLayout" ) ) {
      int windowLayout = geometry.readNumEntry( "windowLayout", 0 );
      if( windowLayout < 0 || windowLayout > 4 )
          windowLayout = 0;
      mWindowLayoutBG->setButton( windowLayout );
  }

  if( geometry.hasKey( "showMIME" ) ) {
    int num = geometry.readNumEntry( "showMIME" );
    if ( num < 0 || num > 2 ) num = 1;
    mShowMIMETreeMode->setButton( num );
  }


  if ( geometry.hasKey( "nestingPolicy" ) ) {
    int num = geometry.readNumEntry( "nestingPolicy" );
    if ( num < 0 || num > 3 ) num = 3;
    mNestingPolicy->setButton( num );
  }

  if ( general.hasKey( "dateFormat" ) ) {
    KMime::DateFormatter::FormatType dateFormat = static_cast<KMime::DateFormatter::FormatType>( general.readNumEntry( "dateFormat", KMime::DateFormatter::Fancy ) );
    for ( int i = 0 ; i < numDateDisplayConfig ; i++ )
      if ( dateFormat
	   == dateDisplayConfig[i].dateDisplay ) {
	mDateDisplay->setButton( i );
	break;
      }
  }
}

void AppearancePage::LayoutTab::apply() {
  KConfigGroup reader( kapp->config(), "Reader" );
  KConfigGroup geometry( kapp->config(), "Geometry" );
  KConfigGroup general( kapp->config(), "General" );

  reader.writeEntry( "showColorbar", mShowColorbarCheck->isChecked() );

  if (geometry.readBoolEntry( "nestedMessages", false )
    != mNestedMessagesCheck->isChecked())
  {
    if (KMessageBox::warningContinueCancel(this, i18n("Changing the global "
      "threading setting will override all folder specific values."),
      QString::null, QString::null, "threadOverride")
      == KMessageBox::Continue)
    {
      geometry.writeEntry( "nestedMessages",
        mNestedMessagesCheck->isChecked() );
      QStringList names;
      QValueList<QGuardedPtr<KMFolder> > folders;
      kernel->folderMgr()->createFolderList(&names, &folders);
      kernel->imapFolderMgr()->createFolderList(&names, &folders);
      for (QValueList<QGuardedPtr<KMFolder> >::iterator it = folders.begin();
        it != folders.end(); ++it)
      {
        if (*it)
        {
          KConfigGroupSaver saver(kapp->config(),
            "Folder-" + (*it)->idString());
          kapp->config()->writeEntry("threadMessagesOverride", false);
        }
      }
    }
  }

  geometry.writeEntry( "windowLayout",
                       mWindowLayoutBG->id( mWindowLayoutBG->selected() ) );
  geometry.writeEntry( "showMIME",
                       mShowMIMETreeMode->id( mShowMIMETreeMode->selected()));

  geometry.writeEntry( "nestingPolicy",
		       mNestingPolicy->id( mNestingPolicy->selected() ) );
  general.writeEntry( "showMessageSize", mMessageSizeCheck->isChecked() );
  general.writeEntry( "showCryptoIcons", mCryptoIconsCheck->isChecked() );

  int dateDisplayID = mDateDisplay->id( mDateDisplay->selected() );
  // check bounds:
  if ( dateDisplayID < 0 || dateDisplayID > numDateDisplayConfig - 1 )
    dateDisplayID = numDateDisplayConfig - 2;//Fancy
  general.writeEntry( "dateFormat",
		      dateDisplayConfig[ dateDisplayID ].dateDisplay );
  if ( dateDisplayID == numDateDisplayConfig - 1 ) {//custom
    QObjectListIt it( *mDateDisplay->queryList( "QLineEdit" ) );
    general.writeEntry( "customDateFormat",
			static_cast<QLineEdit*>(it.current())->text() );
  }
}


QString AppearancePage::ProfileTab::title() {
  return i18n("&Profiles");
}

QString AppearancePage::ProfileTab::helpAnchor() {
  return QString::fromLatin1("configure-appearance-profiles");
}

AppearancePageProfileTab::AppearancePageProfileTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name )
{
  // tmp. vars:
  QVBoxLayout *vlay;

  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  mListView = new KListView( this, "mListView" );
  mListView->addColumn( i18n("Available Profiles") );
  mListView->addColumn( i18n("Description") );
  mListView->setFullWidth();
  mListView->setAllColumnsShowFocus( true );
  mListView->setFrameStyle( QFrame::WinPanel + QFrame::Sunken );
  mListView->setSorting( -1 );

  vlay->addWidget( new QLabel( mListView,
			       i18n("&Select a profile and hit apply to take "
				    "over its settings:"), this ) );
  vlay->addWidget( mListView, 1 );

  /* not implemented (yet?)
  hlay = new QHBoxLayout( vlay );
  QPushButton *pushButton = new QPushButton(i18n("&New"), page4 );
  pushButton->setAutoDefault( false );
  hlay->addWidget( pushButton );
  mAppearance.profileDeleteButton = new QPushButton(i18n("Dele&te"), page4 );
  mAppearance.profileDeleteButton->setAutoDefault( false );
  hlay->addWidget( mAppearance.profileDeleteButton );
  hlay->addStretch(10);
  */
}

void AppearancePage::ProfileTab::setup() {
  mListView->clear();
  // find all profiles (config files named "profile-xyz-rc"):
  QString profileFilenameFilter = QString::fromLatin1("profile-*-rc");
  mProfileList = KGlobal::dirs()->findAllResources( "appdata",
						    profileFilenameFilter );

  kdDebug(5006) << "Profile manager: found " << mProfileList.count()
		<< " profiles:" << endl;

  // build the list and populate the list view:
  QListViewItem * listItem = 0;
  for ( QStringList::Iterator it = mProfileList.begin() ;
	it != mProfileList.end() ; ++it ) {
    KConfig profile( (*it), true /* read-only */, false /* no KDE global */ );
    profile.setGroup("KMail Profile");
    QString name = profile.readEntry( "Name" );
    if ( name.isEmpty() ) {
      kdWarning(5006) << "File \"" << (*it)
		      << "\" doesn't provide a profile name!" << endl;
      name = i18n("Unnamed");
    }
    QString desc = profile.readEntry( "Comment" );
    if ( desc.isEmpty() ) {
      kdWarning(5006) << "File \"" << (*it)
		      << "\" doesn't provide a description!" << endl;
      desc = i18n("Not available");
    }
    listItem = new QListViewItem( mListView, listItem, name, desc );
  }
}


void AppearancePage::ProfileTab::apply() {
  if ( !this->isVisible() ) return; // don't apply when not currently shown

  int index = mListView->itemIndex( mListView->selectedItem() );
  if ( index < 0 ) return; // non selected

  assert( index < (int)mProfileList.count() );

  KConfig profile( *mProfileList.at(index), true, false );
  emit profileSelected( &profile );
}


// *************************************************************
// *                                                           *
// *                      ComposerPage                         *
// *                                                           *
// *************************************************************



QString ComposerPage::iconLabel() {
  return i18n("Composer");
}

const char * ComposerPage::iconName() {
  return "edit";
}

QString ComposerPage::title() {
  return i18n("Phrases & General Behavior");
}

QString ComposerPage::helpAnchor() {
  return QString::fromLatin1("configure-composer");
}

ComposerPage::ComposerPage( QWidget * parent, const char * name )
  : TabbedConfigurationPage( parent, name )
{
  //
  // "General" tab:
  //
  mGeneralTab = new GeneralTab();
  addTab( mGeneralTab, mGeneralTab->title() );

  //
  // "Phrases" tab:
  //
  mPhrasesTab = new PhrasesTab();
  addTab( mPhrasesTab, mPhrasesTab->title() );

  //
  // "Subject" tab:
  //
  mSubjectTab = new SubjectTab();
  addTab( mSubjectTab, mSubjectTab->title() );

  //
  // "Charset" tab:
  //
  mCharsetTab = new CharsetTab();
  addTab( mCharsetTab, mCharsetTab->title() );

  //
  // "Headers" tab:
  //
  mHeadersTab = new HeadersTab();
  addTab( mHeadersTab, mHeadersTab->title() );
}

void ComposerPage::setup() {
  mGeneralTab->setup();
  mPhrasesTab->setup();
  mSubjectTab->setup();
  mCharsetTab->setup();
  mHeadersTab->setup();
}

void ComposerPage::installProfile( KConfig * profile ) {
  mGeneralTab->installProfile( profile );
  mPhrasesTab->installProfile( profile );
  mSubjectTab->installProfile( profile );
  mCharsetTab->installProfile( profile );
  mHeadersTab->installProfile( profile );
}

void ComposerPage::apply() {
  mGeneralTab->apply();
  mPhrasesTab->apply();
  mSubjectTab->apply();
  mCharsetTab->apply();
  mHeadersTab->apply();
}


QString ComposerPage::GeneralTab::title() {
  return i18n("&General");
}

QString ComposerPage::GeneralTab::helpAnchor() {
  return QString::fromLatin1("configure-composer-general");
};


ComposerPageGeneralTab::ComposerPageGeneralTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name )
{
  // tmp. vars:
  QVBoxLayout *vlay;
  QHBoxLayout *hlay;
  QGroupBox   *group;
  QLabel      *label;
  QHBox       *hbox;

  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // some check buttons...
  mAutoAppSignFileCheck =
    new QCheckBox( i18n("A&utomatically append signature"), this );
  vlay->addWidget( mAutoAppSignFileCheck );

  mSmartQuoteCheck = new QCheckBox( i18n("Use smart &quoting"), this );
  vlay->addWidget( mSmartQuoteCheck );

  mPgpAutoSignatureCheck =
    new QCheckBox( i18n("Automatically sig&n messages using OpenPGP"), this );
  vlay->addWidget( mPgpAutoSignatureCheck );

  mPgpAutoEncryptCheck =
    new QCheckBox( i18n("Automatically encr&ypt messages whenever possible"), this );
  vlay->addWidget( mPgpAutoEncryptCheck );

  // a checkbutton for "word wrap" and a spinbox for the column in
  // which to wrap:
  hlay = new QHBoxLayout( vlay ); // inherits spacing
  mWordWrapCheck = new QCheckBox( i18n("Word &wrap at column:"), this );
  hlay->addWidget( mWordWrapCheck );
  mWrapColumnSpin = new KIntSpinBox( 30/*min*/, 78/*max*/, 1/*step*/,
				     78/*init*/, 10 /*base*/, this );
  mWrapColumnSpin->setEnabled( false ); // since !mWordWrapCheck->isChecked()
  hlay->addWidget( mWrapColumnSpin );
  hlay->addStretch( 1 );
  // only enable the spinbox if the checkbox is checked:
  connect( mWordWrapCheck, SIGNAL(toggled(bool)),
	   mWrapColumnSpin, SLOT(setEnabled(bool)) );

  // The "exteral editor" group:
  group = new QVGroupBox( i18n("External Editor"), this );
  group->layout()->setSpacing( KDialog::spacingHint() );

  mExternalEditorCheck =
    new QCheckBox( i18n("Use e&xternal editor instead of composer"), group );

  hbox = new QHBox( group );
  label = new QLabel( i18n("Specify e&ditor:"), hbox );
  mEditorRequester = new KURLRequester( hbox );
  hbox->setStretchFactor( mEditorRequester, 1 );
  label->setBuddy( mEditorRequester );
  label->setEnabled( false ); // since !mExternalEditorCheck->isChecked()
  // ### FIXME: allow only executables (x-bit when available..)
  mEditorRequester->setFilter( "application/x-executable "
			       "application/x-shellscript "
			       "application/x-desktop" );
  mEditorRequester->setEnabled( false ); // !mExternalEditorCheck->isChecked()
  connect( mExternalEditorCheck, SIGNAL(toggled(bool)),
	   label, SLOT(setEnabled(bool)) );
  connect( mExternalEditorCheck, SIGNAL(toggled(bool)),
	   mEditorRequester, SLOT(setEnabled(bool)) );

  label = new QLabel( i18n("\"%f\" will be replaced with the "
			   "filename to edit."), group );
  label->setEnabled( false ); // see above
  connect( mExternalEditorCheck, SIGNAL(toggled(bool)),
	   label, SLOT(setEnabled(bool)) );

  vlay->addWidget( group );
  vlay->addStretch( 100 );
}

void ComposerPage::GeneralTab::setup() {
  KConfigGroup composer( kapp->config(), "Composer" );
  KConfigGroup general( kapp->config(), "General" );

  // various check boxes:
  bool state = ( composer.readEntry("signature").lower() != "manual" );
  mAutoAppSignFileCheck->setChecked( state );
  mSmartQuoteCheck->setChecked( composer.readBoolEntry( "smart-quote", true ) );
  mPgpAutoSignatureCheck->setChecked( composer.readBoolEntry( "pgp-auto-sign", false ) );
  mPgpAutoEncryptCheck->setChecked( composer.readBoolEntry( "pgp-auto-encrypt", false ) );
  mWordWrapCheck->setChecked( composer.readBoolEntry( "word-wrap", true ) );
  mWrapColumnSpin->setValue( composer.readNumEntry( "break-at", 78 ) );

  // editor group:
  mExternalEditorCheck->setChecked( general.readBoolEntry( "use-external-editor", false ) );
  mEditorRequester->setURL( general.readEntry( "external-editor", "" ) );
}

void ComposerPage::GeneralTab::installProfile( KConfig * profile ) {
  KConfigGroup composer( profile, "Composer" );
  KConfigGroup general( profile, "General" );

  if ( composer.hasKey( "signature" ) ) {
    bool state = ( composer.readEntry("signature").lower() == "auto" );
    mAutoAppSignFileCheck->setChecked( state );
  }
  if ( composer.hasKey( "smart-quote" ) )
    mSmartQuoteCheck->setChecked( composer.readBoolEntry( "smart-quote" ) );
  if ( composer.hasKey( "pgp-auto-sign" ) )
    mPgpAutoSignatureCheck->setChecked( composer.readBoolEntry( "pgp-auto-sign" ) );
  if ( composer.hasKey( "pgp-auto-encrypt" ) )
    mPgpAutoEncryptCheck->setChecked( composer.readBoolEntry( "pgp-auto-encrypt" ) );
  if ( composer.hasKey( "word-wrap" ) )
    mWordWrapCheck->setChecked( composer.readBoolEntry( "word-wrap" ) );
  if ( composer.hasKey( "break-at" ) )
    mWrapColumnSpin->setValue( composer.readNumEntry( "break-at" ) );

  if ( general.hasKey( "use-external-editor" )
       && general.hasKey( "external-editor" ) ) {
    mExternalEditorCheck->setChecked( general.readBoolEntry( "use-external-editor" ) );
    mEditorRequester->setURL( general.readEntry( "external-editor" ) );
  }
}

void ComposerPage::GeneralTab::apply() {
  KConfigGroup general( kapp->config(), "General" );
  KConfigGroup composer( kapp->config(), "Composer" );

  general.writeEntry( "use-external-editor", mExternalEditorCheck->isChecked() );
  general.writeEntry( "external-editor", mEditorRequester->url() );

  bool autoSignature = mAutoAppSignFileCheck->isChecked();
  composer.writeEntry( "signature", autoSignature ? "auto" : "manual" );
  composer.writeEntry( "smart-quote", mSmartQuoteCheck->isChecked() );
  composer.writeEntry( "pgp-auto-sign",
		       mPgpAutoSignatureCheck->isChecked() );
  composer.writeEntry( "pgp-auto-encrypt",
		      mPgpAutoEncryptCheck->isChecked() );
  composer.writeEntry( "word-wrap", mWordWrapCheck->isChecked() );
  composer.writeEntry( "break-at", mWrapColumnSpin->value() );
}



QString ComposerPage::PhrasesTab::title() {
  return i18n("&Phrases");
}

QString ComposerPage::PhrasesTab::helpAnchor() {
  return QString::fromLatin1("configure-composer-phrases");
}

ComposerPagePhrasesTab::ComposerPagePhrasesTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name )
{
  // tmp. vars:
  QGridLayout *glay;
  QPushButton *button;

  glay = new QGridLayout( this, 7, 3, KDialog::spacingHint() );
  glay->setMargin( KDialog::marginHint() );
  glay->setColStretch( 1, 1 );
  glay->setColStretch( 2, 1 );
  glay->setRowStretch( 7, 1 );

  // row 0: help text
  glay->addMultiCellWidget( new QLabel( i18n("The following placeholders are "
					     "supported in the reply phrases:\n"
					     "%D=date, %S=subject, \n"
                                             "%e=sender's address, %F=sender's name, %f=sender's initials,\n"
                                             "%T=recipient's name, %t=recipient's name and address\n"
					     "%%=percent sign, %_=space, "
					     "%L=linebreak"), this ),
			    0, 0, 0, 2 ); // row 0; cols 0..2

  // row 1: label and language combo box:
  mPhraseLanguageCombo = new LanguageComboBox( false, this );
  glay->addWidget( new QLabel( mPhraseLanguageCombo,
			       i18n("&Language:"), this ), 1, 0 );
  glay->addMultiCellWidget( mPhraseLanguageCombo, 1, 1, 1, 2 );
  connect( mPhraseLanguageCombo, SIGNAL(activated(const QString&)),
           this, SLOT(slotLanguageChanged(const QString&)) );

  // row 2: "add..." and "remove" push buttons:
  button = new QPushButton( i18n("A&dd..."), this );
  button->setAutoDefault( false );
  glay->addWidget( button, 2, 1 );
  mRemoveButton = new QPushButton( i18n("Re&move"), this );
  mRemoveButton->setAutoDefault( false );
  mRemoveButton->setEnabled( false ); // combo doesn't contain anything...
  glay->addWidget( mRemoveButton, 2, 2 );
  connect( button, SIGNAL(clicked()),
           this, SLOT(slotNewLanguage()) );
  connect( mRemoveButton, SIGNAL(clicked()),
           this, SLOT(slotRemoveLanguage()) );

  // row 3: "reply to sender" line edit and label:
  mPhraseReplyEdit = new QLineEdit( this );
  glay->addWidget( new QLabel( mPhraseReplyEdit,
			       i18n("Repl&y to sender:"), this ), 3, 0 );
  glay->addMultiCellWidget( mPhraseReplyEdit, 3, 3, 1, 2 ); // cols 1..2

  // row 4: "reply to all" line edit and label:
  mPhraseReplyAllEdit = new QLineEdit( this );
  glay->addWidget( new QLabel( mPhraseReplyAllEdit,
			       i18n("Reply &to all:"), this ), 4, 0 );
  glay->addMultiCellWidget( mPhraseReplyAllEdit, 4, 4, 1, 2 ); // cols 1..2

  // row 5: "forward" line edit and label:
  mPhraseForwardEdit = new QLineEdit( this );
  glay->addWidget( new QLabel( mPhraseForwardEdit,
			       i18n("&Forward:"), this ), 5, 0 );
  glay->addMultiCellWidget( mPhraseForwardEdit, 5, 5, 1, 2 ); // cols 1..2

  // row 6: "quote indicator" line edit and label:
  mPhraseIndentPrefixEdit = new QLineEdit( this );
  glay->addWidget( new QLabel( mPhraseIndentPrefixEdit,
			       i18n("&Quote indicator:"), this ), 6, 0 );
  glay->addMultiCellWidget( mPhraseIndentPrefixEdit, 6, 6, 1, 2 );

  // row 7: spacer
};


void ComposerPage::PhrasesTab::setLanguageItemInformation( int index ) {
  assert( 0 <= index && index < (int)mLanguageList.count() );

  LanguageItem &l = *mLanguageList.at( index );

  mPhraseReplyEdit->setText( l.mReply );
  mPhraseReplyAllEdit->setText( l.mReplyAll );
  mPhraseForwardEdit->setText( l.mForward );
  mPhraseIndentPrefixEdit->setText( l.mIndentPrefix );
}

void ComposerPage::PhrasesTab::saveActiveLanguageItem() {
  int index = mActiveLanguageItem;
  if (index == -1) return;
  assert( 0 <= index && index < (int)mLanguageList.count() );

  LanguageItem &l = *mLanguageList.at( index );

  l.mReply = mPhraseReplyEdit->text();
  l.mReplyAll = mPhraseReplyAllEdit->text();
  l.mForward = mPhraseForwardEdit->text();
  l.mIndentPrefix = mPhraseIndentPrefixEdit->text();
}

void ComposerPage::PhrasesTab::slotNewLanguage()
{
  NewLanguageDialog dialog( mLanguageList,
			    dynamic_cast<QWidget*>(parent()), "New", true );
  int result = dialog.exec();
  if ( result == QDialog::Accepted ) slotAddNewLanguage( dialog.language() );
}

void ComposerPage::PhrasesTab::slotAddNewLanguage( const QString& lang )
{
  mPhraseLanguageCombo->setCurrentItem(
    mPhraseLanguageCombo->insertLanguage( lang ) );
  KLocale locale("kmail");
  locale.setLanguage( lang );
  mLanguageList.append(
     LanguageItem( lang,
		   locale.translate("On %D, you wrote:"),
		   locale.translate("On %D, %F wrote:"),
		   locale.translate("Forwarded Message"),
		   locale.translate(">%_") ) );
  mRemoveButton->setEnabled( true );
  slotLanguageChanged( QString::null );
}

void ComposerPage::PhrasesTab::slotRemoveLanguage()
{
  assert( mPhraseLanguageCombo->count() > 1 );
  int index = mPhraseLanguageCombo->currentItem();
  assert( 0 <= index && index < (int)mLanguageList.count() );

  // remove current item from internal list and combobox:
  mLanguageList.remove( mLanguageList.at( index ) );
  mPhraseLanguageCombo->removeItem( index );

  if ( index >= (int)mLanguageList.count() ) index--;

  mActiveLanguageItem = index;
  setLanguageItemInformation( index );
  mRemoveButton->setEnabled( mLanguageList.count() > 1 );
}

void ComposerPage::PhrasesTab::slotLanguageChanged( const QString& )
{
  int index = mPhraseLanguageCombo->currentItem();
  assert( index < (int)mLanguageList.count() );
  saveActiveLanguageItem();
  mActiveLanguageItem = index;
  setLanguageItemInformation( index );
}


void ComposerPage::PhrasesTab::setup() {
  KConfigGroup general( kapp->config(), "General" );

  mLanguageList.clear();
  mPhraseLanguageCombo->clear();
  mActiveLanguageItem = -1;

  int num = general.readNumEntry( "reply-languages", 0 );
  int currentNr = general.readNumEntry( "reply-current-language" ,0 );

  // build mLanguageList and mPhraseLanguageCombo:
  for ( int i = 0 ; i < num ; i++ ) {
    KConfigGroup config( kapp->config(),
			 QCString("KMMessage #") + QCString().setNum(i) );
    QString lang = config.readEntry( "language" );
    mLanguageList.append(
         LanguageItem( lang,
		       config.readEntry( "phrase-reply" ),
		       config.readEntry( "phrase-reply-all" ),
		       config.readEntry( "phrase-forward" ),
		       config.readEntry( "indent-prefix" ) ) );
    mPhraseLanguageCombo->insertLanguage( lang );
  }

  if ( num == 0 )
    slotAddNewLanguage( KGlobal::locale()->language() );

  if ( currentNr >= num || currentNr < 0 )
    currentNr = 0;

  mPhraseLanguageCombo->setCurrentItem( currentNr );
  mActiveLanguageItem = currentNr;
  setLanguageItemInformation( currentNr );
  mRemoveButton->setEnabled( mLanguageList.count() > 1 );
}

void ComposerPage::PhrasesTab::apply() {
  KConfigGroup general( kapp->config(), "General" );

  general.writeEntry( "reply-languages", mLanguageList.count() );
  general.writeEntry( "reply-current-language", mPhraseLanguageCombo->currentItem() );

  saveActiveLanguageItem();
  LanguageItemList::Iterator it = mLanguageList.begin();
  for ( int i = 0 ; it != mLanguageList.end() ; ++it, ++i ) {
    KConfigGroup config( kapp->config(),
			 QCString("KMMessage #") + QCString().setNum(i) );
    config.writeEntry( "language", (*it).mLanguage );
    config.writeEntry( "phrase-reply", (*it).mReply );
    config.writeEntry( "phrase-reply-all", (*it).mReplyAll );
    config.writeEntry( "phrase-forward", (*it).mForward );
    config.writeEntry( "indent-prefix", (*it).mIndentPrefix );
  }
}



QString ComposerPage::SubjectTab::title() {
  return i18n("&Subject");
}

QString ComposerPage::SubjectTab::helpAnchor() {
  return QString::fromLatin1("configure-composer-subject");
}

ComposerPageSubjectTab::ComposerPageSubjectTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name )
{
  // tmp. vars:
  QVBoxLayout *vlay;
  QGroupBox   *group;
  QLabel      *label;


  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  group = new QVGroupBox( i18n("Repl&y Subject Prefixes"), this );
  group->layout()->setSpacing( KDialog::spacingHint() );

  // row 0: help text:
  label = new QLabel( i18n("Recognize the following prefixes "
			   "(matching is case-insensitive):"), group );
  label->setAlignment( AlignLeft|WordBreak );

  // row 1, string list editor:
  SimpleStringListEditor::ButtonCode buttonCode =
    static_cast<SimpleStringListEditor::ButtonCode>( SimpleStringListEditor::Add|SimpleStringListEditor::Remove );
  mReplyListEditor =
    new SimpleStringListEditor( group, 0, buttonCode,
				i18n("A&dd..."), i18n("Re&move"),
				QString::null,
				i18n("Enter new reply prefix:") );

  // row 2: "replace [...]" check box:
  mReplaceReplyPrefixCheck =
     new QCheckBox( i18n("Replace recognized prefi&x with Re:"), group );

  vlay->addWidget( group );


  group = new QVGroupBox( i18n("Forward Subject Prefixes"), this );
  group->layout()->setSpacing( KDialog::marginHint() );

  // row 0: help text:
  label= new QLabel( i18n("Recognize the following prefixes "
			  "(matching is case-insensitive):"), group );
  label->setAlignment( AlignLeft|WordBreak );

  // row 1: string list editor
  mForwardListEditor =
    new SimpleStringListEditor( group, 0, buttonCode,
				i18n("Add..."),
				i18n("Remo&ve"), QString::null,
				i18n("Enter new forward prefix:") );

  // row 3: "replace [...]" check box:
  mReplaceForwardPrefixCheck =
     new QCheckBox( i18n("Replace recognized prefix with &Fwd:"), group );

  vlay->addWidget( group );
}

void ComposerPage::SubjectTab::setup() {
  KConfigGroup composer( kapp->config(), "Composer" );

  QStringList prefixList = composer.readListEntry( "reply-prefixes", ',' );
  if ( prefixList.isEmpty() )
    prefixList << QString::fromLatin1("Re:");
  mReplyListEditor->setStringList( prefixList );

  mReplaceReplyPrefixCheck->setChecked( composer.readBoolEntry("replace-reply-prefix", true ) );

  prefixList = composer.readListEntry( "forward-prefixes", ',' );
  if ( prefixList.isEmpty() )
    prefixList << QString::fromLatin1("Fwd:");
  mForwardListEditor->setStringList( prefixList );

  mReplaceForwardPrefixCheck->setChecked( composer.readBoolEntry( "replace-forward-prefix", true ) );
}

void ComposerPage::SubjectTab::apply() {
  KConfigGroup composer( kapp->config(), "Composer" );


  composer.writeEntry( "reply-prefixes", mReplyListEditor->stringList() );
  composer.writeEntry( "forward-prefixes", mForwardListEditor->stringList() );
  composer.writeEntry( "replace-reply-prefix",
		       mReplaceReplyPrefixCheck->isChecked() );
  composer.writeEntry( "replace-forward-prefix",
		       mReplaceForwardPrefixCheck->isChecked() );
}



QString ComposerPage::CharsetTab::title() {
  return i18n("Cha&rset");
}

QString ComposerPage::CharsetTab::helpAnchor() {
  return QString::fromLatin1("configure-composer-charset");
}

ComposerPageCharsetTab::ComposerPageCharsetTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name )
{
  // tmp. vars:
  QVBoxLayout *vlay;
  QLabel      *label;

  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  label = new QLabel( i18n("This list is checked for every outgoing message "
                           "from the top to the bottom for a charset that "
                           "contains all required characters."), this );
  label->setAlignment( WordBreak);
  vlay->addWidget( label );

  mCharsetListEditor =
    new SimpleStringListEditor( this, 0, SimpleStringListEditor::All,
				i18n("A&dd..."), i18n("Remo&ve"),
				i18n("&Modify"), i18n("Enter charset:") );
  vlay->addWidget( mCharsetListEditor, 1 );

  mKeepReplyCharsetCheck = new QCheckBox( i18n("&Keep original charset when "
						"replying or forwarding (if "
						"possible)."), this );
  vlay->addWidget( mKeepReplyCharsetCheck );

  connect( mCharsetListEditor, SIGNAL(aboutToAdd(QString&)),
	   this, SLOT(slotVerifyCharset(QString&)) );
}

void ComposerPage::CharsetTab::slotVerifyCharset( QString & charset ) {
  if ( charset.isEmpty() ) return;

  if ( charset.lower() == QString::fromLatin1("locale") ) {
    charset =  QString::fromLatin1("%1 (locale)")
      .arg( QCString( kernel->networkCodec()->mimeName() ).lower() );
    return;
  }

  bool ok = false;
  QTextCodec *codec = KGlobal::charsets()->codecForName( charset, ok );
  if ( ok && codec ) {
    charset = QString::fromLatin1( codec->mimeName() ).lower();
    return;
  }

  KMessageBox::sorry( this, i18n("This charset is not supported.") );
  charset = QString::null;
}

void ComposerPage::CharsetTab::setup() {
  KConfigGroup composer( kapp->config(), "Composer" );

  QStringList charsets = composer.readListEntry( "pref-charsets" );
  for ( QStringList::Iterator it = charsets.begin() ;
	it != charsets.end() ; ++it )
      if ( (*it) == QString::fromLatin1("locale") )
	(*it) = QString("%1 (locale)")
	  .arg( QCString( kernel->networkCodec()->mimeName() ).lower() );

  mCharsetListEditor->setStringList( charsets );
  mKeepReplyCharsetCheck->setChecked( !composer.readBoolEntry( "force-reply-charset", false ) );
}

void ComposerPage::CharsetTab::apply() {
  KConfigGroup composer( kapp->config(), "Composer" );

  QStringList charsetList = mCharsetListEditor->stringList();
  QStringList::Iterator it = charsetList.begin();
  for ( ; it != charsetList.end() ; ++it )
    if ( (*it).endsWith("(locale)") )
      (*it) = "locale";
  composer.writeEntry( "pref-charsets", charsetList );
  composer.writeEntry( "force-reply-charset",
		       !mKeepReplyCharsetCheck->isChecked() );
}


QString ComposerPage::HeadersTab::title() {
  return i18n("H&eaders");
}

QString ComposerPage::HeadersTab::helpAnchor() {
  return QString::fromLatin1("configure-composer-headers");
}

ComposerPageHeadersTab::ComposerPageHeadersTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name )
{
  // tmp. vars:
  QVBoxLayout *vlay;
  QHBoxLayout *hlay;
  QGridLayout *glay;
  QLabel      *label;
  QPushButton *button;

  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // "Use custom Message-Id suffix" checkbox:
  mCreateOwnMessageIdCheck =
    new QCheckBox( i18n("&Use custom message-id suffix"), this );
  vlay->addWidget( mCreateOwnMessageIdCheck );

  // "Message-Id suffix" line edit and label:
  hlay = new QHBoxLayout( vlay ); // inherits spacing
  mMessageIdSuffixEdit = new QLineEdit( this );
  // only ASCII letters, digits, plus, minus and dots are allowed
  mMessageIdSuffixValidator =
    new QRegExpValidator( QRegExp( "[a-zA-Z0-9+-]+(?:\\.[a-zA-Z0-9+-]+)*" ), 0 );
  mMessageIdSuffixEdit->setValidator( mMessageIdSuffixValidator );
  label = new QLabel( mMessageIdSuffixEdit,
		      i18n("Custom message-&id suffix:"), this );
  label->setEnabled( false ); // since !mCreateOwnMessageIdCheck->isChecked()
  mMessageIdSuffixEdit->setEnabled( false );
  hlay->addWidget( label );
  hlay->addWidget( mMessageIdSuffixEdit, 1 );
  connect( mCreateOwnMessageIdCheck, SIGNAL(toggled(bool) ),
	   label, SLOT(setEnabled(bool)) );
  connect( mCreateOwnMessageIdCheck, SIGNAL(toggled(bool) ),
	   mMessageIdSuffixEdit, SLOT(setEnabled(bool)) );

  // horizontal rule and "custom header fields" label:
  vlay->addWidget( new KSeparator( KSeparator::HLine, this ) );
  vlay->addWidget( new QLabel( i18n("Define custom mime header fields:"), this) );

  // "custom header fields" listbox:
  glay = new QGridLayout( vlay, 5, 3 ); // inherits spacing
  glay->setRowStretch( 2, 1 );
  glay->setColStretch( 1, 1 );
  mTagList = new ListView( this, "tagList" );
  mTagList->addColumn( i18n("Name") );
  mTagList->addColumn( i18n("Value") );
  mTagList->setAllColumnsShowFocus( true );
  mTagList->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  mTagList->setSorting( -1 );
  connect( mTagList, SIGNAL(selectionChanged()),
	   this, SLOT(slotMimeHeaderSelectionChanged()) );
  glay->addMultiCellWidget( mTagList, 0, 2, 0, 1 );

  // "new" and "remove" buttons:
  button = new QPushButton( i18n("Ne&w"), this );
  connect( button, SIGNAL(clicked()), this, SLOT(slotNewMimeHeader()) );
  button->setAutoDefault( false );
  glay->addWidget( button, 0, 2 );
  mRemoveHeaderButton = new QPushButton( i18n("Re&move"), this );
  connect( mRemoveHeaderButton, SIGNAL(clicked()),
	   this, SLOT(slotRemoveMimeHeader()) );
  button->setAutoDefault( false );
  glay->addWidget( mRemoveHeaderButton, 1, 2 );

  // "name" and "value" line edits and labels:
  mTagNameEdit = new QLineEdit( this );
  mTagNameEdit->setEnabled( false );
  mTagNameLabel = new QLabel( mTagNameEdit, i18n("&Name:"), this );
  mTagNameLabel->setEnabled( false );
  glay->addWidget( mTagNameLabel, 3, 0 );
  glay->addWidget( mTagNameEdit, 3, 1 );
  connect( mTagNameEdit, SIGNAL(textChanged(const QString&)),
	   this, SLOT(slotMimeHeaderNameChanged(const QString&)) );

  mTagValueEdit = new QLineEdit( this );
  mTagValueEdit->setEnabled( false );
  mTagValueLabel = new QLabel( mTagValueEdit, i18n("&Value:"), this );
  mTagValueLabel->setEnabled( false );
  glay->addWidget( mTagValueLabel, 4, 0 );
  glay->addWidget( mTagValueEdit, 4, 1 );
  connect( mTagValueEdit, SIGNAL(textChanged(const QString&)),
	   this, SLOT(slotMimeHeaderValueChanged(const QString&)) );
}

void ComposerPage::HeadersTab::slotMimeHeaderSelectionChanged()
{
  QListViewItem * item = mTagList->selectedItem();

  if ( item ) {
    mTagNameEdit->setText( item->text( 0 ) );
    mTagValueEdit->setText( item->text( 1 ) );
  } else {
    mTagNameEdit->clear();
    mTagValueEdit->clear();
  }
  mRemoveHeaderButton->setEnabled( item );
  mTagNameEdit->setEnabled( item );
  mTagValueEdit->setEnabled( item );
  mTagNameLabel->setEnabled( item );
  mTagValueLabel->setEnabled( item );
}


void ComposerPage::HeadersTab::slotMimeHeaderNameChanged( const QString & text ) {
  // is called on ::setup(), when clearing the line edits. So be
  // prepared to not find a selection:
  QListViewItem * item = mTagList->selectedItem();
  if ( item )
    item->setText( 0, text );
}


void ComposerPage::HeadersTab::slotMimeHeaderValueChanged( const QString & text ) {
  // is called on ::setup(), when clearing the line edits. So be
  // prepared to not find a selection:
  QListViewItem * item = mTagList->selectedItem();
  if ( item )
    item->setText( 1, text );
}


void ComposerPage::HeadersTab::slotNewMimeHeader()
{
  QListViewItem *listItem = new QListViewItem( mTagList );
  mTagList->setCurrentItem( listItem );
  mTagList->setSelected( listItem, true );
}


void ComposerPage::HeadersTab::slotRemoveMimeHeader()
{
  // calling this w/o selection is a programming error:
  QListViewItem * item = mTagList->selectedItem();
  assert( item );

  QListViewItem * below = item->nextSibling();
  delete item;

  if ( below )
    mTagList->setSelected( below, true );
  else if ( mTagList->lastItem() )
    mTagList->setSelected( mTagList->lastItem(), true );
}

void ComposerPage::HeadersTab::setup() {
  KConfigGroup general( kapp->config(), "General" );

  QString suffix = general.readEntry( "myMessageIdSuffix", "" );
  mMessageIdSuffixEdit->setText( suffix );
  bool state = ( !suffix.isEmpty() &&
	    general.readBoolEntry( "useCustomMessageIdSuffix", false ) );
  mCreateOwnMessageIdCheck->setChecked( state );

  mTagList->clear();
  mTagNameEdit->clear();
  mTagValueEdit->clear();

  QListViewItem * item = 0;

  int count = general.readNumEntry( "mime-header-count", 0 );
  for( int i = 0 ; i < count ; i++ ) {
    KConfigGroup config( kapp->config(),
			 QCString("Mime #") + QCString().setNum(i) );
    QString name  = config.readEntry( "name" );
    QString value = config.readEntry( "value" );
    if( !name.isEmpty() )
      item = new QListViewItem( mTagList, item, name, value );
  }
  if ( mTagList->childCount() ) {
    mTagList->setCurrentItem( mTagList->firstChild() );
    mTagList->setSelected( mTagList->firstChild(), true );
  }
}

void ComposerPage::HeadersTab::apply() {
  KConfigGroup general( kapp->config(), "General" );

  general.writeEntry( "useCustomMessageIdSuffix",
		      mCreateOwnMessageIdCheck->isChecked() );
  general.writeEntry( "myMessageIdSuffix",
		      mMessageIdSuffixEdit->text() );

  int numValidEntries = 0;
  QListViewItem * item = mTagList->firstChild();
  for ( ; item ; item = item->itemBelow() )
    if( !item->text(0).isEmpty() ) {
      KConfigGroup config( kapp->config(), QCString("Mime #")
			     + QCString().setNum( numValidEntries ) );
      config.writeEntry( "name",  item->text( 0 ) );
      config.writeEntry( "value", item->text( 1 ) );
      numValidEntries++;
    }
  general.writeEntry( "mime-header-count", numValidEntries );
}


// *************************************************************
// *                                                           *
// *                      SecurityPage                         *
// *                                                           *
// *************************************************************




QString SecurityPage::iconLabel() {
  return i18n("Security");
}

const char * SecurityPage::iconName() {
  return "encrypted";
}

QString SecurityPage::title() {
  return i18n("Security & Privacy Settings");
}

QString SecurityPage::helpAnchor() {
  return QString::fromLatin1("configure-security");
}

SecurityPage::SecurityPage( QWidget * parent, const char * name )
  : TabbedConfigurationPage( parent, name )
{
  //
  // "General" tab:
  //
  mGeneralTab = new GeneralTab();
  addTab( mGeneralTab, mGeneralTab->title() );

  //
  // "PGP" tab:
  //
  mPgpTab = new Kpgp::Config();
  if ( mPgpTab->layout() ) {
    mPgpTab->layout()->setSpacing( KDialog::spacingHint() );
    mPgpTab->layout()->setMargin( KDialog::marginHint() );
  }
  addTab( mPgpTab, i18n("Open&PGP") );
}

void SecurityPage::setup() {
  mGeneralTab->setup();
  mPgpTab->setValues();
}

void SecurityPage::installProfile( KConfig * profile ) {
  mGeneralTab->installProfile( profile );
}

void SecurityPage::apply() {
  mGeneralTab->apply();
  mPgpTab->applySettings();
}

QString SecurityPage::GeneralTab::title() {
  return i18n("&General");
}

QString SecurityPage::GeneralTab::helpAnchor() {
  return QString::fromLatin1("configure-security-general");
}

SecurityPageGeneralTab::SecurityPageGeneralTab( QWidget * parent, const char * name )
  : ConfigurationPage ( parent, name )
{
  // tmp. vars:
  QVBoxLayout *vlay;
  QGroupBox   *group;
  QLabel      *label;
  QString     msg;

  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // "HTML Messages" group box:
  group = new QVGroupBox( i18n( "HTML Messages" ), this );
  group->layout()->setSpacing( KDialog::spacingHint() );

  mHtmlMailCheck = new QCheckBox( i18n("Prefer H&TML to plain text"), group );
  mExternalReferences = new QCheckBox( i18n("Allow messages to load e&xternal "
					    "references from the Internet" ), group );
  label = new QLabel( i18n("<qt><b>WARNING:</b> Allowing HTML in email may "
			   "increase the risk that your system will be "
			   "compromised by present and anticipated security "
			   "exploits. Use \"What's this\" help (Shift-F1) for "
			   "detailed information on each option.</qt>"),
		      group );
  label->setAlignment( WordBreak);

  vlay->addWidget( group );

  group = new QVGroupBox( i18n( "Delivery && Read Confirmations" ), this );
  group->layout()->setSpacing( KDialog::spacingHint() );

  mSendReceiptCheck = new QCheckBox( i18n("Automatically &send receive- and "
					  "read confirmations"), group );
  label = new QLabel( i18n( "<qt><b>WARNING:</b> Unconditionally returning "
			    "confirmations undermines your privacy. See "
			    "\"What's this\" help (Shift-F1) for more.</qt>" ),
		      group );
  label->setAlignment( WordBreak);

  vlay->addWidget( group );
  vlay->addStretch( 10 ); // spacer

  // and now: adding QWhat'sThis all over the place:
  msg = i18n( "<qt><p>Messages sometimes come in both formats. This option "
	      "controls whether you want the HTML part or the plain text "
	      "part to be displayed.</p>"
	      "<p>Displaying the HTML part makes the message look better, "
	      "but at the same time increases the risk of security holes "
	      "being exploited.</p>"
	      "<p>Displaying the plain text part loses much of the message's "
	      "formatting, but makes it almost <em>impossible</em> "
	      "to exploit security holes in the HTML renderer (Konqueror).</p>"
	      "<p>The option below guards against one common misuse of HTML "
	      "messages, but it cannot guard against security issues that were "
	      "not known at the time this version of KMail was written.</p>"
	      "<p>It is therefore advisable to <em>not</em> prefer HTML to "
	      "plain text.</p>"
	      "<p><b>Note:</b> You can set this option on a per-folder basis "
	      "from the <i>Folder</i> menu of KMail's main window.</p></qt>" );
  QWhatsThis::add( mHtmlMailCheck, msg );

  msg = i18n( "<qt><p>Some mail advertisements are in HTML and contain "
	      "references to, for example, images that the advertisers employ to "
	      "find out that you have read their message (\"web bugs\").</p>"
	      "<p>There is no valid reason to load images off the Internet like "
	      "this, since the sender can always attach the required images "
	      "directly to the message.</p>"
	      "<p>To guard from such a misuse of the HTML displaying feature "
	      "of KMail, this option is <em>disabled</em> by default.</p>"
	      "<p>However, if you wish to, for example, view images in HTML "
	      "messages that were not attached to it, you can enable this "
	      "option, but you should be aware of the possible problem.</p></qt>" );
  QWhatsThis::add( mExternalReferences, msg );

  msg = i18n( "<qt><p>This options enables the <em>unconditional</em> sending "
	      "of delivery- and read confirmations (\"receipts\").</p>"
	      "<p>Returning these confirmations (so-called <em>receipts</em>) "
	      "makes it easy for the sender to track whether and - more "
	      "importantly - <em>when</em> you read his/her message.</p>"
	      "<p>You can return <em>delivery</em> confirmations in a "
	      "fine-grained manner using the \"confirm delivery\" filter "
	      "action. We advise against issuing <em>read</em> confirmations "
	      "at all.</p></qt>");
  QWhatsThis::add( mSendReceiptCheck, msg );
}

void SecurityPage::GeneralTab::setup() {
  KConfigGroup general( kapp->config(), "General" );
  KConfigGroup reader( kapp->config(), "Reader" );


  mHtmlMailCheck->setChecked( reader.readBoolEntry( "htmlMail", false ) );
  mExternalReferences->setChecked( reader.readBoolEntry( "htmlLoadExternal", false ) );
  mSendReceiptCheck->setChecked( general.readBoolEntry( "send-receipts", false ) );
}

void SecurityPage::GeneralTab::installProfile( KConfig * profile ) {
  KConfigGroup general( profile, "General" );
  KConfigGroup reader( profile, "Reader" );

  if ( reader.hasKey( "htmlMail" ) )
    mHtmlMailCheck->setChecked( reader.readBoolEntry( "htmlMail" ) );
  if ( reader.hasKey( "htmlLoadExternal" ) )
    mExternalReferences->setChecked( reader.readBoolEntry( "htmlLoadExternal" ) );
  if ( general.hasKey( "send-receipts" ) )
    mSendReceiptCheck->setChecked( general.readBoolEntry( "send-receipts" ) );
};

void SecurityPage::GeneralTab::apply() {
  KConfigGroup general( kapp->config(), "General" );
  KConfigGroup reader( kapp->config(), "Reader" );

  if (reader.readBoolEntry( "htmlMail", false ) != mHtmlMailCheck->isChecked())
  {
    if (KMessageBox::warningContinueCancel(this, i18n("Changing the global "
      "HTML setting will override all folder specific values."), QString::null,
      QString::null, "htmlMailOverride") == KMessageBox::Continue)
    {
      reader.writeEntry( "htmlMail", mHtmlMailCheck->isChecked() );
      QStringList names;
      QValueList<QGuardedPtr<KMFolder> > folders;
      kernel->folderMgr()->createFolderList(&names, &folders);
      kernel->imapFolderMgr()->createFolderList(&names, &folders);
      for (QValueList<QGuardedPtr<KMFolder> >::iterator it = folders.begin();
        it != folders.end(); ++it)
      {
        if (*it)
        {
          KConfigGroupSaver saver(kapp->config(),
            "Folder-" + (*it)->idString());
          kapp->config()->writeEntry("htmlMailOverride", false);
        }
      }
    }
  }
  reader.writeEntry( "htmlLoadExternal", mExternalReferences->isChecked() );
  general.writeEntry( "send-receipts", mSendReceiptCheck->isChecked() );
}


// *************************************************************
// *                                                           *
// *                        MiscPage                           *
// *                                                           *
// *************************************************************



QString MiscPage::iconLabel() {
  return i18n("Miscellaneous");
}

const char * MiscPage::iconName() {
  return "misc";
}

QString MiscPage::title() {
  return i18n("Various Settings That Don't Fit Elsewhere");
}

QString MiscPage::helpAnchor() {
  return QString::fromLatin1("configure-misc");
}

MiscPage::MiscPage( QWidget * parent, const char * name )
  : TabbedConfigurationPage( parent, name )
{
  //
  // "Folders" tab:
  //
  mFoldersTab = new FoldersTab();
  addTab( mFoldersTab, mFoldersTab->title() );

// Disabled until kab is ported to libkabc.
#if 0
  //
  // "Addressbook" tab:
  //
  mAddressbookTab = new AddressbookTab();
  addTab( mAddressbookTab, mAddressbookTab->title() );
#endif
}

void MiscPage::setup() {
  mFoldersTab->setup();
#if 0
  mAddressbookTab->setup();
#endif
}

void MiscPage::installProfile( KConfig * profile ) {
  mFoldersTab->installProfile( profile );
#if 0
  mAddressbookTab->installProfile( profile );
#endif
}

void MiscPage::apply() {
  mFoldersTab->apply();
#if 0
  mAddressbookTab->apply();
#endif
}


QString MiscPage::FoldersTab::title() {
  return i18n("&Folders");
}

QString MiscPage::FoldersTab::helpAnchor() {
  return QString::fromLatin1("configure-misc-folders");
}


MiscPageFoldersTab::MiscPageFoldersTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name )
{
  // temp. vars:
  QVBoxLayout *vlay;
  QHBoxLayout *hlay;
  QGroupBox   *group;
  QLabel      *label;

  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // "confirm before emptying folder" check box: stretch 0
  mEmptyFolderConfirmCheck =
    new QCheckBox( i18n("Co&nfirm before emptying folders"), this );
  vlay->addWidget( mEmptyFolderConfirmCheck );
  mWarnBeforeExpire =
    new QCheckBox( i18n("&Warn before expiring messages"), this );
  vlay->addWidget( mWarnBeforeExpire );
  mLoopOnGotoUnread =
    new QCheckBox( i18n("&Loop in the current folder when trying to find "
			"unread messages"), this );
  vlay->addWidget( mLoopOnGotoUnread );
  mJumpToUnread =
    new QCheckBox( i18n("&Jump to first unread message when entering a "
			"folder"), this );
  vlay->addWidget( mJumpToUnread );

  hlay = new QHBoxLayout( vlay ); // inherits spacing
  mDelayedMarkAsRead = new QCheckBox( i18n("Mar&k selected message as read after"), this );
  hlay->addWidget( mDelayedMarkAsRead );
  mDelayedMarkTime = new KIntSpinBox( 0 /*min*/, 60 /*max*/, 1/*step*/,
				      0 /*init*/, 10 /*base*/, this);
  mDelayedMarkTime->setSuffix( i18n(" sec") );
  mDelayedMarkTime->setEnabled( false ); // since mDelayedMarkAsREad is off
  hlay->addWidget( mDelayedMarkTime );
  hlay->addStretch( 1 );

  connect(mDelayedMarkAsRead, SIGNAL(toggled(bool)), mDelayedMarkTime,
  	SLOT(setEnabled(bool)));

  // "show popup after Drag'n'Drop" checkbox: stretch 0
  mShowPopupAfterDnD =
    new QCheckBox( i18n("Ask for action after dragging messages to another folder"), this );
  vlay->addWidget( mShowPopupAfterDnD );

  // "default mailbox format" combo + label: stretch 0
  hlay = new QHBoxLayout( vlay ); // inherits spacing
  mMailboxPrefCombo = new QComboBox( false, this );
  label = new QLabel( mMailboxPrefCombo,
		      i18n("to be continued with \"flat files\" and "
			   "\"directories\", resp.",
			   "By default, &message folders on disk are:"), this );
  mMailboxPrefCombo->insertStringList( QStringList()
	  << i18n("continuation of \"By default, &message folders on disk are\"",
		  "Flat Files (\"mbox\" format)")
	  << i18n("continuation of \"By default, &message folders on disk are\"",
		  "Directories (\"maildir\" format)") );
  hlay->addWidget( label );
  hlay->addWidget( mMailboxPrefCombo, 1 );

  // "On exit..." groupbox:
  group = new QVGroupBox( i18n("On Program Exit, "
			       "Perform Following Tasks"), this );
  group->layout()->setSpacing( KDialog::spacingHint() );
  mCompactOnExitCheck = new QCheckBox( i18n("Com&pact all folders"), group );
  mEmptyTrashCheck = new QCheckBox( i18n("Empty &trash"), group );
  mExpireAtExit = new QCheckBox( i18n("&Expire old messages"), group );

  vlay->addWidget( group );
  vlay->addStretch( 1 );

  // and now: add QWhatsThis:
  QString msg = i18n( "what's this help",
		      "<qt><p>This selects which mailbox format will be "
		      "the default for local folders:</p>"
		      "<p><b>mbox:</b> KMail's mail "
		      "folders are represented by a single file each. "
		      "Individual messages are separated from each other by a "
		      "line starting with \"From \". This saves space on "
		      "disk, but may be less robust, e.g. when moving messages "
		      "between folders.</p>"
		      "<p><b>maildir:</b> KMail's mail folders are "
		      "represented by real folders on disk. Individual messages "
		      "are separate files. This may waste a bit of space on "
		      "disk, but should be more robust, e.g. when moving "
		      "messages between folders.</p></qt>");
  QWhatsThis::add( mMailboxPrefCombo, msg );
  QWhatsThis::add( label, msg );

  msg = i18n( "what's this help",
	      "<qt><p>When jumping to the next unread message, it may occur "
	      "that no more unread messages are below the current message.</p>"
	      "<p>When this option is checked, the search will start at the "
	      "top of the message list. Otherwise, it will do nothing.</p>"
	      "<p>Similarly, when searching for the previous unread message, "
	      "the search will start from the bottom of the message list if "
	      "this option is checked.</p></qt>" );
  QWhatsThis::add( mLoopOnGotoUnread, msg );
}

void MiscPage::FoldersTab::setup() {
  KConfigGroup general( kapp->config(), "General" );
  KConfigGroup behaviour( kapp->config(), "Behaviour" );

  mEmptyTrashCheck->setChecked( general.readBoolEntry( "empty-trash-on-exit", false ) );
  mExpireAtExit->setChecked( general.readNumEntry( "when-to-expire", 0 ) ); // set if non-zero
  mWarnBeforeExpire->setChecked( general.readBoolEntry( "warn-before-expire", true ) );
  mCompactOnExitCheck->setChecked( general.readBoolEntry( "compact-all-on-exit", true ) );
  mEmptyFolderConfirmCheck->setChecked( general.readBoolEntry( "confirm-before-empty", true ) );
  mLoopOnGotoUnread->setChecked( behaviour.readBoolEntry( "LoopOnGotoUnread", true ) );
  mJumpToUnread->setChecked( behaviour.readBoolEntry( "JumpToUnread", false ) );
  mDelayedMarkAsRead->setChecked( behaviour.readBoolEntry( "DelayedMarkAsRead", true ) );
  mDelayedMarkTime->setValue( behaviour.readNumEntry( "DelayedMarkTime", 0 ) );
  mShowPopupAfterDnD->setChecked( behaviour.readBoolEntry( "ShowPopupAfterDnD", true ) );

  int num = general.readNumEntry("default-mailbox-format", 1 );
  if ( num < 0 || num > 1 ) num = 1;
  mMailboxPrefCombo->setCurrentItem( num );
}

void MiscPage::FoldersTab::apply() {
  KConfigGroup general( kapp->config(), "General" );
  KConfigGroup behaviour( kapp->config(), "Behaviour" );

  general.writeEntry( "empty-trash-on-exit", mEmptyTrashCheck->isChecked() );
  general.writeEntry( "compact-all-on-exit", mCompactOnExitCheck->isChecked() );
  general.writeEntry( "confirm-before-empty", mEmptyFolderConfirmCheck->isChecked() );
  general.writeEntry( "default-mailbox-format", mMailboxPrefCombo->currentItem() );
  general.writeEntry( "warn-before-expire", mWarnBeforeExpire->isChecked() );
  behaviour.writeEntry( "LoopOnGotoUnread", mLoopOnGotoUnread->isChecked() );
  behaviour.writeEntry( "JumpToUnread", mJumpToUnread->isChecked() );
  behaviour.writeEntry( "DelayedMarkAsRead", mDelayedMarkAsRead->isChecked() );
  behaviour.writeEntry( "DelayedMarkTime", mDelayedMarkTime->value() );
  behaviour.writeEntry( "ShowPopupAfterDnD", mShowPopupAfterDnD->isChecked() );

  if ( mExpireAtExit->isChecked() )
    general.writeEntry( "when-to-expire", expireAtExit );
  else
    general.writeEntry( "when-to-expire", expireManual );
}



QString MiscPage::AddressbookTab::title() {
  return i18n("Address&book");
}

QString MiscPage::AddressbookTab::helpAnchor() {
  return QString::fromLatin1("configure-misc-addressbook");
}


static const struct {
  const char * label;
  const char * description;
} addressBooks[] = {
#if 0
  { I18N_NOOP("KAB"),
    I18N_NOOP("The KDE Address Book graphical interface (KAB) using the "
	      "standard KDE Address Book (KAB) database. Requires the "
	      "kdeutils package to be installed.") },
#endif
  { I18N_NOOP("KAddressbook"),
    I18N_NOOP("The new KDE Address Book graphical interface "
	      "(KAddressbook) using the standard KDE Address Book (KAB) "
	      "database.") },
};
static const int numAddressBooks = sizeof addressBooks / sizeof *addressBooks;

MiscPageAddressbookTab::MiscPageAddressbookTab( QWidget * parent, const char * name )
  : ConfigurationPage( parent, name )
{
  // tmp. vars:
  QVBoxLayout  *vlay;
  QHBoxLayout  *hlay;
  QWidgetStack *widgetStack;
  QGroupBox    *group;
  QLabel       *label;

  vlay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // addressbook combo box and label:
  hlay = new QHBoxLayout( vlay ); // inherits spacing
  mAddressbookCombo = new QComboBox( false, this );
  for ( int i = 0 ; i < numAddressBooks ; i++ )
    mAddressbookCombo->insertItem( i18n( addressBooks[i].label ) );

  hlay->addWidget( new QLabel( mAddressbookCombo,
			       i18n("Choose a&ddressbook:"), this ) );
  hlay->addWidget( mAddressbookCombo, 1 );

  group = new QVGroupBox( i18n("Description"), this );

  widgetStack = new QWidgetStack( group );
  for ( int i = 0 ; i < numAddressBooks ; i++ ) {
    label = new QLabel( i18n( addressBooks[i].description ), widgetStack );
    label->setAlignment( AlignTop|WordBreak );
    widgetStack->addWidget( label, i );
  }
  widgetStack->raiseWidget( 0 );
  connect( mAddressbookCombo, SIGNAL(highlighted(int)),
	   widgetStack, SLOT(raiseWidget(int)) );

  vlay->addWidget( group );
  vlay->addStretch( 100 );
}

void MiscPage::AddressbookTab::setup() {
  int num = KConfigGroup( kapp->config(), "General" )
    .readNumEntry( "addressbook", numAddressBooks - 1 );
  mAddressbookCombo->setCurrentItem( num );
}

void MiscPage::AddressbookTab::apply() {
  KConfigGroup general( kapp->config(), "General" );
  general.writeEntry( "addressbook", mAddressbookCombo->currentItem() );
}


// *************************************************************
// *                                                           *
// *                       PluginPage                          *
// *                                                           *
// *************************************************************



QString pluginNameToNumberedItem( QString name, int number )
{
    QString item = QString("   %1. ").arg( number );
    item += name;
    item += "    ";
    return item;
}

QString PluginPage::iconLabel() {
  return i18n("Plugins");
}

const char * PluginPage::iconName() {
  return "connect_established";
}

QString PluginPage::title() {
  return i18n("Load & Configure KMail Plugins");
}

QString PluginPage::helpAnchor() {
  return QString::fromLatin1("configure-plugins");
}

PluginPage::PluginPage( CryptPlugWrapperList* cryptPlugList,
                        QWidget * parent, const char * name )
    : TabbedConfigurationPage( parent, name ),
      mCryptPlugList( cryptPlugList )
{
    _generalPage = new GeneralPage( this );
    addTab( _generalPage, i18n( "&General") );

    _certificatesPage = new CertificatesPage( this );
    addTab( _certificatesPage, i18n( "Cert&ificates" ) );

    _signaturePage = new SignaturePage( this );
    addTab( _signaturePage, i18n("Signature Configuration") );

    _encryptionPage = new EncryptionPage( this );
    addTab( _encryptionPage, i18n("&Encryption Configuration") );

// KHZ NO FULLTEST  _dirservicesPage = new DirServicesPage( this );
// KHZ NO FULLTEST  addTab( _dirservicesPage, i18n("&Directory Services") );
_dirservicesPage = new DirServicesPage( 0 );
_dirservicesPage->hide();


    connect( mTabWidget, SIGNAL(currentChanged( QWidget * )),
             this, SLOT(slotCurrentPlugInTabPageChanged( QWidget * )) );

    //slotPlugSelectionChanged();
}

void PluginPage::dismiss()
{
    _generalPage->dismiss();
}


void PluginPage::setup()
{
    _generalPage->setup();
    _certificatesPage->setup();
    _signaturePage->setup();
    _encryptionPage->setup();
    _dirservicesPage->setup();
}


void PluginPage::installProfile( KConfig * profile )
{
    _generalPage->installProfile( profile );
    _certificatesPage->installProfile( profile );
    _signaturePage->installProfile( profile );
    _encryptionPage->installProfile( profile );
    _dirservicesPage->installProfile( profile );
}


// PENDING(kalle) Consider splitting savePluginConfig() as well
void PluginPage::apply()
{
    _generalPage->apply(); // also saves the other pages
}


void PluginPage::slotCurrentPlugInTabPageChanged( QWidget * page )
{
    // Find the combo box of the tab page we are going to enter
    QComboBox* pCBox = 0;
    if( _certificatesPage == page )
        pCBox = _certificatesPage->plugListBoxCertConf;
    else if( _signaturePage == page )
        pCBox = _signaturePage->plugListBoxSignConf;
    else if( _encryptionPage == page )
        pCBox = _encryptionPage->plugListBoxEncryptConf;
    else if( _dirservicesPage == page )
        pCBox = _dirservicesPage->plugListBoxDirServConf;
    // adjust or insert the plug in names in this tab page's combo box
    if( pCBox ) {
        pCBox->clear();
        if( _generalPage->currentPlugItem ) {
            // adjust or insert the plug in names in this tab page's combo box
            int numEntry = _generalPage->plugList->childCount();
            QListViewItem *item = _generalPage->plugList->firstChild();
            int i = 0;
            int curI = -1;
            for (i = 0; i < numEntry; ++i) {
                QString itemTxt( pluginNameToNumberedItem( item->text(0), 1+i ) );
                if( i < pCBox->count() )
                    pCBox->changeItem( itemTxt, i );
                else
                    pCBox->insertItem( itemTxt );
                if( item == _generalPage->currentPlugItem )
                    curI = i;
                item = item->nextSibling();
            }
            if( -1 < curI ) {
                pCBox->setCurrentItem( curI );
                // Changes made?
                CryptPlugWrapper* wrapper = mCryptPlugList->at( curI );
                if( ! ( wrapper &&
                        (wrapper->displayName() == _generalPage->currentPlugItem->text(0)) &&
                        (wrapper->libName()     == _generalPage->currentPlugItem->text(1)) &&
                        (wrapper->updateURL()   == _generalPage->currentPlugItem->text(2)) &&
                        isPluginConfigEqual( curI ) ) ) {
                    if( KMessageBox::questionYesNo( this, i18n( "Do you want to save\nany changes you might have made\nto the previous plugin configuration?" ),
                                                    i18n( "Save Plugin Configuration" ) ) == KMessageBox::Yes ) {
                        _generalPage->savePluginsConfig( false );
                        savePluginConfig( curI );
                        // Enable/disable the fields in the dialog pages
                        // according to the plugin capabilities.
                        if( _signaturePage && _signaturePage->sigDialog )
                            _signaturePage->sigDialog->enableDisable( wrapper );
                        if( _encryptionPage && _encryptionPage->encDialog )
                            _encryptionPage->encDialog->enableDisable( wrapper );
                        if( _dirservicesPage && _dirservicesPage->dirservDialog )
                            _dirservicesPage->dirservDialog->enableDisable( wrapper );
                    }
                }
            }
        }
    }
}


void PluginPage::slotPlugSelectionChanged()
{
    if( _generalPage->plugList->selectedItem() != _generalPage->currentPlugItem ) {

        // If there already was a current plugin and there were
        // changes made, ask user to save the changes.
        if( _generalPage->currentPlugItem ) {
            // Find the number of the current plugin
            int pos = 0;
            QListViewItemIterator lvit( _generalPage->plugList );
            QListViewItem* current;
            while( ( current = lvit.current() ) ) {
                ++lvit;
                if( current == _generalPage->currentPlugItem )
                    break;
                ++pos;
            }

            // Changes made?
            CryptPlugWrapper* wrapper = mCryptPlugList->at( pos );
            if( ! ( wrapper &&
                    (wrapper->displayName() == _generalPage->currentPlugItem->text(0)) &&
                    (wrapper->libName()     == _generalPage->currentPlugItem->text(1)) &&
                    (wrapper->updateURL()   == _generalPage->currentPlugItem->text(2)) &&
                    isPluginConfigEqual( pos ) ) ) {
                if( KMessageBox::questionYesNo( this, i18n( "Do you want to save\nany changes you might have made\nto the previous plugin configuration?" ),
                                                i18n( "Save Plugin Configuration" ) ) == KMessageBox::Yes ) {
                    _generalPage->savePluginsConfig( false );
                    savePluginConfig( pos );
                }
            } else
                ;
        }

        _generalPage->currentPlugItem = _generalPage->plugList->selectedItem();
        if( _generalPage->currentPlugItem != 0 ) {
            // (De)activate Activate button
            if( _generalPage->currentPlugItem->text( 3 ) == "" )
                _generalPage->activateCryptPlugButton->setText( i18n("Ac&tivate")  );
            else
                _generalPage->activateCryptPlugButton->setText( i18n("Deac&tivate") );

            _generalPage->plugNameEdit->setText( _generalPage->currentPlugItem->text(0) );
            _generalPage->plugLocationRequester->setURL( _generalPage->currentPlugItem->text(1) );
            _generalPage->plugUpdateURLEdit->setText( _generalPage->currentPlugItem->text(2) );
            _generalPage->plugNameEdit->setEnabled( true );
            _generalPage->plugLocationRequester->setEnabled( true );
            _generalPage->plugUpdateURLEdit->setEnabled( true );
            // synchronize configuration list boxes
            int numEntry = _generalPage->plugList->childCount();
            QListViewItem *item = _generalPage->plugList->firstChild();
            int i = 0;
            for (i = 0; i < numEntry; ++i) {
                if( item == _generalPage->currentPlugItem ) {
                    if( _certificatesPage->plugListBoxCertConf && i < _certificatesPage->plugListBoxCertConf->count() ) {
                        _certificatesPage->plugListBoxCertConf->setCurrentItem( i );
                    }
                    if( _signaturePage->plugListBoxSignConf    && i < _signaturePage->plugListBoxSignConf->count() ) {
                        _signaturePage->plugListBoxSignConf->setCurrentItem( i );
                    }
                    if( _encryptionPage->plugListBoxEncryptConf && i < _encryptionPage->plugListBoxEncryptConf->count() ) {
                        _encryptionPage->plugListBoxEncryptConf->setCurrentItem( i );
                    }
                    if( _dirservicesPage->plugListBoxDirServConf && i < _dirservicesPage->plugListBoxDirServConf->count() ) {
                        _dirservicesPage->plugListBoxDirServConf->setCurrentItem( i );
                    }

                    break;
                }
                item = item->nextSibling();
            }

            if( i < numEntry ) {
                // i contains the position of the plugin
                CryptPlugWrapper* wrapper = mCryptPlugList->at( i );
                Q_ASSERT( wrapper );
                if( !wrapper ) return;


                // Enable/disable the fields in the dialog pages
                // according to the plugin capabilities.
                _signaturePage->sigDialog->enableDisable( wrapper );
                _encryptionPage->encDialog->enableDisable( wrapper );
                _dirservicesPage->dirservDialog->enableDisable( wrapper );

                // Fill the fields of the tab pages with the data from
                // the current plugin.

                // Signature tab

                // Sign Messages group box
                switch( wrapper->signEmail() ) {
                case SignEmail_SignAll:
                    _signaturePage->sigDialog->signAllPartsRB->setChecked( true );
                    break;
                case SignEmail_Ask:
                    _signaturePage->sigDialog->askEachPartRB->setChecked( true );
                    break;
                case SignEmail_DontSign:
                    _signaturePage->sigDialog->dontSignRB->setChecked( true );
                    break;
                default:
                    kdDebug( 5006 ) << "Unknown email sign setting" << endl;
                };
                _signaturePage->sigDialog->warnUnsignedCB->setChecked( wrapper->warnSendUnsigned() );

                // Compound Mode group box
                switch( wrapper->signatureCompoundMode() ) {
                case SignatureCompoundMode_Opaque:
                    _signaturePage->sigDialog->sendSigOpaqueRB->setChecked( true );
                    break;
                case SignatureCompoundMode_Detached:
                    _signaturePage->sigDialog->sendSigMultiPartRB->setChecked( true );
                    break;
                default:
                    _signaturePage->sigDialog->sendSigMultiPartRB->setChecked( true );
                    kdDebug( 5006 ) << "Unknown signature compound mode setting, default set to multipart/signed" << endl;
                };

                // Sending Certificates group box
                switch( wrapper->sendCertificates() ) {
                case SendCert_DontSend:
                    _signaturePage->sigDialog->dontSendCertificatesRB->setChecked( true );
                    break;
                case SendCert_SendOwn:
                    _signaturePage->sigDialog->sendYourOwnCertificateRB->setChecked( true );
                    break;
                case SendCert_SendChainWithoutRoot:
                    _signaturePage->sigDialog->sendChainWithoutRootRB->setChecked( true );
                    break;
                case SendCert_SendChainWithRoot:
                    _signaturePage->sigDialog->sendChainWithRootRB->setChecked( true );
                    break;
                default:
                    kdDebug( 5006 ) << "Unknown send certificate setting" << endl;                }

                // Signature Settings group box
                SignatureAlgorithm sigAlgo = wrapper->signatureAlgorithm();
                QString sigAlgoStr;
                switch( sigAlgo ) {
                case SignAlg_SHA1:
                    sigAlgoStr = "SHA1";
                    break;
                default:
                    kdDebug( 5006 ) << "Unknown signature algorithm" << endl;
                };

                for( int i = 0;
                     i < _signaturePage->sigDialog->signatureAlgorithmCO->count(); ++i )
                    if( _signaturePage->sigDialog->signatureAlgorithmCO->text( i ) ==
                        sigAlgoStr ) {
                        _signaturePage->sigDialog->signatureAlgorithmCO->setCurrentItem( i );
                        break;
                    }

                _signaturePage->sigDialog->warnSignatureCertificateExpiresCB->setChecked( wrapper->signatureCertificateExpiryNearWarning() );
                _signaturePage->sigDialog->warnSignatureCertificateExpiresSB->setValue( wrapper->signatureCertificateExpiryNearInterval() );
                _signaturePage->sigDialog->warnCACertificateExpiresCB->setChecked( wrapper->caCertificateExpiryNearWarning() );
                _signaturePage->sigDialog->warnCACertificateExpiresSB->setValue( wrapper->caCertificateExpiryNearInterval() );
                _signaturePage->sigDialog->warnRootCertificateExpiresCB->setChecked( wrapper->rootCertificateExpiryNearWarning() );
                _signaturePage->sigDialog->warnRootCertificateExpiresSB->setValue( wrapper->rootCertificateExpiryNearInterval() );

                _signaturePage->sigDialog->warnAddressNotInCertificateCB->setChecked( wrapper->warnNoCertificate() );

                // PIN Entry group box
                switch( wrapper->numPINRequests() ) {
                case PinRequest_OncePerSession:
                    _signaturePage->sigDialog->pinOncePerSessionRB->setChecked( true );
                    break;
                case PinRequest_Always:
                    _signaturePage->sigDialog->pinAlwaysRB->setChecked( true );
                    break;
                case PinRequest_WhenAddingCerts:
                    _signaturePage->sigDialog->pinAddCertificatesRB->setChecked( true );
                    break;
                case PinRequest_AlwaysWhenSigning:
                    _signaturePage->sigDialog->pinAlwaysWhenSigningRB->setChecked( true );
                    break;
                case PinRequest_AfterMinutes:
                    _signaturePage->sigDialog->pinIntervalRB->setChecked( true );
                    break;
                default:
                    kdDebug( 5006 ) << "Unknown pin request setting" << endl;
                };

                _signaturePage->sigDialog->pinIntervalSB->setValue( wrapper->numPINRequestsInterval() );

                // Save Messages group box
                _signaturePage->sigDialog->saveSentSigsCB->setChecked( wrapper->saveSentSignatures() );

                // The Encryption tab

                // Encrypt Messages group box
                switch( wrapper->encryptEmail() ) {
                case EncryptEmail_EncryptAll:
                    _encryptionPage->encDialog->encryptAllPartsRB->setChecked( true );
                    break;
                case EncryptEmail_Ask:
                    _encryptionPage->encDialog->askEachPartRB->setChecked( true );
                    break;
                case EncryptEmail_DontEncrypt:
                    _encryptionPage->encDialog->dontEncryptRB->setChecked( true );
                    break;
                default:
                    kdDebug( 5006 ) << "Unknown email encryption setting" << endl;
                };
                _encryptionPage->encDialog->warnUnencryptedCB->setChecked( wrapper->warnSendUnencrypted() );

                // Encryption Settings group box
                QString encAlgoStr;
                switch( wrapper->encryptionAlgorithm() ) {
                case EncryptAlg_RSA:
                    encAlgoStr = "RSA";
                    break;
                case EncryptAlg_TripleDES:
                    encAlgoStr = "Triple-DES";
                    break;
                case EncryptAlg_SHA1:
                    encAlgoStr = "SHA-1";
                    break;
                default:
                    kdDebug( 5006 ) << "Unknown encryption algorithm" << endl;
                };

                for( int i = 0;
                     i < _encryptionPage->encDialog->encryptionAlgorithmCO->count(); ++i )
                    if( _encryptionPage->encDialog->encryptionAlgorithmCO->text( i ) ==
                        encAlgoStr ) {
                        _encryptionPage->encDialog->encryptionAlgorithmCO->setCurrentItem( i );
                        break;
                    }

                _encryptionPage->encDialog->warnReceiverCertificateExpiresCB->setChecked( wrapper->receiverCertificateExpiryNearWarning() );
                _encryptionPage->encDialog->warnReceiverCertificateExpiresSB->setValue( wrapper->receiverCertificateExpiryNearWarningInterval() );
                _encryptionPage->encDialog->warnChainCertificateExpiresCB->setChecked( wrapper->certificateInChainExpiryNearWarning() );
                _encryptionPage->encDialog->warnChainCertificateExpiresSB->setValue( wrapper->certificateInChainExpiryNearWarningInterval() );
                _encryptionPage->encDialog->warnReceiverNotInCertificateCB->setChecked( wrapper->receiverEmailAddressNotInCertificateWarning() );

                // CRL group box
                _encryptionPage->encDialog->useCRLsCB->setChecked( wrapper->encryptionUseCRLs() );
                _encryptionPage->encDialog->warnCRLExpireCB->setChecked( wrapper->encryptionCRLExpiryNearWarning() );
                _encryptionPage->encDialog->warnCRLExpireSB->setValue( wrapper->encryptionCRLNearExpiryInterval() );

                // Save Messages group box
                _encryptionPage->encDialog->storeEncryptedCB->setChecked( wrapper->saveMessagesEncrypted() );

                // Certificate Path Check group box
                _encryptionPage->encDialog->checkCertificatePathCB->setChecked( wrapper->checkCertificatePath() );
                if( wrapper->checkEncryptionCertificatePathToRoot() )
                    _encryptionPage->encDialog->alwaysCheckRootRB->setChecked( true );
                else
                    _encryptionPage->encDialog->pathMayEndLocallyCB->setChecked( true );

                // Directory Services tab page

                int numServers;
                CryptPlugWrapper::DirectoryServer* servers = wrapper->directoryServers( &numServers );
                if( servers ) {
                    QListViewItem* previous = 0;
                    for( int i = 0; i < numServers; i++ ) {
                        previous = new QListViewItem( _dirservicesPage->dirservDialog->x500LV,
                                                      previous,
                                                      QString::fromUtf8( servers[i].servername ),
                                                      QString::number( servers[i].port ),
                                                      QString::fromUtf8( servers[i].description ) );
                    }
                }

                // Local/Remote Certificates group box
                switch( wrapper->certificateSource() ) {
                case CertSrc_ServerLocal:
                    _dirservicesPage->dirservDialog->firstLocalThenDSCertRB->setChecked( true );
                    break;
                case CertSrc_Local:
                    _dirservicesPage->dirservDialog->localOnlyCertRB->setChecked( true );
                    break;
                case CertSrc_Server:
                    _dirservicesPage->dirservDialog->dsOnlyCertRB->setChecked( true );
                    break;
                default:
                    kdDebug( 5006 ) << "Unknown certificate source" << endl;
                }

                // Local/Remote CRL group box
                switch( wrapper->crlSource() ) {
                case CertSrc_ServerLocal:
                    _dirservicesPage->dirservDialog->firstLocalThenDSCRLRB->setChecked( true );
                    break;
                case CertSrc_Local:
                    _dirservicesPage->dirservDialog->localOnlyCRLRB->setChecked( true );
                    break;
                case CertSrc_Server:
                    _dirservicesPage->dirservDialog->dsOnlyCRLRB->setChecked( true );
                    break;
                default:
                    kdDebug( 5006 ) << "Unknown certificate source" << endl;
                }
            }
        }
    }
}


bool PluginPage::isPluginConfigEqual( int pluginno ) const
{
    if ( mCryptPlugList->isEmpty() ) // happens initially
        return true;
    CryptPlugWrapper* wrapper = mCryptPlugList->at( pluginno );
    Q_ASSERT( wrapper );
    if( !wrapper ) {
      // No wrapper? Better not annoy the user...
      return true;
    }

    // if the wrapper is not initialized, it does not return
    // reasonable values, and saving them won't help either - just
    // return true
    if( wrapper->initStatus( 0 ) != CryptPlugWrapper::InitStatus_Ok )
        return true;

    bool ret = true;
    ret &= ( ( ( wrapper->signEmail() == SignEmail_SignAll ) &&
               _signaturePage->sigDialog->signAllPartsRB->isChecked() ) ||
             ( ( wrapper->signEmail() == SignEmail_Ask ) &&
               _signaturePage->sigDialog->askEachPartRB->isChecked() ) ||
             ( ( wrapper->signEmail() == SignEmail_DontSign ) &&
               _signaturePage->sigDialog->dontSignRB->isChecked() ) );
    kdDebug(5006) << "1) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->warnSendUnsigned() ==
             _signaturePage->sigDialog->warnUnsignedCB->isChecked() );
    kdDebug(5006) << "2) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( ( ( wrapper->signatureCompoundMode() == SignatureCompoundMode_Opaque ) &&
               _signaturePage->sigDialog->sendSigOpaqueRB->isChecked() ) ||
             ( ( wrapper->signatureCompoundMode() == SignatureCompoundMode_Detached ) &&
               _signaturePage->sigDialog->sendSigMultiPartRB->isChecked() ) ||
             ( ( wrapper->signatureCompoundMode() == SignatureCompoundMode_undef ) &&
               !_signaturePage->sigDialog->sendSigOpaqueRB->isChecked() &&
               !_signaturePage->sigDialog->sendSigMultiPartRB->isChecked() ) );
    kdDebug(5006) << "2b) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( ( ( wrapper->sendCertificates() == SendCert_DontSend ) &&
               _signaturePage->sigDialog->dontSendCertificatesRB->isChecked() ) ||
             ( ( wrapper->sendCertificates() == SendCert_SendOwn ) &&
               _signaturePage->sigDialog->sendYourOwnCertificateRB->isChecked() ) ||
             ( ( wrapper->sendCertificates() == SendCert_SendChainWithoutRoot ) &&
               _signaturePage->sigDialog->sendChainWithoutRootRB->isChecked() ) ||
             ( ( wrapper->sendCertificates() == SendCert_SendChainWithRoot ) ||
               _signaturePage->sigDialog->sendChainWithRootRB->isChecked() ) );
    kdDebug(5006) << "3) RET = " << ret << endl;
    if( !ret )
        return false;

    if( !_signaturePage ||
        !_signaturePage->sigDialog ||
        !_signaturePage->sigDialog->signatureAlgorithmCO )
        return true;
    kdDebug(5006) << "4) RET = " << ret << endl;

    ret &= ( ( ( wrapper->signatureAlgorithm() == SignAlg_SHA1 ) &&
               _signaturePage &&
               _signaturePage->sigDialog &&
               _signaturePage->sigDialog->signatureAlgorithmCO &&
               ( _signaturePage->sigDialog->signatureAlgorithmCO->currentText() ==
                 "RSA + SHA-1" ) ) );
    kdDebug(5006) << "5) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->signatureCertificateExpiryNearWarning() ==
             _signaturePage->sigDialog->warnSignatureCertificateExpiresCB->isChecked() );
    kdDebug(5006) << "6) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->signatureCertificateExpiryNearInterval() ==
             _signaturePage->sigDialog->warnSignatureCertificateExpiresSB->value() );
    kdDebug(5006) << "7) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->caCertificateExpiryNearWarning() ==
             _signaturePage->sigDialog->warnCACertificateExpiresCB->isChecked() );
    kdDebug(5006) << "8) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->caCertificateExpiryNearInterval() ==
             _signaturePage->sigDialog->warnCACertificateExpiresSB->value() );
    kdDebug(5006) << "9) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->rootCertificateExpiryNearWarning() ==
             _signaturePage->sigDialog->warnRootCertificateExpiresCB->isChecked() );
    kdDebug(5006) << "10) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->rootCertificateExpiryNearInterval() ==
             _signaturePage->sigDialog->warnRootCertificateExpiresSB->value() );
    kdDebug(5006) << "11) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->warnNoCertificate() ==
             _signaturePage->sigDialog->warnAddressNotInCertificateCB->isChecked() );

    kdDebug(5006) << "12) RET = " << ret << endl;
    if( !ret )
        return false;


    ret &= ( ( ( wrapper->numPINRequests() == PinRequest_OncePerSession ) &&
               _signaturePage->sigDialog->pinOncePerSessionRB->isChecked() ) ||
             ( ( wrapper->numPINRequests() == PinRequest_Always ) &&
               _signaturePage->sigDialog->pinAlwaysRB->isChecked() ) ||
             ( ( wrapper->numPINRequests() == PinRequest_WhenAddingCerts ) &&
               _signaturePage->sigDialog->pinAddCertificatesRB->isChecked() ) ||
             ( ( wrapper->numPINRequests() == PinRequest_AlwaysWhenSigning ) &&
               _signaturePage->sigDialog->pinAlwaysWhenSigningRB->isChecked() ) ||
             ( ( wrapper->numPINRequests() == PinRequest_AfterMinutes ) ) );
    kdDebug(5006) << "13) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->numPINRequestsInterval() ==
             _signaturePage->sigDialog->pinIntervalSB->value() );
    kdDebug(5006) << "14) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->saveSentSignatures() ==
             _signaturePage->sigDialog->saveSentSigsCB->isChecked() );
    kdDebug(5006) << "15) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( ( ( wrapper->encryptEmail() == EncryptEmail_EncryptAll ) &&
               _encryptionPage->encDialog->encryptAllPartsRB->isChecked() ) ||
             ( ( wrapper->encryptEmail() == EncryptEmail_Ask ) &&
               _encryptionPage->encDialog->askEachPartRB->isChecked() ) ||
             ( ( wrapper->encryptEmail() == EncryptEmail_DontEncrypt ) &&
               _encryptionPage->encDialog->dontEncryptRB->isChecked() ) );
    kdDebug(5006) << "16) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->warnSendUnencrypted() ==
             _encryptionPage->encDialog->warnUnencryptedCB->isChecked() );
    kdDebug(5006) << "17) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( ( ( wrapper->encryptionAlgorithm() == EncryptAlg_SHA1 ) &&
               ( _encryptionPage->encDialog->encryptionAlgorithmCO->currentText() ==
                 "SHA-1" ) ) ||
             ( ( wrapper->encryptionAlgorithm() == EncryptAlg_TripleDES ) &&
               ( _encryptionPage->encDialog->encryptionAlgorithmCO->currentText() ==
                 "Triple-DES" ) ) ||
             ( ( wrapper->encryptionAlgorithm() == EncryptAlg_RSA ) &&
               ( _encryptionPage->encDialog->encryptionAlgorithmCO->currentText() ==
                 "RSA" ) ) );
    kdDebug(5006) << "18) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->receiverCertificateExpiryNearWarning() ==
             _encryptionPage->encDialog->warnReceiverCertificateExpiresCB->isChecked() );
    kdDebug(5006) << "19) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->receiverCertificateExpiryNearWarningInterval() ==
             _encryptionPage->encDialog->warnReceiverCertificateExpiresSB->value() );
    kdDebug(5006) << "20) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->certificateInChainExpiryNearWarning() ==
             _encryptionPage->encDialog->warnChainCertificateExpiresCB->isChecked() );
    kdDebug(5006) << "21) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->certificateInChainExpiryNearWarningInterval() ==
             _encryptionPage->encDialog->warnChainCertificateExpiresSB->value() );
    kdDebug(5006) << "22) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->saveMessagesEncrypted() ==
             _encryptionPage->encDialog->storeEncryptedCB->isChecked() );
    kdDebug(5006) << "23) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->checkCertificatePath() ==
             _encryptionPage->encDialog->checkCertificatePathCB->isChecked() );
    kdDebug(5006) << "24) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( wrapper->checkEncryptionCertificatePathToRoot() ==
             _encryptionPage->encDialog->alwaysCheckRootRB->isChecked() );
    kdDebug(5006) << "25) RET = " << ret << endl;
    if( !ret )
        return false;

    bool dirServersEqual = true;
    int numDirServers;
    CryptPlugWrapper::DirectoryServer* servers =
        wrapper->directoryServers( &numDirServers );
    dirServersEqual &= ( numDirServers == _dirservicesPage->dirservDialog->x500LV->childCount() );
    kdDebug(5006) << "25.5) dirServersEqual = " << dirServersEqual << endl;
    if( dirServersEqual ) {
        // same number of entries in plugin and dialog
        QListViewItemIterator lvit( _dirservicesPage->dirservDialog->x500LV );
        QListViewItem* current;
        int pos = 0;
        while( ( current = lvit.current() ) && dirServersEqual ) {
            ++lvit;
            dirServersEqual &= ( current->text( 0 ) ==
                                 QString::fromUtf8( servers[pos].servername ) );
            dirServersEqual &= ( current->text( 1 ).toInt() ==
                                 servers[pos].port );
            dirServersEqual &= ( current->text( 2 ) ==
                                 QString::fromUtf8( servers[pos].description ) );

            pos++;
        }
    }
    ret &= dirServersEqual;
    kdDebug(5006) << "26) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( ( ( wrapper->certificateSource() == CertSrc_ServerLocal ) &&
               _dirservicesPage->dirservDialog->firstLocalThenDSCertRB->isChecked() ) ||
             ( ( wrapper->certificateSource() == CertSrc_Local ) &&
               _dirservicesPage->dirservDialog->localOnlyCertRB->isChecked() ) ||
             ( ( wrapper->certificateSource() == CertSrc_Server ) &&
               _dirservicesPage->dirservDialog->dsOnlyCertRB->isChecked() ) );
    kdDebug(5006) << "27) RET = " << ret << endl;
    if( !ret )
        return false;

    ret &= ( ( ( wrapper->crlSource() == CertSrc_ServerLocal ) &&
               _dirservicesPage->dirservDialog->firstLocalThenDSCRLRB->isChecked() ) ||
             ( ( wrapper->crlSource() == CertSrc_Local ) &&
               _dirservicesPage->dirservDialog->localOnlyCRLRB->isChecked() ) ||
             ( ( wrapper->crlSource() == CertSrc_Server ) &&
               _dirservicesPage->dirservDialog->dsOnlyCRLRB->isChecked() ) );
    kdDebug(5006) << "28) RET = " << ret << endl;
    if( !ret )
        return false;

    kdDebug(5006) << "29) RET = " << ret << endl;
    return ret;
}


void PluginPage::savePluginConfig( int pluginno )
{
    if ( mCryptPlugList->isEmpty() )
        return;
    CryptPlugWrapper* wrapper = mCryptPlugList->at( pluginno );
    Q_ASSERT( wrapper );
    if( !wrapper )
        return;

    KConfig* config = kapp->config();
    KConfigGroupSaver saver(config, "General");

    // Set the right config group
    config->setGroup( QString( "CryptPlug #%1" ).arg( pluginno ) );


/*
            int numEntry = _generalPage->plugList->childCount();
            QListViewItem *item = _generalPage->plugList->firstChild();
            int i = 0;
            int curI = -1;
            for (i = 0; i < numEntry; ++i) {
                QString itemTxt( pluginNameToNumberedItem( item->text(0), 1+i ) );
         wrapper = _pluginPage->mCryptPlugList->next(), ++i ) {
        if( ! wrapper->displayName().isEmpty() ) {
            QString item = pluginNameToNumberedItem( wrapper->displayName(), 1+i );
            _pluginPage->_certificatesPage->plugListBoxCertConf->insertItem( item );
            _pluginPage->_signaturePage->plugListBoxSignConf->insertItem(    item );
            _pluginPage->_encryptionPage->plugListBoxEncryptConf->insertItem( item );
            _pluginPage->_dirservicesPage->plugListBoxDirServConf->insertItem( item );

*/



    // The signature tab - everything here needs to be written both
    // into config and into the crypt plug wrapper.

    // Sign Messages group box
    if( _signaturePage->sigDialog->signAllPartsRB->isChecked() ) {
        wrapper->setSignEmail( SignEmail_SignAll );
        config->writeEntry( "SignEmail", SignEmail_SignAll );
    } else if( _signaturePage->sigDialog->askEachPartRB->isChecked() ) {
        wrapper->setSignEmail( SignEmail_Ask );
        config->writeEntry( "SignEmail", SignEmail_Ask );
    } else {
        wrapper->setSignEmail( SignEmail_DontSign );
        config->writeEntry( "SignEmail", SignEmail_DontSign );
    }
    bool warnSendUnsigned = _signaturePage->sigDialog->warnUnsignedCB->isChecked();
    wrapper->setWarnSendUnsigned( warnSendUnsigned );
    config->writeEntry( "WarnSendUnsigned", warnSendUnsigned );

    // Signature Compound Mode group box
    if( _signaturePage->sigDialog->sendSigOpaqueRB->isChecked() ) {
        wrapper->setSignatureCompoundMode( SignatureCompoundMode_Opaque );
        config->writeEntry( "SignatureCompoundMode", SignatureCompoundMode_Opaque );
    } else if( _signaturePage->sigDialog->sendSigMultiPartRB->isChecked() ) {
        wrapper->setSignatureCompoundMode( SignatureCompoundMode_Detached);
        config->writeEntry( "SignatureCompoundMode", SignatureCompoundMode_Detached );
    } else {
        wrapper->setSignatureCompoundMode( SignatureCompoundMode_undef );
        config->writeEntry( "SignatureCompoundMode", SignatureCompoundMode_undef );
    }

    // Sending Certificates group box
    if( _signaturePage->sigDialog->dontSendCertificatesRB->isChecked() ) {
        wrapper->setSendCertificates( SendCert_DontSend );
        config->writeEntry( "SendCerts", SendCert_DontSend );
    } else if( _signaturePage->sigDialog->sendYourOwnCertificateRB->isChecked() ) {
        wrapper->setSendCertificates( SendCert_SendOwn );
        config->writeEntry( "SendCerts", SendCert_SendOwn );
    } else if( _signaturePage->sigDialog->sendChainWithoutRootRB->isChecked() ) {
        wrapper->setSendCertificates( SendCert_SendChainWithoutRoot );
        config->writeEntry( "SendCerts", SendCert_SendChainWithoutRoot );
    } else {
        wrapper->setSendCertificates( SendCert_SendChainWithRoot );
        config->writeEntry( "SendCerts", SendCert_SendChainWithRoot );
    }

    // Signature Settings group box
    QString sigAlgoStr = _signaturePage->sigDialog->signatureAlgorithmCO->currentText();
    SignatureAlgorithm sigAlgo = SignAlg_SHA1;
    if( sigAlgoStr == "RSA + SHA-1" )
        sigAlgo = SignAlg_SHA1;
    else
        kdDebug(5006) << "Unknown signature algorithm " << sigAlgoStr << endl;
    wrapper->setSignatureAlgorithm( sigAlgo );
    config->writeEntry( "SigAlgo", sigAlgo );

    bool warnSigCertExp = _signaturePage->sigDialog->warnSignatureCertificateExpiresCB->isChecked();
    wrapper->setSignatureCertificateExpiryNearWarning( warnSigCertExp );
    config->writeEntry( "SigCertWarnNearExpire", warnSigCertExp );

    int warnSigCertExpInt = _signaturePage->sigDialog->warnSignatureCertificateExpiresSB->value();
    wrapper->setSignatureCertificateExpiryNearInterval( warnSigCertExpInt );
    config->writeEntry( "SigCertWarnNearExpireInt", warnSigCertExpInt );

    bool warnCACertExp = _signaturePage->sigDialog->warnCACertificateExpiresCB->isChecked();
    wrapper->setCACertificateExpiryNearWarning( warnCACertExp );
    config->writeEntry( "CACertWarnNearExpire", warnCACertExp );

    int warnCACertExpInt = _signaturePage->sigDialog->warnCACertificateExpiresSB->value();
    wrapper->setCACertificateExpiryNearInterval( warnCACertExpInt );
    config->writeEntry( "CACertWarnNearExpireInt", warnCACertExpInt );

    bool warnRootCertExp = _signaturePage->sigDialog->warnRootCertificateExpiresCB->isChecked();
    wrapper->setRootCertificateExpiryNearWarning( warnRootCertExp );
    config->writeEntry( "RootCertWarnNearExpire", warnRootCertExp );

    int warnRootCertExpInt = _signaturePage->sigDialog->warnRootCertificateExpiresSB->value();
    wrapper->setRootCertificateExpiryNearInterval( warnRootCertExpInt );
    config->writeEntry( "RootCertWarnNearExpireInt", warnRootCertExpInt );

    bool warnNoCertificate = _signaturePage->sigDialog->warnAddressNotInCertificateCB->isChecked();
    wrapper->setWarnNoCertificate( warnNoCertificate );
    config->writeEntry( "WarnEmailNotInCert", warnNoCertificate );

    // PIN Entry group box
    if( _signaturePage->sigDialog->pinOncePerSessionRB->isChecked() ) {
        wrapper->setNumPINRequests( PinRequest_OncePerSession );
        config->writeEntry( "NumPINRequests", PinRequest_OncePerSession );
    } else if( _signaturePage->sigDialog->pinAlwaysRB->isChecked() ) {
        wrapper->setNumPINRequests( PinRequest_Always );
        config->writeEntry( "NumPINRequests", PinRequest_Always );
    } else if( _signaturePage->sigDialog->pinAddCertificatesRB->isChecked() ) {
        wrapper->setNumPINRequests( PinRequest_WhenAddingCerts );
        config->writeEntry( "NumPINRequests", PinRequest_WhenAddingCerts );
    } else if( _signaturePage->sigDialog->pinAlwaysWhenSigningRB->isChecked() ) {
        wrapper->setNumPINRequests( PinRequest_AlwaysWhenSigning );
        config->writeEntry( "NumPINRequests", PinRequest_AlwaysWhenSigning );
    } else {
        wrapper->setNumPINRequests( PinRequest_AfterMinutes );
        config->writeEntry( "NumPINRequests", PinRequest_AfterMinutes );
    }
    int pinInt = _signaturePage->sigDialog->pinIntervalSB->value();
    wrapper->setNumPINRequestsInterval( pinInt );
    config->writeEntry( "NumPINRequestsInt", pinInt );

    // Save Messages group box
    bool saveSigs = _signaturePage->sigDialog->saveSentSigsCB->isChecked();
    wrapper->setSaveSentSignatures( saveSigs );
    config->writeEntry( "SaveSentSigs", saveSigs );

    // The encryption tab - everything here needs to be written both
    // into config and into the crypt plug wrapper.

    // Encrypt Messages group box
    if( _encryptionPage->encDialog->encryptAllPartsRB->isChecked() ) {
        wrapper->setEncryptEmail( EncryptEmail_EncryptAll );
        config->writeEntry( "EncryptEmail", EncryptEmail_EncryptAll );
    } else if( _encryptionPage->encDialog->askEachPartRB->isChecked() ) {
        wrapper->setEncryptEmail( EncryptEmail_Ask );
        config->writeEntry( "EncryptEmail", EncryptEmail_Ask );
    } else {
        wrapper->setEncryptEmail( EncryptEmail_DontEncrypt );
        config->writeEntry( "EncryptEmail", EncryptEmail_DontEncrypt );
    }
    bool warnSendUnencrypted = _encryptionPage->encDialog->warnUnencryptedCB->isChecked();
    wrapper->setWarnSendUnencrypted( warnSendUnencrypted );
    config->writeEntry( "WarnSendUnencrypted", warnSendUnencrypted );

    // Encryption Settings group box
    QString encAlgoStr = _encryptionPage->encDialog->encryptionAlgorithmCO->currentText();
    EncryptionAlgorithm encAlgo = EncryptAlg_RSA;
    if( encAlgoStr == "RSA" )
        encAlgo = EncryptAlg_RSA;
    else if( encAlgoStr == "Triple-DES" )
        encAlgo = EncryptAlg_TripleDES;
    else if( encAlgoStr == "SHA-1" )
        encAlgo = EncryptAlg_SHA1;
    else
        kdDebug(5006) << "Unknown encryption algorithm " << encAlgoStr << endl;
    wrapper->setEncryptionAlgorithm( encAlgo );
    config->writeEntry( "EncryptAlgo", encAlgo );

    bool recvCertExp = _encryptionPage->encDialog->warnReceiverCertificateExpiresCB->isChecked();
    wrapper->setReceiverCertificateExpiryNearWarning( recvCertExp );
   config->writeEntry( "WarnRecvCertNearExpire", recvCertExp );

    int recvCertExpInt = _encryptionPage->encDialog->warnReceiverCertificateExpiresSB->value();
    wrapper->setReceiverCertificateExpiryNearWarningInterval( recvCertExpInt );
    config->writeEntry( "WarnRecvCertNearExpireInt", recvCertExpInt );

    bool certChainExp = _encryptionPage->encDialog->warnChainCertificateExpiresCB->isChecked();
   wrapper->setCertificateInChainExpiryNearWarning( certChainExp );
    config->writeEntry( "WarnCertInChainNearExpire", certChainExp );

   int certChainExpInt = _encryptionPage->encDialog->warnChainCertificateExpiresSB->value();
    wrapper->setCertificateInChainExpiryNearWarningInterval( certChainExpInt );
    config->writeEntry( "WarnCertInChainNearExpireInt", certChainExpInt );

    bool recvNotInCert = _encryptionPage->encDialog->warnReceiverNotInCertificateCB->isChecked();
    wrapper->setReceiverEmailAddressNotInCertificateWarning( recvNotInCert );
   config->writeEntry( "WarnRecvAddrNotInCert", recvNotInCert );

    // CRL group box
    bool useCRL = _encryptionPage->encDialog->useCRLsCB->isChecked();
    wrapper->setEncryptionUseCRLs( useCRL );
    config->writeEntry( "EncryptUseCRLs", useCRL );

    bool warnCRLExp = _encryptionPage->encDialog->warnCRLExpireCB->isChecked();
    wrapper->setEncryptionCRLExpiryNearWarning( warnCRLExp );
    config->writeEntry( "EncryptCRLWarnNearExpire", warnCRLExp );

    int warnCRLExpInt = _encryptionPage->encDialog->warnCRLExpireSB->value();
    wrapper->setEncryptionCRLNearExpiryInterval( warnCRLExpInt );
    config->writeEntry( "EncryptCRLWarnNearExpireInt", warnCRLExpInt );

    // Save Messages group box
    bool saveEnc = _encryptionPage->encDialog->storeEncryptedCB->isChecked();
    wrapper->setSaveMessagesEncrypted( saveEnc );
    config->writeEntry( "SaveMsgsEncrypted", saveEnc );

    // Certificate Path Check group box
    bool checkPath = _encryptionPage->encDialog->checkCertificatePathCB->isChecked();
    wrapper->setCheckCertificatePath( checkPath );
    config->writeEntry( "CheckCertPath", checkPath );

    bool checkToRoot = _encryptionPage->encDialog->alwaysCheckRootRB->isChecked();
    wrapper->setCheckEncryptionCertificatePathToRoot( checkToRoot );
    config->writeEntry( "CheckEncryptCertToRoot", checkToRoot );

    // The directory services tab - everything here needs to be written both
    // into config and into the crypt plug wrapper.

    uint numDirServers = _dirservicesPage->dirservDialog->x500LV->childCount();
    CryptPlugWrapper::DirectoryServer* servers = new CryptPlugWrapper::DirectoryServer[numDirServers];
    config->writeEntry( "NumDirServers", numDirServers );
    QListViewItemIterator lvit( _dirservicesPage->dirservDialog->x500LV );
    QListViewItem* current;
    int pos = 0;
    while( ( current = lvit.current() ) ) {
        ++lvit;
        const char* servername = current->text( 0 ).utf8();
        int port = current->text( 1 ).toInt();
        const char* description = current->text( 2 ).utf8();
        servers[pos].servername = new char[strlen( servername )+1];
        strcpy( servers[pos].servername, servername );
        servers[pos].port = port;
        servers[pos].description = new char[strlen( description)+1];
        strcpy( servers[pos].description, description );
        config->writeEntry( QString( "DirServer%1Name" ).arg( pos ),
                            current->text( 0 ) );
        config->writeEntry( QString( "DirServer%1Port" ).arg( pos ),
                            port );
        config->writeEntry( QString( "DirServer%1Descr" ).arg( pos ),
                            current->text( 2 ) );
        pos++;
    }
    wrapper->setDirectoryServers( servers, numDirServers );
    for( uint i = 0; i < numDirServers; i++ ) {
        delete[] servers[i].servername;
        delete[] servers[i].description;
    }
    delete[] servers;

    // Local/Remote Certificates group box
    if( _dirservicesPage->dirservDialog->firstLocalThenDSCertRB->isChecked() ) {
        wrapper->setCertificateSource( CertSrc_ServerLocal );
        config->writeEntry( "CertSource", CertSrc_ServerLocal );
    } else if( _dirservicesPage->dirservDialog->localOnlyCertRB->isChecked() ) {
        wrapper->setCertificateSource( CertSrc_Local );
        config->writeEntry( "CertSource", CertSrc_Local );
    } else {
        wrapper->setCertificateSource( CertSrc_Server );
        config->writeEntry( "CertSource", CertSrc_Server );
    }

    // Local/Remote CRLs group box
    if( _dirservicesPage->dirservDialog->firstLocalThenDSCRLRB->isChecked() ) {
        wrapper->setCRLSource( CertSrc_ServerLocal );
        config->writeEntry( "CRLSource", CertSrc_ServerLocal );
    } else if( _dirservicesPage->dirservDialog->localOnlyCRLRB->isChecked() ) {
        wrapper->setCRLSource( CertSrc_Local );
        config->writeEntry( "CRLSource", CertSrc_Local );
    } else {
        wrapper->setCRLSource( CertSrc_Server );
        config->writeEntry( "CRLSource", CertSrc_Server );
    }
}


void PluginPage::slotPlugListBoxConfigurationChanged( int item )
{
    if( -1 < item ) {
        QListViewItem* lvi = _generalPage->plugList->firstChild();
        for (int i = 0; i < item; ++i)
            lvi = lvi->nextSibling();
        _generalPage->plugList->setCurrentItem( lvi );
        _generalPage->plugList->setSelected( lvi, true );
    }
}




GeneralPage::GeneralPage( PluginPage* parent, const char* name ) :
    ConfigurationPage( parent, name ),
    _pluginPage( parent ),
    currentPlugItem(0)
{
  QVBoxLayout *vlay = new QVBoxLayout( this, KDialog::marginHint(),
                                       KDialog::spacingHint() );

  // "custom header fields" listbox:
  QGridLayout *glay = new QGridLayout( vlay, 9, 3 ); // inherits spacing
  glay->setRowStretch( 2, 1 );
  glay->setRowStretch( 5, 1 );
  glay->setColStretch( 1, 1 );
  plugList = new ListView( this, "plugList" );
  plugList->addColumn( i18n("Name") );
  plugList->addColumn( i18n("Location") );
  plugList->addColumn( i18n("Update URL") );
  plugList->addColumn( i18n("Active") );
  plugList->addColumn( i18n("Initialized" ) );
  plugList->setAllColumnsShowFocus( true );
  plugList->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  plugList->setSorting( -1 );
  plugList->header()->setClickEnabled( false );
  connect( plugList, SIGNAL(selectionChanged()),
           _pluginPage, SLOT(  slotPlugSelectionChanged()) );
  glay->addMultiCellWidget( plugList, 0, 5, 0, 1 );

  addCryptPlugButton       = new QPushButton( i18n("Add &New Plugin"), this );
  removeCryptPlugButton    = new QPushButton( i18n("&Remove Current"), this );
  activateCryptPlugButton  = new QPushButton( i18n("Ac&tivate"),       this );
  connect( addCryptPlugButton,       SIGNAL(clicked()),
                                          this, SLOT(  slotNewPlugIn()) );
  connect( removeCryptPlugButton,    SIGNAL(clicked()),
                                          this, SLOT(  slotDeletePlugIn()) );
  connect( activateCryptPlugButton,  SIGNAL(clicked()),
                                          this, SLOT(  slotActivatePlugIn()) );
  addCryptPlugButton->setAutoDefault(       false );
  removeCryptPlugButton->setAutoDefault(    false );
  activateCryptPlugButton->setAutoDefault(  false );
  glay->addWidget( addCryptPlugButton,       0, 2 );
  glay->addWidget( removeCryptPlugButton,    1, 2 );
  glay->addWidget( activateCryptPlugButton,  3, 2 );

  // "name" and "location" and "Update URL" line edits and labels:
  plugNameEdit = new QLineEdit( this );
  plugNameEdit->setEnabled( false );
  QLabel* plugNameLabel = new QLabel( plugNameEdit, i18n("Na&me:"), this );
  glay->addWidget( plugNameLabel, 6, 0 );
  glay->addWidget( plugNameEdit,  6, 1 );
  connect( plugNameEdit, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotPlugNameChanged(const QString&)) );

  QLabel* plugLocationLabel = new QLabel( plugNameEdit,
                                          i18n("&Location:"), this );
  glay->addWidget( plugLocationLabel, 7, 0 );
  plugLocationRequester = new KURLRequester( this );
  plugLocationRequester->setEnabled( false );
  glay->addWidget( plugLocationRequester, 7, 1 );
  connect( plugLocationRequester, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotPlugLocationChanged(const QString&)) );

  plugUpdateURLEdit = new QLineEdit( this );
  plugUpdateURLEdit->setEnabled( false );
  QLabel* plugUpdateURLLabel = new QLabel( plugNameEdit, i18n("&Update URL:"), this );
  glay->addWidget( plugUpdateURLLabel, 8, 0 );
  glay->addWidget( plugUpdateURLEdit,  8, 1 );
  connect( plugUpdateURLEdit, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotPlugUpdateURLChanged(const QString&)) );
}

void GeneralPage::dismiss()
{
    QListViewItem *tmp;
    while ( currentPlugItem &&
            currentPlugItem->text(0).isEmpty() &&
            currentPlugItem->text(1).isEmpty() &&
            currentPlugItem->text(2).isEmpty() ) {
        tmp = currentPlugItem;
        if ( currentPlugItem->itemBelow() )
            currentPlugItem = currentPlugItem->itemBelow();
        else if ( currentPlugItem->itemAbove() )
            currentPlugItem = currentPlugItem->itemAbove();
        else
            currentPlugItem = 0;
        delete tmp;
    }
    if ( currentPlugItem == 0 ) {
        plugNameEdit->setEnabled( false );
        plugLocationRequester->setEnabled( false );
        plugUpdateURLEdit->setEnabled( false );
    }
}

void GeneralPage::setup()
{
    KConfig *config = kapp->config();
    KConfigGroupSaver saver(config, "General");
    QListViewItem *top = 0;
    plugList->clear();
    _pluginPage->_certificatesPage->plugListBoxCertConf->clear();
    _pluginPage->_signaturePage->plugListBoxSignConf->clear();
    _pluginPage->_encryptionPage->plugListBoxEncryptConf->clear();
    _pluginPage->_dirservicesPage->plugListBoxDirServConf->clear();

    int i = 0;
    for( CryptPlugWrapper* wrapper = _pluginPage->mCryptPlugList->first(); wrapper;
         wrapper = _pluginPage->mCryptPlugList->next(), ++i ) {
        if( ! wrapper->displayName().isEmpty() ) {
            QString item = pluginNameToNumberedItem( wrapper->displayName(), 1+i );
            _pluginPage->_certificatesPage->plugListBoxCertConf->insertItem( item );
            _pluginPage->_signaturePage->plugListBoxSignConf->insertItem(    item );
            _pluginPage->_encryptionPage->plugListBoxEncryptConf->insertItem( item );
            _pluginPage->_dirservicesPage->plugListBoxDirServConf->insertItem( item );
            top = new QListViewItem( plugList, top,
                                     wrapper->displayName(),
                                     wrapper->libName(),
                                     wrapper->updateURL(),
                                     wrapper->active() ? "*" : "",
                                     ( wrapper->initStatus( 0 ) == CryptPlugWrapper::InitStatus_Ok ) ? "*" : "" );
        }
    }

    if( _pluginPage->mCryptPlugList->count() )
        _pluginPage->_certificatesPage->startCertManagerPB->setEnabled( true );
    else
        _pluginPage->_certificatesPage->startCertManagerPB->setEnabled( false );

    if( 0 < plugList->childCount() ) {
        plugList->setCurrentItem( plugList->firstChild());
        plugList->setSelected(   plugList->firstChild(), TRUE);
    }

    _pluginPage->slotPlugSelectionChanged();
}


void GeneralPage::apply()
{
    // Save the "General" tab
    savePluginsConfig( true );

    // Find the number of the current plugin.
    int currentPlugin = _pluginPage->_signaturePage->plugListBoxSignConf->currentItem();
    Q_ASSERT( currentPlugin == _pluginPage->_certificatesPage->plugListBoxCertConf->currentItem() );
    Q_ASSERT( currentPlugin == _pluginPage->_encryptionPage->plugListBoxEncryptConf->currentItem() );
    Q_ASSERT( currentPlugin == _pluginPage->_dirservicesPage->plugListBoxDirServConf->currentItem() );

    _pluginPage->savePluginConfig( currentPlugin );
}


void GeneralPage::savePluginsConfig( bool silent )
{
    // The "General" tab (lists the plugins)
    int numValidEntry = 0;
    int numEntry = plugList->childCount();
    QListViewItem *item = plugList->firstChild();
    KConfig* config = kapp->config();
    for (int i = 0; i < numEntry; ++i) {
        KConfigGroupSaver saver(config,
                                QString("CryptPlug #%1").arg( numValidEntry ));
        if( ! item->text(0).isEmpty() ) {
            bool isActive = 0 < item->text(3).length();
            config->writeEntry( "name",     item->text(0) );
            config->writeEntry( "location", item->text(1) );
            config->writeEntry( "updates",  item->text(2) );
            config->writeEntry( "active",   isActive ? "1" : "0" );
            if( _pluginPage && _pluginPage->mCryptPlugList ) {
                CryptPlugWrapper* wrapper = _pluginPage->mCryptPlugList->at( numValidEntry );
                if( wrapper ) {
                    wrapper->setDisplayName( item->text(0) );
                    if( wrapper->libName() != item->text(1) ) {
                        wrapper->deinitialize();
                        wrapper->setLibName( item->text(1) );
                        CryptPlugWrapper::InitStatus status;
                        QString errorMsg;
                        wrapper->initialize(&status, &errorMsg);
                        if( CryptPlugWrapper::InitStatus_Ok != status ) {
                            if( !silent )
                                _pluginPage->mCryptPlugList->showPluginInitError( wrapper, status, errorMsg );
                            currentPlugItem->setText(4, "");
                        } else {
                            currentPlugItem->setText(4, "*");
                        }
                    }
                    wrapper->setUpdateURL( item->text(2) );
                    wrapper->setActive( isActive ? true : false );
                }
            }
            numValidEntry++;
        }
        item = item->nextSibling();
    }
    config->setGroup( "General" );
    config->writeEntry("crypt-plug-count", numValidEntry );
}


void GeneralPage::installProfile( KConfig* /*profile*/ )
{
    // PENDING(kalle) Implement this
    qDebug( "GeneralPage::installProfile() not implemented" );
}


void GeneralPage::slotPlugNameChanged( const QString &text )
{
  if( !currentPlugItem )
    currentPlugItem = plugList->lastItem();
  if( currentPlugItem )
    currentPlugItem->setText(0, text );
}

void GeneralPage::slotPlugLocationChanged( const QString &text )
{
  if( !currentPlugItem )
    currentPlugItem = plugList->lastItem();
  if( currentPlugItem )
    currentPlugItem->setText(1, text );
}

void GeneralPage::slotPlugUpdateURLChanged( const QString &text )
{
  if( !currentPlugItem )
    currentPlugItem = plugList->lastItem();
  if( currentPlugItem )
    currentPlugItem->setText(2, text );
}

void GeneralPage::slotNewPlugIn( void )
{
    CryptPlugWrapper* newWrapper = new CryptPlugWrapper( this, "", "", "" );
    _pluginPage->mCryptPlugList->append( newWrapper );

    QListViewItem *listItem = new QListViewItem( plugList,
                                                 plugList->lastItem(),
                                                 "", "", "", "" );
    plugList->setCurrentItem( listItem );
    plugList->setSelected( listItem, true );
    currentPlugItem = plugList->selectedItem();

    if( currentPlugItem != 0 ) {
        plugNameEdit->setEnabled( true);
        plugLocationRequester->setEnabled(  true);
        plugUpdateURLEdit->setEnabled( true);
        plugNameEdit->setFocus();
    }
}

void GeneralPage::slotDeletePlugIn( void )
{
    // PENDING(kalle) Delete from mCryptPlugList as well.
    if( currentPlugItem != 0 )
  {
    QListViewItem *next = currentPlugItem->itemAbove();
    if( next == 0 )
    {
      next = currentPlugItem->itemBelow();
    }

    plugNameEdit->clear();
    plugLocationRequester->clear();
    plugUpdateURLEdit->clear();
    plugNameEdit->setEnabled(      false);
    plugLocationRequester->setEnabled(  false);
    plugUpdateURLEdit->setEnabled( false);

    plugList->takeItem( currentPlugItem );
    currentPlugItem = 0;

    if( next != 0 )
    {
     plugList->setSelected( next, true );
    }
  }
}

void GeneralPage::slotActivatePlugIn( void )
{
    if ( !currentPlugItem)
        return;
    // find out whether the plug-in is to be activated or de-activated
    bool activate = (currentPlugItem->text( 3 ) == "");
    // (De)activate this plug-in
    // and deactivate all other plugins if necessarry
    QListViewItemIterator lvit( plugList );
    QListViewItem* current;
    int pos = 0;
    while( ( current = lvit.current() ) )
    {
        CryptPlugWrapper* plug = _pluginPage->mCryptPlugList->at( pos );
        if( plug ) {
            if(  current == currentPlugItem )
            {
                // This is the one the user wants to (de)activate
                plug->setActive( activate );
                current->setText( 3, activate ? "*" : "" );
            }
            else
            {
                // This is one of the other entries
                plug->setActive( false );
                current->setText( 3, "" );
            }
        }
        ++lvit;
        ++pos;
    }
    if( activate )
        activateCryptPlugButton->setText( i18n("Deac&tivate")  );
    else
        activateCryptPlugButton->setText( i18n("Ac&tivate") );
}


CertificatesPage::CertificatesPage( PluginPage* parent,
                                    const char* name ) :
    ConfigurationPage( parent, name ),
    _pluginPage( parent )
{
  QVBoxLayout* vlay = new QVBoxLayout( this, KDialog::spacingHint() );
  QHBoxLayout *hlay = new QHBoxLayout( vlay );
  plugListBoxCertConf = new QComboBox( this, "plugListBoxCertConf" );
  hlay->addWidget( new QLabel( plugListBoxCertConf, i18n("Select &plugin:"), this ), 0, AlignVCenter );
  hlay->addWidget( plugListBoxCertConf, 2 );
  KSeparator *hline = new KSeparator( KSeparator::HLine, this);
  vlay->addWidget( hline );


#ifdef CERTIFICATE_HANDLING_IN_KMAIL
  certDialog = new CertificateHandlingDialogImpl( this, "CertificateHandlingDialogImpl" );
  if ( certDialog ) {
    vlay->addWidget( certDialog );
    vlay->addStretch(10);
  }
#else
  certDialog = 0;

  startCertManagerPB = new QPushButton( i18n( "&Start Certificate Manager" ), this, "startcertmanagerpb" );
  connect( startCertManagerPB, SIGNAL( clicked() ),
           this, SLOT( slotStartCertManager() ) );
  startCertManagerPB->setFixedSize( startCertManagerPB->sizeHint() );
  vlay->addWidget( startCertManagerPB, 0, AlignCenter | AlignVCenter );
#endif
}


void CertificatesPage::slotStartCertManager()
{
	if ( plugListBoxCertConf->count() && (0 <= plugListBoxCertConf->currentItem()) )
	{
	    KProcess certManagerProc; // save to create on the heap, since
                              // there is no parent
    	certManagerProc << "kgpgcertmanager";
    	certManagerProc << _pluginPage->mCryptPlugList->at( plugListBoxCertConf->currentItem() )->displayName();
    	certManagerProc << _pluginPage->mCryptPlugList->at( plugListBoxCertConf->currentItem() )->libName();

	    if( !certManagerProc.start( KProcess::DontCare ) )
    	    KMessageBox::error( this, i18n( "Could not start certificate manager. Please check your installation!" ),
        	                    i18n( "KMail Error" ) );
        else
            kdDebug(5006) << "\nCertificatesPage::slotStartCertManager(): certificate manager started.\n" << endl;
	}
    // process continues to run even after the KProcess object goes
    // out of scope here, since it is started in DontCare run mode.
}


void CertificatesPage::setup()
{
    // no need to call anything here; the CryptPlugWrapperList is
    // always uptodate
}


void CertificatesPage::apply()
{
    // nothing to do here, GeneralPage::apply() has already called
    // savePluginConfig()
}


void CertificatesPage::installProfile( KConfig* /*profile*/ )
{
    // PENDING(kalle) Implement this
    qDebug( "PluginPage::CertificatesPage::installProfile() not implemented" );
}




SignaturePage::SignaturePage( PluginPage* parent,
                                          const char* name ) :
    ConfigurationPage( parent, name ),
    _pluginPage( parent )
{
  QVBoxLayout* vlay = new QVBoxLayout( this, KDialog::spacingHint() );
  QHBoxLayout *hlay = new QHBoxLayout( vlay );
  plugListBoxSignConf = new QComboBox( this, "plugListBoxSignConf" );
  hlay->addWidget( new QLabel( plugListBoxSignConf, i18n("Select &plugin:"), this ), 0, AlignVCenter );
  hlay->addWidget( plugListBoxSignConf, 2 );
  connect( plugListBoxSignConf, SIGNAL( activated( int ) ),
    _pluginPage, SLOT( slotPlugListBoxConfigurationChanged( int ) ) );
  KSeparator *hline = new KSeparator( KSeparator::HLine, this);
  vlay->addWidget( hline );
  sigDialog = new SignatureConfigurationDialogImpl( this, "SignatureConfigurationDialogLayout" );
  if ( sigDialog ) {
    vlay->addWidget( sigDialog );
    vlay->addStretch(10);
  }
}


void SignaturePage::setup()
{
    // no need to call anything here; the CryptPlugWrapperList is
    // always uptodate
}


void SignaturePage::apply()
{
    // nothing to do here, GeneralPage::apply() has already called
    // savePluginConfig()
}


void SignaturePage::installProfile( KConfig* /*profile*/ )
{
    // PENDING(kalle) Implement this
    qDebug( "PluginPage::SignaturePage::installProfile() not implemented" );
}


EncryptionPage::EncryptionPage( PluginPage* parent,
                                            const char* name ) :
    ConfigurationPage( parent, name ),
    _pluginPage( parent )
{
  QVBoxLayout* vlay = new QVBoxLayout( this, KDialog::spacingHint() );
  QHBoxLayout* hlay = new QHBoxLayout( vlay );
  plugListBoxEncryptConf = new QComboBox( this, "plugListBoxEncryptConf" );
  hlay->addWidget( new QLabel( plugListBoxEncryptConf, i18n("Select &plugin:"), this ), 0, AlignVCenter );
  hlay->addWidget( plugListBoxEncryptConf, 2 );
  connect( plugListBoxEncryptConf, SIGNAL( activated( int ) ),
    _pluginPage, SLOT( slotPlugListBoxConfigurationChanged( int ) ) );
  KSeparator* hline = new KSeparator( KSeparator::HLine, this);
  vlay->addWidget( hline );
  encDialog = new EncryptionConfigurationDialogImpl( this, "EncryptionConfigurationDialog" );
  if ( encDialog ) {
    vlay->addWidget( encDialog );
    vlay->addStretch(10);
  }
}


void EncryptionPage::setup()
{
    // no need to call anything here; the CryptPlugWrapperList is
    // always uptodate
}


void EncryptionPage::apply()
{
    // nothing to do here, GeneralPage::apply() has already called
    // savePluginConfig()
}



void EncryptionPage::installProfile( KConfig* /*profile*/ )
{
    // PENDING(kalle) Implement this
    qDebug( "EncryptionPage::installProfile() not implemented" );
}


DirServicesPage::DirServicesPage( PluginPage* parent,
                                              const char* name ) :
    ConfigurationPage( parent, name ),
    _pluginPage( parent )
{
  QVBoxLayout* vlay = new QVBoxLayout( this, KDialog::spacingHint() );
  QHBoxLayout* hlay = new QHBoxLayout( vlay );
  plugListBoxDirServConf = new QComboBox( this, "plugListBoxDirServConf" );
  hlay->addWidget( new QLabel( plugListBoxDirServConf, i18n("Select &plugin:"), this ), 0, AlignVCenter );
  hlay->addWidget( plugListBoxDirServConf, 2 );
  connect( plugListBoxDirServConf, SIGNAL( activated( int ) ),
    _pluginPage, SLOT( slotPlugListBoxConfigurationChanged( int ) ) );
  KSeparator* hline = new KSeparator( KSeparator::HLine, this);
  vlay->addWidget( hline );
  dirservDialog = new DirectoryServicesConfigurationDialogImpl( this, "DirectoryServicesConfigurationDialog" );
  if ( dirservDialog ) {
    vlay->addWidget( dirservDialog );
    vlay->addStretch(10);
  }
}


void DirServicesPage::setup()
{
    // no need to call anything here; the CryptPlugWrapperList is
    // always uptodate
}


void DirServicesPage::apply()
{
    // nothing to do here, GeneralPage::apply() has already called
    // savePluginConfig()
}



void DirServicesPage::installProfile( KConfig* /*profile*/ )
{
    // PENDING(kalle) Implement this
    qDebug( "DirServicesPage::installProfile() not implemented" );
}


//------------
#include "configuredialog.moc"
