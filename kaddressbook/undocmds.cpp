/*
    This file is part of KAddressBook.
    Copyright (C) 1999 Don Sanders <sanders@kde.org>

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

#include <qtextstream.h>
#include <qapplication.h>
#include <qclipboard.h>

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kabc/addressbook.h>

#include "addresseeutil.h"
#include "addresseeconfig.h"
#include "core.h"

#include "undocmds.h"

/////////////////////////////////
// PwDelete Methods

PwDeleteCommand::PwDeleteCommand( KABC::AddressBook *ab, 
                                  const QStringList &uidList)
  : Command( ab ), mAddresseeList(), mUIDList( uidList )
{
  redo();
}

PwDeleteCommand::~PwDeleteCommand()
{
}

QString PwDeleteCommand::name()
{
  return i18n( "Delete" );
}

void PwDeleteCommand::undo()
{
  // Put it back in the document
  KABC::Addressee::List::Iterator it;
  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) {
    addressBook()->insertAddressee( *it );
  }

  mAddresseeList.clear();
}

void PwDeleteCommand::redo()
{
  // Just remove it from the document. This is enough to make the user
  // Think the item has been deleted
  KABC::Addressee addr;
  QStringList::Iterator it;
  for ( it = mUIDList.begin(); it != mUIDList.end(); ++it ) {
    addr = addressBook()->findByUid( *it );
    addressBook()->removeAddressee( addr );
    mAddresseeList.append( addr );
    AddresseeConfig cfg( addr );
    cfg.remove();
  }
}

/////////////////////////////////
// PwPaste Methods

PwPasteCommand::PwPasteCommand( KAB::Core *core,
                                const KABC::Addressee::List &list )
  : Command( core->addressBook() ), mCore( core ), mAddresseeList( list )
{
  redo();
}

QString PwPasteCommand::name()
{
  return i18n( "Paste" );
}

void PwPasteCommand::undo()
{
  KABC::Addressee::List::Iterator it;
  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) 
    addressBook()->removeAddressee( *it );
}

void PwPasteCommand::redo()
{
  QStringList uids;
  KABC::Addressee::List::Iterator it;
  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) {
    /* we have to set a new uid for the contact, otherwise insertAddressee()
       ignore it.
     */ 
    (*it).setUid( KApplication::randomString( 10 ) );
    uids.append( (*it).uid() );
    addressBook()->insertAddressee( *it );
  }

  QStringList::Iterator uidIt;
  for ( uidIt = uids.begin(); uidIt != uids.end(); ++uidIt )
    mCore->editContact( *uidIt );
}

/////////////////////////////////
// PwNew Methods

PwNewCommand::PwNewCommand( KABC::AddressBook *ab, const KABC::Addressee &addr )
  : Command( ab ), mAddr( addr )
{
  redo();
}

PwNewCommand::~PwNewCommand()
{
}

QString PwNewCommand::name()
{
  return i18n( "New Contact" );
}

void PwNewCommand::undo()
{
  addressBook()->removeAddressee( mAddr );
}

void PwNewCommand::redo()
{
  addressBook()->insertAddressee( mAddr );
}

/////////////////////////////////
// PwEdit Methods

PwEditCommand::PwEditCommand( KABC::AddressBook *ab,
                              const KABC::Addressee &oldAddr,
                              const KABC::Addressee &newAddr )
     : Command( ab ), mOldAddr( oldAddr ), mNewAddr( newAddr )
{
  redo();
}

PwEditCommand::~PwEditCommand()
{
}

QString PwEditCommand::name()
{
  return i18n( "Entry Edit" );
}

void PwEditCommand::undo()
{
  addressBook()->insertAddressee( mOldAddr );
}

void PwEditCommand::redo()
{
  addressBook()->insertAddressee( mNewAddr );
}

/////////////////////////////////
// PwCut Methods

PwCutCommand::PwCutCommand( KABC::AddressBook *ab, const QStringList &uidList )
    : Command( ab ), mUIDList( uidList )
{
  redo();
}

QString PwCutCommand::name()
{
  return i18n( "Cut" );
}

void PwCutCommand::undo()
{
  KABC::Addressee::List::Iterator it;
  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) {
    addressBook()->insertAddressee( *it );
  }

  mAddresseeList.clear();
  
  QClipboard *cb = QApplication::clipboard();
  kapp->processEvents();
  cb->setText( mOldText );
}

void PwCutCommand::redo()
{
  KABC::Addressee addr;
  QStringList::Iterator it;
  for ( it = mUIDList.begin(); it != mUIDList.end(); ++it ) {
    addr = addressBook()->findByUid( *it );
    addressBook()->removeAddressee( addr );
    mAddresseeList.append( addr );
  }

  // Convert to clipboard
  mClipText = AddresseeUtil::addresseesToClipboard( mAddresseeList );

  QClipboard *cb = QApplication::clipboard();
  mOldText = cb->text();
  kapp->processEvents();
  cb->setText( mClipText );
}
