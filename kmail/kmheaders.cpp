// kmheaders.cpp

#include "kmfolder.h"
#include "kmheaders.h"
#include "kmmessage.h"
#include "kbusyptr.h"
#include "kmdragdata.h"
#include "kmglobal.h"
#include "kmmainwin.h"
#include "kmcomposewin.h"

#include <drag.h>
#include <qstrlist.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapp.h>

//-----------------------------------------------------------------------------
KMHeaders::KMHeaders(KMMainWin *aOwner, QWidget *parent=0, 
		     const char *name=0) :
  KMHeadersInherited(parent, name, 4)
{
  QString kdir = app->kdedir();
  KIconLoader* loader = app->getIconLoader();
  static QPixmap pixNew, pixUns, pixDel, pixOld, pixRep, pixSent, pixQueued;

  mOwner  = aOwner;
  mFolder = NULL;
  getMsgIndex = -1;

  //setNumCols(4);
  setColumn(0, nls->translate("F"), 16, KMHeadersInherited::PixmapColumn);
  setColumn(1, nls->translate("Sender"), 200);
  setColumn(2, nls->translate("Subject"), 270);
  setColumn(3, nls->translate("Date"), 300);
  readConfig();

  pixNew   = loader->loadIcon("kmmsgnew.xpm");
  pixUns   = loader->loadIcon("kmmsgunseen.xpm");
  pixDel   = loader->loadIcon("kmmsgdel.xpm");
  pixOld   = loader->loadIcon("kmmsgold.xpm");
  pixRep   = loader->loadIcon("kmmsgreplied.xpm");
  pixQueued= loader->loadIcon("kmmsgqueued.xpm");
  pixSent  = loader->loadIcon("kmmsgsent.xpm");

  dict().insert(KMMsgBase::statusToStr(KMMsgStatusNew), &pixNew);
  dict().insert(KMMsgBase::statusToStr(KMMsgStatusUnread), &pixUns);
  dict().insert(KMMsgBase::statusToStr(KMMsgStatusDeleted), &pixDel);
  dict().insert(KMMsgBase::statusToStr(KMMsgStatusOld), &pixOld);
  dict().insert(KMMsgBase::statusToStr(KMMsgStatusReplied), &pixRep);
  dict().insert(KMMsgBase::statusToStr(KMMsgStatusQueued), &pixQueued);
  dict().insert(KMMsgBase::statusToStr(KMMsgStatusSent), &pixSent);

  connect(this,SIGNAL(selected(int,int)),
	  this,SLOT(selectMessage(int,int)));
  connect(this,SIGNAL(highlighted(int,int)),
	  this,SLOT(highlightMessage(int,int)));
  connect(this,SIGNAL(headerClicked(int)),
	  this,SLOT(headerClicked(int)));
}


//-----------------------------------------------------------------------------
KMHeaders::~KMHeaders ()
{
  if (mFolder) mFolder->close();
}


//-----------------------------------------------------------------------------
void KMHeaders::setFolder (KMFolder *aFolder)
{
  if (mFolder) 
  {
    mFolder->close();
    disconnect(mFolder, SIGNAL(msgHeaderChanged(int)),
	       this, SLOT(msgHeaderChanged(int)));
    disconnect(mFolder, SIGNAL(msgAdded(int)),
	       this, SLOT(msgAdded(int)));
    disconnect(mFolder, SIGNAL(msgRemoved(int)),
	       this, SLOT(msgRemoved(int)));
    disconnect(mFolder, SIGNAL(changed()),
	       this, SLOT(msgChanged()));
    disconnect(mFolder, SIGNAL(statusMsg(const char*)), 
	       mOwner, SLOT(statusMsg(const char*)));
  }

  mFolder = aFolder;

  if (mFolder)
  {
    connect(mFolder, SIGNAL(msgHeaderChanged(int)), 
	    this, SLOT(msgHeaderChanged(int)));
    connect(mFolder, SIGNAL(msgAdded(int)),
	    this, SLOT(msgAdded(int)));
    connect(mFolder, SIGNAL(msgRemoved(int)),
	    this, SLOT(msgRemoved(int)));
    connect(mFolder, SIGNAL(changed()),
	    this, SLOT(msgChanged()));
    connect(mFolder, SIGNAL(statusMsg(const char*)),
	    mOwner, SLOT(statusMsg(const char*)));
    mFolder->open();
  }

  updateMessageList();
  //if (mFolder) setCurrentItem(0);
}


//-----------------------------------------------------------------------------
void KMHeaders::msgChanged()
{
  debug("msgChanged() called");

  int i = topItem();
  updateMessageList();
  setTopItem(i);
}


//-----------------------------------------------------------------------------
void KMHeaders::msgAdded(int id)
{
  if (!autoUpdate()) return;
  debug("msgAdded(%d) called", id);
  insertItem("", id);
  msgHeaderChanged(id);
}


