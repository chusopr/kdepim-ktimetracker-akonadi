/*
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

#include "core.h"
#include "summarywidget.h"

SummaryWidget::SummaryWidget( Kontact::Plugin *plugin, QWidget *parent, const char *name )
  : QWidget( parent, name ), mPlugin(plugin)
{
  setPaletteBackgroundColor( QColor( 240, 240, 240 ) );

  mLayout = new QGridLayout( this, 8, 3, 3 );
  mLayout->setRowStretch( 7, 1 );

  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  QLabel *label = new QLabel( this );
  label->setAlignment( AlignLeft );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "kmail", KIcon::Desktop,
                                                     KIcon::SizeMedium ) );
  mLayout->addWidget( label, 0, 0 );

  label = new QLabel( i18n( "EMails" ), this );
  label->setAlignment( AlignRight );
  label->setFont( boldFont );
  mLayout->addMultiCellWidget( label, 0, 0, 1, 2 );

  QString error;
  QCString appID;

  if ( kapp->dcopClient()->isApplicationRegistered( "kmail" ) )
    mDCOPApp = "kmail";
  else {
    plugin->part(); // start part to have dcop iface available
    mDCOPApp = "kontact";
  }

  connect( &mTimer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
  mTimer.start( 0 );
}

void SummaryWidget::show()
{
  timeout();
  QWidget::show();
}

void SummaryWidget::raisePart()
{
  mPlugin->core()->selectPlugin( mPlugin );
}

void SummaryWidget::timeout()
{
  mTimer.stop();

  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  DCOPRef dcopCall( mDCOPApp.latin1(), "KMailIface" );
  QStringList folderList = dcopCall.call( "folderList()" );

  int counter = 1;
  QStringList::Iterator it;
  for ( it = folderList.begin(); it != folderList.end() && counter < 7; ++it, ++counter ) {
    DCOPRef folderRef;
    dcopCall.call( "getFolder(QString)", *it ).get( folderRef );
    int numMsg = folderRef.call( "messages()" );
    int numUnreadMsg = folderRef.call( "unreadMessages()" );
    KURLLabel *urlLabel = new KURLLabel( QString::null, (*it).mid( 1 ), this );
    urlLabel->setAlignment( AlignLeft );
    urlLabel->show();
    // ### FIXME emit dcop signal to jumo to actual folder
    connect( urlLabel, SIGNAL( leftClickedURL() ), SLOT( raisePart() ) );
    mLayout->addWidget( urlLabel, counter, 0 );
    mLabels.append( urlLabel );
    QLabel *label = new QLabel( QString( "%1 / %2" ).arg( numUnreadMsg ).arg( numMsg ), this );
    if ( numUnreadMsg > 0 ) {
      QFont font = label->font();
      font.setBold( true );
      label->setFont( font );
    }
    label->setAlignment( AlignLeft );
    label->show();
    mLayout->addWidget( label, counter, 0 );
    mLayout->addWidget( label, counter, 2 );
    mLabels.append( label );
  }

  mTimer.start( 15 * 1000 );
}

#include "summarywidget.moc"
