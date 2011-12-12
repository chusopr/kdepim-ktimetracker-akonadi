/*
  Filter Dialog
  Author: Marc Mutz <Marc@Mutz.com>
  based upon work by Stefan Taferner <taferner@kde.org>

  Copyright (c) 2011 Laurent Montel <montel@kde.org>


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
*/

#include "kmfilterdialog.h"


// other KMail headers:
#include "searchpatternedit.h"
#include "filteractionwidget.h"
#include "filterimporterexporter.h"
#include "filtermanager.h"
#include "mailutil.h"
#include "mailkernel.h"
using MailCommon::FilterImporterExporter;

// KDEPIMLIBS headers
#include <Akonadi/AgentType>
#include <Akonadi/AgentInstance>

// other KDE headers:
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kiconloader.h>
#include <kwindowsystem.h>
#include <kicondialog.h>
#include <kkeysequencewidget.h>
#include <kpushbutton.h>
#include <kconfiggroup.h>
#include <ktabwidget.h>
#include <klistwidgetsearchline.h>

// Qt headers:
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QRadioButton>
#include <QApplication>


// other headers:
#include <assert.h>

using namespace MailCommon;
namespace MailCommon {


AccountList::AccountList( QWidget *parent )
  : QTreeWidget( parent )
{
    setObjectName( "accountList" );
    setColumnCount( 2 );
    QStringList headerNames;
    headerNames << i18n("Account Name") << i18n("Type");
    setHeaderItem( new QTreeWidgetItem( headerNames ) );
    setAllColumnsShowFocus( true );
    setFrameStyle( QFrame::WinPanel + QFrame::Sunken );
    setSortingEnabled( false );
    setRootIsDecorated( false );
}

AccountList::~AccountList()
{
}

void AccountList::updateAccountList(MailCommon::MailFilter *filter)
{
  clear();

  QTreeWidgetItem *top = 0;
  // Block the signals here, otherwise we end up calling
  // slotApplicableAccountsChanged(), which will read the incomplete item
  // state and write that back to the filter
  blockSignals( true );
  const Akonadi::AgentInstance::List lst = MailCommon::Util::agentInstances();
  const int nbAccount = lst.count();
  for ( int i = 0; i <nbAccount; ++i ) {
    const Akonadi::AgentInstance agent = lst.at( i );
    QTreeWidgetItem *listItem = new QTreeWidgetItem( this, top );
    listItem->setText( 0, agent.name() );
    listItem->setText( 1, agent.type().name() );
    listItem->setText( 2, agent.identifier() );
    if ( filter )
      listItem->setCheckState( 0, filter->applyOnAccount( agent.identifier() ) ?
                                  Qt::Checked : Qt::Unchecked );
    top = listItem;
  }
  blockSignals( false );

  // make sure our hidden column is really hidden (Qt tends to re-show it)
  hideColumn( 2 );
  resizeColumnToContents( 0 );
  resizeColumnToContents( 1 );

  top = topLevelItem( 0 );
  if ( top ) {
    setCurrentItem( top );
  }
}

void AccountList::applyOnAccount(MailCommon::MailFilter *filter)
{
  QTreeWidgetItemIterator it( this );

  while( QTreeWidgetItem *item = *it ) {
    const QString id = item->text( 2 );
    filter->setApplyOnAccount( id, item->checkState( 0 ) == Qt::Checked );
    ++it;
  }
}

void AccountList::applyOnAccount(const QStringList & lstAccount)
{
  clear();

  QTreeWidgetItem *top = 0;
  // Block the signals here, otherwise we end up calling
  // slotApplicableAccountsChanged(), which will read the incomplete item
  // state and write that back to the filter
  blockSignals( true );
  const Akonadi::AgentInstance::List lst = MailCommon::Util::agentInstances();
  const int nbAccount = lst.count();
  for ( int i = 0; i <nbAccount; ++i ) {
    const Akonadi::AgentInstance agent = lst.at( i );
    QTreeWidgetItem *listItem = new QTreeWidgetItem( this, top );
    listItem->setText( 0, agent.name() );
    listItem->setText( 1, agent.type().name() );
    listItem->setText( 2, agent.identifier() );
    listItem->setCheckState( 0, lstAccount.contains( agent.identifier() ) ?
                             Qt::Checked : Qt::Unchecked );
    top = listItem;
  }
  blockSignals( false );

  // make sure our hidden column is really hidden (Qt tends to re-show it)
  hideColumn( 2 );
  resizeColumnToContents( 0 );
  resizeColumnToContents( 1 );

  top = topLevelItem( 0 );
  if ( top ) {
    setCurrentItem( top );
  }
}


QStringList AccountList::selectedAccount()
{
  QStringList lstAccount;
  QTreeWidgetItemIterator it( this );

  while( QTreeWidgetItem *item = *it ) {
    if ( item->checkState( 0 ) == Qt::Checked )
      lstAccount<<item->text( 2 );
    ++it;
  }
  return lstAccount;
}


QListWidgetFilterItem::QListWidgetFilterItem( const QString & text, QListWidget * parent )
  : QListWidgetItem( text, parent ), mFilter( 0 )
{
}

QListWidgetFilterItem::~QListWidgetFilterItem()
{
  delete mFilter;
}

void QListWidgetFilterItem::setFilter( MailCommon::MailFilter *filter )
{
  mFilter = filter;
  setCheckState(  filter->isEnabled() ? Qt::Checked :  Qt::Unchecked );
}

MailCommon::MailFilter* QListWidgetFilterItem::filter()
{
  return mFilter;
}


// What's this help texts
const char * _wt_filterlist =
I18N_NOOP( "<qt><p>This is the list of defined filters. "
	   "They are processed top-to-bottom.</p>"
	   "<p>Click on any filter to edit it "
	   "using the controls in the right-hand half "
	   "of the dialog.</p></qt>" );
const char * _wt_filterlist_new =
I18N_NOOP( "<qt><p>Click this button to create a new filter.</p>"
	   "<p>The filter will be inserted just before the currently-"
	   "selected one, but you can always change that "
	   "later on.</p>"
	   "<p>If you have clicked this button accidentally, you can undo this "
	   "by clicking on the <em>Delete</em> button.</p></qt>" );
const char * _wt_filterlist_copy =
I18N_NOOP( "<qt><p>Click this button to copy a filter.</p>"
	   "<p>If you have clicked this button accidentally, you can undo this "
	   "by clicking on the <em>Delete</em> button.</p></qt>" );
const char * _wt_filterlist_delete =
I18N_NOOP( "<qt><p>Click this button to <em>delete</em> the currently-"
	   "selected filter from the list above.</p>"
	   "<p>There is no way to get the filter back once "
	   "it is deleted, but you can always leave the "
	   "dialog by clicking <em>Cancel</em> to discard the "
	   "changes made.</p></qt>" );
const char * _wt_filterlist_up =
I18N_NOOP( "<qt><p>Click this button to move the currently-"
	   "selected filter <em>up</em> one in the list above.</p>"
	   "<p>This is useful since the order of the filters in the list "
	   "determines the order in which they are tried on messages: "
	   "The topmost filter gets tried first.</p>"
	   "<p>If you have clicked this button accidentally, you can undo this "
	   "by clicking on the <em>Down</em> button.</p></qt>" );
const char * _wt_filterlist_down =
I18N_NOOP( "<qt><p>Click this button to move the currently-"
	   "selected filter <em>down</em> one in the list above.</p>"
	   "<p>This is useful since the order of the filters in the list "
	   "determines the order in which they are tried on messages: "
	   "The topmost filter gets tried first.</p>"
	   "<p>If you have clicked this button accidentally, you can undo this "
	   "by clicking on the <em>Up</em> button.</p></qt>" );

const char * _wt_filterlist_top =
I18N_NOOP( "<qt><p>Click this button to move the currently-"
	   "selected filter to top of list.</p>"
	   "<p>This is useful since the order of the filters in the list "
	   "determines the order in which they are tried on messages: "
	   "The topmost filter gets tried first.</p></qt>" );

const char * _wt_filterlist_bottom =
I18N_NOOP( "<qt><p>Click this button to move the currently-"
	   "selected filter to bottom of list.</p>"
	   "<p>This is useful since the order of the filters in the list "
	   "determines the order in which they are tried on messages: "
	   "The topmost filter gets tried first.</p></qt>" );


const char * _wt_filterlist_rename =
I18N_NOOP( "<qt><p>Click this button to rename the currently-selected filter.</p>"
	   "<p>Filters are named automatically, as long as they start with "
	   "\"&lt;\".</p>"
	   "<p>If you have renamed a filter accidentally and want automatic "
	   "naming back, click this button and select <em>Clear</em> followed "
	   "by <em>OK</em> in the appearing dialog.</p></qt>" );
const char * _wt_filterdlg_showLater =
I18N_NOOP( "<qt><p>Check this button to force the confirmation dialog to be "
	   "displayed.</p><p>This is useful if you have defined a ruleset that tags "
           "messages to be downloaded later. Without the possibility to force "
           "the dialog popup, these messages could never be downloaded if no "
           "other large messages were waiting on the server, or if you wanted to "
           "change the ruleset to tag the messages differently.</p></qt>" );

//=============================================================================
//
// class KMFilterDialog (the filter dialog)
//
//=============================================================================

KMFilterDialog::KMFilterDialog(const QList<KActionCollection*>& actionCollection, QWidget* parent, bool createDummyFilter )
  : KDialog( parent ),
  mDoNotClose( false ),
  mIgnoreFilterUpdates( true )
{
  setCaption( i18n("Filter Rules") );
  setButtons( Help|Ok|Apply|Cancel|User1|User2 );
  setModal( false );
  setButtonFocus( Ok );
  KWindowSystem::setIcons( winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop),IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small),IconSize(KIconLoader::Small)) );
  setHelp( "filters", "kmail" );
  setButtonText( User1, i18n("Import...") );
  setButtonText( User2, i18n("Export...") );
  connect( this, SIGNAL(user1Clicked()),
           this, SLOT(slotImportFilters()) );
  connect( this, SIGNAL(user2Clicked()),
           this, SLOT(slotExportFilters()) );
  enableButtonApply( false );

  QWidget *w = new QWidget( this );
  setMainWidget( w );
  QHBoxLayout *topLayout = new QHBoxLayout( w );
  topLayout->setObjectName( "topLayout" );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );
  QHBoxLayout *hbl = topLayout;
  QVBoxLayout *vbl2 = 0;
  QWidget *page1 = 0;
  QWidget *page2 = 0;

  mFilterList = new KMFilterListBox( i18n("Available Filters"), w );
  topLayout->addWidget( mFilterList, 1 /*stretch*/ );

  KTabWidget *tabWidget = new KTabWidget( w );
  tabWidget->setObjectName( "kmfd_tab" );
  topLayout->addWidget( tabWidget );

  page1 = new QWidget( tabWidget );
  tabWidget->addTab( page1, i18nc("General mail filter settings.", "General") );
  hbl = new QHBoxLayout( page1 );
  hbl->setObjectName( "kmfd_hbl" );
  hbl->setSpacing( spacingHint() );
  hbl->setMargin( marginHint() );

  page2 = new QWidget( tabWidget );
  tabWidget->addTab( page2, i18nc("Advanced mail filter settings.","Advanced") );
  vbl2 = new QVBoxLayout( page2 );
  vbl2->setObjectName( "kmfd_vbl2" );
  vbl2->setSpacing( spacingHint() );
  vbl2->setMargin( marginHint() );

  QVBoxLayout *vbl = new QVBoxLayout();
  hbl->addLayout( vbl );
  vbl->setObjectName( "kmfd_vbl" );
  vbl->setSpacing( spacingHint() );
  hbl->setStretchFactor( vbl, 2 );

  QGroupBox *patternGroupBox = new QGroupBox( i18n("Filter Criteria"), page1 );
  QHBoxLayout *layout = new QHBoxLayout( patternGroupBox );
  layout->setContentsMargins( 0, 0, 0, 0 );
  mPatternEdit = new MailCommon::SearchPatternEdit(patternGroupBox);
  layout->addWidget( mPatternEdit );

  vbl->addWidget( patternGroupBox, 0, Qt::AlignTop );

  QGroupBox *agb = new QGroupBox( i18n("Filter Actions"), page1 );
  QHBoxLayout *layout2 = new QHBoxLayout;
  mActionLister = new MailCommon::FilterActionWidgetLister( agb );
  layout2->addWidget( mActionLister );
  agb->setLayout( layout2 );
  vbl->addWidget( agb, 0, Qt::AlignTop );

  mAdvOptsGroup = new QGroupBox (i18n("Advanced Options"), page2);
  {
    QGridLayout *gl = new QGridLayout();
    QVBoxLayout *vbl3 = new QVBoxLayout();
    gl->addLayout( vbl3, 0, 0 );
    vbl3->setObjectName( "vbl3" );
    vbl3->setSpacing( spacingHint() );
    vbl3->addStretch( 1 );
    mApplyOnIn = new QCheckBox( i18n("Apply this filter to incoming messages:"), mAdvOptsGroup );
    vbl3->addWidget( mApplyOnIn );
    QButtonGroup *bg = new QButtonGroup( mAdvOptsGroup );
    bg->setObjectName( "bg" );
    mApplyOnForAll = new QRadioButton( i18n("from all accounts"), mAdvOptsGroup );
    bg->addButton( mApplyOnForAll );
    vbl3->addWidget( mApplyOnForAll );
    mApplyOnForTraditional = new QRadioButton( i18n("from all but online IMAP accounts"), mAdvOptsGroup );
    bg->addButton( mApplyOnForTraditional );
    vbl3->addWidget( mApplyOnForTraditional );
    mApplyOnForChecked = new QRadioButton( i18n("from checked accounts only"), mAdvOptsGroup );
    bg->addButton( mApplyOnForChecked );
    vbl3->addWidget( mApplyOnForChecked );
    vbl3->addStretch( 2 );

    mAccountList = new AccountList( mAdvOptsGroup );
    gl->addWidget( mAccountList, 0, 1, 4, 3 );

    mApplyBeforeOut = new QCheckBox( i18n("Apply this filter &before sending messages"), mAdvOptsGroup );
    mApplyBeforeOut->setToolTip( i18n( "<p>The filter will be triggered <b>before</b> the message is sent and it will affect both the local copy and the sent copy of the message.</p>"
          "<p>This is required if the recipient's copy also needs to be modified.</p>" ) );
    gl->addWidget( mApplyBeforeOut, 5, 0, 1, 4 );

    mApplyOnOut = new QCheckBox( i18n("Apply this filter to &sent messages"), mAdvOptsGroup );
    mApplyOnOut->setToolTip( i18n( "<p>The filter will be triggered <b>after</b> the message is sent and it will only affect the local copy of the message.</p>"
          "<p>If the recipient's copy also needs to be modified, please use \"Apply this filter <b>before</b> sending messages\".</p>" ) );
    gl->addWidget( mApplyOnOut, 4, 0, 1, 4 );

    mApplyOnCtrlJ = new QCheckBox( i18n("Apply this filter on manual &filtering"), mAdvOptsGroup );
    gl->addWidget( mApplyOnCtrlJ, 6, 0, 1, 4 );

    mStopProcessingHere = new QCheckBox( i18n("If this filter &matches, stop processing here"), mAdvOptsGroup );
    gl->addWidget( mStopProcessingHere, 7, 0, 1, 4 );
    mConfigureShortcut = new QCheckBox( i18n("Add this filter to the Apply Filter menu"), mAdvOptsGroup );
    gl->addWidget( mConfigureShortcut, 8, 0, 1, 2 );
    QLabel *keyButtonLabel = new QLabel( i18n( "Shortcut:" ), mAdvOptsGroup );
    keyButtonLabel->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
    gl->addWidget( keyButtonLabel, 8, 2, 1, 1);
    mKeySeqWidget = new KKeySequenceWidget( mAdvOptsGroup );
    mKeySeqWidget->setObjectName( "FilterShortcutSelector" );
    gl->addWidget( mKeySeqWidget, 8, 3, 1, 1);
    mKeySeqWidget->setEnabled( false );
    mKeySeqWidget->setModifierlessAllowed( true );
    mKeySeqWidget->setCheckActionCollections( actionCollection );
    mConfigureToolbar = new QCheckBox( i18n("Additionally add this filter to the toolbar"), mAdvOptsGroup );
    gl->addWidget( mConfigureToolbar, 9, 0, 1, 4 );
    mConfigureToolbar->setEnabled( false );

    KHBox *hbox = new KHBox( mAdvOptsGroup );
    mFilterActionLabel = new QLabel( i18n( "Icon for this filter:" ),
                                     hbox );
    mFilterActionLabel->setEnabled( false );

    mFilterActionIconButton = new KIconButton( hbox );
    mFilterActionLabel->setBuddy( mFilterActionIconButton );
    mFilterActionIconButton->setIconType( KIconLoader::NoGroup, KIconLoader::Action, false );
    mFilterActionIconButton->setIconSize( 16 );
    mFilterActionIconButton->setIcon( "system-run" );
    mFilterActionIconButton->setEnabled( false );

    gl->addWidget( hbox, 10, 0, 1, 4 );

    mAdvOptsGroup->setLayout( gl );
  }
  vbl2->addWidget( mAdvOptsGroup, 0, Qt::AlignTop );

  // spacer:
  vbl->addStretch( 1 );

  // load the filter parts into the edit widgets
  connect( mFilterList, SIGNAL(filterSelected(MailCommon::MailFilter*)),
           this, SLOT(slotFilterSelected(MailCommon::MailFilter*)) );

  // transfer changes from the 'Apply this filter on...'
  // combo box to the filter
  connect( mApplyOnIn, SIGNAL(clicked()),
           this, SLOT(slotApplicabilityChanged()) );
  connect( mApplyOnForAll, SIGNAL(clicked()),
           this, SLOT(slotApplicabilityChanged()) );
  connect( mApplyOnForTraditional, SIGNAL(clicked()),
           this, SLOT(slotApplicabilityChanged()) );
  connect( mApplyOnForChecked, SIGNAL(clicked()),
           this, SLOT(slotApplicabilityChanged()) );
  connect( mApplyBeforeOut, SIGNAL(clicked()),
           this, SLOT(slotApplicabilityChanged()) );
  connect( mApplyOnOut, SIGNAL(clicked()),
           this, SLOT(slotApplicabilityChanged()) );
  connect( mApplyOnCtrlJ, SIGNAL(clicked()),
           this, SLOT(slotApplicabilityChanged()) );
  connect( mAccountList, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
           this, SLOT(slotApplicableAccountsChanged()) );

  // transfer changes from the 'stop processing here'
  // check box to the filter
  connect( mStopProcessingHere, SIGNAL(toggled(bool)),
           this, SLOT(slotStopProcessingButtonToggled(bool)) );

  connect( mConfigureShortcut, SIGNAL(toggled(bool)),
           this, SLOT(slotConfigureShortcutButtonToggled(bool)) );

  connect( mKeySeqWidget, SIGNAL(keySequenceChanged(QKeySequence)),
           this, SLOT(slotShortcutChanged(QKeySequence)) );

  connect( mConfigureToolbar, SIGNAL(toggled(bool)),
           this, SLOT(slotConfigureToolbarButtonToggled(bool)) );

  connect( mFilterActionIconButton, SIGNAL(iconChanged(QString)),
           this, SLOT(slotFilterActionIconChanged(QString)) );

  // reset all widgets here
  connect( mFilterList, SIGNAL(resetWidgets()),
           this, SLOT(slotReset()) );

  connect( mFilterList, SIGNAL(applyWidgets()),
           this, SLOT(slotUpdateFilter()) );

  // support auto-naming the filter
  connect( mPatternEdit, SIGNAL(maybeNameChanged()),
           mFilterList, SLOT(slotUpdateFilterName()) );

  // save filters on 'Apply' or 'OK'
  connect( this, SIGNAL(buttonClicked(KDialog::ButtonCode)),
           mFilterList, SLOT(slotApplyFilterChanges(KDialog::ButtonCode)) );
  connect( button( KDialog::Apply ), SIGNAL(clicked(bool)), this, SLOT(slotApply()) );

  // save dialog size on 'OK'
  connect( this, SIGNAL(okClicked()),
           this, SLOT(slotSaveSize()) );

  // destruct the dialog on close and Cancel
  connect( this, SIGNAL(closeClicked()),
           this, SLOT(slotFinished()) );
  connect( this, SIGNAL(cancelClicked()),
           this, SLOT(slotFinished()) );

  // disable closing when user wants to continue editing
  connect( mFilterList, SIGNAL(abortClosing()),
           this, SLOT(slotDisableAccept()) );

  connect( mFilterList, SIGNAL(filterCreated()), this, SLOT(slotDialogUpdated()) );
  connect( mFilterList, SIGNAL(filterRemoved(MailCommon::MailFilter*)),
           this, SLOT(slotDialogUpdated()) );
  connect( mFilterList, SIGNAL(filterUpdated(MailCommon::MailFilter*)),
           this, SLOT(slotDialogUpdated()) );
  connect( mFilterList, SIGNAL(filterOrderAltered()), this, SLOT(slotDialogUpdated()) );
  connect( mPatternEdit, SIGNAL(patternChanged()), this, SLOT(slotDialogUpdated()) );
  connect( mActionLister, SIGNAL(widgetAdded(QWidget*)), this, SLOT(slotDialogUpdated()) );
  connect( mActionLister, SIGNAL(widgetRemoved()), this, SLOT(slotDialogUpdated()) );
  connect( mActionLister, SIGNAL(filterModified()), this, SLOT(slotDialogUpdated()) );
  KConfigGroup myGroup( KernelIf->config(), "Geometry" );
  const QSize size = myGroup.readEntry( "filterDialogSize", QSize() );
  if ( size != QSize()  )
    resize( size );
  else
    adjustSize();

  // load the filter list (emits filterSelected())
  mFilterList->loadFilterList( createDummyFilter );
  mIgnoreFilterUpdates = false;
}

