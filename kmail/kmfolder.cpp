// -*- mode: C++; c-file-style: "gnu" -*-
// kmfolder.cpp
// Author: Stefan Taferner <taferner@alpin.or.at>

#include <config.h>

#include "kmfolder.h"

#include "kmfolderimap.h" //for the nasty imap hacks, FIXME
#include "undostack.h"
#include "kmmsgdict.h"
#include "identitymanager.h"
#include "kmidentity.h"
#include "kmfoldermgr.h"
#include "kmkernel.h"
#include "kmcommands.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>

#include <qfile.h>
#include <qregexp.h>

#include <mimelib/mimepp.h>
#include <errno.h>

//-----------------------------------------------------------------------------

KMFolder :: KMFolder(KMFolderDir* aParent, const QString& aName) :
  KMFolderNode(aParent, aName)
{
  mOpenCount      = 0;
  mQuiet	  = 0;
  mChanged        = FALSE;
  mAutoCreateIndex= TRUE;
  mIsSystemFolder = FALSE;
  mType           = "plain";
  mAcctList       = 0;
  mDirty          = FALSE;
  mUnreadMsgs      = -1;
  mGuessedUnreadMsgs = -1;
  mTotalMsgs      = -1;
  needsCompact    = FALSE;
  mChild          = 0;
  mConvertToUtf8  = FALSE;
  mMailingListEnabled = FALSE;
  mCompactable     = TRUE;
  mNoContent      = FALSE;
  expireMessages = FALSE;
  unreadExpireAge = 28;
  unreadExpireUnits = expireNever;
  readExpireAge = 14;
  readExpireUnits = expireNever;
  mRDict = 0;
  mUseCustomIcons = false;
  mDirtyTimer = new QTimer(this);
  connect(mDirtyTimer, SIGNAL(timeout()),
	  this, SLOT(updateIndex()));

  if ( aParent ) {
      connect(this, SIGNAL(msgAdded(KMFolder*, Q_UINT32)),
              parent()->manager(), SIGNAL(msgAdded(KMFolder*, Q_UINT32)));
      connect(this, SIGNAL(msgRemoved(KMFolder*, Q_UINT32)),
              parent()->manager(), SIGNAL(msgRemoved(KMFolder*, Q_UINT32)));
      connect(this, SIGNAL(msgChanged(KMFolder*, Q_UINT32, int)),
              parent()->manager(), SIGNAL(msgChanged(KMFolder*, Q_UINT32, int)));
      connect(this, SIGNAL(msgHeaderChanged(KMFolder*,  int)),
              parent()->manager(), SIGNAL(msgHeaderChanged(KMFolder*, int)));
  }
  //FIXME: Centralize all the readConfig calls somehow - Zack
  readConfig();
}


//-----------------------------------------------------------------------------
KMFolder :: ~KMFolder()
{
  delete mAcctList;
  mJobList.setAutoDelete( true );
  QObject::disconnect( SIGNAL(destroyed(QObject*)), this, 0 );
  mJobList.clear();
  KMMsgDict::deleteRentry(mRDict);
}


//-----------------------------------------------------------------------------
QString KMFolder::dotEscape(const QString& aStr) const
{
  if (aStr[0] != '.') return aStr;
  return aStr.left(aStr.find(QRegExp("[^\\.]"))) + aStr;
}

void KMFolder::addJob( FolderJob* job ) const
{
  QObject::connect( job, SIGNAL(destroyed(QObject*)),
                    SLOT(removeJob(QObject*)) );
  mJobList.append( job );
}

void KMFolder::removeJob( QObject* job )
{
  mJobList.remove( static_cast<FolderJob*>( job ) );
}


//-----------------------------------------------------------------------------
QString KMFolder::location() const
{
  QString sLocation(path());

  if (!sLocation.isEmpty()) sLocation += '/';
  sLocation += dotEscape(fileName());

  return sLocation;
}



//-----------------------------------------------------------------------------
QString KMFolder::subdirLocation() const
{
  QString sLocation(path());

  if (!sLocation.isEmpty()) sLocation += '/';
  sLocation += '.';
  sLocation += dotEscape(fileName());
  sLocation += ".directory";

  return sLocation;
}

