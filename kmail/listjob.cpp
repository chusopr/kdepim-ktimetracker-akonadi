/**
 * Copyright (c) 2004 Carsten Burghardt <burghardt@kde.org>
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

#include "listjob.h"
#include "kmfolderimap.h"
#include "kmfoldercachedimap.h"
#include "kmacctimap.h"
#include "kmacctcachedimap.h"
#include "folderstorage.h"

#include <kdebug.h>
#include <kurl.h>
#include <kio/scheduler.h>
#include <kio/job.h>
#include <kio/global.h>
#include <klocale.h>

using namespace KMail;

ListJob::ListJob( FolderStorage* storage, ImapAccountBase* account,
    ImapAccountBase::ListType type,
    bool secondStep, bool complete, bool hasInbox, const QString& path )
 : FolderJob( 0, tOther, (storage ? storage->folder() : 0) ),
   mStorage( storage ), mAccount( account ), mType( type ),
   mHasInbox( hasInbox ), mSecondStep( secondStep ), mComplete( complete ),
   mPath( path )
{
}

ListJob::~ListJob()
{
}

void ListJob::execute()
{
  if ( mAccount->makeConnection() == ImapAccountBase::Error )
  {
    delete this;
    return;
  }
  // this is needed until we have a common base class for d(imap)
  if ( mPath.isEmpty() )
  {
    if ( mStorage && mStorage->folderType() == KMFolderTypeImap ) {
      mPath = static_cast<KMFolderImap*>(mStorage)->imapPath();
    } else if ( mStorage && mStorage->folderType() == KMFolderTypeCachedImap ) {
      mPath = static_cast<KMFolderCachedImap*>(mStorage)->imapPath();
    } else {
      kdError(5006) << "ListJob - no valid path and no folder given" << endl;
      delete this;
      return;
    }
  }
  // create jobData
  ImapAccountBase::jobData jd;
  jd.total = 1; jd.done = 0;
  jd.cancellable = true;
  jd.createInbox = ( mSecondStep && !mHasInbox ) ? true : false;
  jd.parent = mDestFolder;
  jd.onlySubscribed = ( mType != ImapAccountBase::List );
  jd.path = mPath;
  // this is needed if you have a prefix
  // as the INBOX is located in your root ("/") and needs a special listing
  jd.inboxOnly = !mSecondStep && mAccount->prefix() != "/"
    && mPath == mAccount->prefix() && !mHasInbox;
  // make the URL
  QString ltype = "LIST";
  if ( mType == ImapAccountBase::ListSubscribed )
    ltype = "LSUB";
  else if ( mType == ImapAccountBase::ListSubscribedNoCheck )
    ltype = "LSUBNOCHECK";
  KURL url = mAccount->getUrl();
  url.setPath( ( jd.inboxOnly ? QString("/") : mPath )
      + ";TYPE=" + ltype
      + ( mComplete ? ";SECTION=COMPLETE" : QString::null) );
  // go
  KIO::SimpleJob *job = KIO::listDir( url, false );
  KIO::Scheduler::assignJobToSlave( mAccount->slave(), job );
  mAccount->insertJob( job, jd );
  connect( job, SIGNAL(result(KIO::Job *)),
      this, SLOT(slotListResult(KIO::Job *)) );
  connect( job, SIGNAL(entries(KIO::Job *, const KIO::UDSEntryList &)),
      this, SLOT(slotListEntries(KIO::Job *, const KIO::UDSEntryList &)) );
}

void ListJob::slotListResult( KIO::Job* job )
{
  ImapAccountBase::JobIterator it = mAccount->findJob( job );
  if ( it == mAccount->jobsEnd() )
  {
    delete this;
    return;
  }
  if ( job->error() )
  {
    mAccount->handleJobError( job,
        i18n( "Error while listing folder %1: " ).arg((*it).path),
        true );
  } else
  {
    // transport the information, include the jobData
    emit receivedFolders( mSubfolderNames, mSubfolderPaths,
        mSubfolderMimeTypes, mSubfolderAttributes, *it );
    mAccount->removeJob( it );
  }
  delete this;
}

void ListJob::slotListEntries( KIO::Job* job, const KIO::UDSEntryList& uds )
{
  ImapAccountBase::JobIterator it = mAccount->findJob( job );
  if ( it == mAccount->jobsEnd() )
  {
    delete this;
    return;
  }
  QString name;
  KURL url;
  QString mimeType;
  QString attributes;
  for ( KIO::UDSEntryList::ConstIterator udsIt = uds.begin();
        udsIt != uds.end(); udsIt++ )
  {
    mimeType = QString::null;
    attributes = QString::null;
    for ( KIO::UDSEntry::ConstIterator eIt = (*udsIt).begin();
          eIt != (*udsIt).end(); eIt++ )
    {
      // get the needed information
      if ( (*eIt).m_uds == KIO::UDS_NAME )
        name = (*eIt).m_str;
      else if ( (*eIt).m_uds == KIO::UDS_URL )
        url = KURL((*eIt).m_str, 106); // utf-8
      else if ( (*eIt).m_uds == KIO::UDS_MIME_TYPE )
        mimeType = (*eIt).m_str;
      else if ( (*eIt).m_uds == KIO::UDS_EXTRA )
        attributes = (*eIt).m_str;
    }
    if ( (mimeType == "inode/directory" || mimeType == "message/digest"
          || mimeType == "message/directory")
         && name != ".." && (mAccount->hiddenFolders() || name.at(0) != '.')
         && (!(*it).inboxOnly || name.upper() == "INBOX") )
    {
      if ( ((*it).inboxOnly ||
            url.path() == "/INBOX/") && name.upper() == "INBOX" &&
           !mHasInbox )
      {
        // our INBOX
        (*it).createInbox = true;
      }

      // Some servers send _lots_ of duplicates
      // This check is too slow for huge lists
      if ( mSubfolderPaths.count() > 100 ||
           mSubfolderPaths.findIndex(url.path()) == -1 )
      {
        mSubfolderNames.append( name );
        mSubfolderPaths.append( url.path() );
        mSubfolderMimeTypes.append( mimeType );
        mSubfolderAttributes.append( attributes );
      }
    }
  }
}

#include "listjob.moc"