void KMFilterDialog::accept()
{
  if ( mDoNotClose ) {
    mDoNotClose = false; // only abort current close attempt
  } else {
    KDialog::accept();
    slotFinished();
  }
}

void KMFilterDialog::slotApply()
{
  enableButtonApply( false );
}

void KMFilterDialog::slotFinished() {
  deleteLater();
}

void KMFilterDialog::slotSaveSize() {
  KConfigGroup myGroup( KernelIf->config(), "Geometry" );
  myGroup.writeEntry( "filterDialogSize",size() );
  myGroup.sync();
}

void KMFilterDialog::slotFilterSelected( MailFilter* aFilter )
{
  assert( aFilter );
  mIgnoreFilterUpdates = true;
  mActionLister->setActionList( aFilter->actions() );

  mAdvOptsGroup->setEnabled( true );

  mPatternEdit->setSearchPattern( aFilter->pattern() );
  mFilter = aFilter;

  kDebug() << "apply on inbound ==" << aFilter->applyOnInbound();
  kDebug() << "apply on outbound ==" << aFilter->applyOnOutbound();
  kDebug() << "apply before outbound == " << aFilter->applyBeforeOutbound();
  kDebug() << "apply on explicit ==" << aFilter->applyOnExplicit();

  // NOTE: setting these values activates the slot that sets them in
  // the filter! So make sure we have the correct values _before_ we
  // set the first one:
  const bool applyOnIn = aFilter->applyOnInbound();
  const bool applyOnForAll = aFilter->applicability() == MailFilter::All;
  const bool applyOnTraditional = aFilter->applicability() == MailFilter::ButImap;
  const bool applyBeforeOut = aFilter->applyBeforeOutbound();
  const bool applyOnOut = aFilter->applyOnOutbound();
  const bool applyOnExplicit = aFilter->applyOnExplicit();
  const bool stopHere = aFilter->stopProcessingHere();
  const bool configureShortcut = aFilter->configureShortcut();
  const bool configureToolbar = aFilter->configureToolbar();
  const QString icon = aFilter->icon();
  const KShortcut shortcut( aFilter->shortcut() );

  mApplyOnIn->setChecked( applyOnIn );
  mApplyOnForAll->setEnabled( applyOnIn );
  mApplyOnForTraditional->setEnabled( applyOnIn );
  mApplyOnForChecked->setEnabled( applyOnIn );
  mApplyOnForAll->setChecked( applyOnForAll );
  mApplyOnForTraditional->setChecked( applyOnTraditional );
  mApplyOnForChecked->setChecked( !applyOnForAll && !applyOnTraditional );
  mAccountList->setEnabled( mApplyOnForChecked->isEnabled() && mApplyOnForChecked->isChecked() );
  slotUpdateAccountList();
  mApplyBeforeOut->setChecked( applyBeforeOut );
  mApplyOnOut->setChecked( applyOnOut );
  mApplyOnCtrlJ->setChecked( applyOnExplicit );
  mStopProcessingHere->setChecked( stopHere );
  mConfigureShortcut->setChecked( configureShortcut );
  mKeySeqWidget->setKeySequence( shortcut.primary(),
                                 KKeySequenceWidget::NoValidate );
  mConfigureToolbar->setChecked( configureToolbar );
  mFilterActionIconButton->setIcon( icon );
  mIgnoreFilterUpdates = false;
}

