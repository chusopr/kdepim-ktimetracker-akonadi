// kmheaders.cpp

#include "kmcomposewin.h"
#include "kmheaders.h"
#include "kmfolder.h"
#include "kmmessage.h"
#include "kbusyptr.h"
#include "kmdragdata.h"
#include "kmglobal.h"

#include <drag.h>
#include <qstrlist.h>
#include <klocale.h>
#include <kiconloader.h>


//-----------------------------------------------------------------------------
KMHeaders::KMHeaders(QWidget *parent=0, const char *name=0) : 
  KTabListBox(parent, name, 4)
{
  QString kdir = app->kdedir();
  KIconLoader* loader = app->getIconLoader();
  static QPixmap pixNew, pixUns, pixDel, pixOld, pixRep;

  folder = NULL;
  getMsgIndex = -1;

  //setNumCols(4);
  setColumn(0, nls->translate("F"), 16, KTabListBox::PixmapColumn);
  setColumn(1, nls->translate("Sender"), 200);
  setColumn(2, nls->translate("Subject"), 250);
  setColumn(3, nls->translate("Date"), 100);
  readConfig();

  pixNew = loader->loadIcon("kmmsgnew.xpm");
  pixUns = loader->loadIcon("kmmsgunseen.xpm");
  pixDel = loader->loadIcon("kmmsgdel.xpm");
  pixOld = loader->loadIcon("kmmsgold.xpm");
  pixRep = loader->loadIcon("kmmsgreplied.xpm");

  dict().insert(KMMessage::statusToStr(KMMessage::stNew), &pixNew);
  dict().insert(KMMessage::statusToStr(KMMessage::stUnread), &pixUns);
  dict().insert(KMMessage::statusToStr(KMMessage::stDeleted), &pixDel);
  dict().insert(KMMessage::statusToStr(KMMessage::stOld), &pixOld);
  dict().insert(KMMessage::statusToStr(KMMessage::stReplied), &pixRep);

  connect(this,SIGNAL(selected(int,int)),
	  this,SLOT(selectMessage(int,int)));
  connect(this,SIGNAL(highlighted(int,int)),
	  this,SLOT(highlightMessage(int,int)));
  connect(this,SIGNAL(headerClicked(int)),
	  this,SLOT(headerClicked(int)));
}


//-----------------------------------------------------------------------------
void KMHeaders::setFolder (KMFolder *f)
{
  if (folder) 
  {
    disconnect(folder, SIGNAL(msgHeaderChanged(int)),
	       this, SLOT(msgHeaderChanged(int)));
    disconnect(folder, SIGNAL(msgAdded(int)),
	       this, SLOT(msgAdded(int)));
    disconnect(folder, SIGNAL(msgRemoved(int)),
	       this, SLOT(msgRemoved(int)));
    disconnect(folder, SIGNAL(changed()),
	       this, SLOT(msgChanged()));
  }

  folder=f;

  if (folder)
  {
    connect(folder, SIGNAL(msgHeaderChanged(int)), 
	    this, SLOT(msgHeaderChanged(int)));
    connect(folder, SIGNAL(msgAdded(int)),
	    this, SLOT(msgAdded(int)));
    connect(folder, SIGNAL(msgRemoved(int)),
	    this, SLOT(msgRemoved(int)));
    connect(folder, SIGNAL(changed()),
	    this, SLOT(msgChanged()));
  }

  updateMessageList();
}


//-----------------------------------------------------------------------------
void KMHeaders::msgChanged()
{
  updateMessageList();
}


//-----------------------------------------------------------------------------
void KMHeaders::msgAdded(int id)
{
  insertItem("", id-1);
  msgHeaderChanged(id);
}


//-----------------------------------------------------------------------------
void KMHeaders::msgRemoved(int id)
{
  removeItem(id-1);
}


//-----------------------------------------------------------------------------
void KMHeaders::msgHeaderChanged(int msgId)
{
  char hdr[256];
  KMMessage::Status flag;

  flag = folder->msgStatus(msgId);
  sprintf(hdr, "%c\n%s\n%s\n%s", (char)flag, folder->msgFrom(msgId), 
	  folder->msgSubject(msgId), folder->msgDate(msgId));
  changeItem(hdr, msgId-1);

  if (flag==KMMessage::stNew) changeItemColor(darkRed, msgId-1);
  else if(flag==KMMessage::stUnread) changeItemColor(darkBlue, msgId-1);
}


//-----------------------------------------------------------------------------
void KMHeaders::headerClicked(int column)
{
  KMFolder::SortField sortField;

  if (column==1)      sortField = KMFolder::sfFrom;
  else if (column==2) sortField = KMFolder::sfSubject;
  else if (column==3) sortField = KMFolder::sfDate;
  else return;

  kbp->busy();
  folder->sort(sortField);
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
  QList<KMMessage> msgList;
  KMMessage* msg;
  int rc, num;
  int cur = currentItem();
  bool curDeleted = (cur>=0 ? isMarked(cur) : FALSE);

  kbp->busy();

  // getMsg gets confused when messages are removed while calling
  // it repeatedly. To avoid this we create a temporary list of
  // the messages that will be moved.
  for (num=0,msg=getMsg(msgId); msg; msg=getMsg(),num++)
    msgList.append(msg);

  if (num > 3)
  {
    folder->quiet(TRUE);
    setAutoUpdate(FALSE);
  }

  unmarkAll();

  // now it is safe to move the messages.
  for (msg=msgList.first(); msg; msg=msgList.next())
    rc = trashFolder->moveMsg(msg);

  if (num > 3)
  {
    setAutoUpdate(TRUE);
    folder->quiet(FALSE);
    // repaint();
  }

  // display proper message if current message was deleted.
  if (curDeleted)
  {
    if (cur >= folder->numMsgs()) cur = folder->numMsgs() - 1;
    setCurrentItem(cur, -1);
  }

  kbp->idle();
}


