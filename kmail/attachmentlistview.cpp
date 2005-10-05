/*  -*- c++ -*-
    attachmentlistview.cpp

    KMail, the KDE mail client.
    Copyright (c) 2003 Ingo Kloecker <kloecker@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// my header file
#include "attachmentlistview.h"

// other KMail headers
#include "kmmsgbase.h"
#include "kmfolder.h"
#include "kmcommands.h"
#include "kmmsgdict.h"
#include "composer.h"

// other module headers
#include <maillistdrag.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QKeyEvent>
#include <QDropEvent>
using KPIM::MailListDrag;

// other KDE headers
#include <kurldrag.h>

// other Qt headers
#include <qevent.h>
#include <q3cstring.h>
#include <qbuffer.h>
#include <q3ptrlist.h>
#include <qdatastream.h>
#include <qstring.h>

// other headers (none)


namespace KMail {

AttachmentListView::AttachmentListView( KMail::Composer * composer,
                                        QWidget* parent )
  : KListView( parent ),
    mComposer( composer )
{
  setAcceptDrops( true );
}

//-----------------------------------------------------------------------------

AttachmentListView::~AttachmentListView()
{
}

//-----------------------------------------------------------------------------

void AttachmentListView::contentsDragEnterEvent( QDragEnterEvent* e )
{
  if( e->provides( MailListDrag::format() ) )
    e->accept( true );
  else
    KListView::dragEnterEvent( e );
}

//-----------------------------------------------------------------------------

void AttachmentListView::contentsDragMoveEvent( QDragMoveEvent* e )
{
  if( e->provides( MailListDrag::format() ) )
    e->accept( true );
  else
    KListView::dragMoveEvent( e );
}

//-----------------------------------------------------------------------------

void AttachmentListView::contentsDropEvent( QDropEvent* e )
{
  if( e->provides( MailListDrag::format() ) ) {
    // Decode the list of serial numbers stored as the drag data
    QByteArray serNums;
    MailListDrag::decode( e, serNums );
    QBuffer serNumBuffer( &serNums );
    serNumBuffer.open( QIODevice::ReadOnly );
    QDataStream serNumStream( &serNumBuffer );
    unsigned long serNum;
    KMFolder *folder = 0;
    int idx;
    Q3PtrList<KMMsgBase> messageList;
    while( !serNumStream.atEnd() ) {
      KMMsgBase *msgBase = 0;
      serNumStream >> serNum;
      KMMsgDict::instance()->getLocation( serNum, &folder, &idx );
      if( folder )
        msgBase = folder->getMsgBase( idx );
      if( msgBase )
        messageList.append( msgBase );
    }
    serNumBuffer.close();
    uint identity = folder ? folder->identity() : 0;
    KMCommand *command = new KMForwardAttachedCommand( mComposer, messageList,
                                                       identity, mComposer );
    command->start();
  }
  else if( KURLDrag::canDecode( e ) ) {
    KURL::List urlList;
    if( KURLDrag::decode( e, urlList ) ) {
      for( KURL::List::Iterator it = urlList.begin();
           it != urlList.end(); ++it ) {
        mComposer->addAttach( *it );
      }
    }
  }
  else {
    KListView::dropEvent( e );
  }
}

//-----------------------------------------------------------------------------

void AttachmentListView::keyPressEvent( QKeyEvent * e )
{
  if ( e->key() == Qt::Key_Delete ) {
    emit attachmentDeleted();
  }
}


} // namespace KMail

#include "attachmentlistview.moc"