//-----------------------------------------------------------------------------
void KMHeaders::msgRemoved(int id)
{
  debug("msgRemoved(%d) called", id);
  removeItem(id);

#if 0
  int i = topItem();
  updateMessageList();
  setTopItem(i);
#endif
}


//-----------------------------------------------------------------------------
void KMHeaders::msgHeaderChanged(int msgId)
{
  char hdr[256];
  KMMsgStatus flag;
  KMMsgBase* mb;

  if (!autoUpdate()) return;
  debug("msgHeaderChanged() called");

  mb = mFolder->getMsgBase(msgId);
  assert(mb != NULL);

  flag = mb->status();
  sprintf(hdr, "%c\n%s\n %s\n%s", (char)flag, (const char*)mb->from(), 
	  (const char*)mb->subject(), (const char*)mb->dateStr());
  changeItem(hdr, msgId);

  if (flag==KMMsgStatusNew) changeItemColor(darkRed, msgId);
  else if(flag==KMMsgStatusUnread) changeItemColor(darkBlue, msgId);
}


//-----------------------------------------------------------------------------
void KMHeaders::headerClicked(int column)
{
  KMMsgList::SortField sortField;

  if (column==0)      sortField = KMMsgList::sfStatus;
  else if (column==1) sortField = KMMsgList::sfFrom;
  else if (column==2) sortField = KMMsgList::sfSubject;
  else if (column==3) sortField = KMMsgList::sfDate;
  else return;

  kbp->busy();
  mFolder->sort(sortField);
  kbp->idle();
}


//-----------------------------------------------------------------------------
void KMHeaders::setMsgRead (int msgId)
{
  KMMessage* msg;

  for (msg=getMsg(msgId); msg; msg=getMsg())
  {
    debug("setMsgRead(%d)", getMsgIndex);
    msg->touch();
    //changeItemPart(msg->status(), getMsgIndex, 0);
    //changeItemColor(black, getMsgIndex);
  }
}


//-----------------------------------------------------------------------------
void KMHeaders::deleteMsg (int msgId)
{
  KMMessage* msg;
  KMMessageList* msgList;

  if (mFolder != trashFolder)
  {
    // move messages into trash folder
    moveMsgToFolder(trashFolder, msgId);
  }
  else
  {
    // We are in the trash folder -> really delete messages
    kbp->busy();
    setAutoUpdate(FALSE);
    msgList = selectedMsgs();
    for (msg=msgList->first(); msg; msg=msgList->next())
    {
      mFolder->removeMsg(msg);
      delete msg;
    }
    setAutoUpdate(TRUE);
    kbp->idle();
  }
}


//-----------------------------------------------------------------------------
void KMHeaders::forwardMsg (int msgId)
{
  KMComposeWin *win;
  KMMessage *msg;

  msg = getMsg(msgId);
  if (!msg) return;

  kbp->busy();
  win = new KMComposeWin(msg->createForward());
  win->show();
  kbp->idle(); 
}


//-----------------------------------------------------------------------------
void KMHeaders::replyToMsg (int msgId)
{
  KMComposeWin *win;
  KMMessage *msg;

  msg = getMsg(msgId);
  if (!msg) return;

  kbp->busy();
  win = new KMComposeWin(msg->createReply(FALSE));
  win->show();
  kbp->idle(); 
}


//-----------------------------------------------------------------------------
void KMHeaders::replyAllToMsg (int msgId)
{
  KMComposeWin *win;
  KMMessage *msg;

  msg = getMsg(msgId);
  if (!msg) return;

  kbp->busy();
  win = new KMComposeWin(msg->createReply(TRUE));
  win->show();
  kbp->idle(); 
}


//-----------------------------------------------------------------------------
void KMHeaders::moveMsgToFolder (KMFolder* destFolder, int msgId)
{
  QList<KMMessage> msgList;
  KMMessage* msg;
  int rc, num, top;
  int cur = currentItem();
  bool curMoved = (cur>=0 ? isMarked(cur) : FALSE);

  assert(destFolder != NULL);

  kbp->busy();
  setAutoUpdate(FALSE);
  top = topItem();

  destFolder->open();
  // getMsg gets confused when messages are removed while calling
  // it repeatedly. To avoid this we create a temporary list of
  // the messages that will be moved.
  for (num=0,msg=getMsg(msgId); msg; msg=getMsg(),num++)
    msgList.append(msg);

  unmarkAll();

  // now it is safe to move the messages.
  for (msg=msgList.first(); msg; msg=msgList.next())
    rc = destFolder->moveMsg(msg);

  setTopItem(top);
  setAutoUpdate(TRUE);

  // display proper message if current message was moved.
  if (curMoved)
  {
    if (cur >= mFolder->count()) cur = mFolder->count() - 1;
    setCurrentItem(cur, -1);
  }
  destFolder->close();
  kbp->idle();
}


