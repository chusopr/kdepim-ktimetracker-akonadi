/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

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
//#define MESSAGEVIEWER_READER_HTML_DEBUG 1
#include "viewer_p.h"
#include "viewer.h"
#include "objecttreeemptysource.h"
#include "objecttreeviewersource.h"

#ifdef MESSAGEVIEWER_READER_HTML_DEBUG
#include "filehtmlwriter.h"
#include "teehtmlwriter.h"
#endif
#include <unistd.h> // link()
#include <errno.h>
//KDE includes
#include <KAction>
#include <KActionCollection>
#include <KActionMenu>
#include <kascii.h>
#include <KCharsets>
#include <kcursorsaver.h>
#include <KFileDialog>
#include <KGuiItem>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <KMenu>
#include <KMessageBox>
#include <KMimeType>
#include <KMimeTypeChooser>
#include <KMimeTypeTrader>
#include <KRun>
#include <KSelectAction>
#include <KSharedConfigPtr>
#include <KStandardDirs>
#include <KStandardGuiItem>
#include <KTemporaryFile>
#include <KToggleAction>

#include <KIO/NetAccess>
#include <KABC/Addressee>
#include <KABC/VCardConverter>
#include <KPIMUtils/Email>
#include <Akonadi/ItemModifyJob>

#include <kpimutils/kfileio.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/cryptobackend.h>

#include <QWebElement>

#include <mailtransport/errorattribute.h>

//Qt includes
#include <QClipboard>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QImageReader>
#include <QItemSelectionModel>
#include <QSignalMapper>
#include <QSplitter>
#include <QStyle>
#include <QTextDocument>
#include <QTreeView>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QScrollArea>
#include <QPrinter>
#include <QPrintDialog>
#include <QHeaderView>

//libkdepim
#include "libkdepim/broadcaststatus.h"
#include <messagecore/attachmentpropertiesdialog.h>

#include <akonadi/collection.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/kmime/specialmailcollections.h>
#include <kleo/specialjob.h>

#include "chiasmuskeyselector.h"
#include "autoqpointer.h"


//own includes
#include "attachmentdialog.h"
#include "attachmentstrategy.h"
#include "csshelper.h"
#include "editorwatcher.h"
#include "globalsettings.h"
#include "headerstyle.h"
#include "headerstrategy.h"
#include "htmlstatusbar.h"
#include "webkitparthtmlwriter.h"
#include <kparts/browserextension.h>
#include "mailsourceviewer.h"
#include "mimetreemodel.h"
#include "nodehelper.h"
#include "objecttreeparser.h"
#include "urlhandlermanager.h"
#include "util.h"
#include "vcardviewer.h"
#include "mailwebview.h"
#include "findbar/findbar.h"

#include "interfaces/bodypart.h"
#include "interfaces/htmlwriter.h"

#include <messagecore/stringutil.h>

#include <kio/jobuidelegate.h>

#include <gpgme++/error.h>
#include <messagecore/nodehelper.h>
#include "messagecore/globalsettings.h"
#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>
#include <Akonadi/CollectionFetchJob>
#include <akonadi/collectionfetchscope.h>

using namespace MailTransport;
using namespace MessageViewer;
using namespace MessageCore;

const int ViewerPrivate::delay = 150;

ViewerPrivate::ViewerPrivate( Viewer *aParent, QWidget *mainWindow,
                              KActionCollection *actionCollection )
  : QObject(aParent),
    mNodeHelper( new NodeHelper ),
    mFindBar( 0 ),
    mAttachmentStrategy( 0 ),
    mHeaderStrategy( 0 ),
    mHeaderStyle( 0 ),
    mUpdateReaderWinTimer( 0 ),
    mResizeTimer( 0 ),
    mOldGlobalOverrideEncoding( "---" ), // init with dummy value
    mMsgDisplay( true ),
    mCSSHelper( 0 ),
    mMainWindow( mainWindow ),
    mActionCollection( actionCollection ),
    mCopyAction( 0 ),
    mCopyURLAction( 0 ),
    mUrlOpenAction( 0 ),
    mSelectAllAction( 0 ),
    mScrollUpAction( 0 ),
    mScrollDownAction( 0 ),
    mScrollUpMoreAction( 0 ),
    mScrollDownMoreAction( 0 ),
    mSelectEncodingAction( 0 ),
    mToggleFixFontAction( 0 ),
    mToggleDisplayModeAction( 0 ),
    mToggleMimePartTreeAction( 0 ),
    mCanStartDrag( false ),
    mHtmlWriter( 0 ),
    mSavedRelativePosition( 0 ),
    mDecrytMessageOverwrite( false ),
    mShowSignatureDetails( false ),
    mShowAttachmentQuicklist( true ),
    mShowRawToltecMail( false ),
    mRecursionCountForDisplayMessage( 0 ),
    mCurrentContent( 0 ),
    mJob( 0 ),
    q( aParent )
{
  if ( !mainWindow )
    mainWindow = aParent;

  mHtmlOverride = false;
  mHtmlLoadExtOverride = false;
  mHtmlLoadExternal = false;

  mUpdateReaderWinTimer.setObjectName( "mUpdateReaderWinTimer" );
  mResizeTimer.setObjectName( "mResizeTimer" );

  mExternalWindow  = ( aParent == mainWindow );
  mSplitterSizes << 180 << 100;
  mPrinting = false;

  createWidgets();
  createActions();
  initHtmlWidget();
  readConfig();

  mLevelQuote = GlobalSettings::self()->collapseQuoteLevelSpin() - 1;

  mResizeTimer.setSingleShot( true );
  connect( &mResizeTimer, SIGNAL(timeout()),
           this, SLOT(slotDelayedResize()) );

  mUpdateReaderWinTimer.setSingleShot( true );
  connect( &mUpdateReaderWinTimer, SIGNAL(timeout()),
           this, SLOT(updateReaderWin()) );

  connect( mColorBar, SIGNAL( clicked() ),
           this, SLOT( slotToggleHtmlMode() ) );

  // FIXME: Don't use the full payload here when attachment loading on demand is used, just
  //        like in KMMainWidget::slotMessageActivated().
  Akonadi::ItemFetchScope fs;
  fs.fetchFullPayload();
  mMonitor.setItemFetchScope( fs );
  connect( &mMonitor, SIGNAL( itemChanged( Akonadi::Item,QSet<QByteArray> ) ),
           this, SLOT( slotItemChanged( Akonadi::Item, QSet<QByteArray> ) ) );
}

ViewerPrivate::~ViewerPrivate()
{
  GlobalSettings::self()->writeConfig();
  delete mHtmlWriter; mHtmlWriter = 0;
  delete mViewer; mViewer = 0;
  delete mCSSHelper;
  mNodeHelper->removeTempFiles();
  delete mNodeHelper;
}

//-----------------------------------------------------------------------------
KMime::Content * ViewerPrivate::nodeFromUrl( const KUrl & url )
{
  KMime::Content *node = 0;
  if ( url.isEmpty() )
    return mMessage.get();
  if ( !url.isLocalFile() ) {
    QString path = url.path(KUrl::RemoveTrailingSlash);
    if ( path.contains(':') ) {
      //if the content was not found, it might be in an extra node. Get the index of the extra node (the first part of the url),
      //and use the remaining part as a ContentIndex to find the node insde the extra node
      int i = path.left( path.indexOf(':') ).toInt();
      path = path.mid( path.indexOf(':') + 1 );
      KMime::ContentIndex idx(path);
      QList<KMime::Content*> extras = mNodeHelper->extraContents( mMessage.get() );
      if ( i >= 0 && i < extras.size() ) {
        KMime::Content* c = extras[i];
        node = c->content( idx );
      }
    } else {
      node= mMessage->content( KMime::ContentIndex( path ) );
    }
  } else {
    QString path = url.toLocalFile();
    uint right = path.lastIndexOf( '/' );
    uint left = path.lastIndexOf( '.', right );

    KMime::ContentIndex index(path.mid( left + 1, right - left - 1 ));
    node = mMessage->content( index );
  }
  return node;
}

void ViewerPrivate::openAttachment( KMime::Content* node, const QString & name )
{
  if( !node ) {
    return;
  }

  const bool isEncapsulatedMessage = node->parent() && node->parent()->bodyIsMessage();
  if ( isEncapsulatedMessage ) {

    // the viewer/urlhandlermanager expects that the message (mMessage) it is passed is the root when doing index calculation
    // in urls. Simply passing the result of bodyAsMessage() does not cut it as the resulting pointer is a child in its tree.
    KMime::Message::Ptr m = KMime::Message::Ptr( new KMime::Message );
    m->setContent( node->parent()->bodyAsMessage()->encodedContent() );
    m->parse();
    atmViewMsg( m );
    return;
  }
  // determine the MIME type of the attachment
  KMimeType::Ptr mimetype;
  // prefer the value of the Content-Type header
  mimetype = KMimeType::mimeType( QString::fromLatin1( node->contentType()->mimeType().toLower() ),
                                  KMimeType::ResolveAliases );
  if ( !mimetype.isNull() && mimetype->is( KABC::Addressee::mimeType() ) ) {
    showVCard( node );
    return;
  }

  // special case treatment on mac
  QString atmName = name;
  if ( name.isEmpty() )
    atmName = mNodeHelper->tempFileUrlFromNode( node ).toLocalFile();
  if ( Util::handleUrlOnMac( atmName ) )
    return;

  if ( mimetype.isNull() || mimetype->name() == "application/octet-stream" ) {
    // consider the filename if mimetype can not be found by content-type
    mimetype = KMimeType::findByPath( name, 0, true /* no disk access */  );

  }
  if ( mimetype->name() == "application/octet-stream" ) {
    // consider the attachment's contents if neither the Content-Type header
    // nor the filename give us a clue
    mimetype = KMimeType::findByFileContent( name );
  }

  KService::Ptr offer =
      KMimeTypeTrader::self()->preferredService( mimetype->name(), "Application" );

  const QString filenameText = NodeHelper::fileName( node );

  AttachmentDialog dialog( mMainWindow, filenameText, offer ? offer->name() : QString(),
                           QString::fromLatin1( "askSave_" ) + mimetype->name() );
  const int choice = dialog.exec();

  if ( choice == AttachmentDialog::Save ) {
    Util::saveContents( mMainWindow, KMime::Content::List() << node );
  }
  else if ( choice == AttachmentDialog::Open ) { // Open
    attachmentOpen( node );
  } else if ( choice == AttachmentDialog::OpenWith ) {
    attachmentOpenWith( node );
  } else { // Cancel
    kDebug() << "Canceled opening attachment";
  }

}

bool ViewerPrivate::deleteAttachment(KMime::Content * node, bool showWarning)
{
  if ( !node )
    return true;
  KMime::Content *parent = node->parent();
  if ( !parent )
    return true;

  QList<KMime::Content*> extraNodes = mNodeHelper->extraContents( mMessage.get() );
  if ( extraNodes.contains( node->topLevel() ) ) {
      KMessageBox::error( mMainWindow,
        i18n("Deleting an attachment from an encrypted or old-style mailman message is not supported."),
        i18n("Delete Attachment") );
      return true; //cancelled
  }

  if ( showWarning && KMessageBox::warningContinueCancel( mMainWindow,
       i18n("Deleting an attachment might invalidate any digital signature on this message."),
       i18n("Delete Attachment"), KStandardGuiItem::del(), KStandardGuiItem::cancel(),
       "DeleteAttachmentSignatureWarning" )
     != KMessageBox::Continue ) {
    return false; //cancelled
  }

  delete mMimePartModel->root();
  mMimePartModel->setRoot( 0 ); //don't confuse the model

  parent->removeContent( node, true );
  parent->assemble();

  KMime::Message* modifiedMessage = mNodeHelper->messageWithExtraContent( mMessage.get() );
  mMimePartModel->setRoot( modifiedMessage );

  mMessageItem.setPayloadFromData( modifiedMessage->encodedContent() );
  Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob( mMessageItem );
  connect( job, SIGNAL(result(KJob*)), SLOT(itemModifiedResult(KJob*)) );
  return true;
}


void ViewerPrivate::itemModifiedResult(KJob* job)
{
  if ( job->error() ) {
    kDebug() << "Item update failed!";
  } else {
    setMessageItem( mMessageItem, MessageViewer::Viewer::Force);
  }
}


bool ViewerPrivate::editAttachment( KMime::Content * node, bool showWarning )
{
  //FIXME(Andras) just that I don't forget...handle the case when the user starts editing and switches to another message meantime
  if ( showWarning && KMessageBox::warningContinueCancel( mMainWindow,
        i18n("Modifying an attachment might invalidate any digital signature on this message."),
        i18n("Edit Attachment"), KGuiItem( i18n("Edit"), "document-properties" ), KStandardGuiItem::cancel(),
        "EditAttachmentSignatureWarning" )
        != KMessageBox::Continue ) {
    return false;
  }

  KTemporaryFile file;
  file.setAutoRemove( false );
  if ( !file.open() ) {
    kWarning() << "Edit Attachment: Unable to open temp file.";
    return true;
  }
  file.write( node->decodedContent() );
  file.flush();

  EditorWatcher *watcher =
    new EditorWatcher( KUrl( file.fileName() ), node->contentType()->mimeType(),
                       false, this, mMainWindow );
  mEditorWatchers[ watcher ] = node;

  connect( watcher, SIGNAL(editDone(EditorWatcher*)), SLOT(slotAttachmentEditDone(EditorWatcher*)) );
  if ( !watcher->start() ) {
    QFile::remove( file.fileName() );
  }

  return true;
}

// Checks if the given node has a child node that is a DIV which has an ID attribute
// with the value specified here
static bool hasParentDivWithId( const QWebElement &start, const QString &id )
{
  if ( start.isNull() )
    return false;

  if ( start.tagName().toLower() == "div" ) {
    if ( start.attribute( "id", "" ) == id )
      return true;
  }

  if ( !start.parent().isNull() )
    return hasParentDivWithId( start.parent(), id );
  else return false;
}

