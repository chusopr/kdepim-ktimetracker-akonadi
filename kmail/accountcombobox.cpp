/** -*- mode: C++ -*-
 * Copyright (c) 2004 David Faure <faure@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#include "accountcombobox.h"
#include "kmkernel.h"
#include "kmfolder.h"
#include "kmfolderdir.h"
#include "kmacctmgr.h"
#include <kdebug.h>

using namespace KMail;

AccountComboBox::AccountComboBox( bool needsInbox, QWidget* parent, const char* name )
  : QComboBox( parent, name ), mNeedsInbox( needsInbox )
{
  connect( kmkernel->acctMgr(), SIGNAL( accountAdded( KMAccount* ) ),
           this, SLOT( slotRefreshAccounts() ) );
  connect( kmkernel->acctMgr(), SIGNAL( accountRemoved( KMAccount* ) ),
           this, SLOT( slotRefreshAccounts() ) );
  slotRefreshAccounts();
}

void AccountComboBox::slotRefreshAccounts()
{
  clear();
  // Note that this won't take into account newly-created-in-configuredialog accounts
  // until clicking OK or Apply. This would make this class much more complex
  // (this would have to be different depending on whether this combo is in the
  // configuration dialog or not...)
  QStringList accountNames;
  QValueList<KMAccount *> lst = applicableAccounts();
  QValueList<KMAccount *>::ConstIterator it = lst.begin();
  for ( ; it != lst.end() ; ++it )
    accountNames.append( (*it)->name() );
  insertStringList( accountNames );
}


void AccountComboBox::setCurrentAccount( KMAccount* account )
{
  int i = 0;
  for( KMAccount *a = kmkernel->acctMgr()->first(); a;
       a = kmkernel->acctMgr()->next(), ++i ) {
    if ( a == account ) {
      setCurrentItem( i );
      return;
    }
  }
}

KMAccount* AccountComboBox::currentAccount() const
{
  int i = 0;
  QValueList<KMAccount *> lst = applicableAccounts();
  QValueList<KMAccount *>::ConstIterator it = lst.begin();
  while ( it != lst.end() && i < currentItem() ) {
    ++it;
    ++i;
  }
  if ( it != lst.end() )
    return *it;
  return 0;
}

QValueList<KMAccount *> KMail::AccountComboBox::applicableAccounts() const
{
  QValueList<KMAccount *> lst;
  for( KMAccount *a = kmkernel->acctMgr()->first(); a;
       a = kmkernel->acctMgr()->next() ) {
    Q_ASSERT( a->folder() );
    if ( a && a->folder() ) {
      bool ok = false;
      if ( mNeedsInbox ) {
        KMFolderDir* child = a->folder()->child();
        Q_ASSERT( child );
        if ( child ) {
          for (KMFolderNode* node = child->first(); node; node = child->next()) {
            if (!node->isDir() && node->name() == "INBOX") {
              ok = true;
              break;
            }
          }
        }
      } else
        ok = true;
      if ( ok )
        lst.append( a );
    }
  }
  return lst;
}

#include "accountcombobox.moc"