void KMFilterDialog::slotReset()
{
  mFilter = 0;
  mPatternEdit->reset();

  mActionLister->reset();
  mAdvOptsGroup->setEnabled( false );
  slotUpdateAccountList();
}

void KMFilterDialog::slotUpdateFilter()
{
  mPatternEdit->updateSearchPattern();
  mActionLister->updateActionList();
}

void KMFilterDialog::slotApplicabilityChanged()
{
  if ( mFilter ) {
    mFilter->setApplyOnInbound( mApplyOnIn->isChecked() );
    mFilter->setApplyBeforeOutbound( mApplyBeforeOut->isChecked() );
    mFilter->setApplyOnOutbound( mApplyOnOut->isChecked() );
    mFilter->setApplyOnExplicit( mApplyOnCtrlJ->isChecked() );
    if ( mApplyOnForAll->isChecked() )
      mFilter->setApplicability( MailFilter::All );
    else if ( mApplyOnForTraditional->isChecked() )
      mFilter->setApplicability( MailFilter::ButImap );
    else if ( mApplyOnForChecked->isChecked() )
      mFilter->setApplicability( MailFilter::Checked );

    mApplyOnForAll->setEnabled( mApplyOnIn->isChecked() );
    mApplyOnForTraditional->setEnabled(  mApplyOnIn->isChecked() );
    mApplyOnForChecked->setEnabled( mApplyOnIn->isChecked() );
    mAccountList->setEnabled( mApplyOnForChecked->isEnabled() && mApplyOnForChecked->isChecked() );

    // Advanced tab functionality - Update list of accounts this filter applies to
    mAccountList->applyOnAccount(mFilter);

    // Enable the apply button
    slotDialogUpdated();

    kDebug() << "Setting filter to be applied at"
                 << ( mFilter->applyOnInbound() ? "incoming " : "" )
                 << ( mFilter->applyOnOutbound() ? "outgoing " : "" )
                 << ( mFilter->applyBeforeOutbound() ? "before_outgoing " : "" )
                 << ( mFilter->applyOnExplicit() ? "explicit CTRL-J" : "" );
  }
}