void ViewerPrivate::showAttachmentPopup( KMime::Content* node, const QString & name, const QPoint &p )
{
  prepareHandleAttachment( node, name );
  KMenu *menu = new KMenu();
  QAction *action;

  QSignalMapper *attachmentMapper = new QSignalMapper( menu );
  connect( attachmentMapper, SIGNAL( mapped( int ) ),
           this, SLOT( slotHandleAttachment( int ) ) );

  action = menu->addAction(SmallIcon("document-open"),i18nc("to open", "Open"));
  connect( action, SIGNAL( triggered(bool) ), attachmentMapper, SLOT( map() ) );
  attachmentMapper->setMapping( action, Viewer::Open );

  action = menu->addAction(i18n("Open With..."));
  connect( action, SIGNAL( triggered(bool) ), attachmentMapper, SLOT( map() ) );
  attachmentMapper->setMapping( action, Viewer::OpenWith );

  action = menu->addAction(i18nc("to view something", "View") );
  connect( action, SIGNAL( triggered(bool) ), attachmentMapper, SLOT( map() ) );
  attachmentMapper->setMapping( action, Viewer::View );

  const QPoint local = mViewer->page()->view()->mapFromGlobal( p );
  const QWebHitTestResult hit = mViewer->page()->currentFrame()->hitTestContent( local );
  const bool attachmentInHeader = hasParentDivWithId(
      hit.enclosingBlockElement(), "attachmentInjectionPoint" );
  const bool hasScrollbar = mViewer->page()->mainFrame()->scrollBarGeometry( Qt::Vertical ).isValid();
  if ( attachmentInHeader && hasScrollbar ) {
    action = menu->addAction( i18n( "Scroll To" ) );
    connect( action, SIGNAL( triggered(bool) ), attachmentMapper, SLOT( map() ) );
    attachmentMapper->setMapping( action, Viewer::ScrollTo );
  }

  action = menu->addAction(SmallIcon("document-save-as"),i18n("Save As...") );
  connect( action, SIGNAL( triggered(bool) ), attachmentMapper, SLOT( map() ) );
  attachmentMapper->setMapping( action, Viewer::Save );

  action = menu->addAction(SmallIcon("edit-copy"), i18n("Copy") );
  connect( action, SIGNAL( triggered(bool) ), attachmentMapper, SLOT( map() ) );
  attachmentMapper->setMapping( action, Viewer::Copy );

  const bool isEncapsulatedMessage = node->parent() && node->parent()->bodyIsMessage();
  const bool canChange = mMessageItem.isValid() && mMessageItem.parentCollection().isValid() &&
                         ( mMessageItem.parentCollection().rights() != Akonadi::Collection::ReadOnly ) &&
                         !isEncapsulatedMessage;


  if ( GlobalSettings::self()->allowAttachmentEditing() ) {
    action = menu->addAction(SmallIcon("document-properties"), i18n("Edit Attachment") );
    connect( action, SIGNAL(triggered()), attachmentMapper, SLOT(map()) );
    attachmentMapper->setMapping( action, Viewer::Edit );
    action->setEnabled( canChange );
  }
  if ( GlobalSettings::self()->allowAttachmentDeletion() ) {
    action = menu->addAction(SmallIcon("edit-delete"), i18n("Delete Attachment") );
    connect( action, SIGNAL(triggered()), attachmentMapper, SLOT(map()) );
    attachmentMapper->setMapping( action, Viewer::Delete );
    action->setEnabled( canChange );
  }
  if ( name.endsWith( QLatin1String(".xia"), Qt::CaseInsensitive )
       && Kleo::CryptoBackendFactory::instance()->protocol( "Chiasmus" )) {
    action = menu->addAction( i18n( "Decrypt With Chiasmus..." ) );
    connect( action, SIGNAL( triggered(bool) ), attachmentMapper, SLOT( map() ) );
    attachmentMapper->setMapping( action, Viewer::ChiasmusEncrypt );
  }
  action = menu->addAction(i18n("Properties") );
  connect( action, SIGNAL( triggered(bool) ), attachmentMapper, SLOT( map() ) );
  attachmentMapper->setMapping( action, Viewer::Properties );
  menu->exec( p );
  delete menu;
}

void ViewerPrivate::prepareHandleAttachment( KMime::Content *node, const QString& fileName )
{
  mCurrentContent = node;
  mCurrentFileName = fileName;
}

// This function returns the complete data that were in this
// message parts - *after* all encryption has been removed that
// could be removed.
// - This is used to store the message in decrypted form.
void ViewerPrivate::objectTreeToDecryptedMsg( KMime::Content* node,
                                            QByteArray& resultingData,
                                            KMime::Message::Ptr theMessage,
                                            bool weAreReplacingTheRootNode,
                                            int recCount )
{
  kDebug() << "-------------------------------------------------";
  kDebug() << "START" << "(" << recCount << ")";
  if( node ) {

    KMime::Content* curNode = node;
    KMime::Content* dataNode = curNode;
    KMime::Content * child = MessageCore::NodeHelper::firstChild( node );
    const QString type = curNode->contentType()->mediaType().toLower();
    const QString subType = curNode->contentType()->subType().toLower();
    const bool bIsMultipart = type == "multipart";
    bool bKeepPartAsIs = false;

    kDebug() << type << "/" << subType;

    if ( type == "multipart" ) {
      if ( subType == "signed" ) {
        bKeepPartAsIs = true;
      } else if ( subType == "encrypted" ) {
        if ( child ) {
          dataNode = child;
        }
      }
    } else if ( type == "message" ) {
      if ( subType == "rfc822" ) {
        if ( child ) {
          dataNode = child;
        }
      }
    } else if ( type == "application" ) {
      if ( subType == "octet-stream" ) {
        if ( child ) {
          dataNode = child;
        }
      } else if ( subType == "x-pkcs7-signature" ) {
        // note: subtype Pkcs7Signature specifies a signature part
        //       which we do NOT want to remove!
        bKeepPartAsIs = true;
      } else if ( subType == "x-pkcs7-mime" ) {
        // note: subtype Pkcs7Mime can also be signed
        //       and we do NOT want to remove the signature!
        if ( child && mNodeHelper->encryptionState( curNode ) != KMMsgNotEncrypted ) {
          dataNode = child;
        }
      }
    }

    KMime::Content* headerContent = 0;
    if ( !dataNode->head().isEmpty() ) {
      headerContent = dataNode;
    }
    if ( weAreReplacingTheRootNode || !dataNode->parent() ) {
      headerContent = theMessage.get();
    }
    if( dataNode == curNode ) {
      kDebug() << "dataNode == curNode:  Save curNode without replacing it.";

      // A) Store the headers of this part IF curNode is not the root node
      //    AND we are not replacing a node that already *has* replaced
      //    the root node in previous recursion steps of this function...
      if ( !headerContent->head().isEmpty() ) {
        if( dataNode->parent() && !weAreReplacingTheRootNode ) {
          kDebug() << "dataNode is NOT replacing the root node:  Store the headers.";
          resultingData += headerContent->head();
        } else if( weAreReplacingTheRootNode && !dataNode->head().isEmpty() ){
          kDebug() << "dataNode replace the root node:  Do NOT store the headers but change";
          kDebug() << "                                 the Message's headers accordingly.";
          kDebug() << "              old Content-Type =" << theMessage->contentType()->asUnicodeString();
          kDebug() << "              new Content-Type =" << headerContent->contentType()->asUnicodeString();
          theMessage->contentType()->from7BitString( headerContent->contentType()->as7BitString() );
          theMessage->contentTransferEncoding()->from7BitString(
              headerContent->contentTransferEncoding(false)
            ? headerContent->contentTransferEncoding()->as7BitString()
            : "" );
          theMessage->contentDescription()->from7BitString( headerContent->contentDescription()->as7BitString() );
          theMessage->contentDisposition()->from7BitString( headerContent->contentDisposition()->as7BitString() );
          theMessage->assemble();
        }
      }

      if ( bKeepPartAsIs ) {
        resultingData += dataNode->encodedContent();
      } else {

        // B) Store the body of this part.
        if( headerContent && bIsMultipart && !dataNode->contents().isEmpty() )  {
          kDebug() << "is valid Multipart, processing children:";
          QByteArray boundary = headerContent->contentType()->boundary();
          curNode = MessageCore::NodeHelper::firstChild( dataNode );
          // store children of multipart
          while( curNode ) {
            kDebug() << "--boundary";
            if( resultingData.size() &&
                ( '\n' != resultingData.at( resultingData.size()-1 ) ) )
              resultingData += '\n';
            resultingData += '\n';
            resultingData += "--";
            resultingData += boundary;
            resultingData += '\n';
            // note: We are processing a harmless multipart that is *not*
            //       to be replaced by one of it's children, therefor
            //       we set their doStoreHeaders to true.
            objectTreeToDecryptedMsg( curNode,
                                      resultingData,
                                      theMessage,
                                      false,
                                      recCount + 1 );
            curNode = MessageCore::NodeHelper::nextSibling( curNode );
          }
          kDebug() << "--boundary--";
          resultingData += "\n--";
          resultingData += boundary;
          resultingData += "--\n\n";
          kDebug() << "Multipart processing children - DONE";
        } else {
          // store simple part
          kDebug() << "is Simple part or invalid Multipart, storing body data .. DONE";
          resultingData += dataNode->body();
        }
      }
    } else {
      kDebug() << "dataNode != curNode:  Replace curNode by dataNode.";
      bool rootNodeReplaceFlag = weAreReplacingTheRootNode || !curNode->parent();
      if ( rootNodeReplaceFlag ) {
        kDebug() << "                      Root node will be replaced.";
      } else {
        kDebug() << "                      Root node will NOT be replaced.";
      }
      // store special data to replace the current part
      // (e.g. decrypted data or embedded RfC 822 data)
      objectTreeToDecryptedMsg( dataNode,
                                resultingData,
                                theMessage,
                                rootNodeReplaceFlag,
                                recCount + 1 );
    }
  }
  kDebug() << "END" << "(" << recCount << ")";
}


QString ViewerPrivate::createAtmFileLink( const QString& atmFileName ) const
{
  QFileInfo atmFileInfo( atmFileName );

  KTemporaryFile *linkFile = new KTemporaryFile();
  linkFile->setPrefix( atmFileInfo.fileName() +"_[" );
  linkFile->setSuffix( "]." + KMimeType::extractKnownExtension( atmFileInfo.fileName() ) );
  linkFile->open();
  const QString linkName = linkFile->fileName();
  delete linkFile;

  if ( ::link(QFile::encodeName( atmFileName ), QFile::encodeName( linkName )) == 0 ) {
    return linkName; // success
  }
  return QString();
}

KService::Ptr ViewerPrivate::getServiceOffer( KMime::Content *content)
{
  QString fileName = mNodeHelper->writeNodeToTempFile( content );

  const QString contentTypeStr = content->contentType()->mimeType();

  // determine the MIME type of the attachment
  KMimeType::Ptr mimetype;
  // prefer the value of the Content-Type header
  mimetype = KMimeType::mimeType( contentTypeStr, KMimeType::ResolveAliases );

  if ( !mimetype.isNull() && mimetype->is( KABC::Addressee::mimeType() ) ) {
    attachmentView( content );
    return KService::Ptr( 0 );
  }

  if ( mimetype.isNull() ) {
    // consider the filename if mimetype can not be found by content-type
    mimetype = KMimeType::findByPath( fileName, 0, true /* no disk access */ );
  }
  if ( ( mimetype->name() == "application/octet-stream" )
    /*TODO(Andris) port when on-demand loading is done   && msgPart.isComplete() */) {
    // consider the attachment's contents if neither the Content-Type header
    // nor the filename give us a clue
    mimetype = KMimeType::findByFileContent( fileName );
  }
  return KMimeTypeTrader::self()->preferredService( mimetype->name(), "Application" );
}

KMime::Content::List ViewerPrivate::selectedContents()
{
  KMime::Content::List contents;
  QItemSelectionModel *selectionModel = mMimePartTree->selectionModel();
  QModelIndexList selectedRows = selectionModel->selectedRows();

  Q_FOREACH(QModelIndex index, selectedRows)
  {
     KMime::Content *content = static_cast<KMime::Content*>( index.internalPointer() );
     if ( content )
       contents.append( content );
  }

  return contents;
}


void ViewerPrivate::attachmentOpenWith( KMime::Content *node )
{
  QString name = mNodeHelper->writeNodeToTempFile( node );
  QString linkName = createAtmFileLink( name );
  KUrl::List lst;
  KUrl url;
  bool autoDelete = true;

  if ( linkName.isEmpty() ) {
  autoDelete = false;
  linkName = name;
  }

  url.setPath( linkName );
  lst.append( url );
  if ( (! KRun::displayOpenWithDialog(lst, mMainWindow, autoDelete)) && autoDelete ) {
    QFile::remove( url.toLocalFile() );
  }
}

void ViewerPrivate::attachmentOpen( KMime::Content *node )
{
  KService::Ptr offer(0);
  offer = getServiceOffer( node );
  if ( !offer ) {
    kDebug() << "got no offer";
    return;
  }
  QString name = mNodeHelper->writeNodeToTempFile( node );
  KUrl::List lst;
  KUrl url;
  bool autoDelete = true;
  QString fname = createAtmFileLink( name );

  if ( fname.isNull() ) {
    autoDelete = false;
    fname = name;
  }

  url.setPath( fname );
  lst.append( url );
  if ( (!KRun::run( *offer, lst, 0, autoDelete )) && autoDelete ) {
      QFile::remove(url.toLocalFile());
  }
}


CSSHelper* ViewerPrivate::cssHelper() const
{
  return mCSSHelper;
}

bool ViewerPrivate::decryptMessage() const
{
  if ( !GlobalSettings::self()->alwaysDecrypt() )
    return mDecrytMessageOverwrite;
  else
    return true;
}

int ViewerPrivate::pointsToPixel(int pointSize) const
{
  return (pointSize * mViewer->logicalDpiY() + 36) / 72;
}

