/* kmail main window
 * Maintained by Stefan Taferner <taferner@kde.org>
 * This code is under the GPL
 */
#ifndef __KMMAINWIN
#define __KMMAINWIN

#include <ktopwidget.h>

class KMFolder;
class KMFolderTree;
class KMHeaders;
class KMReaderView;
class KNewPanner;
class KMenuBar;
class KToolBar;
class KStatusBar;
class KMMessage;
class KMFolder;

#define KMMainWinInherited KTopLevelWidget

class KMMainWin : public KTopLevelWidget
{
  Q_OBJECT

public:
  KMMainWin(QWidget *parent = 0, char *name = 0);
  virtual ~KMMainWin();
  virtual void show();
  bool showInline;
  QPopupMenu *bodyParts;

  /** Insert a text field to the status bar and return ID of this field. */
  virtual int statusBarAddItem(const char* text);

  /** Change contents of a text field. */
  virtual void statusBarChangeItem(int id, const char* text);

  /** Easy access to main components of the window. */
  KMReaderView* messageView(void) const { return mMsgView; }
  KToolBar*     toolBar(void) const     { return mToolBar; }
  KStatusBar*   statusBar(void) const   { return mStatusBar; }
  KMFolderTree* folderTree(void) const  { return mFolderTree; }

public slots:
  /** Output given message in the statusbar message field. */
  void statusMsg(const char* text);

protected:
  virtual void closeEvent(QCloseEvent *);

  void parseConfiguration();
  void setupMenuBar();
  void setupToolBar();
  void setupStatusBar();

protected slots:
  void doAbout();
  void doClose();
  void doHelp();
  void doNewMailReader();
  void doSettings();
  void doFilter();
  void doUnimplemented();
  void doViewChange();
  void doAddFolder();
  void doCheckMail();
  void doCompose();
  void doModifyFolder();
  void doRemoveFolder();
  void doEmptyFolder();
  void doReplyToMsg();
  void doReplyAllToMsg();
  void doForwardMsg();
  void doDeleteMsg();
  void doPrintMsg();
  void doMoveMsg();

  void folderSelected(KMFolder*);
  void messageSelected(KMMessage*);
  //void pannerHasChanged();
  //void resizeEvent(QResizeEvent*);
  //void initIntegrated();
  //void initSeparated();

protected:
  KMenuBar     *mMenuBar;
  KToolBar     *mToolBar;
  KStatusBar   *mStatusBar;
  KMFolderTree *mFolderTree;
  KMReaderView *mMsgView;
  KNewPanner   *mHorizPanner, *mVertPanner;
  KMHeaders    *mHeaders;
  KMFolder *mFolder;
  bool		mIntegrated;
  int		mMessageStatusId;
  int		mHorizPannerSep, mVertPannerSep;
};

#endif