void KMFilterDialog::slotApplicableAccountsChanged()
{
  // Advanced tab functionality - Update list of accounts this filter applies to
  if ( mFilter && mApplyOnForChecked->isEnabled() && mApplyOnForChecked->isChecked() ) {

    QTreeWidgetItemIterator it( mAccountList );

    while( QTreeWidgetItem *item = *it ) {
      const QString id = item->text( 2 );
      mFilter->setApplyOnAccount( id, item->checkState( 0 ) == Qt::Checked );
      ++it;
    }

    // Enable the apply button
    slotDialogUpdated();
  }
}

void KMFilterDialog::slotStopProcessingButtonToggled( bool aChecked )
{
  if ( mFilter ) {
    mFilter->setStopProcessingHere( aChecked );

    // Enable the apply button
    slotDialogUpdated();
  }
}

void KMFilterDialog::slotConfigureShortcutButtonToggled( bool aChecked )
{
  if ( mFilter ) {
    mFilter->setConfigureShortcut( aChecked );
    mKeySeqWidget->setEnabled( aChecked );
    mConfigureToolbar->setEnabled( aChecked );
    mFilterActionIconButton->setEnabled( aChecked );
    mFilterActionLabel->setEnabled( aChecked );

    // Enable the apply button
    slotDialogUpdated();
  }
}