//-----------------------------------------------------------------------------
KMFolderDir* KMFolder::createChildFolder()
{
  QString childName = "." + fileName() + ".directory";
  QString childDir = path() + "/" + childName;
  bool ok = true;

  if (mChild)
    return mChild;

  if (access(QFile::encodeName(childDir), W_OK) != 0) // Not there or not writable
  {
    if (mkdir(QFile::encodeName(childDir), S_IRWXU) != 0
      && chmod(QFile::encodeName(childDir), S_IRWXU) != 0)
        ok=false; //failed create new or chmod existing tmp/
  }

  if (!ok) {
    QString wmsg = QString(" '%1': %2").arg(childDir).arg(strerror(errno));
    KMessageBox::information(0,i18n("Failed to create folder") + wmsg);
    return 0;
  }

  KMFolderDir* folderDir = new KMFolderDir(parent(), childName,
    (folderType() == KMFolderTypeImap) ? KMImapDir : KMStandardDir);
  if (!folderDir)
    return 0;
  folderDir->reload();
  parent()->append(folderDir);
  mChild = folderDir;
  return folderDir;
}

//-----------------------------------------------------------------------------
void KMFolder::setAutoCreateIndex(bool autoIndex)
{
  mAutoCreateIndex = autoIndex;
}

//-----------------------------------------------------------------------------
void KMFolder::setDirty(bool f)
{
  mDirty = f;
  if (mDirty  && mAutoCreateIndex)
    mDirtyTimer->changeInterval( mDirtyTimerInterval );
  else
    mDirtyTimer->stop();
}

//-----------------------------------------------------------------------------
void KMFolder::setIdentity( uint identity ) {
  mIdentity = identity;
  kmkernel->slotRequestConfigSync();
}

//-----------------------------------------------------------------------------
void KMFolder::markNewAsUnread()
{
  KMMsgBase* msgBase;
  int i;

  for (i=0; i< count(); ++i)
  {
    if (!(msgBase = getMsgBase(i))) continue;
    if (msgBase->isNew())
    {
      msgBase->setStatus(KMMsgStatusUnread);
      msgBase->setDirty(TRUE);
    }
  }
}

void KMFolder::markUnreadAsRead()
{
  KMMsgBase* msgBase;
  SerNumList serNums;

  for (int i=count()-1; i>=0; --i)
  {
    msgBase = getMsgBase(i);
    assert(msgBase);
    if (msgBase->isNew() || msgBase->isUnread())
    {
      serNums.append( msgBase->getMsgSerNum() );
    }
  }
  if (serNums.empty())
    return;

  KMCommand *command = new KMSetStatusCommand( KMMsgStatusRead, serNums );
  command->start();
}

//-----------------------------------------------------------------------------
void KMFolder::quiet(bool beQuiet)
{
  if (beQuiet)
    mQuiet++;
  else {
    mQuiet--;
    if (mQuiet <= 0)
    {
      mQuiet = 0;
      if (mChanged)
       emit changed();
      mChanged = FALSE;
    }
  }
}

//-----------------------------------------------------------------------------

// Needed to use QSortedList in reduceSize()

/** Compare message's date. This is useful for message sorting */
int operator<( KMMsgBase & m1, KMMsgBase & m2 )
{
  return (m1.date() < m2.date());
}

/** Compare message's date. This is useful for message sorting */
int operator==( KMMsgBase & m1, KMMsgBase & m2 )
{
  return (m1.date() == m2.date());
}


//-----------------------------------------------------------------------------
int KMFolder::expungeOldMsg(int days)
{
  int i, msgnb=0;
  time_t msgTime, maxTime;
  const KMMsgBase* mb;
  QValueList<int> rmvMsgList;

  maxTime = time(0) - days * 3600 * 24;

  for (i=count()-1; i>=0; i--) {
    mb = getMsgBase(i);
    assert(mb);
    msgTime = mb->date();

    if (msgTime < maxTime) {
      //kdDebug(5006) << "deleting msg " << i << " : " << mb->subject() << " - " << mb->dateStr(); // << endl;
      removeMsg( i );
      msgnb++;
    }
  }
  return msgnb;
}


//-----------------------------------------------------------------------------
/**
 * Return the number of days given some value, and the units for that
 * value. Currently, supported units are days, weeks and months.
 */
