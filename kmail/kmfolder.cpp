// kmfolder.cpp
// Author: Stefan Taferner <taferner@alpin.or.at>

#include "kmglobal.h"
#include "kmfolder.h"
#include "kmmessage.h"
#include "kmfolderdir.h"
#include "kmflock.h"

#include <klocale.h>
#include <mimelib/mimepp.h>
#include <qregexp.h>

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define MAX_LINE 4096
#define INIT_MSGS 8

// Current version of the table of contents (index) files
#define INDEX_VERSION 1100

// Regular expression to find the line that seperates messages in a mail
// folder:
#define MSG_SEPERATOR_START "From "
#define MSG_SEPERATOR_REGEX "^From .*..:..:...*$"
static short msgSepLen = strlen(MSG_SEPERATOR_START);


//-----------------------------------------------------------------------------
KMFolder :: KMFolder(KMFolderDir* aParent, const char* aName) : 
  KMFolderInherited(aParent, aName), mMsgList(INIT_MSGS)
{
  //-- in case that the compiler has problems with the static version above:
  //msgSepLen = strlen(MSG_SEPERATOR_START);

  initMetaObject();

  mStream         = NULL;
  mIndexStream    = NULL;
  mOpenCount      = 0;
  mQuiet          = 0;
  mHeaderOffset   = 0;
  mAutoCreateIndex= TRUE;
  mFilesLocked    = FALSE;
  mIsSystemFolder = FALSE;
  mType           = "plain";
  mAcctList       = NULL;
  mDirty          = FALSE;
}


//-----------------------------------------------------------------------------
KMFolder :: ~KMFolder()
{
  if (mOpenCount>0) close(TRUE);

  //if (mAcctList) delete mAcctList;
  /* Well, this is a problem. If I add the above line then kmfolder depends
   * on kmaccount and is then not that portable. Hmm.
   */
}


//-----------------------------------------------------------------------------
const QString KMFolder::location(void) const
{
  QString sLocation;

  sLocation = path().copy();
  if (!sLocation.isEmpty()) sLocation += '/';
  sLocation += name();

  return sLocation;
}


//-----------------------------------------------------------------------------
const QString KMFolder::indexLocation(void) const
{
  QString sLocation;

  sLocation = path().copy();
  if (!sLocation.isEmpty()) sLocation += '/';
  sLocation += '.';
  sLocation += name();
  sLocation += ".index";

  return sLocation;
}


//-----------------------------------------------------------------------------
int KMFolder::open(void)
{
  int rc = 0;

  mOpenCount++;
  if (mOpenCount > 1) return 0;  // already open

  assert(name() != NULL);

  mStream = fopen(location(), "r+"); // messages file
  if (!mStream) 
  {
    debug("Cannot open folder `%s': %s", (const char*)location(), 
	  strerror(errno));
    return errno;
  }

  if (!path().isEmpty())
  {
    mIndexStream = fopen(indexLocation(), "r+"); // index file
    if (!mIndexStream) rc = createIndexFromContents();
    else readIndex();
  }
  else
  {
    debug("No path specified for folder `" + name() +
	  "' -- Turning autoCreateIndex off");
    mAutoCreateIndex = FALSE;
    rc = createIndexFromContents();
  }

  if (!rc) lock();
  mQuiet = 0;

  return rc;
}


//-----------------------------------------------------------------------------
int KMFolder::create(void)
{
  int rc;

  assert(name() != NULL);
  assert(mOpenCount == 0);

  mStream = fopen(location(), "w");
  if (!mStream) return errno;

  if (!path().isEmpty())
  {
    mIndexStream = fopen(indexLocation(), "w");
    if (!mIndexStream) return errno;
  }
  else
  {
    debug("Folder `" + name() +
	  "' has no path specified -- turning autoCreateIndex off");
    mAutoCreateIndex = FALSE;
  }

  mOpenCount++;
  mQuiet = 0;

  rc = writeIndex();
  if (!rc) lock();
  return rc;
}


//-----------------------------------------------------------------------------
void KMFolder::close(bool aForced)
{
  if (mOpenCount > 0) mOpenCount--;
  if (mOpenCount > 0 && !aForced) return;
  if (mAutoCreateIndex) 
  {
    if (mDirty) writeIndex();
    else sync();
  }

  unlock();

  if (mStream) fclose(mStream);
  if (mIndexStream) fclose(mIndexStream);

  mOpenCount   = 0;
  mStream      = NULL;
  mIndexStream = NULL;
  mFilesLocked = FALSE;

  mMsgList.reset(INIT_MSGS);
}