void ViewerPrivate::displaySplashPage( const QString &info )
{
  mMsgDisplay = false;
  adjustLayout();

  QString location = KStandardDirs::locate("data", "kmail/about/main.html");//FIXME(Andras) copy to $KDEDIR/share/apps/mailviewer
  QString content = KPIMUtils::kFileToByteArray( location );
  content = content.arg( KStandardDirs::locate( "data", "kdeui/about/kde_infopage.css" ) );
  if ( QApplication::isRightToLeft() )
    content = content.arg( "@import \"" + KStandardDirs::locate( "data",
                           "kdeui/about/kde_infopage_rtl.css" ) +  "\";");
  else
    content = content.arg( "" );

  QString fontSize = QString::number( pointsToPixel( mCSSHelper->bodyFont().pointSize() ) );
  QString catchPhrase = ""; //not enough space for a catch phrase at default window size i18n("Part of the Kontact Suite");
  QString quickDescription = i18n("The KDE email client.");

  mViewer->setHtml( content.arg(fontSize).arg(mAppName).arg(catchPhrase).arg(quickDescription).arg(info), KUrl::fromPath( location ) );
  mViewer->show();
}

void ViewerPrivate::enableMessageDisplay()
{
  mMsgDisplay = true;
  adjustLayout();
}

void ViewerPrivate::displayMessage()
{
  /*FIXME(Andras) port to Akonadi
  mMimePartTree->clearAndResetSortOrder();
  */
  showHideMimeTree();

  mNodeHelper->setOverrideCodec( mMessage.get(), overrideCodec() );

  htmlWriter()->begin( QString() );
  htmlWriter()->queue( mCSSHelper->htmlHead( mUseFixedFont ) );

  if ( !mMainWindow )
      q->setWindowTitle( mMessage->subject()->asUnicodeString() );

  // Don't update here, parseMsg() can overwrite the HTML mode, which would lead to flicker.
  // It is updated right after parseMsg() instead.
  mColorBar->setMode( Util::Normal, HtmlStatusBar::NoUpdate );

  if ( mMessageItem.hasAttribute<ErrorAttribute>() ) {
    //TODO: Insert link to clear error so that message might be resent
    const ErrorAttribute* const attr = mMessageItem.attribute<ErrorAttribute>();
    Q_ASSERT( attr );
    const QColor foreground = KColorScheme( QPalette::Active, KColorScheme::View ).foreground( KColorScheme::NegativeText ).color();
    const QColor background = KColorScheme( QPalette::Active, KColorScheme::View ).background( KColorScheme::NegativeBackground ).color();

    htmlWriter()->queue( QString::fromLatin1("<div style=\"background:%1;color:%2;border:1px solid %3\">%4</div>").arg( background.name(), foreground.name(), foreground.name(), Qt::escape( attr->message() ) ) );
    htmlWriter()->queue( QLatin1String("<p></p>") );
  }

  parseContent( mMessage.get() );
  delete mMimePartModel->root();
  mMimePartModel->setRoot( mNodeHelper->messageWithExtraContent( mMessage.get() ) );
  mColorBar->update();

  htmlWriter()->queue("</body></html>");
  connect( mPartHtmlWriter, SIGNAL( finished() ), this, SLOT( injectAttachments() ) );
  connect( mPartHtmlWriter, SIGNAL( finished() ), this, SLOT( toggleFullAddressList() ) );
  htmlWriter()->flush();
}

void ViewerPrivate::removeEncryptedPart(KMime::Content* node)
{
  bool changed = false;
  KMime::Content* data = ObjectTreeParser::findType( node, "application/pgp-encrypted", true, true );
  if ( data ) {
    changed = true;
//     qDebug() << "REMOVING: " << data->encodedContent();
    KMime::Content *encryptedData = MessageCore::NodeHelper::next( data, false ); //this is the encrypted data
    if ( encryptedData && encryptedData->contentType()->mimeType() == "application/octet-stream" ) {
//       qDebug() << "REMOVING: " << encryptedData->encodedContent();
      data = MessageCore::NodeHelper::next( encryptedData, false ); //this will be the decrypted content
      if ( data ) {
        QByteArray decryptedData = data->encodedContent();
        node->setBody( decryptedData );
        node->contentType()->from7BitString( data->contentType()->as7BitString( false ) );
        node->contentTransferEncoding()->from7BitString( data->contentTransferEncoding()->as7BitString( false ) );
        node->assemble();
        return;
      }
    }
    data = 0;
  }
  else {
    data = ObjectTreeParser::findType( node, "application/pkcs7-mime", true, true );
    if ( data ) {
        //node->removeContent( data, true );
        kDebug() << "Unfinished Akonadi port in " << Q_FUNC_INFO;
        data = 0;
    }
  }

}


void ViewerPrivate::createDecryptedMessage()
{
  //check if the message is in the outbox folder
//   kDebug() << "Item is in the collection : " << mMessageItem.parentCollection().id() << mMessageItem.isValid();
  //FIXME: using root() is too much, but using mMessageItem.parentCollection() returns no collections in job->collections()
  Akonadi::CollectionFetchJob* job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( collectionFetchResult( KJob* ) ) );
}

void ViewerPrivate::collectionFetchResult( KJob* job )
{
  if ( job->error() )
    return;
  
  Akonadi::Collection col;
  Q_FOREACH(Akonadi::Collection c, static_cast<Akonadi::CollectionFetchJob*>( job )->collections() ) {
    if ( c == mMessageItem.parentCollection() ) {
      col = c;
      break;
    }
  }
  if ( !col.isValid() )
    return;
  Akonadi::AgentInstance::List instances = Akonadi::AgentManager::self()->instances();
  QString itemResource = col.resource();
//   kDebug() << "Item resource : " << itemResource << col.id() << col.name();
  Akonadi::AgentInstance resourceInstance;
  foreach ( const Akonadi::AgentInstance &instance, instances ) {
//     kDebug() << "Instance " << instance.name() << " identifier : " << instance.identifier();
    if ( instance.identifier() == itemResource ) {
      resourceInstance = instance;
      break;
    }
  }
  bool isInOutbox = true;
  Akonadi::Collection outboxCollection = Akonadi::SpecialMailCollections::self()->collection( Akonadi::SpecialMailCollections::Outbox, resourceInstance );
  if ( resourceInstance.isValid() && outboxCollection != col )
    isInOutbox = false;

  if ( !isInOutbox ) {
      KMime::Message* message = mNodeHelper->messageWithExtraContent( mMessage.get() );
      removeEncryptedPart( message );

      mMessage.reset( message );

      mMessageItem.setPayloadFromData( message->encodedContent() );
      Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob( mMessageItem );
      connect( job, SIGNAL(result(KJob*)), SLOT(itemModifiedResult(KJob*)) );
  //   kDebug() << "Decrypted message: " << message->encodedContent();
  }
}

void ViewerPrivate::parseContent( KMime::Content *content )
{
  assert( content != 0 );

  // Check if any part of this message is a v-card
  // v-cards can be either text/x-vcard or text/directory, so we need to check
  // both.
  KMime::Content* vCardContent = findContentByType( content, "text/x-vcard" );
  if ( !vCardContent )
    vCardContent = findContentByType( content, "text/directory" );

  bool hasVCard = false;

  if( vCardContent ) {
    // ### FIXME: We should only do this if the vCard belongs to the sender,
    // ### i.e. if the sender's email address is contained in the vCard.
    const QByteArray vCard = vCardContent->decodedContent();
    KABC::VCardConverter t;
    if ( !t.parseVCards( vCard ).isEmpty() ) {
      hasVCard = true;
      mNodeHelper->writeNodeToTempFile( vCardContent );
    }
  }

  if ( !NodeHelper::isToltecMessage( content ) || mShowRawToltecMail ) {
    KMime::Message *message = dynamic_cast<KMime::Message*>( content );
    if ( message ) {
      htmlWriter()->queue( writeMsgHeader( message, hasVCard ? vCardContent : 0, true ) );
    }
  }

  // Pass control to the OTP now, which does the real work
  mNodeHelper->removeTempFiles();
  mNodeHelper->setNodeUnprocessed( mMessage.get(), true );
  MailViewerSource otpSource( this );
  ObjectTreeParser otp( &otpSource, mNodeHelper, 0, mMessage.get() != content /* show only single node */ );
  otp.setAllowAsync( true );
  otp.setShowRawToltecMail( mShowRawToltecMail );
  otp.parseObjectTree( content );

// TODO: Setting the signature state to nodehelper is not enough, it should actually
      // be added to the store, so that the message list correctly displays the signature state
      // of messages that were parsed at least once
  // store encrypted/signed status information in the KMMessage
  //  - this can only be done *after* calling parseObjectTree()
  KMMsgEncryptionState encryptionState = mNodeHelper->overallEncryptionState( content );
  KMMsgSignatureState  signatureState  = mNodeHelper->overallSignatureState( content );
  mNodeHelper->setEncryptionState( content, encryptionState );
  // Don't reset the signature state to "not signed" (e.g. if one canceled the
  // decryption of a signed messages which has already been decrypted before).
  if ( signatureState != KMMsgNotSigned ||
       mNodeHelper->signatureState( content ) == KMMsgSignatureStateUnknown ) {
    mNodeHelper->setSignatureState( content, signatureState );
  }

  bool emitReplaceMsgByUnencryptedVersion = false;
  if ( GlobalSettings::self()->storeDisplayedMessagesUnencrypted() )
  {

    // Hack to make sure the S/MIME CryptPlugs follows the strict requirement
    // of german government:
    // --> All received encrypted messages *must* be stored in unencrypted form
    //     after they have been decrypted once the user has read them.
    //     ( "Aufhebung der Verschluesselung nach dem Lesen" )
    //
    // note: Since there is no configuration option for this, we do that for
    //       all kinds of encryption now - *not* just for S/MIME.
    //       This could be changed in the objectTreeToDecryptedMsg() function
    //       by deciding when (or when not, resp.) to set the 'dataNode' to
    //       something different than 'curNode'.


    //FIXME(Andras) kDebug() <<"(mIdOfLastViewedMessage != aMsg->msgId()) ="       << (mIdOfLastViewedMessage != aMsg->msgId());
//     kDebug() << "aMsg->parent() && aMsg->parent() != kmkernel->outboxFolder() = " << (aMsg->parent() && aMsg->parent() != kmkernel->outboxFolder());
//     kDebug() << "message_was_saved_decrypted_before( aMsg ) = " << message_was_saved_decrypted_before( aMsg );
    kDebug() << "this->decryptMessage() = " << decryptMessage();
    kDebug() << "otp.hasPendingAsyncJobs() = " << otp.hasPendingAsyncJobs();
    kDebug() << "   (KMMsgFullyEncrypted == encryptionState) ="     << (KMMsgFullyEncrypted == encryptionState);
    kDebug() << "|| (KMMsgPartiallyEncrypted == encryptionState) =" << (KMMsgPartiallyEncrypted == encryptionState);

         // only proceed if we were called the normal way - not by
         // double click on the message (==not running in a separate window)
    if( decryptMessage() // only proceed if the message has actually been decrypted          
        && !otp.hasPendingAsyncJobs() // only proceed if no pending async jobs are running:         
        && (    (KMMsgFullyEncrypted == encryptionState)     // only proceed if this message is (at least partially) encrypted
            || (KMMsgPartiallyEncrypted == encryptionState) ) ) {
      createDecryptedMessage();
    }
  }

  showHideMimeTree();
}


QString ViewerPrivate::writeMsgHeader( KMime::Message *aMsg, KMime::Content* vCardNode,
                                       bool topLevel )
{
  if ( !headerStyle() )
    kFatal() << "trying to writeMsgHeader() without a header style set!";
  if ( !headerStrategy() )
    kFatal() << "trying to writeMsgHeader() without a header strategy set!";
  QString href;
  if ( vCardNode )
    href = mNodeHelper->asHREF( vCardNode, "body" );

  headerStyle()->setHeaderStrategy( headerStrategy() );
  headerStyle()->setVCardName( href );
  headerStyle()->setPrinting( mPrinting );
  headerStyle()->setTopLevel( topLevel );
  headerStyle()->setAllowAsync( true );
  headerStyle()->setSourceObject( this );
  headerStyle()->setNodeHelper( mNodeHelper );
  headerStyle()->setMessagePath( mMessagePath );
  return headerStyle()->format( aMsg );
}

void ViewerPrivate::showVCard( KMime::Content* msgPart ) {
  const QByteArray vCard = msgPart->decodedContent();

  VCardViewer *vcv = new VCardViewer( mMainWindow, vCard );
  vcv->setObjectName( "vCardDialog" );
  vcv->show();
}


void ViewerPrivate::initHtmlWidget(void)
{
  mViewer->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
  mViewer->settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
  mViewer->settings()->setAttribute(QWebSettings::JavaEnabled, false);
  mViewer->settings()->setAttribute(QWebSettings::PluginsEnabled, false);

  mViewer->setFocusPolicy( Qt::WheelFocus );
  mViewer->page()->view()->installEventFilter( this );
  mViewer->installEventFilter( this );

  if ( !htmlWriter() ) {
    mPartHtmlWriter = new WebKitPartHtmlWriter( mViewer, 0 );
#ifdef MESSAGEVIEWER_READER_HTML_DEBUG
    mHtmlWriter = new TeeHtmlWriter( new FileHtmlWriter( QString() ),
                                     mPartHtmlWriter );
#else
    mHtmlWriter = mPartHtmlWriter;
#endif
  }
#if 0
  // We do a queued connection below, and for that we need to register the meta types of the
  // parameters.
  //
  // Why do we do a queued connection instead of a direct one? slotUrlOpen() handles those clicks,
  // and can end up in the click handler for accepting invitations. That handler can pop up a dialog
  // asking the user for a comment on the invitation reply. This dialog is started with exec(), i.e.
  // executes a sub-eventloop. This sub-eventloop then eventually re-enters the KHTML event handler,
  // which then thinks we started a drag, and therefore adds a silly drag object to the cursor, with
  // urls like x-kmail-whatever/43/8/accept, and we don't want that drag object.
  //
  // Therefore, use queued connections to avoid the reentry of the KHTML event loop, so we don't
  // get the drag object.
  static bool metaTypesRegistered = false;
  if ( !metaTypesRegistered ) {
    qRegisterMetaType<KParts::OpenUrlArguments>( "KParts::OpenUrlArguments" );
    qRegisterMetaType<KParts::BrowserArguments>( "KParts::BrowserArguments" );
    qRegisterMetaType<KParts::WindowArgs>( "KParts::WindowArgs" );
    metaTypesRegistered = true;
  }
#endif
  connect(mViewer->page(), SIGNAL( linkHovered( const QString &, const QString &, const QString & ) ),
          this, SLOT( slotUrlOn(const QString &, const QString &, const QString & )));
  connect(mViewer->page(), SIGNAL( linkClicked( const QUrl & ) ),this, SLOT( slotUrlOpen( const QUrl & ) ), Qt::QueuedConnection);
  connect( mViewer, SIGNAL(popupMenu(const QString &, const QPoint &) ),
           SLOT(slotUrlPopup(const QString &, const QPoint &)) );
}

