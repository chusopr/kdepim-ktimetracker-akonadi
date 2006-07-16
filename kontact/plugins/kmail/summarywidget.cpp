/*  -*- mode: C++; c-file-style: "gnu" -*-

    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <QLabel>
#include <QLayout>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QEvent>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>

#include "core.h"
#include "summary.h"
#include "summarywidget.h"

#include <time.h>

SummaryWidget::SummaryWidget( Kontact::Plugin *plugin, QWidget *parent )
  : Kontact::Summary( parent ),
//    DCOPObject( "MailSummary" ),
    mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setSpacing( 3 );
  mainLayout->setMargin( 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_mail", K3Icon::Desktop,
                                                  K3Icon::SizeMedium );
  QWidget *header = createHeader(this, icon, i18n("New Messages"));
  mLayout = new QGridLayout();
  mLayout->setSpacing( 3 );

  mainLayout->addWidget(header);
  mainLayout->addLayout(mLayout);

  slotUnreadCountChanged();
#warning Port DCOP signal!
//  connectDCOPSignal( 0, 0, "unreadCountChanged()", "slotUnreadCountChanged()",
//                     false );
}

void SummaryWidget::selectFolder( const QString& folder )
{
  if ( mPlugin->isRunningStandalone() )
    mPlugin->bringToForeground();
  else
    mPlugin->core()->selectPlugin( mPlugin );
  QByteArray data;
  QDataStream arg( &data, QIODevice::WriteOnly );
  arg << folder;
#warning Port DCOP signal!
//  emitDCOPSignal( "kmailSelectFolder(QString)", data );
}

void SummaryWidget::updateSummary( bool )
{
  // check whether we need to update the message counts
#warning Port to DBus!
/*  DCOPRef kmail( "kmail", "KMailIface" );
  const int timeOfLastMessageCountChange =
    kmail.call( "timeOfLastMessageCountChange()" );
  if ( timeOfLastMessageCountChange > mTimeOfLastMessageCountUpdate )
    slotUnreadCountChanged();*/
}

void SummaryWidget::slotUnreadCountChanged()
{
#warning Port to DBus!
/*  DCOPRef kmail( "kmail", "KMailIface" );
  DCOPReply reply = kmail.call( "folderList" );
  if ( reply.isValid() ) {
    QStringList folderList = reply;
    updateFolderList( folderList );
  }
  else {
    kDebug(5602) << "Calling kmail->KMailIface->folderList() via DCOP failed."
                  << endl;
  }*/
  mTimeOfLastMessageCountUpdate = ::time( 0 );
}

void SummaryWidget::updateFolderList( const QStringList& folders )
{
  qDeleteAll( mLabels );
  mLabels.clear();

  KConfig config( "kcmkmailsummaryrc" );
  config.setGroup( "General" );

  QStringList activeFolders;
  if ( !config.hasKey( "ActiveFolders" ) )
    activeFolders << "/Local/inbox";
  else
    activeFolders = config.readEntry( "ActiveFolders" , QStringList() );

  int counter = 0;
  QStringList::ConstIterator it;
#warning Port me to DBus!
/*  DCOPRef kmail( "kmail", "KMailIface" );
  for ( it = folders.begin(); it != folders.end(); ++it ) {
    if ( activeFolders.contains( *it ) ) {
      DCOPRef folderRef = kmail.call( "getFolder(QString)", *it );
      const int numMsg = folderRef.call( "messages()" );
      const int numUnreadMsg = folderRef.call( "unreadMessages()" );

      if ( numUnreadMsg == 0 ) continue;

      QString folderPath;
      if ( config.readEntry( "ShowFullPath", true ) )
        folderRef.call( "displayPath()" ).get( folderPath );
      else
        folderRef.call( "displayName()" ).get( folderPath );

      KUrlLabel *urlLabel = new KUrlLabel( *it, folderPath, this );
      urlLabel->installEventFilter( this );
      urlLabel->setAlignment( Qt::AlignLeft );
      urlLabel->show();
      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
               SLOT( selectFolder( const QString& ) ) );
      mLayout->addWidget( urlLabel, counter, 0 );
      mLabels.append( urlLabel );

      QLabel *label =
        new QLabel( i18nc("%1: number of unread messages "
                                  "%2: total number of messages", "%1 / %2",
                      numUnreadMsg, numMsg ), this );
      label->setAlignment( Qt::AlignLeft );
      label->show();
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      counter++;
    }
  }*/

  if ( counter == 0 ) {
    QLabel *label = new QLabel( i18n( "No unread messages in your monitored folders" ), this );
    label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( label, 0, 0, 1, 3 );
    label->show();
    mLabels.append( label );
  }
}

bool SummaryWidget::eventFilter( QObject *obj, QEvent* e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    KUrlLabel* label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter )
      emit message( i18n( "Open Folder: \"%1\"", label->text() ) );
    if ( e->type() == QEvent::Leave )
      emit message( QString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

QStringList SummaryWidget::configModules() const
{
  return QStringList( "kcmkmailsummary.desktop" );
}

#include "summarywidget.moc"