//-----------------------------------------------------------------------------
int KMFolder::lock(void)
{
  int rc;

  mFilesLocked = FALSE;

  rc = kmflock(fileno(mStream), LOCK_EX|LOCK_NB);
  if (rc) return errno;

  if (mIndexStream >= 0)
  {
    rc = kmflock(fileno(mIndexStream), LOCK_EX|LOCK_NB);
    if (rc) 
    {
      rc = errno;
      kmflock(fileno(mStream), LOCK_UN|LOCK_NB);
      return rc;
    }
  }

  mFilesLocked = TRUE;
  debug("Folder `%s' is now locked.", (const char*)location());
  return 0;
}


//-----------------------------------------------------------------------------
int KMFolder::unlock(void)
{
  int rc;

  mFilesLocked = FALSE;
  debug("Unlocking folder `%s'.", (const char*)location());

  rc = kmflock(fileno(mStream), LOCK_UN|LOCK_NB);
  if (rc) return errno;

  if (mIndexStream >= 0)
  {
    rc = kmflock(fileno(mIndexStream), LOCK_UN|LOCK_NB);
    if (rc) return errno;
  }
  return 0;
}


//-----------------------------------------------------------------------------
int KMFolder::createIndexFromContents(void)
{
  char line[MAX_LINE];
  char status[8];
  QString subjStr, dateStr, fromStr;
  unsigned long offs, size, pos;
  bool atEof = FALSE;
  KMMsgInfo* mi;
  QString msgStr;
  QRegExp regexp(MSG_SEPERATOR_REGEX);
  int i, num;

  assert(mStream != NULL);
  rewind(mStream);

  debug("*** creating index file %s\n", (const char*)indexLocation());

  mMsgList.clear();

  num     = -1;
  offs    = 0;
  size    = 0;
  dateStr = "";
  fromStr = "";
  subjStr = "";
  strcpy(status, "RO");

  while (!atEof)
  {
    if ((num & 127) == 0)
    {
      msgStr.sprintf(nls->translate("Creating index file: %d messages done"), 
		     num);
      emit statusMsg(msgStr);
    }

    pos = ftell(mStream);
    if (!fgets(line, MAX_LINE, mStream)) atEof = TRUE;

    if (atEof ||
	(strncmp(line,MSG_SEPERATOR_START, msgSepLen)==0 && 
	 regexp.match(line) >= 0))
    {
      size = pos - offs;
      pos = ftell(mStream);

      if (num >= 0)
      {
	if (size > 0)
	{
	  mi = new KMMsgInfo(this);
	  mi->init(subjStr, fromStr, 0, KMMsgStatusNew, offs, size);
	  mi->setDate(dateStr);
	  mMsgList.append(mi);

	  strcpy(status, "RO");
	  dateStr = "";
	  fromStr = "";
	  subjStr = "";
	}
	else num--;
      }

      offs = ftell(mStream);
      num++;
    }
    else if (strncmp(line, "Status: ", 8) == 0)
    {
      for(i=0; i<8 && line[i+8] > ' '; i++)
	status[i] = line[i+8];
      status[i] = '\0';
    }
    else if (strncmp(line, "Date: ", 6) == 0)
      dateStr = QString(line+6).copy();
    else if (strncmp(line, "From: ", 6) == 0)
      fromStr = QString(line+6).copy();
    else if (strncmp(line, "Subject: ", 9) == 0)
      subjStr = QString(line+9).copy();
  }

  if (mAutoCreateIndex) writeIndex();
  else mHeaderOffset = 0;

  return 0;
}


//-----------------------------------------------------------------------------
int KMFolder::writeIndex(void)
{
  KMMsgBase* msgBase;
  int i=0;

  if (mIndexStream) fclose(mIndexStream);
  mIndexStream = fopen(indexLocation(), "w");
  if (!mIndexStream) return errno;

  fprintf(mIndexStream, "# KMail-Index V%d\n", INDEX_VERSION);

  mHeaderOffset = ftell(mIndexStream);
  for (i=0; i<mMsgList.high(); i++)
  {
    if (!(msgBase = mMsgList[i])) continue;
    fprintf(mIndexStream, "%s\n", (const char*)msgBase->asIndexString());
  }
  fflush(mIndexStream);

  mDirty = FALSE;
  return 0;
}


//-----------------------------------------------------------------------------
void KMFolder::setAutoCreateIndex(bool autoIndex)
{
  mAutoCreateIndex = autoIndex;
}


//-----------------------------------------------------------------------------
bool KMFolder::readIndexHeader(void)
{
  int indexVersion;

  assert(mIndexStream != NULL);

  fscanf(mIndexStream, "# KMail-Index V%d\n", &indexVersion);
  if (indexVersion < INDEX_VERSION)
  {
    debug("Index index file %s is out of date. Re-creating it.", 
	  (const char*)indexLocation());
    createIndexFromContents();
    return FALSE;
  }

  return TRUE;
}