bool ViewerPrivate::eventFilter( QObject *, QEvent *e )
{
  if ( e->type() == QEvent::MouseButtonPress ) {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if ( me->button() == Qt::LeftButton && ( me->modifiers() & Qt::ShiftModifier ) ) {
      // special processing for shift+click
      URLHandlerManager::instance()->handleShiftClick( mHoveredUrl, this );
      return true;
    }
    if ( me->button() == Qt::LeftButton ) {
      mCanStartDrag = URLHandlerManager::instance()->willHandleDrag( mHoveredUrl, this );
      mLastClickPosition = me->pos();
    }
  }

  if ( e->type() ==  QEvent::MouseButtonRelease ) {
    mCanStartDrag = false;
  }

  if ( e->type() == QEvent::MouseMove ) {

    QMouseEvent* me = static_cast<QMouseEvent*>( e );

    // First, update the hovered URL
    const QPoint local = mViewer->page()->view()->mapFromGlobal( me->globalPos() );
    const QWebHitTestResult hit = mViewer->page()->currentFrame()->hitTestContent( local );
    if ( !hit.linkUrl().isEmpty() || !hit.imageUrl().isEmpty() ) {
      if ( !hit.linkUrl().isEmpty() ) {
        mHoveredUrl = hit.linkUrl();
      } else {
        mHoveredUrl = hit.imageUrl();
      }
    } else {
      mHoveredUrl.clear();
    }

    // If we are potentially handling a drag, deal with that.
    if ( mCanStartDrag && me->buttons() & Qt::LeftButton ) {

      if ( ( mLastClickPosition - me->pos() ).manhattanLength() > KGlobalSettings::dndEventDelay() ) {
        if ( URLHandlerManager::instance()->handleDrag( mHoveredUrl, this ) ) {

          // If the URL handler manager started a drag, don't handle this in the future
          mCanStartDrag = false;
        }
      }

      // Don't tell WebKit about this mouse move event, or it might start its own drag!
      return true;
    }
  }

  // standard event processing
  return false;
}

void ViewerPrivate::readConfig()
{
  delete mCSSHelper;
  mCSSHelper = new CSSHelper( mViewer );

  mUseFixedFont = GlobalSettings::self()->useFixedFont();
  if ( mToggleFixFontAction )
    mToggleFixFontAction->setChecked( mUseFixedFont );

  mHtmlMail = GlobalSettings::self()->htmlMail();
  mHtmlLoadExternal = GlobalSettings::self()->htmlLoadExternal();

  KToggleAction *raction = actionForHeaderStyle( headerStyle(), headerStrategy() );
  if ( raction )
    raction->setChecked( true );

  setAttachmentStrategy( AttachmentStrategy::create( GlobalSettings::self()->attachmentStrategy() ) );
  raction = actionForAttachmentStrategy( attachmentStrategy() );
  if ( raction )
    raction->setChecked( true );

  const int mimeH = GlobalSettings::self()->mimePaneHeight();
  const int messageH = GlobalSettings::self()->messagePaneHeight();
  mSplitterSizes.clear();
  if ( GlobalSettings::self()->mimeTreeLocation() == GlobalSettings::EnumMimeTreeLocation::bottom )
    mSplitterSizes << messageH << mimeH;
  else
    mSplitterSizes << mimeH << messageH;

  adjustLayout();

  readGlobalOverrideCodec();

  // Note that this call triggers an update, see this call has to be at the
  // bottom when all settings are already est.
  setHeaderStyleAndStrategy( HeaderStyle::create( GlobalSettings::self()->headerStyle() ),
                             HeaderStrategy::create( GlobalSettings::self()->headerSetDisplayed() ) );

  if ( mMessage )
    update();
  mColorBar->update();
}


void ViewerPrivate::writeConfig( bool sync )
{
  GlobalSettings::self()->setUseFixedFont( mUseFixedFont );
  if ( headerStyle() )
    GlobalSettings::self()->setHeaderStyle( headerStyle()->name() );
  if ( headerStrategy() )
    GlobalSettings::self()->setHeaderSetDisplayed( headerStrategy()->name() );
  if ( attachmentStrategy() )
    GlobalSettings::self()->setAttachmentStrategy( attachmentStrategy()->name() );

  saveSplitterSizes();
  if ( sync )
    emit requestConfigSync();
}


void ViewerPrivate::setHeaderStyleAndStrategy( HeaderStyle * style,
                                               const HeaderStrategy * strategy ) {
  mHeaderStyle = style ? style : HeaderStyle::fancy();
  mHeaderStrategy = strategy ? strategy : HeaderStrategy::rich();
  update( Viewer::Force );
}


void ViewerPrivate::setAttachmentStrategy( const AttachmentStrategy * strategy ) {
  mAttachmentStrategy = strategy ? strategy : AttachmentStrategy::smart();
  update( Viewer::Force );
}


void ViewerPrivate::setOverrideEncoding( const QString & encoding )
{
  if ( encoding == mOverrideEncoding )
    return;

  mOverrideEncoding = encoding;
  if ( mSelectEncodingAction ) {
    if ( encoding.isEmpty() ) {
      mSelectEncodingAction->setCurrentItem( 0 );
    }
    else {
      QStringList encodings = mSelectEncodingAction->items();
      int i = 0;
      for ( QStringList::const_iterator it = encodings.constBegin(), end = encodings.constEnd(); it != end; ++it, ++i ) {
        if ( NodeHelper::encodingForName( *it ) == encoding ) {
          mSelectEncodingAction->setCurrentItem( i );
          break;
        }
      }
      if ( i == encodings.size() ) {
        // the value of encoding is unknown => use Auto
        kWarning() << "Unknown override character encoding" << encoding
                   << ". Using Auto instead.";
        mSelectEncodingAction->setCurrentItem( 0 );
        mOverrideEncoding.clear();
      }
    }
  }
  update( Viewer::Force );
}

void ViewerPrivate::printMessage( const Akonadi::Item &message )
{
  disconnect( mPartHtmlWriter, SIGNAL( finished() ), this, SLOT( slotPrintMsg() ) );
  connect( mPartHtmlWriter, SIGNAL( finished() ), this, SLOT( slotPrintMsg() ) );
  setMessageItem( message, Viewer::Force );
}

void ViewerPrivate::resetStateForNewMessage()
{
  enableMessageDisplay(); // just to make sure it's on
  mMessage.reset();
  mNodeHelper->clear();
  delete mMimePartModel->root();
  mMimePartModel->setRoot( 0 );
  mSavedRelativePosition = 0;
  setShowSignatureDetails( false );
  mShowRawToltecMail = !GlobalSettings::self()->showToltecReplacementText();
  if ( mPrinting )
    mLevelQuote = -1;
}

void ViewerPrivate::setMessageInternal( const KMime::Message::Ptr message,
                                        Viewer::UpdateMode updateMode )
{
  mMessage = message;
  if ( message ) {
    mNodeHelper->setOverrideCodec( mMessage.get(), overrideCodec() );
  }

  delete mMimePartModel->root();
  mMimePartModel->setRoot( mNodeHelper->messageWithExtraContent( message.get() ) );
  update( updateMode );
}

void ViewerPrivate::setMessageItem( const Akonadi::Item &item, Viewer::UpdateMode updateMode )
{
  resetStateForNewMessage();
  foreach( const Akonadi::Entity::Id monitoredId, mMonitor.itemsMonitored() ) {
    mMonitor.setItemMonitored( Akonadi::Item( monitoredId ), false );
  }
  Q_ASSERT( mMonitor.itemsMonitored().isEmpty() );

  mMessageItem = item;
  if ( mMessageItem.isValid() )
    mMonitor.setItemMonitored( mMessageItem, true );

  if ( !mMessageItem.hasPayload<KMime::Message::Ptr>() ) {
    if ( mMessageItem.isValid() )
      kWarning() << "Payload is not a MessagePtr!";
    return;
  }

  setMessageInternal( mMessageItem.payload<KMime::Message::Ptr>(), updateMode );
}

void ViewerPrivate::setMessage( KMime::Message::Ptr aMsg, Viewer::UpdateMode updateMode )
{
  resetStateForNewMessage();

  Akonadi::Item item;
  item.setPayload( aMsg );
  mMessageItem = item;

  setMessageInternal( aMsg, updateMode );
}

void ViewerPrivate::setMessagePart( KMime::Content * node )
{
  // Cancel scheduled updates of the reader window, as that would stop the
  // timer of the HTML writer, which would make viewing attachment not work
  // anymore as not all HTML is written to the HTML part.
  // We're updating the reader window here ourselves anyway.
  mUpdateReaderWinTimer.stop();

  if ( node ) {
    if ( node->bodyIsMessage() ) {
      mMainWindow->setWindowTitle( node->bodyAsMessage()->subject()->asUnicodeString() );
    } else {
      QString windowTitle = NodeHelper::fileName( node );
      if ( windowTitle.isEmpty() ) {
        windowTitle = node->contentDescription()->asUnicodeString();
      }
      if ( !windowTitle.isEmpty() ) {
        mMainWindow->setWindowTitle( i18n( "View Attachment: %1", windowTitle ) );
      }
    }

    htmlWriter()->begin( QString() );
    htmlWriter()->queue( mCSSHelper->htmlHead( mUseFixedFont ) );

    parseContent( node );

    htmlWriter()->queue("</body></html>");
    htmlWriter()->flush();
  }
}

void ViewerPrivate::showHideMimeTree( )
{
  if ( GlobalSettings::self()->mimeTreeMode() == GlobalSettings::EnumMimeTreeMode::Always )
    mMimePartTree->show();
  else {
    // don't rely on QSplitter maintaining sizes for hidden widgets:
    saveSplitterSizes();
    mMimePartTree->hide();
  }
  if ( mToggleMimePartTreeAction && ( mToggleMimePartTreeAction->isChecked() != mMimePartTree->isVisible() ) )
    mToggleMimePartTreeAction->setChecked( mMimePartTree->isVisible() );
}


void ViewerPrivate::atmViewMsg( KMime::Message::Ptr message )
{
  Q_ASSERT( message );
  emit showMessage( message, overrideEncoding() );
}

void ViewerPrivate::adjustLayout()
{
  if ( GlobalSettings::self()->mimeTreeLocation() == GlobalSettings::EnumMimeTreeLocation::bottom )
    mSplitter->addWidget( mMimePartTree );
  else
    mSplitter->insertWidget( 0, mMimePartTree );
  mSplitter->setSizes( mSplitterSizes );

  if ( GlobalSettings::self()->mimeTreeMode() == GlobalSettings::EnumMimeTreeMode::Always &&
       mMsgDisplay )
    mMimePartTree->show();
  else
    mMimePartTree->hide();

  if (  GlobalSettings::self()->showColorBar() && mMsgDisplay )
    mColorBar->show();
  else
    mColorBar->hide();
}


void ViewerPrivate::saveSplitterSizes() const
{
  if ( !mSplitter || !mMimePartTree )
    return;
  if ( mMimePartTree->isHidden() )
    return; // don't rely on QSplitter maintaining sizes for hidden widgets.

  const bool mimeTreeAtBottom = GlobalSettings::self()->mimeTreeLocation() == GlobalSettings::EnumMimeTreeLocation::bottom;
  GlobalSettings::self()->setMimePaneHeight( mSplitter->sizes()[ mimeTreeAtBottom ? 1 : 0 ] );
  GlobalSettings::self()->setMessagePaneHeight( mSplitter->sizes()[ mimeTreeAtBottom ? 0 : 1 ] );
}

void ViewerPrivate::createWidgets() {
  //TODO: Make a MDN bar similar to Mozillas password bar and show MDNs here as soon as a
  //      MDN enabled message is shown.
  QVBoxLayout * vlay = new QVBoxLayout( q );
  vlay->setMargin( 0 );
  mSplitter = new QSplitter( Qt::Vertical, q );
  mSplitter->setObjectName( "mSplitter" );
  mSplitter->setChildrenCollapsible( false );
  vlay->addWidget( mSplitter );
  mMimePartTree = new QTreeView( mSplitter );
  mMimePartTree->setObjectName( "mMimePartTree" );
  mMimePartModel = new MimeTreeModel( mMimePartTree );
  mMimePartTree->setModel( mMimePartModel );
  mMimePartTree->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mMimePartTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  connect(mMimePartTree, SIGNAL( activated( const QModelIndex& ) ), this, SLOT( slotMimePartSelected( const QModelIndex& ) ) );
  connect(mMimePartTree, SIGNAL( destroyed(QObject*) ), this, SLOT( slotMimePartDestroyed() ) );
  mMimePartTree->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(mMimePartTree, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( slotMimeTreeContextMenuRequested(const QPoint&)) );
  mMimePartTree->header()->setResizeMode( QHeaderView::ResizeToContents );
  mBox = new KHBox( mSplitter );
  mColorBar = new HtmlStatusBar( mBox );
  mColorBar->setObjectName( "mColorBar" );
  mColorBar->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
  KVBox *readerBox = new KVBox( mBox );
  mViewer = new MailWebView( readerBox );
  mViewer->setObjectName( "mViewer" );

#ifdef KDEPIM_MOBILE_UI
  mViewer->page()->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
  mViewer->page()->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
#endif

  mFindBar = new FindBar( mViewer, readerBox );
  mSplitter->setStretchFactor( mSplitter->indexOf(mMimePartTree), 0 );
  mSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
}