int
KMFolder::daysToExpire(int number, ExpireUnits units) {
  switch (units) {
  case expireDays: // Days
    return number;
  case expireWeeks: // Weeks
    return number * 7;
  case expireMonths: // Months - this could be better rather than assuming 31day months.
    return number * 31;
  default: // this avoids a compiler warning (not handled enumeration values)
    ;
  }

  return -1;

}

//-----------------------------------------------------------------------------
/**
 * Expire old messages from this folder. Read and unread messages have
 * different expiry times. An expiry time of 0 or less is considered to
 * mean no-expiry. Also check the general 'expire' flag as well.
 */
void KMFolder::expireOldMessages() {
  FolderJob *job = createJob( 0, FolderJob::tExpireMessages );
  job->start();
}


//-----------------------------------------------------------------------------
void KMFolder::emitMsgAddedSignals(int idx)
{
  Q_UINT32 serNum = kmkernel->msgDict()->getMsgSerNum(this, idx);
  if (!mQuiet) {
    emit msgAdded(idx);
  } else {
    mChanged=true;
  }
  emit msgAdded(this, serNum);
}

//-----------------------------------------------------------------------------
bool KMFolder::canAddMsgNow(KMMessage* aMsg, int* aIndex_ret)
{
  if (aIndex_ret) *aIndex_ret = -1;
  KMFolder *msgParent = aMsg->parent();
  if (aMsg->transferInProgress())
      return false;
  if (!aMsg->isComplete() && msgParent && msgParent->folderType() == KMFolderTypeImap)
  {
    FolderJob *job = msgParent->createJob(aMsg);
    connect(job, SIGNAL(messageRetrieved(KMMessage*)),
            SLOT(reallyAddMsg(KMMessage*)));
    job->start();
    aMsg->setTransferInProgress(TRUE);
    return FALSE;
  }
  return TRUE;
}


//-----------------------------------------------------------------------------
void KMFolder::reallyAddMsg(KMMessage* aMsg)
{
  if (!aMsg) // the signal that is connected can call with aMsg=0
    return;
  aMsg->setTransferInProgress(FALSE);
  KMFolder *folder = aMsg->parent();
  int index;
  ulong serNum = aMsg->getMsgSerNum();
  bool undo = aMsg->enableUndo();
  addMsg(aMsg, &index);
  if (index < 0) return;
  unGetMsg(index);
  if (undo)
  {
    kmkernel->undoStack()->pushSingleAction( serNum, folder, this );
  }
}


//-----------------------------------------------------------------------------
void KMFolder::reallyAddCopyOfMsg(KMMessage* aMsg)
{
  aMsg->setParent( 0 );
  aMsg->setTransferInProgress( false );
  addMsg( aMsg );
  unGetMsg( count() - 1 );
}

int KMFolder::find( const KMMessage * msg ) const {
  return find( &msg->toMsgBase() );
}

//-----------------------------------------------------------------------------
void KMFolder::removeMsg(QPtrList<KMMessage> msgList, bool imapQuiet)
{
  for ( KMMessage* msg = msgList.first(); msg; msg = msgList.next() )
  {
    int idx = find(msg);
    assert( idx != -1);
    removeMsg(idx, imapQuiet);
  }
}

//-----------------------------------------------------------------------------
void KMFolder::removeMsg(int idx, bool)
{
  //assert(idx>=0);
  if(idx < 0)
  {
    kdDebug(5006) << "KMFolder::removeMsg() : idx < 0\n" << endl;
    return;
  }

  KMMsgBase* mb = getMsgBase(idx);

  Q_UINT32 serNum = kmkernel->msgDict()->getMsgSerNum(this, idx);
  if (serNum != 0)
    emit msgRemoved(this, serNum);
  mb = takeIndexEntry( idx );

  setDirty( true );
  needsCompact=true; // message is taken from here - needs to be compacted

  if (mb->isUnread() || mb->isNew() ||
      (this == kmkernel->outboxFolder())) {
    --mUnreadMsgs;
    emit numUnreadMsgsChanged( this );
  }
  --mTotalMsgs;

  QString msgIdMD5 = mb->msgIdMD5();
  QString strippedSubjMD5 = mb->strippedSubjectMD5();
  if (strippedSubjMD5.isEmpty()) {
     mb->initStrippedSubjectMD5();
     strippedSubjMD5 = mb->strippedSubjectMD5();
  }
  emit msgRemoved(idx, msgIdMD5, strippedSubjMD5);
  emit msgRemoved(this);
}