//-----------------------------------------------------------------------------
void KMFolder::readIndex(void)
{
  char line[MAX_LINE];
  KMMsgInfo* mi;

  assert(mIndexStream != NULL);
  rewind(mIndexStream);

  mMsgList.clear();
  if (!readIndexHeader()) return;

  mDirty = FALSE;
  mHeaderOffset = ftell(mIndexStream);

  mMsgList.clear();
  while (!feof(mIndexStream))
  {
    fgets(line, MAX_LINE, mIndexStream);
    if (feof(mIndexStream)) break;

    mi = new KMMsgInfo(this);
    mi->fromIndexString(line);
    if (mi->status() == KMMsgStatusDeleted) 
    {
      delete mi;  // skip messages that are marked as deleted
      mDirty = TRUE;
      continue;
    }
    else if (mi->status() == KMMsgStatusNew)
    {
      mi->setStatus(KMMsgStatusUnread);
      mi->setDirty(FALSE);
    }
    mMsgList.append(mi);
  }
}


//-----------------------------------------------------------------------------
void KMFolder::quiet(bool beQuiet)
{
  if (beQuiet) mQuiet++;
  else 
  {
    mQuiet--;
    if (mQuiet <= 0)
    {
      mQuiet = 0;
      emit changed();
    }
  }
}


//-----------------------------------------------------------------------------
KMMessage* KMFolder::take(int idx)
{
  KMMsgBase* mb;
  KMMessage* msg;

  assert(mStream!=NULL);
  assert(idx>=0 && idx<=mMsgList.high());

  mb = mMsgList[idx];
  if (!mb) return NULL;
  if (!mb->isMessage()) readMsg(idx);

  msg = (KMMessage*)mMsgList.take(idx);
  mDirty = TRUE;
  if (!mQuiet) emit msgRemoved(idx);

  return msg;
}


//-----------------------------------------------------------------------------
KMMessage* KMFolder::getMsg(int idx)
{
  KMMsgBase* mb;

  assert(idx>=0 && idx<=mMsgList.high());

  mb = mMsgList[idx];
  if (!mb) return NULL;

  if (mb->isMessage()) return ((KMMessage*)mb);
  return readMsg(idx);
}


//-----------------------------------------------------------------------------
KMMessage* KMFolder::readMsg(int idx)
{
  KMMessage* msg;
  unsigned long msgSize;
  QString msgText;
  KMMsgInfo* mi = (KMMsgInfo*)mMsgList[idx];

  assert(mi!=NULL && !mi->isMessage());
  assert(mStream != NULL);

  msgSize = mi->msgSize();
  msgText.resize(msgSize+2);

  fseek(mStream, mi->folderOffset(), SEEK_SET);
  fread(msgText.data(), msgSize, 1, mStream);
  msgText[msgSize] = '\0';

  msg = new KMMessage(*mi);
  msg->fromString(msgText);

  mMsgList.set(idx,msg);

  return msg;
}


//-----------------------------------------------------------------------------
int KMFolder::moveMsg(KMMessage* aMsg, int* aIndex_ret)
{
  KMFolder* msgParent;
  int rc;

  assert(aMsg != NULL);
  msgParent = aMsg->parent();

  if (msgParent)
  {
    msgParent->open();
    msgParent->take(msgParent->find(aMsg));
    msgParent->close();
  }

  open();
  rc = addMsg(aMsg, aIndex_ret);
  close();

  return rc;
}


//-----------------------------------------------------------------------------
int KMFolder::addMsg(KMMessage* aMsg, int* aIndex_ret)
{
  long offs, size, len;
  bool opened = FALSE;
  QString msgText;
  int idx;
  KMFolder* msgParent;

  if (!mStream)
  {
    opened = TRUE;
    open();
  }

  // take message out of the folder it is currently in, if any
  msgParent = aMsg->parent();
  if (msgParent)
  {
    if (msgParent==this) return 0;
    idx = msgParent->find(aMsg);
    if (idx >= 0) msgParent->take(idx);
  }

  msgText = aMsg->asString();
  len = msgText.length();

  assert(mStream != NULL);
  if (len <= 0)
  {
    debug("Message added to folder `%s' contains no data. Ignoring it.",
	  (const char*)name());
    return 0;
  }

  // write message to folder file
  fseek(mStream, 0, SEEK_END);
  fwrite("From ???@??? 00:00:00 1997 +0000\n", 33, 1, mStream);
  offs = ftell(mStream);
  fwrite(msgText, len, 1, mStream);
  if (msgText[len-1]!='\n') fwrite("\n", 1, 1, mStream);
  fflush(mStream);
  size = ftell(mStream) - offs;

  // store information about the position in the folder file in the message
  aMsg->setParent(this);
  aMsg->setFolderOffset(offs);
  aMsg->setMsgSize(size);
  idx = mMsgList.append(aMsg);

  // write index entry if desired
  if (mAutoCreateIndex)
  {
    assert(mIndexStream != NULL);
    fseek(mIndexStream, 0, SEEK_END);
    fprintf(mIndexStream, "%s\n", (const char*)aMsg->asIndexString()); 
    fflush(mIndexStream);
  }

  // some "paper work"
  if (aIndex_ret) *aIndex_ret = idx;
  if (!mQuiet) emit msgAdded(idx);
  if (opened) close();

  return 0;
} 