void ViewerPrivate::slotMimePartDestroyed()
{
  //root is either null or a modified tree that we need to clean up
  delete mMimePartModel->root();
}

void ViewerPrivate::createActions()
{
  KActionCollection *ac = mActionCollection;
  if ( !ac ) {
    return;
  }

  KToggleAction *raction = 0;

  // header style
  KActionMenu *headerMenu  = new KActionMenu(i18nc("View->", "&Headers"), this);
  ac->addAction("view_headers", headerMenu );
  headerMenu->setHelpText( i18n("Choose display style of message headers") );

  connect( headerMenu, SIGNAL(triggered(bool)),
           this, SLOT(slotCycleHeaderStyles()) );

  QActionGroup *group = new QActionGroup( this );
  raction = new KToggleAction( i18nc("View->headers->", "&Enterprise Headers"), this);
  ac->addAction( "view_headers_enterprise", raction );
  connect( raction, SIGNAL(triggered(bool)), SLOT(slotEnterpriseHeaders()) );
  raction->setHelpText( i18n("Show the list of headers in Enterprise style") );
  group->addAction( raction );
  headerMenu->addAction( raction );

  raction  = new KToggleAction(i18nc("View->headers->", "&Fancy Headers"), this);
  ac->addAction("view_headers_fancy", raction );
  connect(raction, SIGNAL(triggered(bool) ), SLOT(slotFancyHeaders()));
  raction->setHelpText( i18n("Show the list of headers in a fancy format") );
  group->addAction( raction );
  headerMenu->addAction( raction );

  raction  = new KToggleAction(i18nc("View->headers->", "&Brief Headers"), this);
  ac->addAction("view_headers_brief", raction );
  connect(raction, SIGNAL(triggered(bool) ), SLOT(slotBriefHeaders()));
  raction->setHelpText( i18n("Show brief list of message headers") );
  group->addAction( raction );
  headerMenu->addAction( raction );

  raction  = new KToggleAction(i18nc("View->headers->", "&Standard Headers"), this);
  ac->addAction("view_headers_standard", raction );
  connect(raction, SIGNAL(triggered(bool) ), SLOT(slotStandardHeaders()));
  raction->setHelpText( i18n("Show standard list of message headers") );
  group->addAction( raction );
  headerMenu->addAction( raction );

  raction  = new KToggleAction(i18nc("View->headers->", "&Long Headers"), this);
  ac->addAction("view_headers_long", raction );
  connect(raction, SIGNAL(triggered(bool) ), SLOT(slotLongHeaders()));
  raction->setHelpText( i18n("Show long list of message headers") );
  group->addAction( raction );
  headerMenu->addAction( raction );

  raction  = new KToggleAction(i18nc("View->headers->", "&All Headers"), this);
  ac->addAction("view_headers_all", raction );
  connect(raction, SIGNAL(triggered(bool) ), SLOT(slotAllHeaders()));
  raction->setHelpText( i18n("Show all message headers") );
  group->addAction( raction );
  headerMenu->addAction( raction );

  // attachment style
  KActionMenu *attachmentMenu  = new KActionMenu(i18nc("View->", "&Attachments"), this);
  ac->addAction("view_attachments", attachmentMenu );
  attachmentMenu->setHelpText( i18n("Choose display style of attachments") );
  connect( attachmentMenu, SIGNAL(triggered(bool)),
           this, SLOT(slotCycleAttachmentStrategy()) );

  group = new QActionGroup( this );
  raction  = new KToggleAction(i18nc("View->attachments->", "&As Icons"), this);
  ac->addAction("view_attachments_as_icons", raction );
  connect(raction, SIGNAL(triggered(bool) ), SLOT(slotIconicAttachments()));
  raction->setHelpText( i18n("Show all attachments as icons. Click to see them.") );
  group->addAction( raction );
  attachmentMenu->addAction( raction );

  raction  = new KToggleAction(i18nc("View->attachments->", "&Smart"), this);
  ac->addAction("view_attachments_smart", raction );
  connect(raction, SIGNAL(triggered(bool) ), SLOT(slotSmartAttachments()));
  raction->setHelpText( i18n("Show attachments as suggested by sender.") );
  group->addAction( raction );
  attachmentMenu->addAction( raction );

  raction  = new KToggleAction(i18nc("View->attachments->", "&Inline"), this);
  ac->addAction("view_attachments_inline", raction );
  connect(raction, SIGNAL(triggered(bool) ), SLOT(slotInlineAttachments()));
  raction->setHelpText( i18n("Show all attachments inline (if possible)") );
  group->addAction( raction );
  attachmentMenu->addAction( raction );

  raction  = new KToggleAction(i18nc("View->attachments->", "&Hide"), this);
  ac->addAction("view_attachments_hide", raction );
  connect(raction, SIGNAL(triggered(bool) ), SLOT(slotHideAttachments()));
  raction->setHelpText( i18n("Do not show attachments in the message viewer") );
  group->addAction( raction );
  attachmentMenu->addAction( raction );

  // Set Encoding submenu
  mSelectEncodingAction  = new KSelectAction(KIcon("character-set"), i18n("&Set Encoding"), this);
  mSelectEncodingAction->setToolBarMode( KSelectAction::MenuMode );
  ac->addAction("encoding", mSelectEncodingAction );
  connect(mSelectEncodingAction,SIGNAL( triggered(int)),
          SLOT( slotSetEncoding() ));
  QStringList encodings = NodeHelper::supportedEncodings( false );
  encodings.prepend( i18n( "Auto" ) );
  mSelectEncodingAction->setItems( encodings );
  mSelectEncodingAction->setCurrentItem( 0 );

  //
  // Message Menu
  //

  // copy selected text to clipboard
  mCopyAction = ac->addAction( KStandardAction::Copy, "kmail_copy", this,
                               SLOT(slotCopySelectedText()) );

  // copy all text to clipboard
  mSelectAllAction  = new KAction(i18n("Select All Text"), this);
  ac->addAction("mark_all_text", mSelectAllAction );
  connect(mSelectAllAction, SIGNAL(triggered(bool) ), SLOT(selectAll()));
  mSelectAllAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_T ) );

  // copy Email address to clipboard
  mCopyURLAction = new KAction( KIcon( "edit-copy" ),
                                i18n( "Copy Link Address" ), this );
  ac->addAction( "copy_url", mCopyURLAction );
  connect( mCopyURLAction, SIGNAL(triggered(bool)), SLOT(slotUrlCopy()) );

  // open URL
  mUrlOpenAction = new KAction( KIcon( "document-open" ), i18n( "Open URL" ), this );
  ac->addAction( "open_url", mUrlOpenAction );
  connect( mUrlOpenAction, SIGNAL(triggered(bool)), this, SLOT(slotUrlOpen()) );

  // use fixed font
  mToggleFixFontAction = new KToggleAction( i18n( "Use Fi&xed Font" ), this );
  ac->addAction( "toggle_fixedfont", mToggleFixFontAction );
  connect( mToggleFixFontAction, SIGNAL(triggered(bool)), SLOT(slotToggleFixedFont()) );
  mToggleFixFontAction->setShortcut( QKeySequence( Qt::Key_X ) );

  // Show message structure viewer
  mToggleMimePartTreeAction = new KToggleAction( i18n( "Show Message Structure" ), this );
  ac->addAction( "toggle_mimeparttree", mToggleMimePartTreeAction );
  connect( mToggleMimePartTreeAction, SIGNAL(toggled(bool)),
           SLOT(slotToggleMimePartTree()));

  mViewSourceAction  = new KAction(i18n("&View Source"), this);
  ac->addAction("view_source", mViewSourceAction );
  connect(mViewSourceAction, SIGNAL(triggered(bool) ), SLOT(slotShowMessageSource()));
  mViewSourceAction->setShortcut(QKeySequence(Qt::Key_V));

  mSaveMessageAction = new KAction(i18n("&Save message"), this);
  ac->addAction("save_message", mSaveMessageAction);
  connect(mSaveMessageAction, SIGNAL(triggered(bool) ), SLOT(slotSaveMessage()));
  mSaveMessageAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));

  //
  // Scroll actions
  //
  mScrollUpAction = new KAction( i18n("Scroll Message Up"), this );
  mScrollUpAction->setShortcut( QKeySequence( Qt::Key_Up ) );
  ac->addAction( "scroll_up", mScrollUpAction );
  connect( mScrollUpAction, SIGNAL( triggered( bool ) ),
           q, SLOT( slotScrollUp() ) );

  mScrollDownAction = new KAction( i18n("Scroll Message Down"), this );
  mScrollDownAction->setShortcut( QKeySequence( Qt::Key_Down ) );
  ac->addAction( "scroll_down", mScrollDownAction );
  connect( mScrollDownAction, SIGNAL( triggered( bool ) ),
           q, SLOT( slotScrollDown() ) );

  mScrollUpMoreAction = new KAction( i18n("Scroll Message Up (More)"), this );
  mScrollUpMoreAction->setShortcut( QKeySequence( Qt::Key_PageUp ) );
  ac->addAction( "scroll_up_more", mScrollUpMoreAction );
  connect( mScrollUpMoreAction, SIGNAL( triggered( bool ) ),
           q, SLOT( slotScrollPrior() ) );

  mScrollDownMoreAction = new KAction( i18n("Scroll Message Down (More)"), this );
  mScrollDownMoreAction->setShortcut( QKeySequence( Qt::Key_PageDown ) );
  ac->addAction( "scroll_down_more", mScrollDownMoreAction );
  connect( mScrollDownMoreAction, SIGNAL( triggered( bool ) ),
           q, SLOT( slotScrollNext() ) );

  //
  // Actions not in menu
  //

  // Toggle HTML display mode.
  mToggleDisplayModeAction = new KToggleAction( i18n( "Toggle HTML Display Mode" ), this );
  ac->addAction( "toggle_html_display_mode", mToggleDisplayModeAction );
  connect( mToggleDisplayModeAction, SIGNAL( triggered( bool ) ),
           SLOT( slotToggleHtmlMode() ) );
  mToggleDisplayModeAction->setHelpText( i18n( "Toggle display mode between HTML and plain text" ) );
}


void ViewerPrivate::showContextMenu( KMime::Content* content, const QPoint &pos )
{
  if ( !content )
    return;
  const bool isAttachment = !content->contentType()->isMultipart() && !content->isTopLevel();
  const bool isRoot = ( content == mMessage.get() );

  KMenu popup;

  if ( !isRoot ) {
    popup.addAction( SmallIcon( "document-save-as" ), i18n( "Save &As..." ),
                     this, SLOT( slotAttachmentSaveAs() ) );

    if ( isAttachment ) {
      popup.addAction( SmallIcon( "document-open" ), i18nc( "to open", "Open" ),
                       this, SLOT( slotAttachmentOpen() ) );
      popup.addAction( i18n( "Open With..." ), this, SLOT( slotAttachmentOpenWith() ) );
      popup.addAction( i18nc( "to view something", "View" ), this, SLOT( slotAttachmentView() ) );
    }
  }

  /*
   * FIXME make optional?
  popup.addAction( i18n( "Save as &Encoded..." ), this,
                   SLOT( slotSaveAsEncoded() ) );
  */

  popup.addAction( i18n( "Save All Attachments..." ), this,
                   SLOT( slotAttachmentSaveAll() ) );

  // edit + delete only for attachments
  if ( !isRoot ) {
    if ( isAttachment ) {
      popup.addAction( SmallIcon( "edit-copy" ), i18n( "Copy" ),
                       this, SLOT( slotAttachmentCopy() ) );
      if ( GlobalSettings::self()->allowAttachmentDeletion() )
        popup.addAction( SmallIcon( "edit-delete" ), i18n( "Delete Attachment" ),
                         this, SLOT( slotAttachmentDelete() ) );
      if ( GlobalSettings::self()->allowAttachmentEditing() )
        popup.addAction( SmallIcon( "document-properties" ), i18n( "Edit Attachment" ),
                         this, SLOT( slotAttachmentEdit() ) );
    }

    if ( !content->isTopLevel() )
      popup.addAction( i18n( "Properties" ), this, SLOT( slotAttachmentProperties() ) );
  }
  popup.exec( mMimePartTree->viewport()->mapToGlobal( pos ) );

}


KToggleAction *ViewerPrivate::actionForHeaderStyle( const HeaderStyle * style, const HeaderStrategy * strategy ) {
  if ( !mActionCollection )
    return 0;
  const char * actionName = 0;
  if ( style == HeaderStyle::enterprise() )
    actionName = "view_headers_enterprise";
  if ( style == HeaderStyle::fancy() )
    actionName = "view_headers_fancy";
  else if ( style == HeaderStyle::brief() )
    actionName = "view_headers_brief";
  else if ( style == HeaderStyle::plain() ) {
    if ( strategy == HeaderStrategy::standard() )
      actionName = "view_headers_standard";
    else if ( strategy == HeaderStrategy::rich() )
      actionName = "view_headers_long";
    else if ( strategy == HeaderStrategy::all() )
      actionName = "view_headers_all";
  }
  if ( actionName )
    return static_cast<KToggleAction*>(mActionCollection->action(actionName));
  else
    return 0;
}

KToggleAction *ViewerPrivate::actionForAttachmentStrategy( const AttachmentStrategy * as ) {
  if ( !mActionCollection )
    return 0;
  const char * actionName = 0;
  if ( as == AttachmentStrategy::iconic() )
    actionName = "view_attachments_as_icons";
  else if ( as == AttachmentStrategy::smart() )
    actionName = "view_attachments_smart";
  else if ( as == AttachmentStrategy::inlined() )
    actionName = "view_attachments_inline";
  else if ( as == AttachmentStrategy::hidden() )
    actionName = "view_attachments_hide";

  if ( actionName )
    return static_cast<KToggleAction*>(mActionCollection->action(actionName));
  else
    return 0;
}