//-----------------------------------------------------------------------------
KMMessage* KMFolder::take(int idx)
{
  KMMsgBase* mb;
  KMMessage* msg;

  assert(idx>=0 && idx<=count());

  mb = getMsgBase(idx);
  if (!mb) return 0;
  if (!mb->isMessage()) readMsg(idx);
  Q_UINT32 serNum = kmkernel->msgDict()->getMsgSerNum(this, idx);
  emit msgRemoved(this,serNum);

  msg = (KMMessage*)takeIndexEntry(idx);

  if (msg->isUnread() || msg->isNew() ||
      (this == kmkernel->outboxFolder())) {
    --mUnreadMsgs;
    emit numUnreadMsgsChanged( this );
  }
  --mTotalMsgs;
  msg->setParent(0);
  setDirty( true );
  needsCompact=true; // message is taken from here - needs to be compacted
  QString msgIdMD5 = msg->msgIdMD5();
  QString strippedSubjMD5 = msg->strippedSubjectMD5();
  if (strippedSubjMD5.isEmpty()) {
     msg->initStrippedSubjectMD5();
     strippedSubjMD5 = msg->strippedSubjectMD5();
  }
  emit msgRemoved(idx, msgIdMD5, strippedSubjMD5);
  emit msgRemoved(this);

  return msg;
}

void KMFolder::take(QPtrList<KMMessage> msgList)
{
  for ( KMMessage* msg = msgList.first(); msg; msg = msgList.next() )
  {
    if (msg->parent())
    {
      int idx = msg->parent()->find(msg);
      assert( idx != -1);
      KMFolder::take(idx);
    }
  }
}


//-----------------------------------------------------------------------------
KMMessage* KMFolder::getMsg(int idx)
{
  KMMsgBase* mb;

  if(!(idx >= 0 && idx <= count()))
    return 0;

  mb = getMsgBase(idx);
  if (!mb) return 0;

#if 0
  if (mb->isMessage()) return ((KMMessage*)mb);
  return readMsg(idx);
#else
  KMMessage *msg = 0;
  bool undo = mb->enableUndo();
  if (mb->isMessage()) {
      msg = ((KMMessage*)mb);
  } else {
      QString mbSubject = mb->subject();
      msg = readMsg(idx);
      // sanity check
      if (mCompactable && (!msg || (msg->subject().isEmpty() != mbSubject.isEmpty()))) {
	  kdDebug(5006) << "Error: " << location() <<
	  " Index file is inconsistent with folder file. This should never happen." << endl;
	  mCompactable = FALSE; // Don't compact
	  writeConfig();
      }
  }
  msg->setEnableUndo(undo);

  if (msg->getMsgSerNum() == 0) {
    msg->setMsgSerNum(kmkernel->msgDict()->insert(0, msg, idx));
    kdDebug(5006) << "Serial number generated for message in folder " << label() << endl;
  }
  msg->setComplete( true );
  return msg;
#endif


}


//-----------------------------------------------------------------------------
KMMsgInfo* KMFolder::unGetMsg(int idx)
{
  KMMsgBase* mb;

  if(!(idx >= 0 && idx <= count()))
    return 0;

  mb = getMsgBase(idx);
  if (!mb) return 0;


  if (mb->isMessage()) {
    // Remove this message from all jobs' list it might still be on.
    // setIndexEntry deletes the message.
    KMMessage *msg = static_cast<KMMessage*>(mb);
    if ( msg->transferInProgress() ) return 0;
    ignoreJobsForMessage( msg );
    return setIndexEntry( idx, msg );
  }

  return 0;
}


//-----------------------------------------------------------------------------
bool KMFolder::isMessage(int idx)
{
  KMMsgBase* mb;
  if (!(idx >= 0 && idx <= count())) return FALSE;
  mb = getMsgBase(idx);
  return (mb && mb->isMessage());
}

//-----------------------------------------------------------------------------
FolderJob* KMFolder::createJob( KMMessage *msg, FolderJob::JobType jt,
                                KMFolder *folder, QString partSpecifier ) const
{
  FolderJob * job = doCreateJob( msg, jt, folder, partSpecifier );
  if ( job )
    addJob( job );
  return job;
}

