/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
� � Copyright (c) 2002 Maximilian Rei� <harlekin@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>
#include <kiconloader.h>

#include "systemtray.h"

using namespace KSync;

KSyncSystemTray::KSyncSystemTray(QWidget* parent,  const char* name)
    : KSystemTray(parent,name)
{

    ksyncIconConnected =   KGlobal::iconLoader()->loadIcon( "connect_established", KIcon::Small );
    ksyncIconDisconnected = KGlobal::iconLoader()->loadIcon( "connect_no", KIcon::Small );
    setState( false );
}

KSyncSystemTray::~KSyncSystemTray()
{
}

//void KSyncSystemTray::mousePressEvent( QMouseEvent *mEvent ) {
//
//    if ( mEvent->button() == QEvent::LeftButton) {
//       emit leftClicked ( QPoint(mEvent->x(), mEvent->y()) );
//    } else if ( mEvent->button() == QEvent::RightButton ) {
//        emit rightClicked ( QPoint(mEvent->x(), mEvent->y()) );
//        //contextMenu()->popup(QCursor::pos());
//    } else {}
//}

void setName( QString & )
{
}

void KSyncSystemTray::setState( bool connected )
{
    if ( connected ) {
        setPixmap(ksyncIconConnected);
    } else {
        setPixmap(ksyncIconDisconnected);
    }
}

#include "systemtray.moc"