//-----------------------------------------------------------------------------
void KMHeaders::undeleteMsg (int msgId)
{
  KMMessage* msg;

  for (msg=getMsg(msgId); msg; msg=getMsg())
  {
    msg->undel();
    //changeItemPart(msg->status(), getMsgIndex, 0);
  }
}


//-----------------------------------------------------------------------------
void KMHeaders::toggleDeleteMsg (int msgId)
{
  KMMessage* msg;

  if (!(msg=getMsg(msgId))) return;

  if (msg->status()==KMMessage::stDeleted) undeleteMsg(msgId);
  else deleteMsg(msgId);
}


//-----------------------------------------------------------------------------
void KMHeaders::forwardMsg (int msgId)
{
  KMComposeWin *w;
  KMMessage *msg = new KMMessage();

  kbp->busy();
  for (msg=getMsg(msgId); msg; msg=getMsg())
  {
    w = new KMComposeWin(0, 0, 0, msg, FORWARD);
    w->show();
  }
  kbp->idle(); 
}


//-----------------------------------------------------------------------------
void KMHeaders::replyToMsg (int msgId)
{
  KMComposeWin *w;
  KMMessage *msg = new KMMessage();

  kbp->busy();
  for (msg=getMsg(msgId); msg; msg=getMsg())
  {
    w = new KMComposeWin(0, 0, 0, msg, REPLY);
    w->show();
  }
  kbp->idle(); 
}


//-----------------------------------------------------------------------------
void KMHeaders::replyAllToMsg (int msgId)
{
  KMComposeWin *w;
  KMMessage *msg = new KMMessage();

  kbp->busy();
  for (msg=getMsg(msgId); msg; msg=getMsg())
  {
    w = new KMComposeWin(0, 0, 0, msg, REPLYALL);
    w->show();
  }
  kbp->idle(); 
}


//-----------------------------------------------------------------------------
void KMHeaders::moveMsgToFolder (KMFolder* destFolder, int msgId)
{
  KMMessage *msg = new KMMessage();

  assert(destFolder != NULL);

  kbp->busy();
  for (msg=getMsg(msgId); msg; msg=getMsg())
  {
    destFolder->moveMsg(msg);
  }
  kbp->idle(); 
}


//-----------------------------------------------------------------------------
KMMessage* KMHeaders::getMsg (int msgId)
{
  int i, high;

  if (!folder || msgId < -2)
  {
    getMsgIndex = -1;
    return NULL;
  }
  if (msgId >= 0) 
  {
    getMsgIndex = msgId;
    getMsgMulti = FALSE;
    return folder->getMsg(msgId+1);
  }

  if (msgId == -1)
  {
    getMsgMulti = TRUE;
    getMsgIndex = currentItem();
    for (i=0,high=numRows(); i<high; i++)
    {
      if (itemList[i].isMarked())
      {
	getMsgIndex = i;
	break;
      }
    }
 
    return (getMsgIndex>=0 ? folder->getMsg(getMsgIndex+1) : (KMMessage*)NULL);
  }

  if (getMsgIndex < 0) return NULL;

  if (getMsgMulti) for (getMsgIndex++; getMsgIndex < numRows(); getMsgIndex++)
  {
    if (itemList[getMsgIndex].isMarked()) 
      return folder->getMsg(getMsgIndex+1);
  }

  getMsgIndex = -1;
  return NULL;
}


//-----------------------------------------------------------------------------
void KMHeaders::changeItemPart (char c, int itemIndex, int column)
{
  char str[2];

  str[0] = c;
  str[1] = '\0';

  KTabListBox::changeItemPart((const char*)str, itemIndex, column);
}


//-----------------------------------------------------------------------------
void KMHeaders::highlightMessage(int idx, int/*colId*/)
{
  kbp->busy();
  emit messageSelected(folder->getMsg(idx+1));
  if (idx >= 0) setMsgRead(idx);
  kbp->idle();
}


//-----------------------------------------------------------------------------
void KMHeaders::selectMessage(int idx, int/*colId*/)
{
  KMMessage* msg;

  if (idx >= 0)
  {
    msg = getMsg(idx);
    if (msg->status()==KMMessage::stDeleted) undeleteMsg(idx);
    else deleteMsg(idx);
  }
}


//-----------------------------------------------------------------------------
void KMHeaders::updateMessageList(void)
{
  long i;
  char hdr[256];
  KMMessage::Status flag;
  KMMessage* msg;

  clear();
  if (!folder) return;

  kbp->busy();
  setAutoUpdate(FALSE);

  for (i = 1; i <= folder->numMsgs(); i++)
  {
    flag = folder->msgStatus(i);
    sprintf(hdr, "%c\n%s\n%s\n%s", (char)flag, folder->msgFrom(i),
	    folder->msgSubject(i), folder->msgDate(i));
    insertItem(hdr);

    if (flag==KMMessage::stNew) changeItemColor(darkRed);
    else if(flag==KMMessage::stUnread) changeItemColor(darkBlue);
  }

  setAutoUpdate(TRUE);
  repaint();
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
    if (itemList[i].isMarked())
    {
      from = i;
      break;
    }
  }
  for (i=high-1, to=-1; i>=0; i--)
  {
    if (itemList[i].isMarked())
    {
      to = i;
      break;
    }
  }
  if (from < 0 || to < 0) return FALSE;

  dd.init(folder, from, to);
  *data = (char*)&dd;
  *size = sizeof(dd);
  *type = DndRawData;

  return TRUE;
}


//-----------------------------------------------------------------------------
#include "kmheaders.moc"