void KMFilterDialog::slotShortcutChanged( const QKeySequence &newSeq )
{
  if ( mFilter ) {
    mKeySeqWidget->applyStealShortcut();
    mFilter->setShortcut( KShortcut( newSeq ) );

    // Enable the apply button
    slotDialogUpdated();
  }
}

void KMFilterDialog::slotConfigureToolbarButtonToggled( bool aChecked )
{
  if ( mFilter ) {
    mFilter->setConfigureToolbar( aChecked );
    // Enable the apply button
    slotDialogUpdated();
  }
}

void KMFilterDialog::slotFilterActionIconChanged( const QString &icon )
{
  if ( mFilter ) {
    mFilter->setIcon( icon );
    // Enable the apply button
    slotDialogUpdated();
 }
}

void KMFilterDialog::slotUpdateAccountList()
{
  mAccountList->updateAccountList(mFilter);
}

//=============================================================================
//
// class KMFilterListBox (the filter list manipulator)
//
//=============================================================================

KMFilterListBox::KMFilterListBox( const QString & title, QWidget *parent )
  : QGroupBox( title, parent )
{
  QVBoxLayout *layout = new QVBoxLayout();

  //----------- the list box
  mListWidget = new QListWidget(this);
  mListWidget->setMinimumWidth(150);
  mListWidget->setWhatsThis( i18n(_wt_filterlist) );
  mListWidget->setDragDropMode( QAbstractItemView::InternalMove );
  connect( mListWidget->model(),SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),SLOT(slotRowsMoved(QModelIndex,int,int,QModelIndex,int)) );

  KListWidgetSearchLine* mSearchListWidget = new KListWidgetSearchLine( this, mListWidget );
  mSearchListWidget->setTrapReturnKey(true);
  mSearchListWidget->setClickMessage( i18nc( "@info/plain Displayed grayed-out inside the "
                                             "textbox, verb to search", "Search" ) );

  layout->addWidget( mSearchListWidget );
  layout->addWidget( mListWidget );

  //----------- the first row of buttons
  KHBox *hb = new KHBox(this);
  hb->setSpacing(4);

  mBtnTop = new KPushButton( QString(), hb );
  mBtnTop->setIcon( KIcon( "go-top" ) );
  mBtnTop->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  mBtnTop->setMinimumSize( mBtnTop->sizeHint() * 1.2 );


  mBtnUp = new KPushButton( QString(), hb );
  mBtnUp->setAutoRepeat( true );
  mBtnUp->setIcon( KIcon( "go-up" ) );
  mBtnUp->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  mBtnUp->setMinimumSize( mBtnUp->sizeHint() * 1.2 );
  mBtnDown = new KPushButton( QString(), hb );
  mBtnDown->setAutoRepeat( true );
  mBtnDown->setIcon( KIcon( "go-down" ) );
  mBtnDown->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  mBtnDown->setMinimumSize( mBtnDown->sizeHint() * 1.2 );

  mBtnBottom = new KPushButton( QString(), hb );
  mBtnBottom->setIcon( KIcon( "go-bottom" ) );
  mBtnBottom->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  mBtnBottom->setMinimumSize( mBtnBottom->sizeHint() * 1.2 );


  mBtnUp->setToolTip( i18nc("Move selected filter up.", "Up") );
  mBtnDown->setToolTip( i18nc("Move selected filter down.", "Down") );
  mBtnTop->setToolTip( i18nc("Move selected filter to the top.", "Top") );
  mBtnBottom->setToolTip( i18nc("Move selected filter to the bottom.", "Bottom") );
  mBtnUp->setWhatsThis( i18n(_wt_filterlist_up) );
  mBtnDown->setWhatsThis( i18n(_wt_filterlist_down) );
  mBtnBottom->setWhatsThis( i18n(_wt_filterlist_bottom) );
  mBtnTop->setWhatsThis( i18n(_wt_filterlist_top) );

  layout->addWidget( hb );

  //----------- the second row of buttons
  hb = new KHBox(this);
  hb->setSpacing(4);
  mBtnNew = new QPushButton( QString(), hb );
  mBtnNew->setIcon( KIcon( "document-new" ) );
  mBtnNew->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  mBtnNew->setMinimumSize( mBtnNew->sizeHint() * 1.2 );
  mBtnCopy = new QPushButton( QString(), hb );
  mBtnCopy->setIcon( KIcon( "edit-copy" ) );
  mBtnCopy->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  mBtnCopy->setMinimumSize( mBtnCopy->sizeHint() * 1.2 );
  mBtnDelete = new QPushButton( QString(), hb );
  mBtnDelete->setIcon( KIcon( "edit-delete" ) );
  mBtnDelete->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  mBtnDelete->setMinimumSize( mBtnDelete->sizeHint() * 1.2 );
  mBtnRename = new QPushButton( i18n("Rename..."), hb );
  mBtnNew->setToolTip( i18nc("@action:button in filter list manipulator", "New") );
  mBtnCopy->setToolTip( i18n("Copy") );
  mBtnDelete->setToolTip( i18n("Delete"));
  mBtnNew->setWhatsThis( i18n(_wt_filterlist_new) );
  mBtnCopy->setWhatsThis( i18n(_wt_filterlist_copy) );
  mBtnDelete->setWhatsThis( i18n(_wt_filterlist_delete) );
  mBtnRename->setWhatsThis( i18n(_wt_filterlist_rename) );

  layout->addWidget( hb );
  setLayout( layout );

  //----------- now connect everything
  connect( mListWidget, SIGNAL(currentRowChanged(int)),
	   this, SLOT(slotSelected(int)) );
  connect( mListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
           this, SLOT(slotRename()) );
  connect( mListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
           this, SLOT(slotFilterEnabledChanged(QListWidgetItem*)));

  connect( mBtnUp, SIGNAL(clicked()),
	   this, SLOT(slotUp()) );
  connect( mBtnDown, SIGNAL(clicked()),
	   this, SLOT(slotDown()) );
  connect( mBtnTop, SIGNAL(clicked()),
	   this, SLOT(slotTop()) );
  connect( mBtnBottom, SIGNAL(clicked()),
	   this, SLOT(slotBottom()) );

  connect( mBtnNew, SIGNAL(clicked()),
	   this, SLOT(slotNew()) );
  connect( mBtnCopy, SIGNAL(clicked()),
	   this, SLOT(slotCopy()) );
  connect( mBtnDelete, SIGNAL(clicked()),
	   this, SLOT(slotDelete()) );
  connect( mBtnRename, SIGNAL(clicked()),
	   this, SLOT(slotRename()) );

  // the dialog should call loadFilterList()
  // when all signals are connected.
  enableControls();
}