//-----------------------------------------------------------------------------
FolderJob* KMFolder::createJob( QPtrList<KMMessage>& msgList, const QString& sets,
                                FolderJob::JobType jt, KMFolder *folder ) const
{
  FolderJob * job = doCreateJob( msgList, sets, jt, folder );
  if ( job )
    addJob( job );
  return job;
}

//-----------------------------------------------------------------------------
int KMFolder::moveMsg(KMMessage* aMsg, int* aIndex_ret)
{
  KMFolder* msgParent;
  int rc;

  assert(aMsg != 0);
  msgParent = aMsg->parent();

  if (msgParent)
    msgParent->open();

  open();
  rc = addMsg(aMsg, aIndex_ret);
  close();

  if (msgParent)
    msgParent->close();

  return rc;
}

//-----------------------------------------------------------------------------
int KMFolder::moveMsg(QPtrList<KMMessage> msglist, int* aIndex_ret)
{
  KMFolder* msgParent;
  int rc;

  KMMessage* aMsg = msglist.first();
  assert(aMsg != 0);
  msgParent = aMsg->parent();

  if (msgParent)
    msgParent->open();

  open();
  //FIXME : is it always imap ?
  rc = static_cast<KMFolderImap*>(this)->addMsg(msglist, aIndex_ret); //yuck: Don
  close();

  if (msgParent)
    msgParent->close();

  return rc;
}


//-----------------------------------------------------------------------------
int KMFolder::rename(const QString& newName, KMFolderDir *newParent)
{
  QString oldLoc, oldIndexLoc, oldIdsLoc, newLoc, newIndexLoc, newIdsLoc;
  QString oldSubDirLoc, newSubDirLoc;
  QString oldName;
  int rc=0, openCount=mOpenCount;
  KMFolderDir *oldParent;

  assert(!newName.isEmpty());

  oldLoc = location();
  oldIndexLoc = indexLocation();
  oldSubDirLoc = subdirLocation();
  if (kmkernel->msgDict())
    oldIdsLoc = kmkernel->msgDict()->getFolderIdsLocation(this);

  close(TRUE);

  oldName = fileName();
  oldParent = parent();
  if (newParent)
    setParent( newParent );

  setName(newName);
  newLoc = location();
  newIndexLoc = indexLocation();
  newSubDirLoc = subdirLocation();
  if (kmkernel->msgDict())
    newIdsLoc = kmkernel->msgDict()->getFolderIdsLocation(this);

  if (::rename(QFile::encodeName(oldLoc), QFile::encodeName(newLoc))) {
    setName(oldName);
    setParent(oldParent);
    rc = errno;
  }
  else {
    // rename/move index file and index.sorted file
    if (!oldIndexLoc.isEmpty()) {
      ::rename(QFile::encodeName(oldIndexLoc), QFile::encodeName(newIndexLoc));
      ::rename(QFile::encodeName(oldIndexLoc) + ".sorted",
               QFile::encodeName(newIndexLoc) + ".sorted");
    }

    // rename/move serial number file
    if (!oldIdsLoc.isEmpty())
      ::rename(QFile::encodeName(oldIdsLoc), QFile::encodeName(newIdsLoc));

    // rename/move the subfolder directory
    if (!::rename(QFile::encodeName(oldSubDirLoc), QFile::encodeName(newSubDirLoc) )) {
      // now that the subfolder directory has been renamed and/or moved also
      // change the name that is stored in the corresponding KMFolderNode
      // (provide that the name actually changed)
      if( mChild && ( oldName != newName ) ) {
        mChild->setName( "." + QFile::encodeName(newName) + ".directory" );
      }
    }

    // if the folder is being moved then move its node and, if necessary, also
    // the associated subfolder directory node to the new parent
    if (newParent) {
      if (oldParent->findRef( this ) != -1)
        oldParent->take();
      newParent->inSort( this );
      if (mChild) {
        if (mChild->parent()->findRef( mChild ) != -1)
          mChild->parent()->take();
        newParent->inSort( mChild );
        mChild->setParent( newParent );
      }
    }
  }

  if (openCount > 0)
  {
    open();
    mOpenCount = openCount;
  }

  emit nameChanged();
  return rc;
}


//-----------------------------------------------------------------------------
int KMFolder::remove()
{
  assert(!name().isEmpty());

  clearIndex(true, true); // delete and remove from dict
  close(TRUE);

  if (kmkernel->msgDict()) kmkernel->msgDict()->removeFolderIds(this);
  unlink(QFile::encodeName(indexLocation()) + ".sorted");
  unlink(QFile::encodeName(indexLocation()));

  int rc = removeContents();
  if (rc) return rc;

  needsCompact = false; //we are dead - no need to compact us
  return 0;
}


