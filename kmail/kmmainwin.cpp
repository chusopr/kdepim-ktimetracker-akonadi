// kmmainwin.cpp
//#define MALLOC_DEBUG 1

#include <kwin.h>
#include <kmfldsearch.h>

#ifdef MALLOC_DEBUG
#include <malloc.h>
#endif

#undef Unsorted // X headers...
#include <qdir.h>
#include <qclipboard.h>
#include <qaccel.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qsplitter.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kapp.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstdaccel.h>

#include <krun.h>
#include <kopenwith.h>
#include <kpopupmenu.h>

#include <kmenubar.h>
#include <kmessagebox.h>

#include <kparts/browserextension.h>

#include <kaction.h>
#include <kstdaction.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>
#include <kcharsets.h>

#include "configuredialog.h"
#include "kmbroadcaststatus.h"
#include "kmfoldermgr.h"
#include "kmfolderdia.h"
#include "kmaccount.h"
#include "kmacctmgr.h"
#include "kbusyptr.h"
#include "kmfoldertree.h"
#include "kmheaders.h"
#include "kmreaderwin.h"
#include "kmfolder.h"
#include "kmmessage.h"
#include "kmcomposewin.h"
#include "kmglobal.h"
#include "kmfolderseldlg.h"
#include "kmfiltermgr.h"
#include "kmsender.h"
#include "kmaddrbookdlg.h"
#include "kmaddrbook.h"
#include "kwin.h"

#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <klocale.h>

#include "kmmainwin.moc"

//-----------------------------------------------------------------------------
KMMainWin::KMMainWin(QWidget *, char *name) :
  KMMainWinInherited(name)
{
  // must be the first line of the constructor:
  searchWin = 0;
  mStartupDone = FALSE;
  mbNewMBVisible = false;
  QListViewItem* idx;
  mIntegrated  = TRUE;
  mFolder = NULL;
  mHorizPannerSep = new QValueList<int>;
  mVertPannerSep = new QValueList<int>;
  *mHorizPannerSep << 1 << 1;
  *mVertPannerSep << 1 << 1;

  setMinimumSize(400, 300);

  mConfigureDialog = 0;

  readPreConfig();
  createWidgets();

  setupMenuBar();
  setupStatusBar();

  applyMainWindowSettings(kapp->config(), "Main Window");
  toolbarAction->setChecked(!toolBar()->isHidden());
  statusbarAction->setChecked(!statusBar()->isHidden());

  readConfig();
  activatePanners();

  idx = mFolderTree->indexOfFolder(kernel->inboxFolder());
  if (idx!=0) {
    mFolderTree->setCurrentItem(idx);
    mFolderTree->setSelected(idx,TRUE);
  }

  connect(kernel->msgSender(), SIGNAL(statusMsg(const QString&)),
	  SLOT(statusMsg(const QString&)));
  connect(kernel->acctMgr(), SIGNAL( checkedMail(bool)),
          SLOT( slotMailChecked(bool)));

  setCaption( i18n("KDE Mail Client") );

  // must be the last line of the constructor:
  mStartupDone = TRUE;
}


//-----------------------------------------------------------------------------
KMMainWin::~KMMainWin()
{
  if (searchWin)
    delete searchWin;
  writeConfig();
  writeFolderConfig();

  saveMainWindowSettings(kapp->config(), "Main Window");
  kapp->config()->sync();

  if (mHeaders)    delete mHeaders;
  if (mStatusBar)  delete mStatusBar;
  if (mFolderTree) delete mFolderTree;
}


//-----------------------------------------------------------------------------
void KMMainWin::readPreConfig(void)
{
  KConfig *config = kapp->config();
  QString str;

  config->setGroup("Geometry");
  mLongFolderList = config->readBoolEntry("longFolderList", false);

  config->setGroup("General");
  mEncodingStr = config->readEntry("encoding", "");
}


//-----------------------------------------------------------------------------
void KMMainWin::readFolderConfig(void)
{
  if (!mFolder)
    return;

  KConfig *config = kapp->config();
  int pathLen = mFolder->path().length() - kernel->folderMgr()->basePath().length();
  QString path = mFolder->path().right( pathLen );

  if (!path.isEmpty())
    path = path.right( path.length() - 1 ) + "/";
  config->setGroup("Folder-" + path + mFolder->name());
  mFolderThreadPref = config->readBoolEntry( "threadMessagesOverride", false );
  mFolderHtmlPref = config->readBoolEntry( "htmlMailOverride", false );
}


//-----------------------------------------------------------------------------
void KMMainWin::writeFolderConfig(void)
{
  if (!mFolder)
    return;

  KConfig *config = kapp->config();
  int pathLen = mFolder->path().length() - kernel->folderMgr()->basePath().length();
  QString path = mFolder->path().right( pathLen );

  if (!path.isEmpty())
    path = path.right( path.length() - 1 ) + "/";
  config->setGroup("Folder-" + path + mFolder->name());
  config->writeEntry( "threadMessagesOverride", mFolderThreadPref );
  config->writeEntry( "htmlMailOverride", mFolderHtmlPref );
}


//-----------------------------------------------------------------------------
void KMMainWin::readConfig(void)
{
  KConfig *config = kapp->config();
  bool oldLongFolderList=false;
  int w, h;
  QString str;

  if (mStartupDone)
  {
    writeConfig();
    oldLongFolderList = mLongFolderList;
    readPreConfig();
    mHeaders->refreshNestedState();
    if (oldLongFolderList != mLongFolderList)
    {
      hide();
      if (mHorizPanner->parent()==this) delete mHorizPanner;
      else delete mVertPanner;
      createWidgets();
    }
  }

  config->setGroup("Reader");
  mHtmlPref = config->readBoolEntry( "htmlMail", false );

  config->setGroup("Geometry");
  mThreadPref = config->readBoolEntry( "nestedMessages", false );
  str = config->readEntry("MainWin", "600,600");
  if (!str.isEmpty() && str.find(',')>=0)
  {
    sscanf(str,"%d,%d",&w,&h);
    resize(w,h);
  }
  str = config->readEntry("Panners", "300,130");
  if ((!str.isEmpty()) && (str.find(',')!=-1))
    sscanf(str,"%d,%d",&((*mHorizPannerSep)[0]),&((*mVertPannerSep)[0]));
  else
    (*mHorizPannerSep)[0] = (*mVertPannerSep)[0] = 100;
  (*mHorizPannerSep)[1] = h - (*mHorizPannerSep)[0];
  (*mVertPannerSep)[1] = w - (*mVertPannerSep)[0];

  mMsgView->readConfig();
  mHeaders->readConfig();
  mFolderTree->readConfig();

  config->setGroup("General");
  mSendOnCheck = config->readBoolEntry("sendOnCheck",false);
  mBeepOnNew = config->readBoolEntry("beep-on-mail", false);
  mBoxOnNew = config->readBoolEntry("msgbox-on-mail", false);
  mExecOnNew = config->readBoolEntry("exec-on-mail", false);
  mNewMailCmd = config->readEntry("exec-on-mail-cmd", "");

  // Re-activate panners
  if (mStartupDone)
  {
    if (oldLongFolderList != mLongFolderList)
      activatePanners();
    //    kernel->kbp()->busy(); //Crashes KMail
    mFolderTree->reload();
    QListViewItem *qlvi = mFolderTree->indexOfFolder(mFolder);
    if (qlvi!=0) {
      mFolderTree->setCurrentItem(qlvi);
      mFolderTree->setSelected(qlvi,TRUE);
    }

    // sanders - New code
    mHeaders->setFolder(mFolder);
    int aIdx = mHeaders->currentItemIndex();
    if (aIdx != -1)
      mMsgView->setMsg( mFolder->getMsg(aIdx), true );
    else
      mMsgView->setMsg( 0, true );
    show();
    // sanders - Maybe this fixes a bug?

    /* Old code
    mMsgView->setMsg( mMsgView->msg(), TRUE );
    mHeaders->setFolder(mFolder);
    //    kernel->kbp()->idle(); //For symmetry
    show();
    */
  }
}