void ViewerPrivate::readGlobalOverrideCodec()
{
  // if the global character encoding wasn't changed then there's nothing to do
 if ( MessageCore::GlobalSettings::self()->overrideCharacterEncoding() == mOldGlobalOverrideEncoding )
    return;

  setOverrideEncoding( MessageCore::GlobalSettings::self()->overrideCharacterEncoding() );
  mOldGlobalOverrideEncoding = MessageCore::GlobalSettings::self()->overrideCharacterEncoding();
}


const QTextCodec * ViewerPrivate::overrideCodec() const
{
  if ( mOverrideEncoding.isEmpty() || mOverrideEncoding == "Auto" ) // Auto
    return 0;
  else
    return ViewerPrivate::codecForName( mOverrideEncoding.toLatin1() );
}


static QColor nextColor( const QColor & c )
{
  int h, s, v;
  c.getHsv( &h, &s, &v );
  return QColor::fromHsv( (h + 50) % 360, qMax(s, 64), v );
}

QString ViewerPrivate::renderAttachments( KMime::Content * node, const QColor &bgColor )
{

  if ( !node )
    return QString();

  QString html;
  KMime::Content * child = MessageCore::NodeHelper::firstChild( node );

  if ( child) {
    QString subHtml = renderAttachments( child, nextColor( bgColor ) );
    if ( !subHtml.isEmpty() ) {

      QString visibility;
      if( !mShowAttachmentQuicklist ) {
        visibility.append( "display:none;" );
      }

      QString margin;
      if ( node != mMessage.get() || headerStyle() != HeaderStyle::enterprise() )
        margin = "padding:2px; margin:2px; ";
      QString align = "left";
      if ( headerStyle() == HeaderStyle::enterprise() )
        align = "right";
      if ( node->contentType()->mediaType().toLower() == "message" || node->contentType()->mediaType().toLower() == "multipart" || node == mMessage.get() )
        html += QString::fromLatin1("<div style=\"background:%1; %2"
                "vertical-align:middle; float:%3; %4\">").arg( bgColor.name() ).arg( margin )
                                                         .arg( align ).arg( visibility );
      html += subHtml;
      if ( node->contentType()->mediaType().toLower() == "message" ||  node->contentType()->mediaType().toLower() == "multipart" || node == mMessage.get() )
        html += "</div>";
    }
  } else {
    QString label, icon;
    icon = mNodeHelper->iconName( node, KIconLoader::Small );
    label = node->contentDescription()->asUnicodeString();
    if( label.isEmpty() ) {
      label = NodeHelper::fileName( node );
    }

    bool typeBlacklisted = node->contentType()->mediaType().toLower() == "multipart";
    if ( !typeBlacklisted ) {
      typeBlacklisted = StringUtil::isCryptoPart( node->contentType()->mediaType(),
                                                  node->contentType()->subType(),
                                                  node->contentDisposition()->filename() );
    }
    typeBlacklisted = typeBlacklisted || node == mMessage.get();
    const bool firstTextChildOfEncapsulatedMsg =
        node->contentType()->mediaType().toLower() == "text" &&
        node->contentType()->subType().toLower() == "plain" &&
        node->parent() && node->parent()->contentType()->mediaType().toLower() == "message";
    typeBlacklisted = typeBlacklisted || firstTextChildOfEncapsulatedMsg;
    if ( !label.isEmpty() && !icon.isEmpty() && !typeBlacklisted ) {
      html += "<div style=\"float:left;\">";
      html += QString::fromLatin1( "<span style=\"white-space:nowrap; border-width: 0px; border-left-width: 5px; border-color: %1; 2px; border-left-style: solid;\">" ).arg( bgColor.name() );
      mNodeHelper->writeNodeToTempFile( node );
      QString href = mNodeHelper->asHREF( node, "header" );
      html += QString::fromLatin1( "<a href=\"" ) + href +
              QString::fromLatin1( "\">" );
      html += "<img style=\"vertical-align:middle;\" src=\"" + icon + "\"/>&nbsp;";
      if ( headerStyle() == HeaderStyle::enterprise() ) {
        QFont bodyFont = mCSSHelper->bodyFont( mUseFixedFont );
        QFontMetrics fm( bodyFont );
        html += fm.elidedText( label, Qt::ElideRight, 180 );
      } else if ( headerStyle() == HeaderStyle::fancy() ) {
        QFont bodyFont = mCSSHelper->bodyFont( mUseFixedFont );
        QFontMetrics fm( bodyFont );
        html += fm.elidedText( label, Qt::ElideRight, 1000 );
      } else {
        html += label;
      }
      html += "</a></span></div> ";
    }
  }

  KMime::Content *next  = MessageCore::NodeHelper::nextSibling( node );
  if ( next )
    html += renderAttachments( next, nextColor ( bgColor ) );

  return html;
}

KMime::Content* ViewerPrivate::findContentByType(KMime::Content *content, const QByteArray &type)
{
    KMime::Content::List list = content->contents();
    Q_FOREACH(KMime::Content *c, list)
    {
        if (c->contentType()->mimeType() ==  type)
            return c;
    }
    return 0L;

}


//-----------------------------------------------------------------------------
const QTextCodec* ViewerPrivate::codecForName(const QByteArray& _str)
{
  if (_str.isEmpty())
    return 0;
  QByteArray codec = _str;
  kAsciiToLower(codec.data());
  return KGlobal::charsets()->codecForName(codec);
}

void ViewerPrivate::update( Viewer::UpdateMode updateMode )
{
  // Avoid flicker, somewhat of a cludge
  if ( updateMode == Viewer::Force ) {
    // stop the timer to avoid calling updateReaderWin twice
    mUpdateReaderWinTimer.stop();
    saveRelativePosition();
    updateReaderWin();
  } else if ( mUpdateReaderWinTimer.isActive() ) {
    mUpdateReaderWinTimer.setInterval( delay );
  } else {
    mUpdateReaderWinTimer.start( 0 );
  }
}

void ViewerPrivate::slotUrlOpen( const QUrl& url )
{
  KUrl aUrl(url);
  mClickedUrl = aUrl;

  // First, let's see if the URL handler manager can handle the URL. If not, try KRun for some
  // known URLs, otherwise fallback to emitting a signal.
  // That signal is caught by KMail, and in case of mailto URLs, a composer is shown.

  if ( URLHandlerManager::instance()->handleClick( aUrl, this ) )
    return;

  // TODO: Factor out all this stuff into a KRun URL handler
  if ( ( mClickedUrl.protocol() == "http" ) || ( mClickedUrl.protocol() == "https" ) ||
       ( mClickedUrl.protocol() == "ftp" )  || ( mClickedUrl.protocol() == "file" )  ||
       ( mClickedUrl.protocol() == "ftps" ) || ( mClickedUrl.protocol() == "sftp" ) ||
       ( mClickedUrl.protocol() == "help" ) || ( mClickedUrl.protocol() == "vnc" )   ||
       ( mClickedUrl.protocol() == "smb" )  || ( mClickedUrl.protocol() == "fish" )  ||
       ( mClickedUrl.protocol() == "news" ) )
  {
    KPIM::BroadcastStatus::instance()->setTransientStatusMsg( i18n("Opening URL..."));
    QTimer::singleShot( 2000, KPIM::BroadcastStatus::instance(), SLOT( reset() ) );

    KMimeType::Ptr mime = KMimeType::findByUrl( mClickedUrl );
    if (mime->name() == "application/x-desktop" ||
        mime->name() == "application/x-executable" ||
        mime->name() == "application/x-ms-dos-executable" ||
        mime->name() == "application/x-shellscript" )
    {
      if ( KMessageBox::warningYesNo( 0, i18nc( "@info", "Do you really want to execute <filename>%1</filename>?",
          mClickedUrl.pathOrUrl() ), QString(), KGuiItem(i18n("Execute")), KStandardGuiItem::cancel() ) != KMessageBox::Yes)
        return;
    }
    if ( !MessageViewer::Util::handleUrlOnMac( mClickedUrl.pathOrUrl() ) ) {
      KRun *runner = new KRun( mClickedUrl, q ); // will delete itself
      runner->setRunExecutables( false );
    }

    return;
  }

  emit urlClicked( mMessageItem, mClickedUrl );
}


void ViewerPrivate::slotUrlOn(const QString& link, const QString& title, const QString& textContent )
{
  Q_UNUSED(title)
  Q_UNUSED(textContent)
  const KUrl url(link);
  if ( url.protocol() == "kmail" || url.protocol() == "x-kmail" || url.protocol() == "attachment" ||
       ( url.protocol().isEmpty() && url.path().isEmpty() ) ) {
    mViewer->setAcceptDrops( false );
  } else {
    mViewer->setAcceptDrops( true );
  }

  if ( link.trimmed().isEmpty() ) {
    KPIM::BroadcastStatus::instance()->reset();
    emit showStatusBarMessage( QString() );
    return;
  }

  QString msg = URLHandlerManager::instance()->statusBarMessage( url, this );
  if ( msg.isEmpty() ) {
    if ( !title.isEmpty() ) {
      msg = title;
    } else {
      msg = link;
    }
  }

  KPIM::BroadcastStatus::instance()->setTransientStatusMsg( msg );
  emit showStatusBarMessage( msg );
}

void ViewerPrivate::slotUrlPopup(const QString &aUrl, const QPoint& aPos)
{
  const KUrl url( aUrl );
  mClickedUrl = url;

  if ( URLHandlerManager::instance()->handleContextMenuRequest( url, aPos, this ) )
    return;

  emit popupMenu( mMessageItem, aUrl, aPos );
}

void ViewerPrivate::slotToggleHtmlMode()
{
  setHtmlOverride( !htmlMail() );
  update( Viewer::Force );
}

void ViewerPrivate::slotFind()
{
  mFindBar->show();
  mFindBar->focusAndSetCursor();
}


void ViewerPrivate::slotToggleFixedFont()
{
  mUseFixedFont = !mUseFixedFont;
  update( Viewer::Force );
}

void ViewerPrivate::slotToggleMimePartTree()
{
  if ( mToggleMimePartTreeAction->isChecked() )
    GlobalSettings::self()->setMimeTreeMode( GlobalSettings::EnumMimeTreeMode::Always );
  else
    GlobalSettings::self()->setMimeTreeMode( GlobalSettings::EnumMimeTreeMode::Never );
  showHideMimeTree();
}


void ViewerPrivate::slotShowMessageSource()
{
  mNodeHelper->messageWithExtraContent( mMessage.get() );
  const QString rawMessage = QString::fromAscii(  mMessage->encodedContent() );
  const QString htmlSource = mViewer->page()->mainFrame()->documentElement().toOuterXml();

  MailSourceViewer *viewer = new MailSourceViewer(); // deletes itself upon close
  viewer->setWindowTitle( i18n("Message as Plain Text") );
  viewer->setRawSource( rawMessage );
  viewer->setDisplayedSource( htmlSource );
  if( mUseFixedFont ) {
    viewer->setFont( KGlobalSettings::fixedFont() );
  }

  // Well, there is no widget to be seen here, so we have to use QCursor::pos()
  // Update: (GS) I'm not going to make this code behave according to Xinerama
  //         configuration because this is quite the hack.
  if ( QApplication::desktop()->isVirtualDesktop() ) {
    int scnum = QApplication::desktop()->screenNumber( QCursor::pos() );
    viewer->resize( QApplication::desktop()->screenGeometry( scnum ).width()/2,
                    2 * QApplication::desktop()->screenGeometry( scnum ).height()/3);
  } else {
    viewer->resize( QApplication::desktop()->geometry().width()/2,
                    2 * QApplication::desktop()->geometry().height()/3);
  }
  viewer->show();
}

void ViewerPrivate::updateReaderWin()
{
  if ( !mMsgDisplay ) {
    return;
  }

  if ( mRecursionCountForDisplayMessage + 1 > 1 ) {
    // This recursion here can happen because the ObjectTreeParser in parseMsg() can exec() an
    // eventloop.
    // This happens in two cases:
    //   1) The ContactSearchJob started by FancyHeaderStyle::format
    //   2) Various modal passphrase dialogs for decryption of a message (bug 96498)
    //
    // While the exec() eventloop is running, it is possible that a timer calls updateReaderWin(),
    // and not aborting here would confuse the state terribly.
    kWarning() << "Danger, recursion while displaying a message!";
    return;
  }
  mRecursionCountForDisplayMessage++;

  mViewer->setAllowExternalContent( htmlLoadExternal() );

  htmlWriter()->reset();
  //TODO: if the item doesn't have the payload fetched, try to fetch it? Maybe not here, but in setMessageItem.
  if ( mMessage )
  {
    if ( GlobalSettings::self()->showColorBar() ) {
      mColorBar->show();
    } else {
      mColorBar->hide();
    }
    displayMessage();
  } else {
    mColorBar->hide();
    mMimePartTree->hide();
  //FIXME(Andras)  mMimePartTree->clearAndResetSortOrder();
    htmlWriter()->begin( QString() );
    htmlWriter()->write( mCSSHelper->htmlHead( mUseFixedFont ) + "</body></html>" );
    htmlWriter()->end();
  }

  if ( mSavedRelativePosition ) {
    // FIXME: This doesn't work, Qt resets the scrollbar value somewhere in the event handler.
    //        Using a singleshot timer wouldn't work either, since that introduces visible scrolling.
    const float max = mViewer->page()->mainFrame()->scrollBarMaximum( Qt::Vertical );
    mViewer->page()->currentFrame()->setScrollBarValue( Qt::Vertical, max * mSavedRelativePosition );
    mSavedRelativePosition = 0;
  }
  mRecursionCountForDisplayMessage--;
}