//-----------------------------------------------------------------------------
int KMFolder::expunge()
{
  int openCount = mOpenCount;

  assert(!name().isEmpty());

  clearIndex(true, true);   // delete and remove from dict
  close(TRUE);

  kmkernel->msgDict()->removeFolderIds(this);
  if (mAutoCreateIndex)
    truncateIndex();
  else unlink(QFile::encodeName(indexLocation()));

  int rc = expungeContents();
  if (rc) return rc;

  mDirty = FALSE;
  needsCompact = false; //we're cleared and truncated no need to compact

  if (openCount > 0)
  {
    open();
    mOpenCount = openCount;
  }

  mUnreadMsgs = 0;
  mTotalMsgs = 0;
  emit numUnreadMsgsChanged( this );
  if (mAutoCreateIndex)
    writeConfig();
  emit changed();
  emit expunged();

  return 0;
}


//-----------------------------------------------------------------------------
const char* KMFolder::type() const
{
  if (mAcctList) return "In";
  return KMFolderNode::type();
}


//-----------------------------------------------------------------------------
QString KMFolder::label() const
{
  if (mIsSystemFolder && !mLabel.isEmpty()) return mLabel;
  if (mIsSystemFolder) return i18n(name().latin1());
  return name();
}

int KMFolder::count(bool cache) const
{
  if (cache && mTotalMsgs != -1)
    return mTotalMsgs;
  else
    return -1;
}

//-----------------------------------------------------------------------------
int KMFolder::countUnread()
{
  if (mGuessedUnreadMsgs > -1)
    return mGuessedUnreadMsgs;
  if (mUnreadMsgs > -1)
    return mUnreadMsgs;

  readConfig();

  if (mUnreadMsgs > -1)
    return mUnreadMsgs;

  open(); // will update unreadMsgs
  int unread = mUnreadMsgs;
  close();
  return (unread > 0) ? unread : 0;
}

//-----------------------------------------------------------------------------
int KMFolder::countUnreadRecursive()
{
  KMFolder *folder;
  int count = countUnread();
  KMFolderDir *dir = child();
  if (!dir)
    return count;

  QPtrListIterator<KMFolderNode> it(*dir);
  for ( ; it.current(); ++it )
    if (!it.current()->isDir()) {
      folder = static_cast<KMFolder*>(it.current());
      count += folder->countUnreadRecursive();
    }

  return count;
}

//-----------------------------------------------------------------------------
void KMFolder::msgStatusChanged(const KMMsgStatus oldStatus,
  const KMMsgStatus newStatus, int idx)
{
  int oldUnread = 0;
  int newUnread = 0;

  if (oldStatus & KMMsgStatusUnread || oldStatus & KMMsgStatusNew ||
      (this == kmkernel->outboxFolder()))
    oldUnread = 1;
  if (newStatus & KMMsgStatusUnread || newStatus & KMMsgStatusNew ||
      (this == kmkernel->outboxFolder()))
    newUnread = 1;
  int deltaUnread = newUnread - oldUnread;

  mDirtyTimer->changeInterval(mDirtyTimerInterval);
  if (deltaUnread != 0) {
    if (mUnreadMsgs < 0) mUnreadMsgs = 0;
    mUnreadMsgs += deltaUnread;
    emit numUnreadMsgsChanged( this );

    Q_UINT32 serNum = kmkernel->msgDict()->getMsgSerNum(this, idx);
    emit msgChanged( this, serNum, deltaUnread );
  }
}

//-----------------------------------------------------------------------------
void KMFolder::headerOfMsgChanged(const KMMsgBase* aMsg, int idx)
{
  if (idx < 0)
    idx = aMsg->parent()->find( aMsg );
  if (idx >= 0)
    emit msgHeaderChanged(this, idx);
   else
     mChanged = TRUE;
}