//-----------------------------------------------------------------------------
KMMessageList* KMHeaders::selectedMsgs(void)
{
  KMMessage* msg;

  mSelMsgList.clear();
  for (msg=getMsg(-1); msg; msg=getMsg())
    mSelMsgList.append(msg);

  return &mSelMsgList;
}


//-----------------------------------------------------------------------------
KMMessage* KMHeaders::getMsg (int msgId)
{
  int i, high;

  if (!mFolder || msgId < -2)
  {
    getMsgIndex = -1;
    return NULL;
  }
  if (msgId >= 0) 
  {
    getMsgIndex = msgId;
    getMsgMulti = FALSE;
    return mFolder->getMsg(msgId);
  }

  if (msgId == -1)
  {
    getMsgMulti = TRUE;
    getMsgIndex = currentItem();
    for (i=0,high=numRows(); i<high; i++)
    {
      if (itemList[i]->isMarked())
      {
	getMsgIndex = i;
	break;
      }
    }
 
    return (getMsgIndex>=0 ? mFolder->getMsg(getMsgIndex) : (KMMessage*)NULL);
  }

  if (getMsgIndex < 0) return NULL;

  if (getMsgMulti)
  {
    for (getMsgIndex++; getMsgIndex<numRows(); getMsgIndex++)
    {
      if (itemList[getMsgIndex]->isMarked()) 
	return mFolder->getMsg(getMsgIndex);
    }
  }

  getMsgIndex = -1;
  return NULL;
}


//-----------------------------------------------------------------------------
void KMHeaders::nextMessage()
{
  int idx = currentItem();

  if (idx < mFolder->count())
    setCurrentItem(idx+1);
}


//-----------------------------------------------------------------------------
void KMHeaders::prevMessage()
{
  int idx = currentItem();

  if (idx > 0) 
    setCurrentItem(idx-1);
}  


//-----------------------------------------------------------------------------
void KMHeaders::changeItemPart (char c, int itemIndex, int column)
{
  char str[2];

  str[0] = c;
  str[1] = '\0';

  KMHeadersInherited::changeItemPart((const char*)str, itemIndex, column);
}


//-----------------------------------------------------------------------------
void KMHeaders::highlightMessage(int idx, int/*colId*/)
{
  if (idx < 0 || idx >= numRows()) return;

  kbp->busy();
  mOwner->statusMsg("");
  emit selected(mFolder->getMsg(idx));
  if (idx >= 0) setMsgRead(idx);
  kbp->idle();
}


//-----------------------------------------------------------------------------
void KMHeaders::selectMessage(int idx, int/*colId*/)
{
  if (idx < 0 || idx >= numRows()) return;

  kbp->busy();
  mOwner->statusMsg("");
  emit activated(mFolder->getMsg(idx));
  if (idx >= 0) setMsgRead(idx);
  kbp->idle();
}


//-----------------------------------------------------------------------------
void KMHeaders::updateMessageList(void)
{
  long i;
  QString hdr;
  KMMsgStatus flag;
  KMMsgBase* mb;
 
  debug("updateMessageList() called");

  clear();
  if (!mFolder) return;

  kbp->busy();
  setAutoUpdate(FALSE);

  for (i=0; i<mFolder->count(); i++)
  {
    mb = mFolder->getMsgBase(i);
    assert(mb != NULL); // otherwise using count() above is wrong

    flag = mb->status();
    hdr.sprintf("%c\n%s\n %s\n%s", (char)flag, (const char*)mb->from(),
		(const char*)mb->subject(), (const char*)mb->dateStr());
    insertItem(hdr);

    if (flag==KMMsgStatusNew) changeItemColor(darkRed);
    else if(flag==KMMsgStatusUnread) changeItemColor(darkBlue);
  }

  setAutoUpdate(TRUE);
  repaint();

  hdr.sprintf(nls->translate("%d Messages, %d unread."),
	      mFolder->count(), mFolder->countUnread());
  if (mFolder->isReadOnly()) hdr += nls->translate("Folder is read-only.");

  mOwner->statusMsg(hdr);
  kbp->idle();
}


//-----------------------------------------------------------------------------
bool KMHeaders :: prepareForDrag (int /*aCol*/, int /*aRow*/, char** data, 
				  int* size, int* type)
{
  static KMDragData dd;
  int i, from, to, high;

  high = numRows()-1;
  for (i=0, from=-1; i<=high; i++)
  {
    if (itemList[i]->isMarked())
    {
      from = i;
      break;
    }
  }
  for (i=high-1, to=-1; i>=0; i--)
  {
    if (itemList[i]->isMarked())
    {
      to = i;
      break;
    }
  }
  if (from < 0 || to < 0) return FALSE;

  dd.init(mFolder, from, to);
  *data = (char*)&dd;
  *size = sizeof(dd);
  *type = DndRawData;

  debug("Ready to drag...");
  return TRUE;
}


//-----------------------------------------------------------------------------
#include "kmheaders.moc"
