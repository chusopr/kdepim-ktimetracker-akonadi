
#include "managesievescriptsdialog.h"
#include "managesievescriptsdialog_p.h"
#include "sievetextedit.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kwindowsystem.h>
#include <kinputdialog.h>
#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include <akonadi/agentinstance.h>
#include <kmanagesieve/sievejob.h>
#include <ksieveui/util.h>

#include <QApplication>
#include <QButtonGroup>
#include <QMenu>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPointer>
#include <QFile>
#include <QTextStream>

#include <errno.h>

using namespace KSieveUi;

bool ItemRadioButton::mTreeWidgetIsBeingCleared = false;

ManageSieveScriptsDialog::ManageSieveScriptsDialog( QWidget * parent, const char * name )
  : QDialog( parent ),
    mSieveEditor( 0 ),
    mWasActive( false )
{
  setWindowTitle( i18n( "Manage Sieve Scripts" ) );
  setObjectName( name );
  setModal( false );
  setAttribute( Qt::WA_GroupLeader );
  setAttribute( Qt::WA_DeleteOnClose );
  KWindowSystem::setIcons( winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop),IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small),IconSize(KIconLoader::Small)) );
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QFrame *frame =new QFrame;
  mainLayout->addWidget( frame );
  QVBoxLayout * vlay = new QVBoxLayout( frame );
  vlay->setSpacing( 0 );
  vlay->setMargin( 0 );
  
  mListView = new TreeWidgetWithContextMenu( frame);
  mListView->setHeaderLabel( i18n( "Available Scripts" ) );
  mListView->setRootIsDecorated( true );
  mListView->setAlternatingRowColors( true );
  mListView->setSelectionMode( QAbstractItemView::SingleSelection );
#ifndef QT_NO_CONTEXTMENU
  connect( mListView, SIGNAL(contextMenuRequested(QTreeWidgetItem*,QPoint)),
           this, SLOT(slotContextMenuRequested(QTreeWidgetItem*,QPoint)) );
#endif
  connect( mListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
           this, SLOT(slotDoubleClicked(QTreeWidgetItem*)) );
  connect( mListView, SIGNAL( itemSelectionChanged () ),
           this, SLOT( slotUpdateButtons() ) );
  vlay->addWidget( mListView );

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  vlay->addLayout( buttonLayout );

  mNewScript = new KPushButton( i18n( "New..." ) );
  connect( mNewScript, SIGNAL( clicked() ), SLOT( slotNewScript() ) );
  buttonLayout->addWidget( mNewScript );
  
  mEditScript = new KPushButton( i18n( "Edit..." ) );
  connect( mEditScript, SIGNAL( clicked() ), SLOT( slotEditScript() ) );
  buttonLayout->addWidget( mEditScript );
  
  mDeleteScript = new KPushButton( i18n( "Delete" ) );
  connect( mDeleteScript, SIGNAL( clicked() ), SLOT( slotDeleteScript() ) );
  buttonLayout->addWidget( mDeleteScript );

  mDeactivateScript = new KPushButton( i18n( "Deactivate" ) );
  connect( mDeactivateScript, SIGNAL( clicked() ), SLOT( slotDeactivateScript() ) );
  buttonLayout->addWidget( mDeactivateScript );
  
  KPushButton *mClose = new KPushButton( KStandardGuiItem::close() );
  connect( mClose, SIGNAL( clicked() ), this, SLOT( accept() ) );
  buttonLayout->addWidget( mClose );
  
  resize( sizeHint().width(), sizeHint().height() );
  slotRefresh();
}

ManageSieveScriptsDialog::~ManageSieveScriptsDialog()
{
  clear();
}

void ManageSieveScriptsDialog::killAllJobs()
{
  for ( QMap<KManageSieve::SieveJob*,QTreeWidgetItem*>::const_iterator it = mJobs.constBegin(),
        end = mJobs.constEnd() ; it != end ; ++it )
    it.key()->kill();
  mJobs.clear();
}

void ManageSieveScriptsDialog::slotUpdateButtons()
{

  QTreeWidgetItem * item = mListView->currentItem();

  bool enabled = true;
  if ( !item )
    enabled = false;
  else if ( !item->parent() && !mUrls.count( item ))
    enabled = false;
  if ( !enabled )
  {
    mNewScript->setEnabled( false );
    mEditScript->setEnabled( false );
    mDeleteScript->setEnabled( false );
    mDeactivateScript->setEnabled( false );
  }
  else
  {  
    mNewScript->setEnabled( item && mUrls.count( item ) );
    enabled = item && isFileNameItem( item );
    mEditScript->setEnabled( enabled );
    mDeleteScript->setEnabled( enabled );
    mDeactivateScript->setEnabled( enabled && isRadioButtonChecked( item ));
  }
}


