/*  -*- c++ -*-
    subscriptiondialog.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (C) 2002 Carsten Burghardt <burghardt@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "subscriptiondialog.h"
#include "folderstorage.h"
#include "listjob.h"
#include "imapaccountbase.h"

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

namespace KMail {

SubscriptionDialog::SubscriptionDialog( QWidget *parent, const QString &caption,
    KAccount *acct, QString startPath )
  : KSubscription( parent, caption, acct, User1, QString(), false ),
    mStartPath( startPath ), mSubscribed( false ), mForceSubscriptionEnable( false)
{
  // hide unneeded checkboxes
  hideTreeCheckbox();
  hideNewOnlyCheckbox();

  // ok-button
  connect(this, SIGNAL(okClicked()), SLOT(slotSave()));

  // reload-list button
  connect(this, SIGNAL(user1Clicked()), SLOT(slotLoadFolders()));

  // get the folders
  slotLoadFolders();
}

//------------------------------------------------------------------------------
void SubscriptionDialog::slotListDirectory( const QStringList& subfolderNames,
                                            const QStringList& subfolderPaths,
                                            const QStringList& subfolderMimeTypes,
                                            const QStringList& subfolderAttributes,
                                            const ImapAccountBase::jobData& jobData )
{
  mFolderNames = subfolderNames;
  mFolderPaths = subfolderPaths;
  mFolderMimeTypes = subfolderMimeTypes;
  mFolderAttributes = subfolderAttributes;
  mJobData = jobData;

  mCount = 0;

  createItems();
}

//------------------------------------------------------------------------------
void SubscriptionDialog::createItems()
{
  bool onlySubscribed = mJobData.onlySubscribed;

  GroupItem *parent = 0;
  uint done = 0;
//  kDebug(5006) << "createItems subscribed=" << onlySubscribed <<",folders="
//    << mFolderNames.join(",") << endl;

  for (int i = mCount; i < mFolderNames.count(); ++i)
  {
    // give the dialog a chance to repaint
    if (done == 1000)
    {
      emit listChanged();
      QTimer::singleShot(0, this, SLOT(createItems()));
      return;
    }
    ++mCount;
    ++done;
    GroupItem *item = 0;
    if (!onlySubscribed && mFolderPaths.size() > 0)
    {
      // get the parent
      GroupItem *oldItem = 0;
      QString parentPath;
      findParentItem( mFolderNames[i], mFolderPaths[i], parentPath, &parent, &oldItem );

      if (!parent && parentPath != "/")
      {
        // the parent is not available and it's no root-item
        // this happens when the folders do not arrive in hierarchical order
        // so we create each parent in advance
        QStringList folders = parentPath.split(mDelimiter, QString::SkipEmptyParts);
        uint i = 0;
        for ( QStringList::Iterator it = folders.begin(); it != folders.end(); ++it )
        {
          QString name = *it;
          if (name.startsWith("/"))
            name = name.right(name.length()-1);
          if (name.endsWith("/"))
            name.truncate(name.length()-1);
          KGroupInfo info(name);
          info.subscribed = false;

          QStringList tmpPath;
          for ( uint j = 0; j <= i; ++j )
            tmpPath << folders[j];
          QString path = tmpPath.join(mDelimiter);
          if (!path.startsWith("/"))
            path = '/' + path;
          if (!path.endsWith("/"))
            path = path + '/';
          info.path = path;
          item = 0;
          if (folders.count() > 1)
          {
            // we have to create more then one level, so better check if this
            // folder already exists somewhere
            item = mItemDict[path];
          }
          // as these items are "dummies" we create them non-checkable
          if (!item)
          {
            if (parent)
              item = new GroupItem(parent, info, this, false);
            else
              item = new GroupItem(folderTree(), info, this, false);
            mItemDict.insert(info.path, item);
          }

          parent = item;
          ++i;
        } // folders
      } // parent

      KGroupInfo info(mFolderNames[i]);
      info.path = mFolderPaths[i];

      if ( info.path == "/INBOX/" )
        info.name = i18n("inbox");

      // only checkable when the folder is selectable
      bool checkable = ( mFolderMimeTypes[i] == "inode/directory" ) ? false : true;
      // create a new item
      if (parent)
        item = new GroupItem(parent, info, this, checkable);
      else
        item = new GroupItem(folderTree(), info, this, checkable);

      if (oldItem) // remove old item
        mItemDict.remove(info.path);

      mItemDict.insert(info.path, item);
      if (oldItem)
      {
        // move the old children to the new item
        QList<Q3ListViewItem*> itemsToMove;
        Q3ListViewItem * myChild = oldItem->firstChild();
        while (myChild)
        {
          itemsToMove.append(myChild);
          myChild = myChild->nextSibling();
        }
        QList<Q3ListViewItem*>::const_iterator it;
        Q3ListViewItem *cur;
        for ( it = itemsToMove.constBegin();
            ( cur = *it ) && it != itemsToMove.constEnd(); ++it ) {
          oldItem->takeItem(cur);
          item->insertItem(cur);
          if ( cur->isSelected() ) // we have new parents so open them
            folderTree()->ensureItemVisible( cur );
          ++it;
        }
        delete oldItem;
        itemsToMove.clear();
      }
      // select the start item
      if ( mFolderPaths[i] == mStartPath )
      {
        item->setSelected( true );
        folderTree()->ensureItemVisible( item );
      }

    } else if (onlySubscribed)
    {
      // find the item
      if ( mItemDict[mFolderPaths[i]] )
      {
        GroupItem* item = mItemDict[mFolderPaths[i]];
        item->setOn( true );
      }
    }
  }
  processNext();
}

//------------------------------------------------------------------------------
void SubscriptionDialog::findParentItem( QString &name, QString &path, QString &parentPath,
    GroupItem **parent, GroupItem **oldItem )
{
  // remove the name (and the separator) from the path to get the parent path
  int start = path.length() - (name.length()+2);
  int length = name.length()+1;
  if (start < 0) start = 0;
  parentPath = path;
  parentPath.remove(start, length);

  // find the parent by it's path
  *parent = mItemDict[parentPath];

  // check if the item already exists
  *oldItem = mItemDict[path];
}

//------------------------------------------------------------------------------
void SubscriptionDialog::slotSave()
{
  // subscribe
  Q3ListViewItemIterator it(subView);
  for ( ; it.current(); ++it)
  {
    static_cast<ImapAccountBase*>(account())->changeSubscription(true,
        static_cast<GroupItem*>(it.current())->info().path);
  }

  // unsubscribe
  Q3ListViewItemIterator it2(unsubView);
  for ( ; it2.current(); ++it2)
  {
    static_cast<ImapAccountBase*>(account())->changeSubscription(false,
        static_cast<GroupItem*>(it2.current())->info().path);
  }

  if( mForceSubscriptionEnable ) {
    KMail::ImapAccountBase *account = static_cast<KMail::ImapAccountBase*>(mAcct);
    if( account )
    {
      account->setOnlySubscribedFolders(true);
    }
  }
}

//------------------------------------------------------------------------------
void SubscriptionDialog::slotLoadFolders()
{
  ImapAccountBase* ai = static_cast<ImapAccountBase*>(account());
  // we need a connection
  if ( ai->makeConnection() == ImapAccountBase::Error )
  {
    kWarning(5006) << "SubscriptionDialog - got no connection" << endl;
    return;
  } else if ( ai->makeConnection() == ImapAccountBase::Connecting )
  {
    // We'll wait for the connectionResult signal from the account.
    kDebug(5006) << "SubscriptionDialog - waiting for connection" << endl;
    connect( ai, SIGNAL( connectionResult(int, const QString&) ),
        this, SLOT( slotConnectionResult(int, const QString&) ) );
    return;
  }
  // clear the views
  KSubscription::slotLoadFolders();
  mItemDict.clear();
  mSubscribed = false;
  mLoading = true;

  initPrefixList();

  processNext();
}

//------------------------------------------------------------------------------
void SubscriptionDialog::processNext()
{
  if ( mPrefixList.isEmpty() )
  {
    if ( !mSubscribed )
    {
      mSubscribed = true;
      initPrefixList();
    } else {
      slotLoadingComplete();
      return;
    }
  }
  ImapAccountBase* ai = static_cast<ImapAccountBase*>(account());
  ImapAccountBase::ListType type = ( mSubscribed ?
      ImapAccountBase::ListSubscribedNoCheck : ImapAccountBase::List );

  bool completeListing = true;
  mCurrentNamespace = mPrefixList.first();
  mDelimiter = ai->delimiterForNamespace( mCurrentNamespace );
  mPrefixList.pop_front();
  if ( mCurrentNamespace == "/INBOX/" )
  {
    type = mSubscribed ?
      ImapAccountBase::ListFolderOnlySubscribed : ImapAccountBase::ListFolderOnly;
    completeListing = false;
  }

//  kDebug(5006) << "process " << mCurrentNamespace << ",subscribed=" << mSubscribed << endl;
  ListJob* job = new ListJob( ai, type, 0, ai->addPathToNamespace( mCurrentNamespace ), completeListing );
  connect( job, SIGNAL(receivedFolders(const QStringList&, const QStringList&,
          const QStringList&, const QStringList&, const ImapAccountBase::jobData&)),
      this, SLOT(slotListDirectory(const QStringList&, const QStringList&,
          const QStringList&, const QStringList&, const ImapAccountBase::jobData&)));
  job->start();
}

//------------------------------------------------------------------------------
void SubscriptionDialog::initPrefixList()
{
  ImapAccountBase* ai = static_cast<ImapAccountBase*>(account());
  ImapAccountBase::nsMap map = ai->namespaces();
  mPrefixList.clear();

  bool hasInbox = false;
  const QStringList ns = map[ImapAccountBase::PersonalNS];
  for ( QStringList::ConstIterator it = ns.begin(); it != ns.end(); ++it )
  {
    if ( (*it).isEmpty() )
      hasInbox = true;
  }
  if ( !hasInbox && !ns.isEmpty() )
  {
    // the namespaces includes no listing for the root so start a special
    // listing for the INBOX to make sure we get it
    mPrefixList += "/INBOX/";
  }

  mPrefixList += map[ImapAccountBase::PersonalNS];
  mPrefixList += map[ImapAccountBase::OtherUsersNS];
  mPrefixList += map[ImapAccountBase::SharedNS];
}

void SubscriptionDialog::slotConnectionResult( int errorCode, const QString& errorMsg )
{
  Q_UNUSED( errorMsg );
  if ( !errorCode )
    slotLoadFolders();
}

void SubscriptionDialog::show()
{
  KDialogBase::show();
  KMail::ImapAccountBase *account = static_cast<KMail::ImapAccountBase*>(mAcct);
  if( account )
  {
    if( !account->onlySubscribedFolders() )
    {
      kDebug() << "Not subscribed!!!" << endl;
      int result = KMessageBox::questionYesNoCancel( this,
              i18n("Currently subscriptions are not used for server %1\ndo you want to enable subscriptions?",
                account->name() ),
            i18n("Enable Subscriptions?"), i18n("Enable"), i18n("Do Not Enable"));
        switch(result) {
            case KMessageBox::Yes:
                mForceSubscriptionEnable = true;
                break;
            case KMessageBox::No:
                break;
            case KMessageBox::Cancel:
#warning Port me!
//                cancel();
                break;
        }
    }
  }
}

} // namespace

#include "subscriptiondialog.moc"