void ViewerPrivate::slotMimePartSelected( const QModelIndex &index )
{
  KMime::Content *content = static_cast<KMime::Content*>( index.internalPointer() );
  if ( !mMimePartModel->parent( index ).isValid() && index.row() == 0 ) {
    update( Viewer::Force );
  } else {
    setMessagePart( content );
  }
}


void ViewerPrivate::slotCycleHeaderStyles() {
  const HeaderStrategy * strategy = headerStrategy();
  const HeaderStyle * style = headerStyle();

  const char * actionName = 0;
  if ( style == HeaderStyle::enterprise() ) {
    slotFancyHeaders();
    actionName = "view_headers_fancy";
  }
  if ( style == HeaderStyle::fancy() ) {
    slotBriefHeaders();
    actionName = "view_headers_brief";
  } else if ( style == HeaderStyle::brief() ) {
    slotStandardHeaders();
    actionName = "view_headers_standard";
  } else if ( style == HeaderStyle::plain() ) {
    if ( strategy == HeaderStrategy::standard() ) {
      slotLongHeaders();
      actionName = "view_headers_long";
    } else if ( strategy == HeaderStrategy::rich() ) {
      slotAllHeaders();
      actionName = "view_headers_all";
    } else if ( strategy == HeaderStrategy::all() ) {
      slotEnterpriseHeaders();
      actionName = "view_headers_enterprise";
    }
  }

  if ( actionName )
    static_cast<KToggleAction*>( mActionCollection->action( actionName ) )->setChecked( true );
}


void ViewerPrivate::slotBriefHeaders()
{
  setHeaderStyleAndStrategy( HeaderStyle::brief(),
                             HeaderStrategy::brief() );
  if( !mExternalWindow )
    writeConfig();
}


void ViewerPrivate::slotFancyHeaders()
{
  setHeaderStyleAndStrategy( HeaderStyle::fancy(),
                             HeaderStrategy::rich() );
  if( !mExternalWindow )
    writeConfig();
}


void ViewerPrivate::slotEnterpriseHeaders()
{
  setHeaderStyleAndStrategy( HeaderStyle::enterprise(),
                             HeaderStrategy::rich() );
  if( !mExternalWindow )
    writeConfig();
}


void ViewerPrivate::slotStandardHeaders()
{
  setHeaderStyleAndStrategy( HeaderStyle::plain(),
                             HeaderStrategy::standard());
  writeConfig();
}


void ViewerPrivate::slotLongHeaders()
{
  setHeaderStyleAndStrategy( HeaderStyle::plain(),
                             HeaderStrategy::rich() );
  if( !mExternalWindow )
    writeConfig();
}



void ViewerPrivate::slotAllHeaders() {
  setHeaderStyleAndStrategy( HeaderStyle::plain(),
                             HeaderStrategy::all() );
  if( !mExternalWindow )
    writeConfig();
}


void ViewerPrivate::slotCycleAttachmentStrategy()
{
  setAttachmentStrategy( attachmentStrategy()->next() );
  KToggleAction * action = actionForAttachmentStrategy( attachmentStrategy() );
  assert( action );
  action->setChecked( true );
}


void ViewerPrivate::slotIconicAttachments()
{
  setAttachmentStrategy( AttachmentStrategy::iconic() );
}


void ViewerPrivate::slotSmartAttachments()
{
  setAttachmentStrategy( AttachmentStrategy::smart() );
}


void ViewerPrivate::slotInlineAttachments()
{
  setAttachmentStrategy( AttachmentStrategy::inlined() );
}


void ViewerPrivate::slotHideAttachments()
{
  setAttachmentStrategy( AttachmentStrategy::hidden() );
}


void ViewerPrivate::attachmentView( KMime::Content *atmNode )
{
  if ( atmNode ) {

    const bool isEncapsulatedMessage = atmNode->parent() && atmNode->parent()->bodyIsMessage();
    if ( isEncapsulatedMessage ) {
       atmViewMsg( atmNode->parent()->bodyAsMessage() );
    } else if ((kasciistricmp(atmNode->contentType()->mediaType(), "text")==0) &&
               ( (kasciistricmp(atmNode->contentType()->subType(), "x-vcard")==0) ||
                 (kasciistricmp(atmNode->contentType()->subType(), "directory")==0) )) {
      setMessagePart( atmNode );
    } else {
      emit showReader( atmNode, htmlMail(), overrideEncoding() );
    }
  }
}


void ViewerPrivate::slotDelayedResize()
{
  mSplitter->setGeometry( 0, 0, q->width(), q->height() );
}


void ViewerPrivate::slotPrintMsg()
{
  disconnect( mPartHtmlWriter, SIGNAL( finished() ), this, SLOT( slotPrintMsg() ) );
  if ( !mMessage ) return;

  QPrinter printer;

  QPointer<QPrintDialog> dlg = new QPrintDialog( &printer, mViewer );
  if ( dlg->exec() == QDialog::Accepted && dlg ) {
    mViewer->page()->mainFrame()->print( &printer );
  }

  delete dlg;
}


void ViewerPrivate::slotSetEncoding()
{
  if ( mSelectEncodingAction->currentItem() == 0 ) // Auto
    mOverrideEncoding.clear();
  else
    mOverrideEncoding = NodeHelper::encodingForName( mSelectEncodingAction->currentText() );
  update( Viewer::Force );
}

void ViewerPrivate::injectAttachments()
{
  disconnect( mPartHtmlWriter, SIGNAL(finished()), this, SLOT( injectAttachments() ) );
  // inject attachments in header view
  // we have to do that after the otp has run so we also see encrypted parts

  QWebElement doc = mViewer->page()->currentFrame()->documentElement();
  QWebElement injectionPoint = doc.findFirst( "div#attachmentInjectionPoint" );
  if( injectionPoint.isNull() )
    return;

  QString imgpath( KStandardDirs::locate("data","libmessageviewer/pics/") );
  QString urlHandle;
  QString imgSrc;
  if( !mShowAttachmentQuicklist ) {
    urlHandle.append( "kmail:showAttachmentQuicklist" );
    imgSrc.append( "quicklistClosed.png" );
  } else {
    urlHandle.append( "kmail:hideAttachmentQuicklist" );
    imgSrc.append( "quicklistOpened.png" );
  }

  QColor background = KColorScheme( QPalette::Active, KColorScheme::View ).background().color();
  QString html = renderAttachments( mMessage.get(), background );
  Q_FOREACH( KMime::Content* node, mNodeHelper->extraContents( mMessage.get() ) ) {
    html += renderAttachments( node, background );
  }
  if ( html.isEmpty() )
    return;

  QString link("");
  if ( headerStyle() == HeaderStyle::fancy() ) {
    link += "<div style=\"text-align: left;\"><a href=\""+urlHandle+"\"><img src=\"file://"+imgpath+imgSrc+"\"/></a></div>";
    html.prepend( link );
    html.prepend( QString::fromLatin1("<div style=\"float:left;\">%1&nbsp;</div>" ).arg(i18n("Attachments:")) );
  } else {
    link += "<div style=\"text-align: right;\"><a href=\""+urlHandle+"\"><img src=\"file://"+imgpath+imgSrc+"\"/></a></div>";
    html.prepend( link );
  }

  assert( injectionPoint.tagName().toLower() == "div" );
  injectionPoint.setInnerXml( html );
}

void ViewerPrivate::slotSettingsChanged()
{
  update( Viewer::Force );
}

void ViewerPrivate::slotMimeTreeContextMenuRequested( const QPoint& pos )
{
  QModelIndex index = mMimePartTree->indexAt( pos );
  if ( index.isValid() ) {
     KMime::Content *content = static_cast<KMime::Content*>( index.internalPointer() );
     showContextMenu( content, pos );
  }
}

void ViewerPrivate::slotAttachmentOpenWith()
{
  QItemSelectionModel *selectionModel = mMimePartTree->selectionModel();
  QModelIndexList selectedRows = selectionModel->selectedRows();

  Q_FOREACH(QModelIndex index, selectedRows)
  {
     KMime::Content *content = static_cast<KMime::Content*>( index.internalPointer() );
     attachmentOpenWith( content );
 }
}

void ViewerPrivate::slotAttachmentOpen()
{

  QItemSelectionModel *selectionModel = mMimePartTree->selectionModel();
  QModelIndexList selectedRows = selectionModel->selectedRows();

  Q_FOREACH(QModelIndex index, selectedRows)
  {
    KMime::Content *content = static_cast<KMime::Content*>( index.internalPointer() );
    attachmentOpen( content );
  }
}

void ViewerPrivate::slotAttachmentSaveAs()
{
  KMime::Content::List contents = selectedContents();

  if ( contents.isEmpty() ) {
    KMessageBox::information( mMainWindow, i18n("Found no attachments to save.") );
    return;
  }

  Util::saveContents( mMainWindow, contents );
}

void ViewerPrivate::slotAttachmentSaveAll()
{
  KMime::Content::List contents = Util::extractAttachments( mMessage.get() );

  if ( contents.isEmpty() ) {
    KMessageBox::information( mMainWindow, i18n( "Found no attachments to save." ) );
    return;
  }

  Util::saveContents( mMainWindow, contents );
}

void ViewerPrivate::slotAttachmentView()
{
  KMime::Content::List contents = selectedContents();

  Q_FOREACH(KMime::Content *content, contents)
  {
    attachmentView( content );
  }

}

void ViewerPrivate::slotAttachmentProperties()
{
  KMime::Content::List contents = selectedContents();

  if ( contents.isEmpty() )
     return;

  Q_FOREACH( KMime::Content *content, contents ) {
    attachmentProperties( content );
  }
}

void ViewerPrivate::attachmentProperties( KMime::Content *content )
{
  KPIM::AttachmentPropertiesDialog *dialog = new KPIM::AttachmentPropertiesDialog( content, mMainWindow );
  dialog->setAttribute( Qt::WA_DeleteOnClose );
  dialog->show();
}



void ViewerPrivate::slotAttachmentCopy()
{
  KMime::Content::List contents = selectedContents();
  attachmentCopy( contents );
}

void ViewerPrivate::attachmentCopy( const KMime::Content::List & contents )
{
  if ( contents.isEmpty() )
    return;

  QList<QUrl> urls;
  Q_FOREACH( KMime::Content *content, contents) {
    KUrl kUrl = mNodeHelper->writeNodeToTempFile( content );
    QUrl url = QUrl::fromPercentEncoding( kUrl.toEncoded() );
    if ( !url.isValid() )
      continue;
    urls.append( url );
  }

  if ( urls.isEmpty() )
    return;

  QMimeData *mimeData = new QMimeData;
  mimeData->setUrls( urls );
  QApplication::clipboard()->setMimeData( mimeData, QClipboard::Clipboard );
}


void ViewerPrivate::slotAttachmentDelete()
{
  KMime::Content::List contents = selectedContents();
  if ( contents.isEmpty() )
    return;

  bool showWarning = true;
  Q_FOREACH( KMime::Content *content, contents ) {
    if ( !deleteAttachment( content, showWarning ) )
      return;
    showWarning = false;
  }
  update();
}


void ViewerPrivate::slotAttachmentEdit()
{
  KMime::Content::List contents = selectedContents();
  if ( contents.isEmpty() )
    return;

  bool showWarning = true;
  Q_FOREACH( KMime::Content *content, contents ) {
    if ( !editAttachment( content, showWarning ) )
      return;
    showWarning = false;
  }
}


void ViewerPrivate::slotAttachmentEditDone( EditorWatcher* editorWatcher )
{
  QString name = editorWatcher->url().fileName();
  if ( editorWatcher->fileChanged() ) {
    QFile file( name );
    if ( file.open( QIODevice::ReadOnly ) ) {
      QByteArray data = file.readAll();
      KMime::Content *node = mEditorWatchers[editorWatcher];
      node->setBody( data );
      file.close();

      mMessageItem.setPayloadFromData( mMessage->encodedContent() );
      Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob( mMessageItem );
    }
  }
  mEditorWatchers.remove( editorWatcher );
  QFile::remove( name );
}

void ViewerPrivate::slotLevelQuote( int l )
{
  mLevelQuote = l;
  update( Viewer::Force );
}


void ViewerPrivate::slotHandleAttachment( int choice )
{
  //mAtmUpdate = true;
  if(!mCurrentContent)
    return;
  if ( choice == Viewer::Delete ) {
    deleteAttachment( mCurrentContent );
  } else if ( choice == Viewer::Edit ) {
    editAttachment( mCurrentContent );
  } else if ( choice == Viewer::Properties ) {
    attachmentProperties( mCurrentContent );
  } else if ( choice == Viewer::Save ) {
    Util::saveContents( mMainWindow, KMime::Content::List() << mCurrentContent );
  } else if ( choice == Viewer::OpenWith ) {
    attachmentOpenWith( mCurrentContent );
  } else if ( choice == Viewer::Open ) {
    attachmentOpen( mCurrentContent );
  } else if ( choice == Viewer::View ) {
    attachmentView( mCurrentContent );
  } else if ( choice == Viewer::ChiasmusEncrypt ) {
    attachmentEncryptWithChiasmus( mCurrentContent );
  } else if ( choice == Viewer::Copy ) {
    attachmentCopy( KMime::Content::List()<< mCurrentContent );
  } else if ( choice == Viewer::ScrollTo ) {
    scrollToAttachment( mCurrentContent );
  }
  else {
    kDebug() << " not implemented :" << choice;
  }
}

void ViewerPrivate::slotCopySelectedText()
{
  QString selection = mViewer->selectedText();
  selection.replace( QChar::Nbsp, ' ' );
  QApplication::clipboard()->setText( selection );
}

void ViewerPrivate::selectAll()
{
  mViewer->page()->triggerAction(QWebPage::SelectAll);
}

