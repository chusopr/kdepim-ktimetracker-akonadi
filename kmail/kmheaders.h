#ifndef __KMHEADERS
#define __KMHEADERS

#include <qwidget.h>
#include <qstrlist.h>
#include <ktablistbox.h>
#include "kmmessage.h"

class KMFolder;
class KMMessage;
class KMMainWin;

typedef QList<KMMessage> KMMessageList;

#define KMHeadersInherited KTabListBox
class KMHeaders : public KTabListBox
{
  Q_OBJECT

public:
  KMHeaders(KMMainWin *owner, QWidget *parent=0, const char *name=0);
  virtual ~KMHeaders();
  
  virtual void setFolder(KMFolder *);
  KMFolder* folder(void) { return mFolder; }

  /** Change part of the contents of a line */
  virtual void changeItemPart (char c, int itemIndex, int column);

  /** Set current message. If id<0 then the first message is shown,
    if id>count() the last message is shown. */
  virtual void setCurrentMsg(int msgId);

  /** The following methods process the message in the folder with
    the given msgId, or if no msgId is given all selected
    messages are processed. */
  virtual void setMsgRead(int msgId=-1);
  virtual void deleteMsg(int msgId=-1);
  virtual void forwardMsg(int msgId=-1);
  virtual void replyToMsg(int msgId=-1);
  virtual void replyAllToMsg(int msgId=-1);
  virtual void moveMsgToFolder(KMFolder* destination, int msgId=-1);
  virtual void saveMsg(int msgId=-1);

  /** Returns list of selected messages. Do not delete this object. */
  virtual KMMessageList* selectedMsgs(void);

  /** Returns message with given id or current message if no
    id is given. First call with msgId==-1 returns first
    selected message, subsequent calls with no argument
    return the following selected messages. */
  KMMessage* getMsg (int msgId=-2);

  /** Returns index of message returned by last getMsg() call */
  int indexOfGetMsg (void) const { return getMsgIndex; }

  /** Returns pointer to owning main window. */
  KMMainWin* owner(void) const { return mOwner; }

signals:
  virtual void selected(KMMessage *);
  virtual void activated(KMMessage *);


public slots:
  void selectMessage(int msgId, int colId);
  void highlightMessage(int msgId, int colId);
  void msgHeaderChanged(int msgId);
  void msgChanged();
  void msgAdded(int);
  void msgRemoved(int);
  void headerClicked(int);
  void nextMessage();
  void prevMessage();

protected:
  virtual bool prepareForDrag (int col, int row, char** data, int* size, 
			       int* type);

private:
  virtual void updateMessageList(void);
  KMFolder* mFolder;
  KMMainWin* mOwner;
  int getMsgIndex;
  bool getMsgMulti;
  KMMessageList mSelMsgList;
};

#endif