void ManageSieveScriptsDialog::slotRefresh()
{
  clear();
  QTreeWidgetItem *last = 0;
  Akonadi::AgentInstance::List lst = KSieveUi::Util::imapAgentInstances();
  foreach ( const Akonadi::AgentInstance& type, lst )
  {
    if ( type.status() == Akonadi::AgentInstance::Broken )
      continue;

    last = new QTreeWidgetItem( mListView, last );
    last->setText( 0, type.name() );
    last->setIcon( 0, SmallIcon( "network-server" ) );

    const KUrl u = KSieveUi::Util::findSieveUrlForAccount( type.identifier() );
    if ( u.isEmpty() ) {
      QTreeWidgetItem *item = new QTreeWidgetItem( last );
      item->setText( 0, i18n( "No Sieve URL configured" ) );
      item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
      mListView->expandItem( last );
    } else {
      KManageSieve::SieveJob * job = KManageSieve::SieveJob::list( u );
      connect( job, SIGNAL(item(KManageSieve::SieveJob*,QString,bool)),
               this, SLOT(slotItem(KManageSieve::SieveJob*,QString,bool)) );
      connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
               this, SLOT(slotResult(KManageSieve::SieveJob*,bool,QString,bool)) );
      mJobs.insert( job, last );
      mUrls.insert( last, u );
    }
  }
  slotUpdateButtons();
}

void ManageSieveScriptsDialog::slotResult( KManageSieve::SieveJob * job, bool success, const QString &, bool )
{
  QTreeWidgetItem * parent = mJobs[job];
  if ( !parent )
    return;

  mJobs.remove( job );

  mListView->expandItem( parent );

  if ( success )
    return;

  QTreeWidgetItem * item =
      new QTreeWidgetItem( parent );
  item->setText( 0, i18n( "Failed to fetch the list of scripts" ) );
  item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
}

void ManageSieveScriptsDialog::slotItem( KManageSieve::SieveJob * job, const QString & filename, bool isActive )
{
  QTreeWidgetItem * parent = mJobs[job];
  if ( !parent )
    return;
  QTreeWidgetItem* item = new QTreeWidgetItem( parent );
  addRadioButton( item, filename );
  if ( isActive ) {
    setRadioButtonState( item, true );
    mSelectedItems[parent] = item;
  }
}

void ManageSieveScriptsDialog::slotContextMenuRequested( QTreeWidgetItem *item, QPoint p )
{
  if ( !item )
    return;
  if ( !item->parent() && !mUrls.count( item ))
    return;
  QMenu menu;
  if ( isFileNameItem( item ) ) {
    // script items:
    menu.addAction( i18n( "Delete Script" ), this, SLOT(slotDeleteScript()) );
    menu.addAction( i18n( "Edit Script..." ), this, SLOT(slotEditScript()) );
    if ( isRadioButtonChecked( item ) )
      menu.addAction( i18n( "Deactivate Script" ), this, SLOT(slotDeactivateScript()) );
  } else if ( !item->parent() ) {
    // top-levels:
    menu.addAction( i18n( "New Script..." ), this, SLOT(slotNewScript()) );
  }
  if ( !menu.actions().isEmpty() )
    menu.exec( p );
}

void ManageSieveScriptsDialog::slotDeactivateScript()
{
  QTreeWidgetItem * item = mListView->currentItem();
  if ( !isFileNameItem( item ) )
    return;
  QTreeWidgetItem *parent = item->parent();
  if ( isRadioButtonChecked( item ) ) {
    mSelectedItems[parent] = item;
    changeActiveScript( parent, false );
  }
}

void ManageSieveScriptsDialog::slotSelectionChanged()
{
  QTreeWidgetItem * item = mListView->currentItem();
  if ( !isFileNameItem( item ) )
    return;
  QTreeWidgetItem *parent = item->parent();
  if ( isRadioButtonChecked( item ) && mSelectedItems[parent] != item ) {
    mSelectedItems[parent] = item;
    changeActiveScript( parent, true );
  }
}

