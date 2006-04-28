/*******************************************************************************
**
** Filename   : newfolderdialog.cpp
** Created on : 30 January, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : adam@kde.org
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
*******************************************************************************/

#include "config.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qtooltip.h>

#include <qregexp.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <klocale.h>
#include <kdialogbase.h>
#include <kmessagebox.h>

#include "newfolderdialog.h"
#include "kmfolder.h"
#include "folderstorage.h"
#include "kmfolderimap.h"
#include "kmfoldercachedimap.h"
#include "kmfoldermgr.h"
#include "kmfolderdir.h"
#include "folderstorage.h"
#include "kmailicalifaceimpl.h"
#include "kmacctimap.h"
#include "kmacctcachedimap.h"

using namespace KMail;

NewFolderDialog::NewFolderDialog( QWidget* parent, KMFolder *folder )
    : KDialogBase( parent, "new_folder_dialog", false, i18n( "New Folder" ),
                   KDialogBase::Ok|KDialogBase::Cancel,
                   KDialogBase::Ok, true ),
      mFolder( folder )
{
  setAttribute( Qt::WA_DeleteOnClose );
  if ( folder ) {
    setCaption( i18n("New Subfolder of %1", folder->prettyURL() ) );
  }
  QWidget* privateLayoutWidget = new QWidget( this );
  privateLayoutWidget->setObjectName( "mTopLevelLayout" );
  privateLayoutWidget->setGeometry( QRect( 10, 10, 260, 80 ) );
  setMainWidget( privateLayoutWidget );
  mTopLevelLayout = new QVBoxLayout( privateLayoutWidget );
  mTopLevelLayout->setObjectName( "mTopLevelLayout" );
  mTopLevelLayout->setSpacing( spacingHint() );
  mTopLevelLayout->setMargin( 0 );

  mNameHBox = new QHBoxLayout();
  mNameHBox->setSpacing( 6 );
  mNameHBox->setMargin( 0 );
  mNameHBox->setObjectName( "mNameHBox" );

  mNameLabel = new QLabel( privateLayoutWidget );
  mNameLabel->setObjectName( "mNameLabel" );
  mNameLabel->setText( i18n( "&Name:" ) );
  mNameHBox->addWidget( mNameLabel );

  mNameLineEdit = new QLineEdit( privateLayoutWidget );
  mNameLineEdit->setObjectName( "mNameLineEdit" );
  mNameLabel->setBuddy( mNameLineEdit );
  mNameLineEdit->setWhatsThis( i18n( "Enter a name for the new folder." ) );
  mNameLineEdit->setFocus();
  mNameHBox->addWidget( mNameLineEdit );
  mTopLevelLayout->addLayout( mNameHBox );
  connect( mNameLineEdit, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotFolderNameChanged( const QString & ) ) );

  if ( !mFolder ||
      ( mFolder->folderType() != KMFolderTypeImap &&
        mFolder->folderType() != KMFolderTypeCachedImap ) ) {
    mFormatHBox = new QHBoxLayout();
    mFormatHBox->setSpacing( 6 );
    mFormatHBox->setMargin( 0 );
    mFormatHBox->setObjectName( "mFormatHBox" );
    mMailboxFormatLabel = new QLabel( privateLayoutWidget );
    mMailboxFormatLabel->setObjectName( "mMailboxFormatLabel" );
    mMailboxFormatLabel->setText( i18n( "Mailbox &format:" ) );
    mFormatHBox->addWidget( mMailboxFormatLabel );

    mFormatComboBox = new QComboBox( privateLayoutWidget );
    mFormatComboBox->setEditable( false );
    mFormatComboBox->setObjectName( "mFormatComboBox" );
    mMailboxFormatLabel->setBuddy( mFormatComboBox );
    mFormatComboBox->setWhatsThis( i18n( "Select whether you want to store the messages in this folder as one file per  message (maildir) or as one big file (mbox). KMail uses maildir by default and this only needs to be changed in rare circumstances. If you are unsure, leave this option as-is." ) );

    mFormatComboBox->insertItem(0,"mbox");
    mFormatComboBox->insertItem(1,"maildir");
    // does the below make any sense?
    //  mFormatComboBox->insertItem(2, "search");
    {
      KConfig *config = KMKernel::config();
      KConfigGroup group(config, "General");
      int type = group.readEntry("default-mailbox-format", 1 );
      if ( type < 0 || type > 1 ) type = 1;
      mFormatComboBox->setCurrentIndex( type );
    }
    mFormatHBox->addWidget( mFormatComboBox );
    mTopLevelLayout->addLayout( mFormatHBox );
  }

  // --- contents -----
  if ( kmkernel->iCalIface().isEnabled() ) {
    mContentsHBox = new QHBoxLayout();
    mContentsHBox->setSpacing( 6 );
    mContentsHBox->setMargin( 0 );
    mContentsHBox->setObjectName( "mContentsHBox" );

    mContentsLabel = new QLabel( privateLayoutWidget );
    mContentsLabel->setObjectName( "mContentsLabel" );
    mContentsLabel->setText( i18n( "Folder &contains:" ) );
    mContentsHBox->addWidget( mContentsLabel );

    mContentsComboBox = new QComboBox(  privateLayoutWidget );
    mContentsComboBox->setEditable( false );
    mContentsComboBox->setObjectName( "mContentsComboBox" );
    mContentsLabel->setBuddy( mContentsComboBox );
    mContentsComboBox->setWhatsThis( i18n( "Select whether you want the new folder to be used for mail storage of for storage of groupware items such as tasks or notes. The default is mail. If you are unsure, leave this option as-is." ) );
    mContentsComboBox->addItem( i18n( "Mail" ) );
    mContentsComboBox->addItem( i18n( "Calendar" ) );
    mContentsComboBox->addItem( i18n( "Contacts" ) );
    mContentsComboBox->addItem( i18n( "Notes" ) );
    mContentsComboBox->addItem( i18n( "Tasks" ) );
    mContentsComboBox->addItem( i18n( "Journal" ) );
    if ( mFolder ) // inherit contents type from papa
      mContentsComboBox->setCurrentIndex( mFolder->storage()->contentsType() );
    mContentsHBox->addWidget( mContentsComboBox );
    mTopLevelLayout->addLayout( mContentsHBox );
  }

  if ( mFolder &&
      ( mFolder->folderType() == KMFolderTypeImap ||
        mFolder->folderType() == KMFolderTypeCachedImap ) ) {
    bool rootFolder = false;
    QStringList namespaces;
    if ( mFolder->folderType() == KMFolderTypeImap ) {
      ImapAccountBase* ai = static_cast<KMFolderImap*>(mFolder->storage())->account();
      if ( mFolder->storage() == ai->rootFolder() ) {
        rootFolder = true;
        namespaces = ai->namespaces()[ImapAccountBase::PersonalNS];
      }
    }
    if ( mFolder->folderType() == KMFolderTypeCachedImap ) {
      ImapAccountBase* ai = static_cast<KMFolderCachedImap*>(mFolder->storage())->account();
      if ( mFolder->storage() == ai->rootFolder() ) {
        rootFolder = true;
        namespaces = ai->namespaces()[ImapAccountBase::PersonalNS];
      }
    }
    if ( rootFolder && namespaces.count() > 1 ) {
      mNamespacesHBox = new QHBoxLayout();
      mNamespacesHBox->setSpacing( 6 );
      mNamespacesHBox->setMargin( 0 );
      mNamespacesHBox->setObjectName( "mNamespaceHBox" );

      mNamespacesLabel = new QLabel( privateLayoutWidget );
      mNamespacesLabel->setObjectName( "mNamespacesLabel" );
      mNamespacesLabel->setText( i18n( "Namespace for &folder:" ) );
      mNamespacesHBox->addWidget( mNamespacesLabel );

      mNamespacesComboBox = new QComboBox( privateLayoutWidget );
      mNamespacesComboBox->setEditable( false );
      mNamespacesComboBox->setObjectName( "mNamespacesComboBox" );
      mNamespacesLabel->setBuddy( mNamespacesComboBox );
      mNamespacesComboBox->setWhatsThis( i18n( "Select the personal namespace the folder should be created in." ) );
      mNamespacesComboBox->addItems( namespaces );
      mNamespacesHBox->addWidget( mNamespacesComboBox );
      mTopLevelLayout->addLayout( mNamespacesHBox );
    } else {
      mNamespacesComboBox = 0;
    }
  }

  resize( QSize(282, 108).expandedTo(minimumSizeHint()) );