//-----------------------------------------------------------------------------
void KMMainWin::writeConfig(void)
{
  QString s;
  KConfig *config = kapp->config();
  QRect r = geometry();

  mMsgView->writeConfig();
  mFolderTree->writeConfig();

  config->setGroup("Geometry");

  s.sprintf("%i,%i", r.width(), r.height());
  config->writeEntry("MainWin", s);

  // Get those panner sizes right!
  s.sprintf("%i,%i",
	    (mHorizPanner->sizes()[0] * r.height() ) /
	    (mHorizPanner->sizes()[0] + mHorizPanner->sizes()[1]),
	    ( mVertPanner->sizes()[0] * r.width() ) /
	    (mVertPanner->sizes()[0] + mVertPanner->sizes()[1])
	    );
  config->writeEntry("Panners", s);

  config->setGroup("General");
  config->writeEntry("encoding", mEncodingStr);
}


//-----------------------------------------------------------------------------
void KMMainWin::createWidgets(void)
{
  QSplitter *pnrMsgView, *pnrMsgList, *pnrFldList;
  QAccel *accel = new QAccel(this);

  // create panners
  if (mLongFolderList)
  {
    mVertPanner  = new QSplitter(Qt::Horizontal, this, "vertPanner" );
    mHorizPanner = new QSplitter(Qt::Vertical, mVertPanner, "horizPanner" );
    pnrFldList = mHorizPanner;
    pnrMsgView = mVertPanner;
    pnrMsgList = mVertPanner;
  }
  else
  {
    mHorizPanner = new QSplitter( Qt::Vertical, this, "horizPanner" );
    mVertPanner  = new QSplitter( Qt::Horizontal, mHorizPanner, "vertPanner" );
    pnrMsgView = mVertPanner;
    pnrMsgList = mHorizPanner;
    pnrFldList = mHorizPanner;
  }

  // BUG -sanders these accelerators stop working after switching
  // between long/short folder layout
  // Probably need to disconnect them first.

  // create list of messages
  mHeaders = new KMHeaders(this, pnrMsgList, "headers");
  connect(mHeaders, SIGNAL(selected(KMMessage*)),
	  this, SLOT(slotMsgSelected(KMMessage*)));
  connect(mHeaders, SIGNAL(activated(KMMessage*)),
	  this, SLOT(slotMsgActivated(KMMessage*)));
  accel->connectItem(accel->insertItem(Key_Left),
		     mHeaders, SLOT(prevMessage()));
  accel->connectItem(accel->insertItem(Key_Right),
		     mHeaders, SLOT(nextMessage()));

  QString charset("iso8859-1");
  mCodec = KGlobal::charsets()->codecForName(charset);

  // create HTML reader widget
  mMsgView = new KMReaderWin(pnrMsgView);
  connect(mMsgView, SIGNAL(statusMsg(const QString&)),
	  this, SLOT(statusMsg(const QString&)));
  connect(mMsgView, SIGNAL(popupMenu(const KURL&,const QPoint&)),
	  this, SLOT(slotMsgPopup(const KURL&,const QPoint&)));
  connect(mMsgView, SIGNAL(urlClicked(const KURL&,int)),
	  this, SLOT(slotUrlClicked(const KURL&,int)));
  connect(mMsgView, SIGNAL(showAtmMsg(KMMessage *)),
	  this, SLOT(slotAtmMsg(KMMessage *)));
  connect(mHeaders, SIGNAL(maybeDeleting()),
	  mMsgView, SLOT(clearCache()));
  accel->connectItem(accel->insertItem(Key_Up),
		     mMsgView, SLOT(slotScrollUp()));
  accel->connectItem(accel->insertItem(Key_Down),
		     mMsgView, SLOT(slotScrollDown()));
  accel->connectItem(accel->insertItem(Key_Prior),
		     mMsgView, SLOT(slotScrollPrior()));
  accel->connectItem(accel->insertItem(Key_Next),
		     mMsgView, SLOT(slotScrollNext()));
  accel->connectItem(accel->insertItem(Key_M),
                     this, SLOT(slotMoveMsg()));
  accel->connectItem(accel->insertItem(Key_C),
                     this, SLOT(slotCopyMsg()));
  accel->connectItem(accel->insertItem(Key_Delete),
		     this, SLOT(slotDeleteMsg()));

  // create list of folders
  mFolderTree  = new KMFolderTree(pnrFldList, "folderTree");
  connect(mFolderTree, SIGNAL(folderSelected(KMFolder*)),
	  this, SLOT(folderSelected(KMFolder*)));
  connect(mFolderTree, SIGNAL(folderDrop(KMFolder*)),
	  this, SLOT(slotMoveMsgToFolder(KMFolder*)));
  connect(mFolderTree, SIGNAL(folderDropCopy(KMFolder*)),
          this, SLOT(slotCopyMsgToFolder(KMFolder*)));
}