void ManageSieveScriptsDialog::changeActiveScript( QTreeWidgetItem * item, bool activate )
{
  if ( !item )
    return;
  if ( !mUrls.count( item ) )
    return;
  if ( !mSelectedItems.count( item ) )
    return;
  KUrl u = mUrls[item];
  if ( u.isEmpty() )
    return;
  QTreeWidgetItem* selected = mSelectedItems[item];
  if ( !selected )
    return;
  u.setFileName( itemText( selected ) );

  KManageSieve::SieveJob * job;
  if ( activate )
    job = KManageSieve::SieveJob::activate( u );
  else
    job = KManageSieve::SieveJob::deactivate( u );
  connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
           this, SLOT(slotRefresh()) );
}

void ManageSieveScriptsDialog::addRadioButton( QTreeWidgetItem *item, const QString &text )
{
  Q_ASSERT( item && item->parent() );
  Q_ASSERT( !mListView->itemWidget( item, 0 ) );

  // Create the radio button and set it as item widget
  ItemRadioButton *radioButton = new ItemRadioButton( item );
  radioButton->setAutoExclusive( false );
  radioButton->setText( text );
  mListView->setItemWidget( item, 0, radioButton );
  connect( radioButton, SIGNAL(toggled(bool)),
           this, SLOT(slotSelectionChanged()) );

  // Add the radio button to the button group
  QTreeWidgetItem *parent = item->parent();
  QButtonGroup *buttonGroup = mButtonGroups.value( parent );
  if ( !buttonGroup ) {
    buttonGroup = new QButtonGroup();
    mButtonGroups.insert( parent, buttonGroup );
  }
  buttonGroup->addButton( radioButton );
}

void ManageSieveScriptsDialog::setRadioButtonState( QTreeWidgetItem *item, bool checked )
{
  Q_ASSERT( item && item->parent() );

  ItemRadioButton *radioButton = dynamic_cast<ItemRadioButton*>( mListView->itemWidget( item, 0 ) );
  Q_ASSERT( radioButton );
  radioButton->setChecked( checked );
}


bool ManageSieveScriptsDialog::isRadioButtonChecked( QTreeWidgetItem *item ) const
{
  Q_ASSERT( item && item->parent() );

  ItemRadioButton *radioButton = dynamic_cast<ItemRadioButton*>( mListView->itemWidget( item, 0 ) );
  Q_ASSERT( radioButton );
  return radioButton->isChecked();
}

QString ManageSieveScriptsDialog::itemText( QTreeWidgetItem *item ) const
{
  Q_ASSERT( item && item->parent() );

  ItemRadioButton *radioButton = dynamic_cast<ItemRadioButton*>( mListView->itemWidget( item, 0 ) );
  Q_ASSERT( radioButton );
  return radioButton->text().remove( '&' );
}

bool ManageSieveScriptsDialog::isFileNameItem( QTreeWidgetItem *item ) const
{
   if ( !item || !item->parent() )
     return false;

  ItemRadioButton *radioButton = dynamic_cast<ItemRadioButton*>( mListView->itemWidget( item, 0 ) );
  return ( radioButton != 0 );
}

void ManageSieveScriptsDialog::clear()
{
  killAllJobs();
  mSelectedItems.clear();
  qDeleteAll( mButtonGroups );
  mButtonGroups.clear();
  mUrls.clear();
  ItemRadioButton::setTreeWidgetIsBeingCleared( true );
  mListView->clear();
  ItemRadioButton::setTreeWidgetIsBeingCleared( false );
}

void ManageSieveScriptsDialog::slotDoubleClicked( QTreeWidgetItem * item )
{
  if ( !isFileNameItem( item ) )
    return;

  slotEditScript();
}

void ManageSieveScriptsDialog::slotDeleteScript()
{
  QTreeWidgetItem * currentItem =  mListView->currentItem();
  if ( !isFileNameItem( currentItem ) )
    return;

  QTreeWidgetItem *parent = currentItem->parent();
  if ( !parent )
    return;

  if ( !mUrls.count( parent ) )
    return;

  KUrl u = mUrls[parent];
  if ( u.isEmpty() )
    return;

  u.setFileName( itemText( currentItem ) );

  if ( KMessageBox::warningContinueCancel( this, i18n( "Really delete script \"%1\" from the server?", u.fileName() ),
                                   i18n( "Delete Sieve Script Confirmation" ),
                                   KStandardGuiItem::del() )
       != KMessageBox::Continue )
    return;
  KManageSieve::SieveJob * job = KManageSieve::SieveJob::del( u );
  connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
           this, SLOT(slotRefresh()) );
}

