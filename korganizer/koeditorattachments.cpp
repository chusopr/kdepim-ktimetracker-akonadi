/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "koeditorattachments.h"

#include "kmailIface_stub.h"

#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>

KOEditorAttachments::KOEditorAttachments( int spacing, QWidget *parent,
                                          const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( spacing );

  mAttachments = new QListView( this );
  mAttachments->addColumn( i18n("URI") );
  mAttachments->addColumn( i18n("MIME Type") );
  topLayout->addWidget( mAttachments );
  connect( mAttachments, SIGNAL( doubleClicked( QListViewItem * ) ),
           SLOT( showAttachment( QListViewItem * ) ) );

  QBoxLayout *buttonLayout = new QHBoxLayout( topLayout );
  
  QPushButton *button = new QPushButton( i18n("Add..."), this );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotAdd() ) );

  button = new QPushButton( i18n("Edit..."), this );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotEdit() ) );

  button = new QPushButton( i18n("Remove"), this );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotRemove() ) );

  button = new QPushButton( i18n("Show"), this );
  buttonLayout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( slotShow() ) );
}

KOEditorAttachments::~KOEditorAttachments()
{
}

void KOEditorAttachments::showAttachment( QListViewItem *item )
{
  if ( !item ) return;
  
  QString uri = item->text( 0 );
  
  if ( uri.startsWith( "kmail:" ) ) {
    int pos = uri.find( "/" );
    if ( pos > 5 ) {
      QString serialNumberStr = uri.mid( 6, pos - 6 );
      QString messageId = uri.mid( pos + 1 );
      kdDebug() << "SERIALNUMBERSTR: " << serialNumberStr << " MESSAGEID: "
                << messageId << endl;
      Q_UINT32 serialNumber = serialNumberStr.toUInt();
      kdDebug() << "SERIALNUMBER: " << serialNumber << endl;
      KMailIface_stub kmailIface( "kmail", "KMailIface" );
      kmailIface.showMail( serialNumber, messageId );
    }
  }
}

void KOEditorAttachments::slotAdd()
{
  QString uri = KInputDialog::getText( i18n("Add Attachment"),
                                       i18n("Please put in URI of attachment."),
                                       QString::null, 0, this );
  if ( !uri.isEmpty() ) {
    new QListViewItem( mAttachments, uri );
  }
}

void KOEditorAttachments::slotEdit()
{
  QListViewItem *item = mAttachments->currentItem();
  if ( !item ) return;
  
  QString uri = KInputDialog::getText( i18n("Edit Attachment"),
                                       i18n("Please put in URI of attachment."),
                                       item->text( 0 ), 0, this );

  if ( !uri.isEmpty() ) item->setText( 0, uri );
}

void KOEditorAttachments::slotRemove()
{
  QListViewItem *item = mAttachments->currentItem();
  delete item;
}

void KOEditorAttachments::slotShow()
{
  showAttachment( mAttachments->currentItem() );
}

void KOEditorAttachments::setDefaults()
{
  mAttachments->clear();
}

void KOEditorAttachments::addAttachment( const QString &uri,
                                         const QString &mimeType )
{
  new QListViewItem( mAttachments, uri, mimeType );  
}

void KOEditorAttachments::readEvent( Incidence *i )
{
  mAttachments->clear();

  Attachment::List attachments = i->attachments();
  Attachment *a;
  for( a = attachments.first(); a; a = attachments.next() ) {
    QString uri;
    if ( a->isURI() ) uri = a->uri();
    else uri = i18n("[Binary data]");
    addAttachment( uri, a->mimeType() );
  }
}

void KOEditorAttachments::writeEvent( Incidence *i )
{
  i->clearAttachments();

  QListViewItem *item;
  for( item = mAttachments->firstChild(); item; item = item->nextSibling() ) {
    i->addAttachment( new Attachment( item->text( 0 ), item->text( 1 ) ) );
  }
}

#include "koeditorattachments.moc"