//-----------------------------------------------------------------------------
void KMMainWin::activatePanners(void)
{
  // glue everything together
  if (mLongFolderList)
  {
    mHeaders->reparent( mHorizPanner, 0, QPoint( 0, 0 ) );
    mMsgView->reparent( mHorizPanner, 0, QPoint( 0, 0 ) );
    mFolderTree->reparent( mVertPanner, 0, QPoint( 0, 0 ) );
    mVertPanner->moveToFirst( mFolderTree );
    setCentralWidget(mVertPanner);
  }
  else
  {
    mFolderTree->reparent( mVertPanner, 0, QPoint( 0, 0 ) );
    mHeaders->reparent( mVertPanner, 0, QPoint( 0, 0 ) );
    mMsgView->reparent( mHorizPanner, 0, QPoint( 0, 0 ) );
    setCentralWidget(mHorizPanner);
  }
  mHorizPanner->setSizes( *mHorizPannerSep );
  mVertPanner->setSizes( *mVertPannerSep );

  mVertPanner->setResizeMode( mFolderTree, QSplitter::KeepSize);
  if( mLongFolderList )
  {
    mHorizPanner->setResizeMode( mHeaders, QSplitter::KeepSize);
  }
  else
  {
    mHorizPanner->setResizeMode( mVertPanner, QSplitter::KeepSize);
  }
}

//-----------------------------------------------------------------------------
void KMMainWin::slotSetEncoding()
{
     mEncodingStr = mEncoding->currentText();
     if (mEncoding->currentItem() == 0) // Auto
       mCodec = 0;
     else
       mCodec = KGlobal::charsets()->codecForName( mEncodingStr );
     mMsgView->setCodec(mCodec);
     return;
}

//-----------------------------------------------------------------------------
void KMMainWin::statusMsg(const QString& aText)
{
  QString text = " " + aText + " ";
  int statusWidth = mStatusBar->width() - littleProgress->width()
    - fontMetrics().maxWidth();

  while (!text.isEmpty() && fontMetrics().width( text ) >= statusWidth)
    text.truncate( text.length() - 1);

  mStatusBar->changeItem( text, mMessageStatusId);
}


//-----------------------------------------------------------------------------
void KMMainWin::hide()
{
  KMMainWinInherited::hide();
}