void ManageSieveScriptsDialog::slotEditScript()
{
  QTreeWidgetItem *currentItem = mListView->currentItem();
  if ( !isFileNameItem( currentItem ) )
    return;
  QTreeWidgetItem* parent = currentItem->parent();
  if ( !mUrls.count( parent ) )
    return;
  KUrl url = mUrls[parent];
  if ( url.isEmpty() )
    return;
  url.setFileName( itemText( currentItem ) );
  mCurrentURL = url;
  KManageSieve::SieveJob * job = KManageSieve::SieveJob::get( url );
  connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
           this, SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)) );
}

void ManageSieveScriptsDialog::slotNewScript()
{
  QTreeWidgetItem *currentItem = mListView->currentItem();
  if ( !currentItem )
    return;
  if ( currentItem->parent() )
    currentItem = currentItem->parent();
  if ( !currentItem )
    return;

  if ( !mUrls.count( currentItem ) )
    return;

  KUrl u = mUrls[currentItem];
  if ( u.isEmpty() )
    return;

  bool ok = false;
  const QString name = KInputDialog::getText( i18n( "New Sieve Script" ),
                                              i18n( "Please enter a name for the new Sieve script:" ),
                                              i18n( "unnamed" ), &ok, this );
  if ( !ok || name.isEmpty() )
    return;

  u.setFileName( name );


  QButtonGroup *buttonGroup = mButtonGroups.value( currentItem );

  if ( buttonGroup )
  {
    QList<QAbstractButton *> group = buttonGroup->buttons();
    for ( int i = 0; i < group.count(); ++i )
    {
      if ( group.at( i )->text().replace( "&","" ) == name ) {
        KMessageBox::error( this, i18n( "Script name already used \"%1\".", name ), i18n( "New Script" ) );
        return;
      }
    }
  }
  
  QTreeWidgetItem *newItem =
    new QTreeWidgetItem( currentItem );
  addRadioButton( newItem, name );
  mCurrentURL = u;
  slotGetResult( 0, true, QString(), false );
}

SieveEditor::SieveEditor( QWidget * parent )
  : KDialog( parent )
{
  setCaption( i18n( "Edit Sieve Script" ) );
  setButtons( Ok|Cancel|User1|User2|User3 );
  setButtonText( User1, i18n( "Check Syntax" ) );
  setButtonGuiItem( User2, KStandardGuiItem::saveAs() );
  setButtonText( User3, i18n( "Import..." ) );
  setDefaultButton( Ok );
  setModal( true );
  QSplitter *splitter = new QSplitter(this);
  splitter->setOrientation( Qt::Vertical );
  setMainWidget( splitter );
  QList<int> size;
  size << 400 << 100;
  mTextEdit = new SieveTextEdit( splitter );
  mDebugTextEdit = new QTextEdit;
  mDebugTextEdit->setReadOnly( true );
  splitter->addWidget( mTextEdit );
  splitter->addWidget( mDebugTextEdit );
  splitter->setSizes( size );
  connect( mTextEdit, SIGNAL(textChanged()), SLOT(slotTextChanged()) );
  connect( this, SIGNAL(user2Clicked()), SLOT(slotSaveAs()) );
  connect( this, SIGNAL(user3Clicked()), SLOT(slotImport()) );

  resize( 640,480);
}

SieveEditor::~SieveEditor()
{
}

void SieveEditor::slotSaveAs()
{
  KUrl url;
  QPointer<KFileDialog> fdlg( new KFileDialog( url, QString(), this) );

  fdlg->setMode( KFile::File );
  fdlg->setOperationMode( KFileDialog::Saving );
  if ( fdlg->exec() == QDialog::Accepted && fdlg )
  {
    const QString fileName = fdlg->selectedFile();
    if ( !saveToFile( fileName ) )
    {
      KMessageBox::error( this,
                          i18n( "Could not write the file %1:\n"
                                "\"%2\" is the detailed error description.",
                                fileName,
                                QString::fromLocal8Bit( strerror( errno ) ) ),
                          i18n( "Sieve Editor Error" ) );
    }
  }
  delete fdlg;
 
}

bool SieveEditor::saveToFile( const QString&filename )
{
  QFile file( filename );
  if ( !file.open( QIODevice::WriteOnly|QIODevice::Text ) )
    return false;
  QTextStream out(&file);
  out << mTextEdit->toPlainText();
  return true;
}