KMFilterListBox::~KMFilterListBox()
{
}

void KMFilterListBox::slotFilterEnabledChanged( QListWidgetItem *item )
{
  if ( !item ) {
    kDebug() << "Called while no filter is selected, ignoring.";
    return;
  }
  QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( item );
  MailCommon::MailFilter *filter = itemFilter->filter();
  filter->setEnabled( ( item->checkState() == Qt::Checked ) );
  emit filterUpdated( filter );
}


void KMFilterListBox::slotRowsMoved( const QModelIndex &,
                                     int sourcestart, int sourceEnd,
                                     const QModelIndex &, int destinationRow )
{
  Q_UNUSED( sourceEnd );
  Q_UNUSED( sourcestart );
  Q_UNUSED( destinationRow );
  enableControls();

  emit filterOrderAltered();
}

void KMFilterListBox::createFilter( const QByteArray &field, const QString &value )
{
  SearchRule::Ptr newRule = SearchRule::createInstance( field, SearchRule::FuncContains, value );

  MailFilter *newFilter = new MailFilter();
  newFilter->pattern()->append( newRule );
  newFilter->pattern()->setName( QString::fromLatin1("<%1>:%2").arg( QString::fromLatin1( field ) ).arg( value) );

  FilterActionDesc *desc = MailCommon::FilterManager::filterActionDict()->value( "transfer" );
  if ( desc )
    newFilter->actions()->append( desc->create() );

  insertFilter( newFilter );
  enableControls();
}

void KMFilterListBox::slotUpdateFilterName()
{
  QListWidgetItem *item = mListWidget->currentItem();
  if ( !item ) {
    kDebug() << "Called while no filter is selected, ignoring.";
    return;
  }
  QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( item );
  MailCommon::MailFilter *filter = itemFilter->filter();

  SearchPattern *p = filter->pattern();
  if ( !p ) return;

  QString shouldBeName = p->name();
  QString displayedName = itemFilter->text();

  if ( shouldBeName.trimmed().isEmpty() ) {
    filter->setAutoNaming( true );
  }

  if ( filter->isAutoNaming() ) {
    // auto-naming of patterns
    if ( !p->isEmpty() && p->first() && !p->first()->field().trimmed().isEmpty() )
      shouldBeName = QString::fromLatin1( "<%1>: %2" ).arg( QString::fromLatin1( p->first()->field() ) ).arg( p->first()->contents() );
    else
      shouldBeName = '<' + i18n("unnamed") + '>';
    p->setName( shouldBeName );
  }

  if ( displayedName == shouldBeName ) return;

  filter->setToolbarName( shouldBeName );

  mListWidget->blockSignals(true);
  itemFilter->setText( shouldBeName );
  mListWidget->blockSignals(false);
}

void KMFilterListBox::slotApplyFilterChanges( KDialog::ButtonCode button )
{
  bool closeAfterSaving;
  if ( button == KDialog::Ok )
    closeAfterSaving = true;
  else if ( button == KDialog::Apply )
    closeAfterSaving = false;
  else
    return; // ignore close and cancel

  if ( mListWidget->currentItem() ) {
    emit applyWidgets();
    slotSelected( mListWidget->currentRow() );
  }

  // by now all edit widgets should have written back
  // their widget's data into our filter list.

  const QList<MailFilter*> newFilters = filtersForSaving( closeAfterSaving );

  MailCommon::FilterManager::instance()->setFilters( newFilters );
}