void ViewerPrivate::slotUrlCopy()
{
  QClipboard* clip = QApplication::clipboard();
  if ( mClickedUrl.protocol() == "mailto" ) {
    // put the url into the mouse selection and the clipboard
    QString address = KPIMUtils::decodeMailtoUrl( mClickedUrl );
    clip->setText( address, QClipboard::Clipboard );
    clip->setText( address, QClipboard::Selection );
    KPIM::BroadcastStatus::instance()->setStatusMsg( i18n( "Address copied to clipboard." ));
  } else {
    // put the url into the mouse selection and the clipboard
    clip->setText( mClickedUrl.url(), QClipboard::Clipboard );
    clip->setText( mClickedUrl.url(), QClipboard::Selection );
    KPIM::BroadcastStatus::instance()->setStatusMsg( i18n( "URL copied to clipboard." ));
  }
}

void ViewerPrivate::slotSaveMessage()
{
  if( !mMessage )
    return;

  const QString initialFileName = MessageCore::StringUtil::cleanFileName(
                                           mMessage->subject()->asUnicodeString().trimmed() );
  const KUrl url = KFileDialog::getSaveUrl( KUrl::fromPath( initialFileName ), "*.mbox", mMainWindow );

  if ( url.isEmpty() )
    return;

  Util::saveContent( mMainWindow, mMessage.get(), url );
}

void ViewerPrivate::saveRelativePosition()
{
  const bool hasScrollbar = mViewer->page()->mainFrame()->scrollBarGeometry( Qt::Vertical ).isValid();
  if ( hasScrollbar ) {
    const float pos = mViewer->page()->mainFrame()->scrollBarValue( Qt::Vertical );
    const float height = mViewer->page()->mainFrame()->scrollBarMaximum( Qt::Vertical );
    mSavedRelativePosition = pos / height;
  }
  else {
    mSavedRelativePosition = 0;
  }
}

//TODO(Andras) inline them
bool ViewerPrivate::htmlMail() const
{
  return ((mHtmlMail && !mHtmlOverride) || (!mHtmlMail && mHtmlOverride));
}

bool ViewerPrivate::htmlLoadExternal() const
{
  return ((mHtmlLoadExternal && !mHtmlLoadExtOverride) ||
          (!mHtmlLoadExternal && mHtmlLoadExtOverride));
}

void ViewerPrivate::setHtmlOverride( bool override )
{
  mHtmlOverride = override;

  // keep toggle display mode action state in sync.
  if ( mToggleDisplayModeAction )
    mToggleDisplayModeAction->setChecked( htmlMail() );
}

bool ViewerPrivate::htmlOverride() const
{
  return mHtmlOverride;
}

void ViewerPrivate::setHtmlLoadExtOverride( bool override )
{
  mHtmlLoadExtOverride = override;
}

bool ViewerPrivate::htmlLoadExtOverride() const
{
  return mHtmlLoadExtOverride;
}

void ViewerPrivate::setDecryptMessageOverwrite( bool overwrite )
{
  mDecrytMessageOverwrite = overwrite;
}

bool ViewerPrivate::showSignatureDetails() const
{
  return mShowSignatureDetails;
}

void ViewerPrivate::setShowSignatureDetails( bool showDetails )
{
  mShowSignatureDetails = showDetails;
}

bool ViewerPrivate::showAttachmentQuicklist() const
{
  return mShowAttachmentQuicklist;
}

void ViewerPrivate::setShowAttachmentQuicklist( bool showAttachmentQuicklist  )
{
  mShowAttachmentQuicklist = showAttachmentQuicklist;
}

void ViewerPrivate::scrollToAttachment( const KMime::Content *node )
{
  QWebElement doc = mViewer->page()->mainFrame()->documentElement();

  // The anchors for this are created in ObjectTreeParser::parseObjectTree()
  QWebElement link = doc.findFirst( QString::fromLatin1( "a#att%1" ).arg( node->index().toString() ) );
  if( link.isNull() ) {
    return;
  }

  int linkPos = link.geometry().bottom();
  int viewerPos  = mViewer->page()->mainFrame()->scrollPosition().y();
  link.setFocus();
  mViewer->page()->mainFrame()->scroll(0, linkPos - viewerPos );

  // Remove any old color markings which might be there
  const KMime::Content *root = node->topLevel();
  int totalChildCount = Util::allContents( root ).size();
  for ( int i = 0; i <= totalChildCount + 1; i++ ) {
    QWebElement attachmentDiv = doc.findFirst( QString( "div#attachmentDiv%1" ).arg( i + 1 ) );
    if ( !attachmentDiv.isNull() )
      attachmentDiv.removeAttribute( "style" );
  }

  // Now, color the div of the attachment in yellow, so that the user sees what happened.
  // We created a special marked div for this in writeAttachmentMarkHeader() in ObjectTreeParser,
  // find and modify that now.

  QWebElement attachmentDiv = doc.findFirst( QString( "div#attachmentDiv%1" ).arg( node->index().toString() ) );
  if ( attachmentDiv.isNull() ) {
    return;
  }
  attachmentDiv.setAttribute( "style", QString( "border:2px solid %1" ).arg( cssHelper()->pgpWarnColor().name() ) );
}

void ViewerPrivate::setUseFixedFont( bool useFixedFont )
{
  mUseFixedFont = useFixedFont;
  if ( mToggleFixFontAction )
  {
    mToggleFixFontAction->setChecked( mUseFixedFont );
  }
}

void ViewerPrivate::attachmentEncryptWithChiasmus( KMime::Content *content )
{
  // FIXME: better detection of mimetype??
  if ( !mCurrentFileName.endsWith( QLatin1String(".xia"), Qt::CaseInsensitive ) )
    return;

  const Kleo::CryptoBackend::Protocol * chiasmus =
    Kleo::CryptoBackendFactory::instance()->protocol( "Chiasmus" );
  Q_ASSERT( chiasmus );
  if ( !chiasmus )
    return;

  const std::auto_ptr<Kleo::SpecialJob> listjob( chiasmus->specialJob( "x-obtain-keys", QMap<QString,QVariant>() ) );
  if ( !listjob.get() ) {
    const QString msg = i18n( "Chiasmus backend does not offer the "
                              "\"x-obtain-keys\" function. Please report this bug." );
    KMessageBox::error( mMainWindow, msg, i18n( "Chiasmus Backend Error" ) );
    return;
  }

  if ( listjob->exec() ) {
    listjob->showErrorDialog( mMainWindow, i18n( "Chiasmus Backend Error" ) );
    return;
  }

  const QVariant result = listjob->property( "result" );
  if ( result.type() != QVariant::StringList ) {
    const QString msg = i18n( "Unexpected return value from Chiasmus backend: "
                              "The \"x-obtain-keys\" function did not return a "
                              "string list. Please report this bug." );
    KMessageBox::error( mMainWindow, msg, i18n( "Chiasmus Backend Error" ) );
    return;
  }

  const QStringList keys = result.toStringList();
  if ( keys.empty() ) {
    const QString msg = i18n( "No keys have been found. Please check that a "
                              "valid key path has been set in the Chiasmus "
                              "configuration." );
    KMessageBox::error( mMainWindow, msg, i18n( "Chiasmus Backend Error" ) );
    return;
  }
  AutoQPointer<ChiasmusKeySelector> selectorDlg;
  selectorDlg = new ChiasmusKeySelector( mMainWindow,
                                         i18n( "Chiasmus Decryption Key Selection" ),
                                         keys, GlobalSettings::chiasmusDecryptionKey(),
                                         GlobalSettings::chiasmusDecryptionOptions() );
  if ( selectorDlg->exec() != QDialog::Accepted || !selectorDlg ) {
    return;
  }

  GlobalSettings::setChiasmusDecryptionOptions( selectorDlg->options() );
  GlobalSettings::setChiasmusDecryptionKey( selectorDlg->key() );
  assert( !GlobalSettings::chiasmusDecryptionKey().isEmpty() );
  Kleo::SpecialJob * job = chiasmus->specialJob( "x-decrypt", QMap<QString,QVariant>() );
  if ( !job ) {
    const QString msg = i18n( "Chiasmus backend does not offer the "
                              "\"x-decrypt\" function. Please report this bug." );
    KMessageBox::error( mMainWindow, msg, i18n( "Chiasmus Backend Error" ) );
    return;
  }

  //PORT IT
  const QByteArray input;// = node->msgPart().bodyDecodedBinary();

  if ( !job->setProperty( "key", GlobalSettings::chiasmusDecryptionKey() ) ||
       !job->setProperty( "options", GlobalSettings::chiasmusDecryptionOptions() ) ||
       !job->setProperty( "input", input ) ) {
    const QString msg = i18n( "The \"x-decrypt\" function does not accept "
                              "the expected parameters. Please report this bug." );
    KMessageBox::error( mMainWindow, msg, i18n( "Chiasmus Backend Error" ) );
    return;
  }

  if ( job->start() ) {
    job->showErrorDialog( mMainWindow, i18n( "Chiasmus Decryption Error" ) );
    return;
  }

  mJob = job;
  connect( job, SIGNAL( result( const GpgME::Error&, const QVariant&) ),
           this, SLOT( slotAtmDecryptWithChiasmusResult( const GpgME::Error&, const QVariant& ) ) );
}


static const QString chomp( const QString & base, const QString & suffix, bool cs ) {
  return base.endsWith( suffix, cs ? (Qt::CaseSensitive) : (Qt::CaseInsensitive) ) ? base.left( base.length() - suffix.length() ) : base ;
}


void ViewerPrivate::slotAtmDecryptWithChiasmusResult( const GpgME::Error & err, const QVariant & result )
{
  if ( !mJob )
    return;
  Q_ASSERT( mJob == sender() );
  if ( mJob != sender() )
    return;
  Kleo::Job * job = mJob;
  mJob = 0;
  if ( err.isCanceled() )
    return;
  if ( err ) {
    job->showErrorDialog( mMainWindow, i18n( "Chiasmus Decryption Error" ) );
    return;
  }

  if ( result.type() != QVariant::ByteArray ) {
    const QString msg = i18n( "Unexpected return value from Chiasmus backend: "
                              "The \"x-decrypt\" function did not return a "
                              "byte array. Please report this bug." );
    KMessageBox::error( mMainWindow, msg, i18n( "Chiasmus Backend Error" ) );
    return;
  }

  const KUrl url = KFileDialog::getSaveUrl( chomp( mCurrentFileName, ".xia", false ), QString(), mMainWindow );
  if ( url.isEmpty() )
    return;

  bool overwrite = Util::checkOverwrite( url, mMainWindow );
  if ( !overwrite )
    return;

  KIO::Job * uploadJob = KIO::storedPut( result.toByteArray(), url, -1, KIO::Overwrite );
  uploadJob->ui()->setWindow( mMainWindow );
  connect( uploadJob, SIGNAL(result(KJob*)),
           this, SLOT(slotAtmDecryptWithChiasmusUploadResult(KJob*)) );
}

void ViewerPrivate::slotAtmDecryptWithChiasmusUploadResult( KJob * job )
{
  if ( job->error() )
    static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
}

bool ViewerPrivate::showFullToAddressList() const
{
  return mShowFullToAddressList;
}

void ViewerPrivate::setShowFullToAddressList( bool showFullToAddressList )
{
  mShowFullToAddressList = showFullToAddressList;
}

bool ViewerPrivate::showFullCcAddressList() const
{
  return mShowFullCcAddressList;
}

void ViewerPrivate::setShowFullCcAddressList( bool showFullCcAddressList )
{
  mShowFullCcAddressList = showFullCcAddressList;
}

void ViewerPrivate::toggleFullAddressList()
{
  toggleFullAddressList( "To" );
  toggleFullAddressList( "Cc" );
}

void ViewerPrivate::toggleFullAddressList( const QString &field )
{
  QWebElement doc = mViewer->page()->currentFrame()->documentElement();
  // First inject the correct icon
  QWebElement tag = doc.findFirst( QString( "span#iconFull%1AddressList" ).arg( field ) );
  if ( tag.isNull() ) {
    return;
  }

  QString imgpath( KStandardDirs::locate( "data","libmessageviewer/pics/" ) );
  QString urlHandle;
  QString imgSrc;
  QString altText;
  bool doShow = ( field == "To" && showFullToAddressList() ) || ( field == "Cc" && showFullCcAddressList() );
  if ( doShow ) {
    urlHandle.append( "kmail:hideFull" + field + "AddressList" );
    imgSrc.append( "quicklistOpened.png" );
    altText = i18n("Hide full address list");
  } else {
    urlHandle.append( "kmail:showFull" + field + "AddressList" );
    imgSrc.append( "quicklistClosed.png" );
    altText = i18n("Show full address list");
  }

  QString link = "<span style=\"text-align: right;\"><a href=\"" + urlHandle + "\"><img src=\"file://" + imgpath + imgSrc + "\""
                 "alt=\"" + altText + "\" /></a></span>";
  tag.setInnerXml( link );

  // Then show/hide the full address list
  QWebElement dotsTag = doc.findFirst( QString( "span#dotsFull%1AddressList" ).arg( field ) );
  Q_ASSERT( !dotsTag.isNull() );

  tag = doc.findFirst( QString( "span#hiddenFull%1AddressList" ).arg( field ) );
  Q_ASSERT( !tag.isNull() );

  if ( doShow ) {
    dotsTag.setStyleProperty( "display", "none" );
    tag.removeAttribute( "display" );
  } else {
    tag.setStyleProperty( "display", "none" );
    dotsTag.removeAttribute( "display" );
  }
}

void ViewerPrivate::itemFetchResult( KJob* job )
{
  if ( job->error() ) {
    displaySplashPage( i18n( "Message loading failed: %1.", job->errorText() ) );
  } else {
    Akonadi::ItemFetchJob* fetch = qobject_cast<Akonadi::ItemFetchJob*>( job );
    Q_ASSERT( fetch );
    if ( fetch->items().size() < 1 ) {
      displaySplashPage( i18n( "Message not found." ) );
    } else {
      setMessageItem( fetch->items().first() );
    }
  }
}

void ViewerPrivate::slotItemChanged( const Akonadi::Item &item, const QSet<QByteArray> & )
{
  if ( item.id() != messageItem().id() ) {
    kDebug() << "Update for an already forgotten item. Weird.";
    return;
  }
  setMessageItem( item );
}

#include "viewer_p.moc"