void SieveEditor::slotImport()
{
  if ( !mTextEdit->toPlainText().isEmpty() )
  {
    if ( KMessageBox::warningYesNo(this, i18n( "You will overwrite script. Do you want to continue?" ), i18n( "Import Script" ) ) == KMessageBox::No )
      return;
  }
  KUrl url;
  QPointer<KFileDialog> fdlg( new KFileDialog( url, QString(), this) );

  fdlg->setMode( KFile::File );
  fdlg->setOperationMode( KFileDialog::Opening );
  if ( fdlg->exec() == QDialog::Accepted && fdlg )
  {
    const QString fileName = fdlg->selectedFile();
    if ( !loadFromFile( fileName ) )
    {
      KMessageBox::error( this,
                          i18n( "Could not load the file %1:\n"
                                "\"%2\" is the detailed error description.",
                                fileName,
                                QString::fromLocal8Bit( strerror( errno ) ) ),
                          i18n( "Sieve Editor Error" ) );
    }
  }
  delete fdlg;
}

bool SieveEditor::loadFromFile( const QString& filename )
{
  QFile file( filename );
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QTextStream in(&file);
  QString line = in.readLine();
  QString scriptText;
  while (!line.isNull()) {
    if ( scriptText.isEmpty() )
      scriptText = line;
    else
      scriptText += QLatin1String( "\n" ) + line;
    line = in.readLine();
  }
  mTextEdit->setPlainText( scriptText );
  return true;
}

void SieveEditor::slotTextChanged()
{
  const bool enabled = !script().isEmpty();
  enableButton( User1, enabled );
  enableButtonOk( enabled );
}

void ManageSieveScriptsDialog::slotGetResult( KManageSieve::SieveJob *, bool success, const QString & script, bool isActive )
{
  if ( !success )
    return;

  if ( mSieveEditor )
    return;

  mSieveEditor = new SieveEditor( this );
  mSieveEditor->setScript( script );
  connect( mSieveEditor, SIGNAL(okClicked()), this, SLOT(slotSieveEditorOkClicked()) );
  connect( mSieveEditor, SIGNAL(cancelClicked()), this, SLOT(slotSieveEditorCancelClicked()) );
  connect( mSieveEditor, SIGNAL(user1Clicked()), this, SLOT(slotSieveEditorCheckSyntaxClicked()) );
  mSieveEditor->show();
  mWasActive = isActive;
}

void ManageSieveScriptsDialog::slotSieveEditorCheckSyntaxClicked()
{
  if ( !mSieveEditor )
    return;
  KManageSieve::SieveJob * job = KManageSieve::SieveJob::put( mCurrentURL,mSieveEditor->script(), mWasActive, mWasActive );
  job->setInteractive( false );
  connect( job, SIGNAL(errorMessage(KManageSieve::SieveJob*,bool,QString)),
           this, SLOT(slotPutResultDebug(KManageSieve::SieveJob*,bool,QString)) );
}

void ManageSieveScriptsDialog::slotSieveEditorOkClicked()
{
  if ( !mSieveEditor )
    return;
  KManageSieve::SieveJob * job = KManageSieve::SieveJob::put( mCurrentURL,mSieveEditor->script(), mWasActive, mWasActive );
  connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
           this, SLOT(slotPutResult(KManageSieve::SieveJob*,bool)) );
}

void ManageSieveScriptsDialog::slotSieveEditorCancelClicked()
{
  mSieveEditor->deleteLater(); mSieveEditor = 0;
  mCurrentURL = KUrl();
  slotRefresh();
}

void ManageSieveScriptsDialog::slotPutResultDebug(KManageSieve::SieveJob*,bool success ,const QString& errorMsg)
{
  if ( success ) {
    addOkMessage( i18n( "No error found." ) );
  } else {
    if ( errorMsg.isEmpty() )
      addFailedMessage( i18n( "Error unknown." ) );
    else
      addFailedMessage( errorMsg );
  }
}

void ManageSieveScriptsDialog::slotPutResult( KManageSieve::SieveJob *, bool success )
{
  if ( success ) {
    KMessageBox::information( this, i18n( "The Sieve script was successfully uploaded." ),
                              i18n( "Sieve Script Upload" ) );
    mSieveEditor->deleteLater(); mSieveEditor = 0;
    mCurrentURL = KUrl();
  } else {
    mSieveEditor->show();
  }
}

void ManageSieveScriptsDialog::addFailedMessage( const QString & err )
{
    addMessageEntry( err,QColor( Qt::darkRed ) );
}

void ManageSieveScriptsDialog::addOkMessage( const QString & err )
{
    addMessageEntry( err, QColor( Qt::darkGreen ) );
}

void ManageSieveScriptsDialog::addMessageEntry( const QString & errorMsg, const QColor& color )
{
  mSieveEditor->setDebugColor( color );
  mSieveEditor->setDebugScript( errorMsg );
}



#include "managesievescriptsdialog.moc"
#include "managesievescriptsdialog_p.moc"