//-----------------------------------------------------------------------------
void KMMainWin::show()
{
  mHorizPanner->setSizes( *mHorizPannerSep );
  mVertPanner->setSizes( *mVertPannerSep );
  KMMainWinInherited::show();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotClose()
{
  close(TRUE);
}

//-------------------------------------------------------------------------
void KMMainWin::slotSearch()
{
  if(!searchWin) {
    QString curFolder = "";
    if( mFolder )
      curFolder = mFolder->name();
    searchWin = new KMFldSearch(this, "Search", curFolder, false);
    connect(searchWin, SIGNAL(destroyed()),
	    this, SLOT(slotSearchClosed()));
  }

  searchWin->show();
  KWin::setActiveWindow(searchWin->winId());
}


//-------------------------------------------------------------------------
void KMMainWin::slotSearchClosed() {
  if(searchWin)
    searchWin = 0;
}


//-------------------------------------------------------------------------
void KMMainWin::slotFind() {
  if( mMsgView )
    mMsgView->slotFind();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotHelp()
{
  kapp->invokeHTMLHelp("","");
}


//-----------------------------------------------------------------------------
void KMMainWin::slotNewMailReader()
{
  KMMainWin *d;

  d = new KMMainWin(NULL);
  d->show();
  d->resize(d->size());
}


//-----------------------------------------------------------------------------
void KMMainWin::slotSettings()
{
  if( mConfigureDialog == 0 )
  {
    mConfigureDialog = new ConfigureDialog( this, "configure", false );
  }
  mConfigureDialog->show();
}




//-----------------------------------------------------------------------------
void KMMainWin::slotFilter()
{
  kernel->filterMgr()->openDialog( this );
}


//-----------------------------------------------------------------------------
void KMMainWin::slotAddrBook()
{
  KMAddrBookExternal::launch(this);
}


//-----------------------------------------------------------------------------
void KMMainWin::slotUnimplemented()
{
  qWarning(i18n("Sorry, but this feature\nis still missing"));
}

//-----------------------------------------------------------------------------
void KMMainWin::slotAddFolder()
{
  KMFolderDialog *d;

  d = new KMFolderDialog(0, &(kernel->folderMgr()->dir()),
			 this, i18n("Create Folder"));
  if (d->exec()) {
    mFolderTree->reload();
    QListViewItem *qlvi = mFolderTree->indexOfFolder( mFolder );
    if (qlvi) {
      qlvi->setOpen(TRUE);
      mFolderTree->setCurrentItem( qlvi );
    }
  }
}

//-----------------------------------------------------------------------------
void KMMainWin::slotCheckMail()
{
 if(kernel->checkingMail())
 {
    KMessageBox::information(this,
		     i18n("Your mail is already being checked."));
    return;
  }

 kernel->setCheckingMail(true);

 kernel->acctMgr()->checkMail(true);

 kernel->setCheckingMail(false);
}


//-----------------------------------------------------------------------------
void KMMainWin::slotCheckOneAccount(int item)
{
  if(kernel->checkingMail())
  {
    KMessageBox::information(this,
		     i18n("Your mail is already being checked."));
    return;
  }

  kernel->setCheckingMail(true);

  //  kbp->busy();
  kernel->acctMgr()->intCheckMail(item);
  // kbp->idle();

  kernel->setCheckingMail(false);
}

void KMMainWin::slotMailChecked(bool newMail) {
  if(mSendOnCheck)
    slotSendQueued();

  if (!newMail)
    return;

  if (mBeepOnNew) {
    KApplication::beep();
  }

  // FIXME: change system() to a KProcess
  if (mExecOnNew) {
    if (mNewMailCmd.length() > 0)
      system((const char *)mNewMailCmd);
  }

  if (mBoxOnNew && !mbNewMBVisible) {
    mbNewMBVisible = true;
    KMessageBox::information(this, QString(i18n("You have new mail!")),
                                   QString(i18n("New Mail")));
    mbNewMBVisible = false;
  }

  // Todo:
  // scroll mHeaders to show new items if current item would
  // still be visible
  //  mHeaders->showNewMail();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotCompose()
{
  KMComposeWin *win;
  KMMessage* msg = new KMMessage;
  msg->initHeader();

  win = new KMComposeWin(msg);
  win->show();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotModifyFolder()
{
  KMFolderDialog *d;

  if (!mFolder) return;
  d = new KMFolderDialog((KMFolder*)mFolder, mFolder->parent(),
			 this, i18n("Modify Folder") );
  if (d->exec()) {
    mFolderTree->reload();
    QListViewItem *qlvi = mFolderTree->indexOfFolder( mFolder );
    if (qlvi) {
      qlvi->setOpen(TRUE);
      mFolderTree->setCurrentItem( qlvi );
    }
  }
}


//-----------------------------------------------------------------------------
void KMMainWin::slotEmptyFolder()
{
  QString str;
  KMMessage* msg;

  if (!mFolder) return;

  mMsgView->clearCache();

  kernel->kbp()->busy();

  // begin of critical part
  // from here to "end..." no signal may change to another mFolder, otherwise
  // the wrong folder will be truncated in expunge (dnaber, 1999-08-29)
  mFolder->open();
  mHeaders->setFolder(NULL);
  mMsgView->clear();

  if (mFolder != kernel->trashFolder())
  {
    // FIXME: If we run out of disk space mail may be lost rather
    // than moved into the trash -sanders
    while ((msg = mFolder->take(0)) != NULL) {
      kernel->trashFolder()->addMsg(msg);
      kernel->trashFolder()->unGetMsg(kernel->trashFolder()->count()-1);
    }
  }

  mFolder->close();
  mFolder->expunge();
  // end of critical
  if (mFolder != kernel->trashFolder())
    statusMsg(i18n("Moved all messages into trash"));

  mHeaders->setFolder(mFolder);
  kernel->kbp()->idle();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotRemoveFolder()
{
  QString str;
  QDir dir;

  if (!mFolder) return;
  if (mFolder->isSystemFolder() || strcmp(mFolder->type(),"plain")!=0)
  {
    qWarning(i18n("Cannot remove a\nsystem folder."));
    return;
  }

  str = i18n("Are you sure you want to remove the folder\n"
	     "\"%1\" and all subfolders, discarding their contents?")
			     .arg(mFolder->label());

  if (KMessageBox::warningContinueCancel(this, str,
                              i18n("Remove folder"), i18n("&Remove") )
      ==
      KMessageBox::Continue)
  {
    KMFolder *folderToDelete = mFolder;
    QListViewItem *qlviCur = mFolderTree->currentItem();
    QListViewItem *qlvi = qlviCur->itemAbove();
    if (!qlvi)
      qlvi = mFolderTree->currentItem()->itemBelow();
    mHeaders->setFolder(0);
    mMsgView->clear();
    mFolderTree->setCurrentItem( qlvi );
    mFolderTree->setSelected( qlvi, TRUE );
    delete qlviCur;
    kernel->folderMgr()->remove(folderToDelete);
  }
}


//-----------------------------------------------------------------------------
void KMMainWin::slotCompactFolder()
{
  int idx = mHeaders->currentItemIndex();
  if (mFolder)
  {
    kernel->kbp()->busy();
    mFolder->compact();
    kernel->kbp()->idle();
  }
  mHeaders->setCurrentItemByIndex(idx);
}


//-----------------------------------------------------------------------------
void KMMainWin::slotOverrideHtml()
{
  mFolderHtmlPref = !mFolderHtmlPref;
  mMsgView->setHtmlOverride(mFolderHtmlPref);
  mMsgView->setMsg( mMsgView->msg(), TRUE );
}

//-----------------------------------------------------------------------------
void KMMainWin::slotOverrideThread()
{
  mFolderThreadPref = !mFolderThreadPref;
  mHeaders->setNestedOverride(mFolderThreadPref);
}

//-----------------------------------------------------------------------------
void KMMainWin::slotPrintMsg()
{
  if(mHeaders->currentItemIndex() >= 0)
    mMsgView->printMsg();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotReplyToMsg()
{
  mHeaders->replyToMsg();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotReplyAllToMsg()
{
  mHeaders->replyAllToMsg();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotForwardMsg()
{
  mHeaders->forwardMsg();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotRedirectMsg()
{
  mHeaders->redirectMsg();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotBounceMsg()
{
  mHeaders->bounceMsg();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotEditMsg()
{
  KMMessage *msg;
  int aIdx;

  if(mFolder != kernel->outboxFolder())
  {
      KMessageBox::sorry(0,
         i18n("Sorry, only messages in the outbox folder can be edited."));
      return;
  }


  if((aIdx = mHeaders->currentItemIndex()) <= -1)
    return;
  if(!(msg = mHeaders->getMsg(aIdx)))
    return;

  KMComposeWin *win = new KMComposeWin;
  win->setMsg(msg,FALSE);
  win->show();
}



//-----------------------------------------------------------------------------
void KMMainWin::slotResendMsg()
{
  mHeaders->resendMsg();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotDeleteMsg()
{
  mHeaders->deleteMsg();
}

//-----------------------------------------------------------------------------
void KMMainWin::slotUndo()
{
  mHeaders->undo();
}

//-----------------------------------------------------------------------------
void KMMainWin::slotShowMsgSrc()
{
  KMMessage* msg = mHeaders->getMsg(-1);
  if (msg) msg->viewSource(i18n("Message as Plain Text"),mCodec);
}


//-----------------------------------------------------------------------------
void KMMainWin::slotMoveMsg()
{
  KMFolderSelDlg dlg(i18n("Move Message - Select Folder"));
  KMFolder* dest;

  if (!dlg.exec()) return;
  if (!(dest = dlg.folder())) return;

  mHeaders->moveMsgToFolder(dest);
}

//-----------------------------------------------------------------------------
void KMMainWin::slotMoveMsgToFolder( KMFolder *dest)
{
  mHeaders->moveMsgToFolder(dest);
}

//-----------------------------------------------------------------------------
void KMMainWin::slotCopyMsgToFolder( KMFolder *dest)
{
  mHeaders->copyMsgToFolder(dest);
}

//-----------------------------------------------------------------------------
void KMMainWin::slotApplyFilters()
{
  mHeaders->applyFiltersOnMsg();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotCopyMsg()
{
  KMFolderSelDlg dlg(i18n("Copy Message - Select Folder"));
  KMFolder* dest;

  if (!dlg.exec()) return;
  if (!(dest = dlg.folder())) return;

  mHeaders->copyMsgToFolder(dest);
}


//-----------------------------------------------------------------------------
void KMMainWin::slotSaveMsg()
{
  if(mHeaders->currentItemIndex() == -1)
    return;
  mHeaders->saveMsg(-1);
}


//-----------------------------------------------------------------------------
void KMMainWin::slotSendQueued()
{
  kernel->msgSender()->sendQueued();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotViewChange()
{
  if(mBodyPartsMenu->isItemChecked(mBodyPartsMenu->idAt(0)))
  {
    mBodyPartsMenu->setItemChecked(mBodyPartsMenu->idAt(0),FALSE);
    mBodyPartsMenu->setItemChecked(mBodyPartsMenu->idAt(1),TRUE);
  }
  else if(mBodyPartsMenu->isItemChecked(mBodyPartsMenu->idAt(1)))
  {
    mBodyPartsMenu->setItemChecked(mBodyPartsMenu->idAt(1),FALSE);
    mBodyPartsMenu->setItemChecked(mBodyPartsMenu->idAt(0),TRUE);
  }

  //mMsgView->setInline(!mMsgView->isInline());
}


//-----------------------------------------------------------------------------
void KMMainWin::slotSetHeaderStyle(int id)
{
  if(id <= 5)
  {
    mViewMenu->setItemChecked((int)mMsgView->headerStyle(), FALSE);
    mMsgView->setHeaderStyle((KMReaderWin::HeaderStyle)id);
    mViewMenu->setItemChecked(id, TRUE);
  }
  else
  {
    mViewMenu->setItemChecked((int)mMsgView->attachmentStyle()+5, FALSE);
    mViewMenu->setItemChecked(id, TRUE);
    mMsgView->setAttachmentStyle(id-5);
  }
  readConfig(); // added this so _all_ the other widgets get this information
}

//-----------------------------------------------------------------------------
void KMMainWin::folderSelected(KMFolder* aFolder)
{
  if (mFolder == aFolder)
    return;

  kernel->kbp()->busy();
  writeFolderConfig();
  mFolder = (KMFolder*)aFolder;
  readFolderConfig();
  mMsgView->setHtmlOverride(mFolderHtmlPref);
  mHeaders->setFolder(mFolder);
  kernel->kbp()->idle();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotMsgSelected(KMMessage *msg)
{
  mMsgView->setMsg(msg);
}

//-----------------------------------------------------------------------------
void KMMainWin::slotSelectFolder(KMFolder* folder)
{
  QListViewItem* item = mFolderTree->indexOfFolder(folder);
  if (item)
    mFolderTree->setCurrentItem( item );
}

//-----------------------------------------------------------------------------
void KMMainWin::slotSelectMessage(KMMessage* msg)
{
  int idx = mFolder->find(msg);
  if (idx != -1) {
    mHeaders->setCurrentMsg(idx);
    mMsgView->setMsg(msg);
  }
}

//-----------------------------------------------------------------------------
void KMMainWin::slotSetMsgStatus(int id)
{
  mHeaders->setMsgStatus((KMMsgStatus)id);
}


//-----------------------------------------------------------------------------
//called from heders. Message must not be deleted on close
void KMMainWin::slotMsgActivated(KMMessage *msg)
{
  KMReaderWin *win;
  assert(msg != NULL);

  win = new KMReaderWin;
  showMsg(win, msg);
}


//called from reader win. message must be deleted on close
void KMMainWin::slotAtmMsg(KMMessage *msg)
{
  KMReaderWin *win;
  assert(msg != NULL);
  win = new KMReaderWin;
  connect(win, SIGNAL(showAtmMsg(KMMessage *)),
	  this, SLOT(slotAtmMsg(KMMessage *)));
  win->setAutoDelete(true); //delete on end
  showMsg(win, msg);
}


void KMMainWin::showMsg(KMReaderWin *win, KMMessage *msg)
{
  KWin::setIcons(win->winId(), kapp->icon(), kapp->miniIcon());
  win->setMsg(msg, true); // hack to work around strange QTimer bug
  win->resize(550,600);

  connect(win, SIGNAL(statusMsg(const QString&)),
          this, SLOT(statusMsg(const QString&)));
  connect(win, SIGNAL(popupMenu(const KURL&,const QPoint&)),
          this, SLOT(slotMsgPopup(const KURL&,const QPoint&)));
  connect(win, SIGNAL(urlClicked(const KURL&,int)),
          this, SLOT(slotUrlClicked(const KURL&,int)));

  QAccel *accel = new QAccel(win);
  accel->connectItem(accel->insertItem(Key_Up),
                     win, SLOT(slotScrollUp()));
  accel->connectItem(accel->insertItem(Key_Down),
                     win, SLOT(slotScrollDown()));
  accel->connectItem(accel->insertItem(Key_Prior),
                     win, SLOT(slotScrollPrior()));
  //accel->connectItem(accel->insertItem(Key_S),
  //                   win, SLOT(slotCopyMsg()));
  win->show();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotCopyText()
{
  QString temp;
  temp = mMsgView->copyText();
  kapp->clipboard()->setText(temp);
}

//-----------------------------------------------------------------------------
void KMMainWin::slotMarkAll() {
  QListViewItem *item;
  for (item = mHeaders->firstChild(); item; item = item->itemBelow())
    mHeaders->setSelected( item, TRUE );
}


//-----------------------------------------------------------------------------
void KMMainWin::slotUrlClicked(const KURL &aUrl, int)
{
  KMComposeWin *win;
  KMMessage* msg;

  if (aUrl.protocol() == "mailto")
  {
    msg = new KMMessage;
    msg->initHeader();
    msg->setTo(aUrl.path());
    QString query=aUrl.query();
    while (!query.isEmpty()) {
      QString queryPart;
      int secondQuery = query.find('?',1);
      if (secondQuery != -1)
	queryPart = query.left(secondQuery);
      else
	queryPart = query;
      query = query.mid(queryPart.length());

      if (queryPart.left(9) == "?subject=")
	msg->setSubject( KURL::decode_string(queryPart.mid(9)) );
      else if (queryPart.left(6) == "?body=")
	// It is correct to convert to latin1() as URL should not contain
	// anything except ascii.
	msg->setBody( KURL::decode_string(queryPart.mid(6)).latin1() );
      else if (queryPart.left(6) == "?cc=")
	msg->setCc( KURL::decode_string(queryPart.mid(4)) );
    }

    win = new KMComposeWin(msg);
    win->show();
  }
  else if ((aUrl.protocol() == "http") || (aUrl.protocol() == "https") ||
	   (aUrl.protocol() ==  "ftp") || (aUrl.protocol() == "file"))
  {
    statusMsg(i18n("Opening URL..."));
    // -- David : replacement for KFM::openURL
    if ( !KOpenWithHandler::exists() )
      (void) new KFileOpenWithHandler();
    (void) new KRun( aUrl );
  }
}


//-----------------------------------------------------------------------------
void KMMainWin::slotMailtoCompose()
{
  KMComposeWin *win;
  KMMessage *msg = new KMMessage;

  msg->initHeader();
  msg->setTo(mUrlCurrent.path());

  win = new KMComposeWin(msg);
  win->show();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotMailtoReply()
{
  KMComposeWin *win;
  KMMessage *msg;

  if (!(msg = mHeaders->getMsg(-1))) return;
  msg = msg->createReply(FALSE);
  msg->setTo(mUrlCurrent.path());

  win = new KMComposeWin(msg);
  win->show();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotMailtoForward()
{
  KMComposeWin *win;
  KMMessage *msg;

  if (!(msg = mHeaders->getMsg(-1))) return;
  msg = msg->createForward();
  msg->setTo(mUrlCurrent.path());

  win = new KMComposeWin(msg);
  win->show();
}


//-----------------------------------------------------------------------------
void KMMainWin::slotMailtoAddAddrBook()
{
  KMAddrBookExternal::addEmail(mUrlCurrent.path(), this);
}


//-----------------------------------------------------------------------------
void KMMainWin::slotUrlCopy()
{
  QClipboard* clip = QApplication::clipboard();

  if (mUrlCurrent.protocol() == "mailto")
  {
    clip->setText(mUrlCurrent.path());
    statusMsg(i18n("Address copied to clipboard."));
  }
  else
  {
    clip->setText(mUrlCurrent.url());
    statusMsg(i18n("URL copied to clipboard."));
  }
}


//-----------------------------------------------------------------------------
void KMMainWin::slotUrlOpen()
{
  if (mUrlCurrent.isEmpty()) return;
  //  mMsgView->slotUrlOpen(mUrlCurrent, QString::null, 0);
  mMsgView->slotUrlOpen( mUrlCurrent, KParts::URLArgs() );
}


//-----------------------------------------------------------------------------
void KMMainWin::slotMsgPopup(const KURL &aUrl, const QPoint& aPoint)
{
  KPopupMenu* menu = new KPopupMenu;

  mUrlCurrent = aUrl;

  if (!aUrl.isEmpty())
  {
    if (aUrl.protocol() == "mailto")
    {
      // popup on a mailto URL
      menu->insertItem(i18n("Send to..."), this,
		       SLOT(slotMailtoCompose()));
      menu->insertItem(i18n("Send reply to..."), this,
		       SLOT(slotMailtoReply()));
      menu->insertItem(i18n("Forward to..."), this,
		       SLOT(slotMailtoForward()));
      menu->insertSeparator();
      menu->insertItem(i18n("Add to addressbook"), this,
		       SLOT(slotMailtoAddAddrBook()));
      menu->insertItem(i18n("Copy to clipboard"), this,
		       SLOT(slotUrlCopy()));
      menu->popup(aPoint,0);
    }
    else
    {
      // popup on a not-mailto URL
      menu->insertItem(i18n("Open URL..."), this,
		       SLOT(slotUrlOpen()));
      menu->insertItem(i18n("Copy to clipboard"), this,
		       SLOT(slotUrlCopy()));
      menu->popup(aPoint,0);
    }
  }
  else
  {
    // popup somewhere else on the document
    replyAction->plug(menu);
    replyAllAction->plug(menu);
    forwardAction->plug(menu);
    redirectAction->plug(menu);
    menu->insertSeparator();
    menu->insertItem(i18n("&Move..."), this,
                     SLOT(slotMoveMsg()), Key_M);
    menu->insertItem(i18n("&Copy..."), this,
                     SLOT(slotCopyMsg()), Key_C); 
    menu->insertSeparator();
    deleteAction->plug(menu);
    menu->popup(aPoint, 0);
  }
}


//-----------------------------------------------------------------------------
void KMMainWin::getAccountMenu()
{
  QStringList actList;

  actMenu->clear();
  actList = kernel->acctMgr()->getAccounts();
  QStringList::Iterator it;
  int id = 0;
  for(it = actList.begin(); it != actList.end() ; ++it, id++)
    actMenu->insertItem(*it, id);
}


//-----------------------------------------------------------------------------
void KMMainWin::setupMenuBar()
{
  //----- File Menu
  (void) new KAction( i18n("&New Mail Client..."), 0, this, SLOT(slotNewMailReader()),
		      actionCollection(), "new_mail_client" );

  saveAsAction = new KAction( i18n("Save &As..."), "filesave",
    KStdAccel::key(KStdAccel::Save),
    this, SLOT(slotSaveMsg()), actionCollection(), "save_as" );

  KStdAction::print (this, SLOT(slotPrintMsg()), actionCollection());

  (void) new KAction( i18n("Compact all &folders"), 0,
		      kernel->folderMgr(), SLOT(compactAll()),
		      actionCollection(), "compact_all_folders" );

  (void) new KAction( i18n("Check &Mail"), "mail_get", CTRL+Key_L,
		      this, SLOT(slotCheckMail()),
		      actionCollection(), "check_mail" );

  KActionMenu *actActionMenu = new
    KActionMenu( i18n("Check Mail in"), actionCollection(), "check_mail_in" );

  actMenu = actActionMenu->popupMenu();
  connect(actMenu,SIGNAL(activated(int)),this,SLOT(slotCheckOneAccount(int)));

  (void) new KAction( i18n("&Send Queued"), 0, this,
		      SLOT(slotSendQueued()), actionCollection(), "send_queued");

  (void) new KAction( i18n("Address &Book..."), "contents", 0, this,
		      SLOT(slotAddrBook()), actionCollection(), "addressbook" );

  KStdAction::close( this, SLOT(slotClose()), actionCollection());
  //KStdAction::quit( this, SLOT(quit()), actionCollection());

  //----- Edit Menu
  KStdAction::undo( this, SLOT(slotUndo()), actionCollection());

  (void) new KAction( i18n("&Copy text"), KStdAccel::key(KStdAccel::Copy), this,
		      SLOT(slotCopyText()), actionCollection(), "copy_text" );

  deleteAction = new KAction( i18n("&Delete"), "editdelete", Key_D, this,
		      SLOT(slotDeleteMsg()), actionCollection(), "delete" );

  (void) new KAction( i18n("&Search messages..."), "find", Key_S, this,
		      SLOT(slotSearch()), actionCollection(), "search_messages" );

  (void) new KAction( i18n("&Find in message..."), KStdAccel::key(KStdAccel::Find), this,
		      SLOT(slotFind()), actionCollection(), "find_in_messages" );

  (void) new KAction( i18n("Select all messages"), Key_K, this,
		      SLOT(slotMarkAll()), actionCollection(), "mark_all_messages" );

  //----- Folder Menu
  (void) new KAction( i18n("&Create..."), 0, this,
		      SLOT(slotAddFolder()), actionCollection(), "create" );

  modifyFolderAction = new KAction( i18n("&Modify..."), 0, this,
		      SLOT(slotModifyFolder()), actionCollection(), "modify" );

  (void) new KAction( i18n("C&ompact"), 0, this,
		      SLOT(slotCompactFolder()), actionCollection(), "compact" );

  (void) new KAction( i18n("&Empty"), 0, this,
		      SLOT(slotEmptyFolder()), actionCollection(), "empty" );

  removeFolderAction = new KAction( i18n("&Remove..."), 0, this,
		      SLOT(slotRemoveFolder()), actionCollection(), "remove" );

  preferHtmlAction = new KToggleAction( i18n("Prefer HTML to plain text"), 0, this,
		      SLOT(slotOverrideHtml()), actionCollection(), "prefer_html" );

  threadMessagesAction = new KToggleAction( i18n("Thread messages"), 0, this,
		      SLOT(slotOverrideThread()), actionCollection(), "thread_messages" );

  //----- Message Menu
  (void) new KAction( i18n("New Message..."), "filenew", KStdAccel::key(KStdAccel::New), this,
		      SLOT(slotCompose()), actionCollection(), "new_message" );

  (void) new KAction( i18n("&Next"), Key_N, mHeaders,
		      SLOT(nextMessage()), actionCollection(), "next" );

  (void) new KAction( i18n("Next unread"), "next", Key_Plus, mHeaders,
		      SLOT(nextUnreadMessage()), actionCollection(), "next_unread" );

  (void) new KAction( i18n("&Previous"), Key_P, mHeaders,
		      SLOT(prevMessage()), actionCollection(), "previous" );

  (void) new KAction( i18n("Previous unread"), "previous", Key_Minus, mHeaders,
		      SLOT(prevUnreadMessage()), actionCollection(), "previous_unread" );

  replyAction = new KAction( i18n("&Reply..."), "mail_reply", Key_R, this,
		      SLOT(slotReplyToMsg()), actionCollection(), "reply" );

  replyAllAction = new KAction( i18n("Reply &All..."), "mail_replyall",
     Key_A, this, SLOT(slotReplyAllToMsg()), actionCollection(), "reply_all" );

  forwardAction = new KAction( i18n("&Forward..."), "mail_forward", Key_F, this,
		      SLOT(slotForwardMsg()), actionCollection(), "forward" );

  redirectAction = new KAction( i18n("R&edirect..."), Key_E, this,
		      SLOT(slotRedirectMsg()), actionCollection(), "redirect" );

  bounceAction = new KAction( i18n("&Bounce..."), 0, this,
		      SLOT(slotBounceMsg()), actionCollection(), "bounce" );

  (void) new KAction( i18n("Send again..."), 0, this,
		      SLOT(slotResendMsg()), actionCollection(), "send_again" );

  //----- Message-Encoding Submenu
  mEncoding = new KSelectAction( i18n( "Set &Encoding" ), 0, this, SLOT( slotSetEncoding() ), actionCollection(), "encoding" );
  QStringList encodings = KGlobal::charsets()->availableEncodingNames();
  encodings.prepend( i18n( "Auto" ) );
  mEncoding->setItems( encodings );
  mEncoding->setCurrentItem(0);
  if (encodings.findIndex( mEncodingStr ) != -1)
      mEncoding->setCurrentItem(encodings.findIndex( mEncodingStr ));

  (void) new KAction( i18n("Edi&t..."), Key_T, this,
		      SLOT(slotEditMsg()), actionCollection(), "edit" );

  KActionMenu *setStatusAction = new KActionMenu( i18n("Set status" ),
					     actionCollection(), "set_status" );
  QPopupMenu *msgStatusMenu = setStatusAction->popupMenu();
  connect(msgStatusMenu, SIGNAL(activated(int)), this,
	  SLOT(slotSetMsgStatus(int)));
  msgStatusMenu->insertItem(i18n("New"), (int)KMMsgStatusNew);
  msgStatusMenu->insertItem(i18n("Unread"), (int)KMMsgStatusUnread);
  msgStatusMenu->insertItem(i18n("Read"), (int)KMMsgStatusOld);
  msgStatusMenu->insertItem(i18n("Replied"), (int)KMMsgStatusReplied);
  msgStatusMenu->insertItem(i18n("Queued"), (int)KMMsgStatusQueued);
  msgStatusMenu->insertItem(i18n("Sent"), (int)KMMsgStatusSent);

  KActionMenu *moveActionMenu = new KActionMenu( i18n("&Move to" ),
					     actionCollection(), "move_to" );
  moveMenu = moveActionMenu->popupMenu();

  KActionMenu *copyActionMenu = new KActionMenu( i18n("&Copy to" ),
					     actionCollection(), "copy_to" );
  copyMenu = copyActionMenu->popupMenu();

  (void) new KAction( i18n("Apply filters"), CTRL+Key_J, this,
		      SLOT(slotApplyFilters()), actionCollection(), "apply_filters" );

  (void) new KAction( i18n("View Source..."), 0, this,
		      SLOT(slotShowMsgSrc()), actionCollection(), "view_source" );


  //----- View Menu
  KActionMenu *viewMenuAction = new
    KActionMenu( i18n("things to show", "&View"), actionCollection(), "view" );

  mViewMenu = viewMenuAction->popupMenu();
  mViewMenu->setCheckable(TRUE);
  connect(mViewMenu,SIGNAL(activated(int)),SLOT(slotSetHeaderStyle(int)));
  mViewMenu->insertItem(i18n("&Brief Headers"), KMReaderWin::HdrBrief);
  mViewMenu->insertItem(i18n("&Fancy Headers"), KMReaderWin::HdrFancy);
  mViewMenu->insertItem(i18n("&Standard Headers"), KMReaderWin::HdrStandard);
  mViewMenu->insertItem(i18n("&Long Headers"), KMReaderWin::HdrLong);
  mViewMenu->insertItem(i18n("&All Headers"), KMReaderWin::HdrAll);
  mViewMenu->insertSeparator();
  mViewMenu->insertItem(i18n("Iconic Attachments"),
		       KMReaderWin::HdrAll + KMReaderWin::IconicAttmnt);
  mViewMenu->insertItem(i18n("Smart Attachments"),
		       KMReaderWin::HdrAll + KMReaderWin::SmartAttmnt);
  mViewMenu->insertItem(i18n("Inlined Attachments"),
		       KMReaderWin::HdrAll + KMReaderWin::InlineAttmnt);
  mViewMenu->setItemChecked((int)mMsgView->headerStyle(), TRUE);
  mViewMenu->setItemChecked((int)mMsgView->attachmentStyle()+5, TRUE);

  //----- Settings Menu
  toolbarAction = KStdAction::showToolbar(this, SLOT(slotToggleToolBar()),
    actionCollection());
  statusbarAction = KStdAction::showStatusbar(this, SLOT(slotToggleStatusBar()),
    actionCollection());
  KStdAction::keyBindings(this, SLOT(slotEditKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(slotEditToolbars()), actionCollection());
  //  KStdAction::preferences(this, SLOT(slotSettings()), actionCollection());
 (void) new KAction( i18n("Configuration..."), 0, this,
		     SLOT(slotSettings()), actionCollection(), "settings" );


  (void) new KAction( i18n("F&ilter Rules..."), 0, this,
 		      SLOT(slotFilter()), actionCollection(), "filter" );

  createGUI( "kmmainwin.rc", false );

  QObject::connect( guiFactory()->container("file", this),
		    SIGNAL( aboutToShow() ), this, SLOT( getAccountMenu() ));

  QObject::connect( guiFactory()->container("folder", this),
		    SIGNAL( aboutToShow() ), this, SLOT( updateFolderMenu() ));

  QObject::connect( guiFactory()->container("message", this),
		    SIGNAL( aboutToShow() ), this, SLOT( updateMessageMenu() ));

  conserveMemory();
}


//-----------------------------------------------------------------------------

void KMMainWin::slotToggleToolBar()
{
  if(toolBar("mainToolBar")->isVisible())
    toolBar("mainToolBar")->hide();
  else
    toolBar("mainToolBar")->show();
}

void KMMainWin::slotToggleStatusBar()
{
  if (statusBar()->isVisible())
    statusBar()->hide();
  else
    statusBar()->show();
}

void KMMainWin::slotEditToolbars()
{
  KEditToolbar dlg(actionCollection(), "kmmainwin.rc");

  if (dlg.exec() == true)
  {
    createGUI("kmmainwin.rc");
  }
}

void KMMainWin::slotEditKeys()
{
  KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}


//-----------------------------------------------------------------------------
void KMMainWin::setupStatusBar()
{
  mStatusBar = new KStatusBar(this);

  littleProgress = new KMLittleProgressDlg( mStatusBar );

  mStatusBar->addWidget( littleProgress, 0 , true );
  mMessageStatusId = 1;
  mStatusBar->insertItem(i18n(" Initializing..."), 1, 1 );
  mStatusBar->setItemAlignment( 1, AlignLeft | AlignVCenter );
  littleProgress->show();
  connect( KMBroadcastStatus::instance(), SIGNAL(statusProgressEnable( bool )),
	   littleProgress, SLOT(slotEnable( bool )));
  connect( KMBroadcastStatus::instance(),
	   SIGNAL(statusProgressPercent( unsigned long )),
	   littleProgress,
	   SLOT(slotJustPercent( unsigned long )));
  connect( KMBroadcastStatus::instance(), SIGNAL(resetRequested()),
	   littleProgress, SLOT(slotClean()));
  connect( KMBroadcastStatus::instance(), SIGNAL(statusMsg( const QString& )),
	   this, SLOT(statusMsg( const QString& )));
  setStatusBar(mStatusBar);
}

void KMMainWin::quit()
{
  qApp->quit();
}


//-----------------------------------------------------------------------------
void KMMainWin::moveSelectedToFolder( int menuId )
{
  if (mMenuToFolder[menuId])
    mHeaders->moveMsgToFolder( mMenuToFolder[menuId] );
}


//-----------------------------------------------------------------------------
void KMMainWin::copySelectedToFolder(int menuId )
{
  if (mMenuToFolder[menuId])
    mHeaders->copyMsgToFolder( mMenuToFolder[menuId] );
}


//-----------------------------------------------------------------------------
QPopupMenu* KMMainWin::folderToPopupMenu(KMFolderDir* aFolderDir,
					 bool move,
					 QObject *receiver,
					 KMMenuToFolder *aMenuToFolder,
					 QPopupMenu *menu )
{
  KMFolderNode *folderNode;
  KMFolder* folder;

  if (move)
  {
    disconnect(menu, SIGNAL(activated(int)), receiver,
           SLOT(moveSelectedToFolder(int)));
    connect(menu, SIGNAL(activated(int)), receiver,
             SLOT(moveSelectedToFolder(int)));
  } else {
    disconnect(menu, SIGNAL(activated(int)), receiver,
           SLOT(copySelectedToFolder(int)));
    connect(menu, SIGNAL(activated(int)), receiver,
             SLOT(copySelectedToFolder(int)));
  }
  for (folderNode = aFolderDir->first();
       folderNode != NULL;
       folderNode = aFolderDir->next())
    if (!folderNode->isDir()) {
      folder = static_cast<KMFolder*>(folderNode);

      int menuId = menu->insertItem(folder->label());
      aMenuToFolder->insert( menuId, folder );

      KMFolderDir *child = folder->child();
      if (child && child->first()) {
	QPopupMenu *subMenu = folderToPopupMenu( child, move, receiver,
						 aMenuToFolder,
						 new QPopupMenu() );
	// add an item to the top of the submenu somehow subMenu
	menu->insertItem(i18n("%1 child").arg(folder->label()), subMenu);
      }
    }
  return menu;
}


//-----------------------------------------------------------------------------
void KMMainWin::updateMessageMenu()
{
  KMFolderDir *dir = &kernel->folderMgr()->dir();
  mMenuToFolder.clear();
  moveMenu->clear();
  folderToPopupMenu( dir, TRUE, this, &mMenuToFolder, moveMenu );
  copyMenu->clear();
  folderToPopupMenu( dir, FALSE, this, &mMenuToFolder, copyMenu );
}


//-----------------------------------------------------------------------------
void KMMainWin::updateFolderMenu()
{
  modifyFolderAction->setEnabled( mFolder ? !mFolder->isSystemFolder()
    : false );
  removeFolderAction->setEnabled( mFolder ? !mFolder->isSystemFolder()
    : false );
  preferHtmlAction->setEnabled( mFolder ? true : false );
  threadMessagesAction->setEnabled( true );
  threadMessagesAction->setEnabled( mFolder ? true : false );

  preferHtmlAction->setChecked( mHtmlPref ? !mFolderHtmlPref : mFolderHtmlPref );
  threadMessagesAction->setChecked( mThreadPref ? !mFolderThreadPref : mFolderThreadPref );
}

#ifdef MALLOC_DEBUG
QString fmt(long n) {
  char buf[32];

  if(n > 1024*1024*1024)
    sprintf(buf, "%0.2f GB", ((double)n)/1024.0/1024.0/1024.0);
  else if(n > 1024*1024)
    sprintf(buf, "%0.2f MB", ((double)n)/1024.0/1024.0);
  else if(n > 1024)
    sprintf(buf, "%0.2f KB", ((double)n)/1024.0);
  else
    sprintf(buf, "%ld Byte", n);
  return QString(buf);
}
#endif

void KMMainWin::slotMemInfo() {
#ifdef MALLOC_DEBUG
  struct mallinfo mi;

  mi = mallinfo();
  QString s = QString("\nMALLOC - Info\n\n"
		      "Number of mmapped regions : %1\n"
		      "Memory allocated in use   : %2\n"
		      "Memory allocated, not used: %3\n"
		      "Memory total allocated    : %4\n"
		      "Max. freeable memory      : %5\n")
    .arg(mi.hblks).arg(fmt(mi.uordblks)).arg(fmt(mi.fordblks))
    .arg(fmt(mi.arena)).arg(fmt(mi.keepcost));
  KMessageBox::information(0, s, "Malloc information", s);
#endif
}