QList<MailFilter *> KMFilterListBox::filtersForSaving( bool closeAfterSaving ) const
{
  const_cast<KMFilterListBox*>( this )->applyWidgets(); // signals aren't const
  QList<MailFilter *> filters;
  QStringList emptyFilters;
  const int numberOfFilter( mListWidget->count() );
  for ( int i = 0; i <numberOfFilter; ++i ) {
    QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( mListWidget->item( i ) );
    MailFilter *f = new MailFilter( *itemFilter->filter() ); // deep copy
    f->purify();
    if ( !f->isEmpty() )
      // the filter is valid:
      filters.append( f );
    else {
      // the filter is invalid:
      emptyFilters << f->name();
      delete f;
    }
  }

  // report on invalid filters:
  if ( !emptyFilters.empty() ) {
    if ( closeAfterSaving ) {
      // Ok clicked. Give option to continue editing
      int response = KMessageBox::warningContinueCancelList(
        0,
        i18n( "The following filters are invalid (e.g. containing no actions "
              "or no search rules). Discard or edit invalid filters?" ),
        emptyFilters,
        QString(),
        KGuiItem( i18n( "Discard" ) ),
        KStandardGuiItem::cancel(),
        "ShowInvalidFilterWarning" );
      if ( response == KMessageBox::Cancel )
        emit abortClosing();
    } else {
      // Apply clicked. Just warn.
      KMessageBox::informationList(
        0,
        i18n( "The following filters have not been saved because they were invalid "
              "(e.g. containing no actions or no search rules)." ),
        emptyFilters,
        QString(),
        "ShowInvalidFilterWarning" );
    }
  }
  return filters;
}

void KMFilterListBox::slotSelected( int aIdx )
{
  if ( aIdx >= 0 && aIdx < mListWidget->count() ) {
    QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( mListWidget->item(aIdx) );
    MailFilter *f = itemFilter->filter();
    if ( f )
      emit filterSelected( f );
    else
      emit resetWidgets();
  } else {
    emit resetWidgets();
  }
  enableControls();
}

void KMFilterListBox::slotNew()
{
  QListWidgetItem *item = mListWidget->currentItem();
  if ( item &&
       item->isHidden() )
    return;

  // just insert a new filter.
  insertFilter( new MailFilter() );
  enableControls();
}

void KMFilterListBox::slotCopy()
{
  QListWidgetItem * item = mListWidget->currentItem();
  if ( !item ) {
    kDebug() << "Called while no filter is selected, ignoring.";
    return;
  }
  if ( item->isHidden() )
    return;

  // make sure that all changes are written to the filter before we copy it
  emit applyWidgets();
  QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( item );

  MailFilter *filter = itemFilter->filter();

  // enableControls should make sure this method is
  // never called when no filter is selected.
  assert( filter );

  // inserts a copy of the current filter.
  insertFilter( new MailFilter( *filter ) );
  enableControls();
}

void KMFilterListBox::slotDelete()
{
  QListWidgetItem *item = mListWidget->currentItem();
  if ( !item ) {
    kDebug() << "Called while no filter is selected, ignoring.";
    return;
  }
  if ( item->isHidden() )
    return;
  QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( item );

  MailCommon::MailFilter *filter = itemFilter->filter();

  const QString filterName = filter->pattern()->name();

  if ( KMessageBox::questionYesNo(this, i18n( "Do you want to remove the filter \"%1\" ?",filterName ), i18n( "Remove Filter" )) == KMessageBox::No )
    return;
  int oIdxSelItem = mListWidget->currentRow();
  // unselect all
  // TODO remove this line: mListWidget->clearSelection();
  // broadcast that all widgets let go
  // of the filter
  emit resetWidgets();

  // remove the filter from both the listbox
  QListWidgetItem *item2 = mListWidget->takeItem( oIdxSelItem );
  delete item2;


  const int count = mListWidget->count();
  // and set the new current item.
  if ( count > oIdxSelItem )
    // oIdxItem is still a valid index
    mListWidget->setCurrentRow( oIdxSelItem );
  else if ( count )
    // oIdxSelIdx is no longer valid, but the
    // list box isn't empty
    mListWidget->setCurrentRow( count - 1 );

  // work around a problem when deleting the first item in a QListWidget:
  // after takeItem, slotSelectionChanged is emitted with 1, but the row 0
  // remains selected and another selectCurrentRow(0) does not trigger the
  // selectionChanged signal
  // (qt-copy as of 2006-12-22 / gungl)
  if ( oIdxSelItem == 0 )
    slotSelected( 0 );

  enableControls();

  emit filterRemoved( filter );
}

void KMFilterListBox::slotTop()
{
  QListWidgetItem *item = mListWidget->currentItem();
  if ( !item ) {
    kDebug() << "Called while no filter is selected, ignoring.";
    return;
  }
  const int currentIndex = mListWidget->currentRow();
  if ( currentIndex == 0 ) {
    kDebug() << "Called while the _topmost_ filter is selected, ignoring.";
    return;
  }

  if ( item->isHidden() )
    return;
  item = mListWidget->takeItem( currentIndex );
  mListWidget->insertItem( 0, item );

  mListWidget->setCurrentItem( mListWidget->item( 0 ) );

  enableControls();

  emit filterOrderAltered();
}

void KMFilterListBox::slotBottom()
{
  QListWidgetItem *item = mListWidget->currentItem();
  if ( !item ) {
    kDebug() << "Called while no filter is selected, ignoring.";
    return;
  }
  const int currentIndex = mListWidget->currentRow();
  if ( currentIndex == (int)mListWidget->count() - 1 ) {
    kDebug() << "Called while the _last_ filter is selected, ignoring.";
    return;
  }
  if ( item->isHidden() )
    return;
  item = mListWidget->takeItem( currentIndex );
  mListWidget->insertItem( mListWidget->count(), item );

  mListWidget->setCurrentItem( mListWidget->item( mListWidget->count() -1 ) );
  enableControls();

  emit filterOrderAltered();
}


void KMFilterListBox::slotUp()
{
  QListWidgetItem *item = mListWidget->currentItem();
  if ( !item ) {
    kDebug() << "Called while no filter is selected, ignoring.";
    return;
  }
  const int currentIndex = mListWidget->currentRow( );
  if ( currentIndex == 0 ) {
    kDebug() << "Called while the _topmost_ filter is selected, ignoring.";
    return;
  }
  if ( item->isHidden() )
    return;
  swapNeighbouringFilters( currentIndex, currentIndex - 1 );
  enableControls();

  emit filterOrderAltered();
}

void KMFilterListBox::slotDown()
{
  QListWidgetItem *item = mListWidget->currentItem();
  if ( !item ) {
    kDebug() << "Called while no filter is selected, ignoring.";
    return;
  }
  const int currentIndex = mListWidget->currentRow();
  if ( currentIndex == (int)mListWidget->count() - 1 ) {
    kDebug() << "Called while the _last_ filter is selected, ignoring.";
    return;
  }
  if ( item->isHidden() )
    return;
  swapNeighbouringFilters( currentIndex, currentIndex + 1);
  enableControls();

  emit filterOrderAltered();
}

