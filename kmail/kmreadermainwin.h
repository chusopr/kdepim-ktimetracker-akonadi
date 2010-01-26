// -*- mode: C++; c-file-style: "gnu" -*-

#ifndef KMReaderMainWin_h
#define KMReaderMainWin_h

#include "secondarywindow.h"

#include <kurl.h>

class KMReaderWin;
class KMMessage;
class KMMessagePart;
class KAction;
class KActionMenu;
class KMFolderIndex;
class KMFolder;
class KFontAction;
class KFontSizeAction;
template <typename T, typename S> class QMap;

namespace KMail {
class MessageActions;
}

class KMReaderMainWin : public KMail::SecondaryWindow
{
  Q_OBJECT

public:
  KMReaderMainWin( bool htmlOverride, bool htmlLoadExtOverride, char *name = 0 );
  KMReaderMainWin( char *name = 0 );
  KMReaderMainWin(KMMessagePart* aMsgPart,
    bool aHTML, const QString& aFileName, const QString& pname,
    const QString & encoding, char *name = 0 );
  virtual ~KMReaderMainWin();

  void setUseFixedFont( bool useFixedFont );

  /**
   * take ownership of and show @param msg
   *
   * The last two paramters, serNumOfOriginalMessage and nodeIdOffset, are needed when @p msg
   * is derived from another message, e.g. the user views an encapsulated message in this window.
   * Then, the reader needs to know about that original message, so those to paramters are passed
   * onto setOriginalMsg() of KMReaderWin.
   */
  void showMsg( const QString & encoding, KMMessage *msg,
                unsigned long serNumOfOriginalMessage = 0, int nodeIdOffset = -1 );

  /**
   * Sets up action list for forward menu.
  */
  void setupForwardingActionsList();

private slots:
  void slotMsgPopup(KMMessage &aMsg, const KURL &aUrl, const QPoint& aPoint);

  /** Copy selected messages to folder with corresponding to given menuid */
  void copySelectedToFolder( int menuId );
  void slotTrashMsg();
  void slotPrintMsg();
  void slotForwardInlineMsg();
  void slotForwardAttachedMsg();
  void slotForwardDigestMsg();
  void slotRedirectMsg();
  void slotShowMsgSrc();
  void slotMarkAll();
  void slotCopy();
  void slotFind();
  void slotFindNext();
  void slotFontAction(const QString &);
  void slotSizeAction(int);
  void slotCreateTodo();
  void slotEditToolbars();

  void slotConfigChanged();
  void slotUpdateToolbars();

  void slotFolderRemoved( QObject* folderPtr );

  /// This closes the window if the setting to close the window after replying or
  /// forwarding is set.
  void slotReplyOrForwardFinished();

private:
  void initKMReaderMainWin();
  void setupAccel();

  /**
   * @see the KMMainWidget function with the same name.
   */
  void setupForwardActions();

  KMReaderWin *mReaderWin;
  KMMessage *mMsg;
  KURL mUrl;
  QMap<int,KMFolder*> mMenuToFolder;
  // a few actions duplicated from kmmainwidget
  KAction *mTrashAction, *mPrintAction, *mSaveAsAction, *mForwardInlineAction,
          *mForwardAttachedAction, *mForwardDigestAction, *mRedirectAction,
          *mViewSourceAction;
  KActionMenu *mForwardActionMenu;
  KFontAction *fontAction;
  KFontSizeAction *fontSizeAction;
  KMail::MessageActions *mMsgActions;

};

#endif /*KMReaderMainWin_h*/
