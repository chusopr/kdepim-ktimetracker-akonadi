#include "kmmimeparttree.h"
#include "kmreaderwin.h"
#include "partNode.h"
#include <qheader.h>
#include <kdebug.h>
#include <klocale.h>
#include <qpopupmenu.h>
#include <kfiledialog.h>
#include <qfile.h>
#include <kmessagebox.h>


KMMimePartTree::KMMimePartTree( KMReaderWin* readerWin,
                                QWidget* parent,
                                const char* name )
    : QListView(  parent, name ),
      mReaderWin( readerWin )
{
    addColumn( i18n("Description") );
    addColumn( i18n("Type") );
    addColumn( i18n("Encoding") );
    addColumn( i18n("Size") );
    setColumnAlignment(3, Qt::AlignRight);
    setResizeMode( QListView::LastColumn );
    connect( this, SIGNAL( clicked( QListViewItem* ) ),
             this, SLOT( itemClicked( QListViewItem* ) ) );
    connect( this, SIGNAL( contextMenuRequested( QListViewItem*,
                                                 const QPoint&, int ) ),
             this, SLOT( itemRightClicked( QListViewItem*, const QPoint& ) ) );
    setRootIsDecorated( false );
    setShowToolTips( true );
    setAllColumnsShowFocus( true );
    setShowToolTips( true );
    setSorting(-1);
}

void KMMimePartTree::itemClicked( QListViewItem* item )
{
    KMMimePartTreeItem* i = dynamic_cast<KMMimePartTreeItem*>( item );
    if ( 0L == i ) {
        kdDebug(5006) << "Item was not a KMMimePartTreeItem!" << endl;
    }
    else {
        kdDebug(5006) << "\n**\n** KMMimePartTree::itemClicked() **\n**" << endl;
        mReaderWin->parseObjectTree( i->node(), true );
    }
}


void KMMimePartTree::itemRightClicked( QListViewItem* item,
                                       const QPoint& point )
{
    mCurrentContextMenuItem = dynamic_cast<KMMimePartTreeItem*>( item );
    if ( 0L == mCurrentContextMenuItem ) {
        kdDebug(5006) << "Item was not a KMMimePartTreeItem!" << endl;
    }
    else {
        kdDebug(5006) << "\n**\n** KMMimePartTree::itemRightClicked() **\n**" << endl;
//        mReaderWin->parseObjectTree( mCurrentContextMenuItem->node(), true );
        QPopupMenu* popup = new QPopupMenu;
        popup->insertItem( i18n( "Save &As..." ), this, SLOT( slotSaveAs() ) );
        popup->insertItem( i18n( "Save As &Encoded..." ), this,
                           SLOT( slotSaveAsEncoded() ) );
        popup->exec( point );
        //mReaderWin->parseObjectTree( mCurrentContextMenuItem->node(), true );
        mCurrentContextMenuItem = 0;
    }
}


void KMMimePartTree::slotSaveAs()
{
    if( mCurrentContextMenuItem ) {  // this should always be true
        QString s = mCurrentContextMenuItem->text(0);
        if( s.startsWith( "file: " ) )
            s = s.mid(6).stripWhiteSpace();
        else
            s = "";
        QString filename = KFileDialog::getSaveFileName( s,
                                                         QString::null,
                                                         this, QString::null );
        if( !filename.isEmpty() ) {
            if( QFile::exists( filename ) ) {
                if( KMessageBox::warningYesNo( this,
                                               i18n( "A file with this name already exists. Do you want to overwrite it?" ),
                                               i18n( "KMail Warning" ) ) ==
                    KMessageBox::No )
                    return;
            }

            bool bSaveEncrypted = false;
            bool bEncryptedParts = mCurrentContextMenuItem->node()->isEncrypted();
            if( bEncryptedParts )
                if( KMessageBox::questionYesNo( this,
                                                i18n( "This part of the message is encrypted. Do you want to keep the encryption when saving?" ),
                                                i18n( "KMail Question" ) ) ==
                    KMessageBox::Yes )
                    bSaveEncrypted = true;

            bool bSaveWithSig = true;
            if( mCurrentContextMenuItem->node()->isSigned() )
                if( KMessageBox::questionYesNo( this,
                                                i18n( "This part of the message is signed. Do you want to keep the signature when saving?" ),
                                                i18n( "KMail Question" ) ) !=
                    KMessageBox::Yes )
                    bSaveWithSig = false;

            QFile file( filename );
            if( file.open( IO_WriteOnly ) ) {
                QDataStream ds( &file );
                if( (bSaveEncrypted || !bEncryptedParts) && bSaveWithSig ) {
                    QByteArray cstr = mCurrentContextMenuItem->node()->msgPart().bodyDecodedBinary();
                    ds.writeRawBytes( cstr, cstr.size() );
                } else {

                    mReaderWin->parseObjectTree( mCurrentContextMenuItem->node(), true,
                                                                                  bSaveEncrypted,
                                                                                  bSaveWithSig );

                }
                file.close();
            } else
                KMessageBox::error( this, i18n( "Could not write the file" ),
                                    i18n( "KMail Error" ) );
        }
    }
}


void KMMimePartTree::slotSaveAsEncoded()
{
    if( mCurrentContextMenuItem ) {  // this should always be true
        QString s = mCurrentContextMenuItem->text(0);
        if( s.startsWith( "file: " ) )
            s = s.mid(6).stripWhiteSpace();
        else
            s = "";
        QString filename = KFileDialog::getSaveFileName( s,
                                                         QString::null,
                                                         this, QString::null );
        if( !filename.isEmpty() ) {
            if( QFile::exists( filename ) ) {
                if( KMessageBox::warningYesNo( this,
                                               i18n( "A file with this name already exists. Do you want to overwrite it?" ),
                                               i18n( "KMail Warning" ) ) ==
                    KMessageBox::No )
                    return;
            }

            // Note:   SaveAsEncoded *allways* stores the ENCRYPTED content with SIGNATURES.
            // reason: SaveAsEncoded does not decode the Message Content-Transfer-Encoding
            //         but saves the _original_ content of the message (or the message part, resp.)

            QFile file( filename );
            if( file.open( IO_WriteOnly ) ) {
                QDataStream ds( &file );
                QCString cstr( mCurrentContextMenuItem->node()->msgPart().body() );
                ds.writeRawBytes( cstr, cstr.length() );
                file.close();
            } else
                KMessageBox::error( this, i18n( "Could not write the file" ),
                                    i18n( "KMail Error" ) );
        }
    }
}


KMMimePartTreeItem::KMMimePartTreeItem( KMMimePartTree& parent,
                                        partNode* node,
                                        const QString& labelDescr,
                                        QString labelCntType,
                                        QString labelEncoding,
                                        QString labelSize )
    : QListViewItem( &parent, labelDescr,
                     labelCntType,
                     labelEncoding,
                     labelSize ),
      mPartNode( node )
{
    if( node )
        node->setMimePartTreeItem( this );
}

KMMimePartTreeItem::KMMimePartTreeItem( KMMimePartTreeItem& parent,
                                        partNode* node,
                                        const QString& labelDescr,
                                        QString labelCntType,
                                        QString labelEncoding,
                                        QString labelSize )
    : QListViewItem( &parent, labelDescr,
                     labelCntType,
                     labelEncoding,
                     labelSize ),
      mPartNode( node )
{
    if( node )
        node->setMimePartTreeItem( this );
}


#include "kmmimeparttree.moc"