void KMFilterListBox::slotRename()
{
  QListWidgetItem * item = mListWidget->currentItem();
  if ( !item ) {
    kDebug() << "Called while no filter is selected, ignoring.";
    return;
  }
  if ( item->isHidden() )
    return;
  QListWidgetFilterItem *itemFilter = static_cast<QListWidgetFilterItem*>( item );

  bool okPressed = false;
  MailFilter *filter = itemFilter->filter();

  // enableControls should make sure this method is
  // never called when no filter is selected.
  assert( filter );

  // allow empty names - those will turn auto-naming on again
  QValidator *validator = new QRegExpValidator( QRegExp( ".*" ), 0 );
  QString newName = KInputDialog::getText
    (
     i18n("Rename Filter"),
     i18n("Rename filter \"%1\" to:\n(leave the field empty for automatic naming)",
          filter->pattern()->name() ) /*label*/,
     filter->pattern()->name() /* initial value */,
     &okPressed, window(), validator
     );
  delete validator;

  if ( !okPressed ) return;

  if ( newName.isEmpty() ) {
    // bait for slotUpdateFilterName to
    // use automatic naming again.
    filter->pattern()->setName( "<>" );
    filter->setAutoNaming( true );
  } else {
    filter->pattern()->setName( newName );
    filter->setAutoNaming( false );
  }

  slotUpdateFilterName();

  emit filterUpdated( filter );
}

void KMFilterListBox::enableControls()
{
  const int currentIndex = mListWidget->currentRow();
  const bool theFirst = ( currentIndex == 0 );
  const bool theLast = ( currentIndex >= (int)mListWidget->count() - 1 );
  const bool aFilterIsSelected = ( currentIndex >= 0 );

  mBtnUp->setEnabled( aFilterIsSelected && !theFirst );
  mBtnDown->setEnabled( aFilterIsSelected && !theLast );
  mBtnCopy->setEnabled( aFilterIsSelected );
  mBtnDelete->setEnabled( aFilterIsSelected );
  mBtnRename->setEnabled( aFilterIsSelected );
  mBtnTop->setEnabled( aFilterIsSelected && !theFirst );
  mBtnBottom->setEnabled( aFilterIsSelected && !theLast );

  if ( aFilterIsSelected )
    mListWidget->scrollToItem( mListWidget->currentItem() );
}

void KMFilterListBox::loadFilterList( bool createDummyFilter )
{
  assert(mListWidget);
  setEnabled(false);
  emit resetWidgets();
  // we don't want the insertion to
  // cause flicker in the edit widgets.
  blockSignals(true);

  // clear both lists
  mListWidget->clear();

  const QList<MailFilter*> filters = MailCommon::FilterManager::instance()->filters();
  foreach ( MailFilter *filter, filters ) {
    QListWidgetFilterItem *item = new QListWidgetFilterItem( filter->pattern()->name(), mListWidget );
    item->setFilter( new MailFilter( *filter ) );
    mListWidget->addItem( item );
  }

  blockSignals(false);
  setEnabled(true);

  // create an empty filter when there's none, to avoid a completely
  // disabled dialog (usability tests indicated that the new-filter
  // button is too hard to find that way):
  const int numberOfItem( mListWidget->count() );
  if ( !numberOfItem && createDummyFilter )
    slotNew();

  if ( numberOfItem > 0 )
    mListWidget->setCurrentRow( 0 );

  enableControls();
}

void KMFilterListBox::insertFilter( MailFilter* aFilter )
{
  // must be really a filter...
  assert( aFilter );
  const int currentIndex = mListWidget->currentRow();
  // if mIdxSelItem < 0, QListBox::insertItem will append.
  QListWidgetFilterItem *item = new QListWidgetFilterItem( aFilter->pattern()->name() );
  item->setFilter(  aFilter );
  mListWidget->insertItem( currentIndex,item );
  if ( currentIndex < 0 ) {
    mListWidget->setCurrentRow( mListWidget->count() - 1 );
  } else {
    // insert just before selected
    mListWidget->setCurrentRow( currentIndex );
  }

  emit filterCreated();
  emit filterOrderAltered();
}

void KMFilterListBox::appendFilter( MailFilter* aFilter )
{
  QListWidgetFilterItem *item = new QListWidgetFilterItem( aFilter->pattern()->name(), mListWidget );
  item->setFilter( aFilter );
  mListWidget->addItem( item );

  emit filterCreated();
}

void KMFilterListBox::swapNeighbouringFilters( int untouchedOne, int movedOne )
{
  // must be neighbours...
  assert( untouchedOne - movedOne == 1 || movedOne - untouchedOne == 1 );

  // untouchedOne is at idx. to move it down(up),
  // remove item at idx+(-)1 w/o deleting it.
  QListWidgetItem *item = mListWidget->takeItem( movedOne );
  // now selected item is at idx(idx-1), so
  // insert the other item at idx, ie. above(below).
  mListWidget->insertItem( untouchedOne, item );
}


void KMFilterDialog::slotImportFilters()
{
  FilterImporterExporter importer( this );
  bool canceled = false;
  QList<MailFilter *> filters = importer.importFilters( canceled );
  if ( canceled )
    return;

  if ( filters.isEmpty() ) {
    KMessageBox::information( this, i18n( "No filter was imported." ) );
    return;
  }
  QStringList listOfFilter;
  QList<MailFilter*>::ConstIterator end( filters.constEnd() );

  for ( QList<MailFilter*>::ConstIterator it = filters.constBegin() ; it != end ; ++it ) {
    mFilterList->appendFilter( *it ); // no need to deep copy, ownership passes to the list
    listOfFilter<<( *it )->name();
  }
  KMessageBox::informationList( this, i18n( "Filters which were imported:" ),listOfFilter );
}

void KMFilterDialog::slotExportFilters()
{
  FilterImporterExporter exporter( this );
  QList<MailFilter *> filters = mFilterList->filtersForSaving( false );
  exporter.exportFilters( filters );
  QList<MailFilter*>::ConstIterator end( filters.constEnd() );
  for ( QList<MailFilter*>::ConstIterator it = filters.constBegin(); it != end; ++it )
    delete *it;
}

void KMFilterDialog::slotDisableAccept()
{
  mDoNotClose = true;
}

void KMFilterDialog::slotDialogUpdated()
{
  kDebug() << "Detected a change in data bound to the dialog!";
  if ( !mIgnoreFilterUpdates ) {
    enableButtonApply( true );
  }
}
}

#include "kmfilterdialog.moc"