#warning Port me!
//  clearWState( WState_Polished );
  slotFolderNameChanged( mNameLineEdit->text());
}

void NewFolderDialog::slotFolderNameChanged( const QString & _text)
{
  enableButtonOK( !_text.isEmpty() );
}

void NewFolderDialog::slotOk()
{
  const QString &fldName = mNameLineEdit->text();
  if ( fldName.isEmpty() ) {
     KMessageBox::error( this, i18n("Please specify a name for the new folder."),
        i18n( "No Name Specified" ) );
     return;
  }
  if ( fldName.contains( '/' ) &&
    ( !mFolder
      || mFolder->folderType() == KMFolderTypeImap
      || mFolder->folderType() == KMFolderTypeCachedImap ) ) {
    KMessageBox::error( this, i18n( "Folder names cannot contain the / (slash) character; please choose another folder name." ) );
    return;
  }

  if ( fldName.startsWith( "." ) ) {
    KMessageBox::error( this, i18n( "Folder names cannot start with a . (dot) character; please choose another folder name." ) );
    return;
  } else if ( mFolder &&
      ( mFolder->folderType() == KMFolderTypeImap ||
        mFolder->folderType() == KMFolderTypeCachedImap ) ) {
    QString delimiter;
    if ( mFolder->folderType() == KMFolderTypeImap ) {
      KMAcctImap* ai = static_cast<KMFolderImap*>( mFolder->storage() )->account();
      delimiter = ai->delimiterForFolder( mFolder->storage() );
    } else {
      KMAcctCachedImap* ai = static_cast<KMFolderCachedImap*>( mFolder->storage() )->account();
      delimiter = ai->delimiterForFolder( mFolder->storage() );
    }
    if ( !delimiter.isEmpty() && fldName.contains( delimiter ) ) {
      KMessageBox::error( this, i18n( "Your IMAP server does not allow the character '%1'; please choose another folder name.", delimiter ) );
      return;
    }
  }

  // default parent is Top Level local folders
  KMFolderDir * selectedFolderDir = &(kmkernel->folderMgr()->dir());
  // we got a parent, let's use that
  if ( mFolder )
    selectedFolderDir = mFolder->createChildFolder();

  // check if the folder already exists
  if( selectedFolderDir->hasNamedFolder( fldName )
      && ( !( mFolder
          && ( selectedFolderDir == mFolder->parent() )
          && ( mFolder->storage()->name() == fldName ) ) ) )
  {
    const QString message = i18n( "<qt>Failed to create folder <b>%1</b>, folder already exists.</qt>" , fldName);
    KMessageBox::error( this, message );
    return;
  }

  /* Ok, obvious errors caught, let's try creating it for real. */
  const QString message = i18n( "<qt>Failed to create folder <b>%1</b>."
            "</qt> " , fldName);
  bool success = false;
  KMFolder *newFolder = 0;

   if ( mFolder && mFolder->folderType() == KMFolderTypeImap ) {
    KMFolderImap* selectedStorage = static_cast<KMFolderImap*>( mFolder->storage() );
    KMAcctImap *anAccount = selectedStorage->account();
    // check if a connection is available BEFORE creating the folder
    if (anAccount->makeConnection() == ImapAccountBase::Connected) {
      newFolder = kmkernel->imapFolderMgr()->createFolder( fldName, false, KMFolderTypeImap, selectedFolderDir );
      if ( newFolder ) {
        QString imapPath, parent;
        if ( mNamespacesComboBox ) {
          // create folder with namespace
          parent = anAccount->addPathToNamespace( mNamespacesComboBox->currentText() );
          imapPath = anAccount->createImapPath( parent, fldName );
        }
        KMFolderImap* newStorage = static_cast<KMFolderImap*>( newFolder->storage() );
        selectedStorage->createFolder(fldName, parent); // create it on the server
        newStorage->initializeFrom( selectedStorage, imapPath, QString() );
        static_cast<KMFolderImap*>(mFolder->storage())->setAccount( selectedStorage->account() );
        success = true;
      }
    }
  } else if ( mFolder && mFolder->folderType() == KMFolderTypeCachedImap ) {
    newFolder = kmkernel->dimapFolderMgr()->createFolder( fldName, false, KMFolderTypeCachedImap, selectedFolderDir );
    if ( newFolder ) {
      KMFolderCachedImap* selectedStorage = static_cast<KMFolderCachedImap*>( mFolder->storage() );
      KMFolderCachedImap* newStorage = static_cast<KMFolderCachedImap*>( newFolder->storage() );
      newStorage->initializeFrom( selectedStorage );
      if ( mNamespacesComboBox ) {
        // create folder with namespace
        QString path = selectedStorage->account()->createImapPath(
            mNamespacesComboBox->currentText(), fldName );
        newStorage->setImapPathForCreation( path );
      }
      success = true;
    }
  } else {
    // local folder
    if (mFormatComboBox->currentIndex() == 1)
      newFolder = kmkernel->folderMgr()->createFolder(fldName, false, KMFolderTypeMaildir, selectedFolderDir );
    else
      newFolder = kmkernel->folderMgr()->createFolder(fldName, false, KMFolderTypeMbox, selectedFolderDir );
    if ( newFolder )
      success = true;
  }
  if ( !success ) {
    KMessageBox::error( this, message );
    return;
  }

  // Set type field
  if ( kmkernel->iCalIface().isEnabled() && mContentsComboBox ) {
    KMail::FolderContentsType type =
      static_cast<KMail::FolderContentsType>( mContentsComboBox->currentIndex() );
    newFolder->storage()->setContentsType( type );
    newFolder->storage()->writeConfig(); // connected slots will read it
  }
  KDialogBase::slotOk();
}

#include "newfolderdialog.moc"
