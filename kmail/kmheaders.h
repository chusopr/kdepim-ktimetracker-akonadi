#ifndef __KMHEADERS
#define __KMHEADERS

#include <qwidget.h>
#include <qstrlist.h>
#include "ktablistbox.h"

class KMFolder;
class KMMessage;

class KMHeaders : public KTabListBox {
  Q_OBJECT
public:
  KMHeaders(QWidget *parent=0, const char *name=0);
  
  virtual void setFolder(KMFolder *);
  KMFolder* currentFolder(void) { return folder; }

  virtual void changeItem (char c, int itemIndex, int column);
	// Change part of the contents of a line

  // The following methods process the message in the folder with
  // the given msgId, or if no msgId is given all selected
  // messages are processed.
  virtual void setMsgUnread(int msgId=-1);
  virtual void setMsgRead(int msgId=-1);
  virtual void deleteMsg(int msgId=-1);
  virtual void undeleteMsg(int msgId=-1);
  virtual void forwardMsg(int msgId=-1);
  virtual void replyToMsg(int msgId=-1);
  virtual void replyAllToMsg(int msgId=-1);
  virtual void moveMsgToFolder(KMFolder* destination, int msgId=-1);
  virtual void toggleDeleteMsg(int msgId=-1);
	// Delete/undelete message(s) depending on the flag of
	// the first selected message.

  KMMessage* getMsg (int msgId=-2);
	// Returns message with given id or current message if no
	// id is given. First call with msgId==-1 returns first
	// selected message, subsequent calls with no argument
	// return the following selected messages.

  int indexOfGetMsg (void) const { return getMsgIndex; }
	// Returns index of message returned by last getMsg() call

signals:
  virtual void messageSelected(KMMessage *);


protected slots:
  void selectMessage(int msgId, int colId);
  void highlightMessage(int msgId, int colId);

protected:
  virtual bool prepareForDrag (int col, int row, char** data, int* size, 
			       int* type);

private:
  virtual void updateMessageList(void);
  KMFolder *folder;
  int getMsgIndex;
  bool getMsgMulti;
};

#endif