//-----------------------------------------------------------------------------
int KMFolder::remove(void)
{
  int rc;

  assert(name() != NULL);

  if (mOpenCount > 0) 
  {
    mOpenCount = 1; // force a close
    close();
  }

  unlink(indexLocation());
  rc = unlink(location());
  if (rc) return rc;

  mMsgList.reset(INIT_MSGS);
  return 0;
}
	

//-----------------------------------------------------------------------------
int KMFolder::expunge(void)
{
  int openCount = mOpenCount;

  assert(name() != NULL);

  close(TRUE);

  if (mAutoCreateIndex) truncate(indexLocation(), mHeaderOffset);
  else unlink(indexLocation());

  if (truncate(location(), 0)) return errno;
  mDirty = FALSE;

  mMsgList.reset(INIT_MSGS);

  if (openCount > 0)
  {
    open();
    mOpenCount = openCount;
  }

  if (!mQuiet) emit changed();

  return 0;
}


//-----------------------------------------------------------------------------
int KMFolder::compact(void)
{
  KMFolder* tempFolder;
  KMMessage* msg;
  QString tempName;
  int openCount = mOpenCount;

  tempName = "." + name();
  tempName.detach();
  tempName += ".compacted";
  unlink(tempName);
  tempFolder = parent()->createFolder(tempName);
  assert(tempFolder != NULL);

  quiet(TRUE);
  tempFolder->open();
  open();

  while(count() > 0)
  {
    msg = getMsg(0);
    tempFolder->moveMsg(msg);
  }
  tempName = tempFolder->location();
  tempFolder->close(TRUE);
  close(TRUE);

  rename(tempName, location());
  rename(tempFolder->indexLocation(), indexLocation());

  if (openCount > 0)
  {
    open();
    mOpenCount = openCount;
  }
  quiet(FALSE);

  if (!mQuiet) emit changed();
  return 0;
}


//-----------------------------------------------------------------------------
int KMFolder::sync(void)
{
  KMMsgBasePtr mb;
  unsigned long offset = mHeaderOffset;
  int i, rc, recSize = KMMsgBase::indexStringLength()+1;
  int high = mMsgList.high();

  if (!mIndexStream) return 0;

  for (rc=0,i=0; i<high; i++)
  {
    mb = mMsgList[i];
    if (mb->dirty())
    {
      fseek(mIndexStream, offset, SEEK_SET);
      fprintf(mIndexStream, "%s\n", (const char*)mb->asIndexString());
      rc = errno;
      if (rc) break;
      mb->setDirty(FALSE);
    }
    offset += recSize;
  }
  fflush(mIndexStream);
  mDirty = FALSE;

  return rc;
}


//-----------------------------------------------------------------------------
void KMFolder::sort(KMMsgList::SortField aField)
{
  mMsgList.sort(aField);
  if (!mQuiet) emit changed();
  mDirty = TRUE;
}


//-----------------------------------------------------------------------------
const char* KMFolder::type(void) const
{
  if (mAcctList) return "in";
  return KMFolderInherited::type();
}


//-----------------------------------------------------------------------------
const QString KMFolder::label(void) const
{
  if (mIsSystemFolder && !mLabel.isEmpty()) return mLabel;
  return name();
}


//-----------------------------------------------------------------------------
long KMFolder::countUnread(void) const
{
  int  i;
  long unread;

  for (i=0, unread=0; i<mMsgList.high(); i++)
  {
    if (mMsgList[i]->status()==KMMsgStatusUnread) unread++;
  }

  return unread;
}


//-----------------------------------------------------------------------------
void KMFolder::headerOfMsgChanged(const KMMsgBase* aMsg)
{
  int idx = mMsgList.find((KMMsgBasePtr)aMsg);
  if (idx >= 0 && !mQuiet) emit msgHeaderChanged(idx);
}


//-----------------------------------------------------------------------------
#include "kmfolder.moc"