//-----------------------------------------------------------------------------
QString KMFolder::idString() const
{
  KMFolderNode* folderNode = parent();
  if (!folderNode)
    return "";
  while (folderNode->parent())
    folderNode = folderNode->parent();
  int pathLen = path().length() - folderNode->path().length();
  QString relativePath = path().right( pathLen );
  if (!relativePath.isEmpty())
    relativePath = relativePath.right( relativePath.length() - 1 ) + "/";
  QString escapedName = QString( name() );
  /* Escape [ and ] as they are disallowed for kconfig sections and that is
     what the idString is primarily used for. */
  escapedName.replace( "[", "%(" );
  escapedName.replace( "]", "%)" );
  return relativePath + escapedName;
}

//-----------------------------------------------------------------------------
void KMFolder::readConfig()
{
  //kdDebug(5006)<<"#### READING CONFIG  = "<< name() <<endl;
  KConfig* config = KMKernel::config();
  KConfigGroupSaver saver(config, "Folder-" + idString());
  if (mUnreadMsgs == -1)
    mUnreadMsgs = config->readNumEntry("UnreadMsgs", -1);
  if (mTotalMsgs == -1)
    mTotalMsgs = config->readNumEntry("TotalMsgs", -1);
  mMailingListEnabled = config->readBoolEntry("MailingListEnabled");
  mMailingListPostingAddress = config->readEntry("MailingListPostingAddress");
  mMailingListAdminAddress = config->readEntry("MailingListAdminAddress");
  mIdentity = config->readUnsignedNumEntry("Identity",0);
  mCompactable = config->readBoolEntry("Compactable", TRUE);

  expireMessages = config->readBoolEntry("ExpireMessages", FALSE);
  readExpireAge = config->readNumEntry("ReadExpireAge", 3);
  readExpireUnits = (ExpireUnits)config->readNumEntry("ReadExpireUnits", expireMonths);
  unreadExpireAge = config->readNumEntry("UnreadExpireAge", 12);
  unreadExpireUnits = (ExpireUnits)config->readNumEntry("UnreadExpireUnits", expireNever);
  setUserWhoField( config->readEntry("WhoField"), false );
  mUseCustomIcons = config->readBoolEntry("UseCustomIcons", false );
  mNormalIconPath = config->readEntry("NormalIconPath" );
  mUnreadIconPath = config->readEntry("UnreadIconPath" );
  if ( mUseCustomIcons )
      emit iconsChanged();
}

//-----------------------------------------------------------------------------
void KMFolder::writeConfig()
{
  KConfig* config = KMKernel::config();
  KConfigGroupSaver saver(config, "Folder-" + idString());
  config->writeEntry("UnreadMsgs", countUnread());
  config->writeEntry("TotalMsgs", mTotalMsgs);
  config->writeEntry("MailingListEnabled", mMailingListEnabled);
  config->writeEntry("MailingListPostingAddress", mMailingListPostingAddress);
  config->writeEntry("MailingListAdminAddress", mMailingListAdminAddress);
  config->writeEntry("Identity", mIdentity);
  config->writeEntry("Compactable", mCompactable);
  config->writeEntry("ExpireMessages", expireMessages);
  config->writeEntry("ReadExpireAge", readExpireAge);
  config->writeEntry("ReadExpireUnits", readExpireUnits);
  config->writeEntry("UnreadExpireAge", unreadExpireAge);
  config->writeEntry("UnreadExpireUnits", unreadExpireUnits);
  config->writeEntry("WhoField", mUserWhoField);

  config->writeEntry("UseCustomIcons", mUseCustomIcons);
  config->writeEntry("NormalIconPath", mNormalIconPath);
  config->writeEntry("UnreadIconPath", mUnreadIconPath);
}

//-----------------------------------------------------------------------------
void KMFolder::correctUnreadMsgsCount()
{
  open();
  close();
  emit numUnreadMsgsChanged( this );
}

//-----------------------------------------------------------------------------
void KMFolder::fillMsgDict(KMMsgDict *dict)
{
  fillDictFromIndex(dict);
}

//-----------------------------------------------------------------------------
int KMFolder::writeMsgDict(KMMsgDict *dict)
{
  int ret = 0;
  if (!dict)
    dict = kmkernel->msgDict();
  if (dict)
    ret = dict->writeFolderIds(this);
  return ret;
}

//-----------------------------------------------------------------------------
int KMFolder::touchMsgDict()
{
  int ret = 0;
  KMMsgDict *dict = kmkernel->msgDict();
  if (dict)
    ret = dict->touchFolderIds(this);
  return ret;
}

