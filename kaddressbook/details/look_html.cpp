/*
    This file is part of KAddressBook.
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

#include <kapplication.h>
#include <kshell.h>
#include <libkdepim/addresseeview.h>

#include "kabprefs.h"

#include "look_html.h"

KABHtmlView::KABHtmlView( QWidget *parent, const char *name )
  : KABBasicLook( parent, name )
{
  mView = new KPIM::AddresseeView( this );

  connect( mView, SIGNAL( phoneNumberClicked( const QString& ) ),
           this, SLOT( phoneNumberClicked( const QString& ) ) );
}

KABHtmlView::~KABHtmlView()
{
}

void KABHtmlView::setAddressee( const KABC::Addressee &addr )
{
  mView->setAddressee( addr );
}

void KABHtmlView::phoneNumberClicked( const QString &number )
{
  QString commandLine = KABPrefs::instance()->mPhoneHookApplication;
  commandLine.replace( "%N", number );

  QStringList tokens = KShell::splitArgs( commandLine, KShell::TildeExpand );

  if ( tokens.count() != 0 )
    KApplication::kdeinitExec( tokens[ 0 ], tokens );
}

#include "look_html.moc"