//-----------------------------------------------------------------------------
int KMFolder::appendtoMsgDict(int idx)
{
  int ret = 0;
  KMMsgDict *dict = kmkernel->msgDict();
  if (dict) {
    if (count() == 1) {
      ret = dict->writeFolderIds(this);
    } else {
      ret = dict->appendtoFolderIds(this, idx);
    }
  }
  return ret;
}

//-----------------------------------------------------------------------------
void KMFolder::setStatus(int idx, KMMsgStatus status, bool toggle)
{
  KMMsgBase *msg = getMsgBase(idx);
  if ( msg ) {
    if (toggle)
      msg->toggleStatus(status, idx);
    else
      msg->setStatus(status, idx);
  }
}

void KMFolder::setRDict(KMMsgDictREntry *rentry) {
  if (rentry == mRDict)
	return;
  KMMsgDict::deleteRentry(mRDict);
  mRDict = rentry;
}

//-----------------------------------------------------------------------------
void KMFolder::setStatus(QValueList<int>& ids, KMMsgStatus status, bool toggle)
{
  for ( QValueList<int>::Iterator it = ids.begin(); it != ids.end(); ++it )
  {
    KMFolder::setStatus(*it, status, toggle);
  }
}

//-----------------------------------------------------------------------------
void KMFolder::setUserWhoField(const QString &whoField, bool aWriteConfig)
{
  mUserWhoField = whoField;
  if ( whoField.isEmpty() )
  {
    // default setting
    const KMIdentity & identity =
      kmkernel->identityManager()->identityForUoidOrDefault( mIdentity );

    if ( mIsSystemFolder && folderType() != KMFolderTypeImap )
    {
      // local system folders
      if ( this == kmkernel->inboxFolder() || this == kmkernel->trashFolder() ) mWhoField = "From";
      if ( this == kmkernel->outboxFolder() || this == kmkernel->sentFolder() || this == kmkernel->draftsFolder() ) mWhoField = "To";

    } else if ( identity.drafts() == idString() || identity.fcc() == idString() ) {
      // drafts or sent of the identity
      mWhoField = "To";
    } else {
      mWhoField = "From";
    }

  } else if ( whoField == "From" || whoField == "To" ) {

    // set the whoField according to the user-setting
    mWhoField = whoField;

  } else {
    // this should not happen...
    kdDebug(5006) << "Illegal setting " << whoField << " for userWhoField!" << endl;
  }

  if (aWriteConfig)
    writeConfig();
}

void KMFolder::ignoreJobsForMessage( KMMessage *msg )
{
  if ( !msg || msg->transferInProgress() )
    return;

  QPtrListIterator<FolderJob> it( mJobList );
  while ( it.current() )
  {
    //FIXME: the questions is : should we iterate through all
    //messages in jobs? I don't think so, because it would
    //mean canceling the jobs that work with other messages
    if ( it.current()->msgList().first() == msg )
    {
      FolderJob* job = it.current();
      mJobList.remove( job );
      delete job;
    } else
      ++it;
  }
}

void KMFolder::setIconPaths(const QString &normalPath, const QString &unreadPath)
{
  mNormalIconPath = normalPath;
  mUnreadIconPath = unreadPath;
  writeConfig();
  emit iconsChanged();
}

//-----------------------------------------------------------------------------
void KMFolder::removeJobs()
{
  mJobList.setAutoDelete( true );
  mJobList.clear();
  mJobList.setAutoDelete( false );
}

//-----------------------------------------------------------------------------
size_t KMFolder::crlf2lf( char* str, const size_t strLen )
{
  const char* source = str;
  const char* sourceEnd = source + strLen;

  // search the first occurrence of "\r\n"
  for ( ; source < sourceEnd - 1; ++source ) {
    if ( *source == '\r' && *( source + 1 ) == '\n' )
      break;
  }

  if ( source == sourceEnd - 1 ) {
    // no "\r\n" found
    return strLen;
  }

  // replace all occurrences of "\r\n" with "\n" (in place)
  char* target = const_cast<char*>( source ); // target points to '\r'
  ++source; // source points to '\n'
  for ( ; source < sourceEnd; ++source ) {
    if ( *source != '\r' || *( source + 1 ) != '\n' )
      *target++ = *source;
  }
  *target = '\0'; // terminate result
  return target - str;
}

#include "kmfolder.moc"
