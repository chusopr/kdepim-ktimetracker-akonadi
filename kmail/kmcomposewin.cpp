// kmcomposewin.cpp
// Author: Markus Wuebben <markus.wuebben@kde.org>
// This code is published under the GPL.

#include <qdir.h>
#include <qprinter.h>
#include <qcombobox.h>
#include <qdragobject.h>
#include <qlistview.h>

#include "kmcomposewin.h"
#include "kmmessage.h"
#include "kmmsgpart.h"
#include "kmsender.h"
#include "kmidentity.h"
#include "kfileio.h"
#include "kbusyptr.h"
#include "kmmsgpartdlg.h"
#include "kpgp.h"
#include "kmaddrbookdlg.h"
#include "kmaddrbook.h"
#include "kfontutils.h"

#include <kaction.h>
#include <kstdaction.h>
#include <kedittoolbar.h>

#ifndef KRN
#include "kmmainwin.h"
#include "kmsettings.h"
#include "kfileio.h"
#endif

#include <assert.h>
#include <kapp.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kstdaccel.h>
#include <mimelib/mimepp.h>
#include <kfiledialog.h>
#include <kwm.h>
#include <kglobal.h>
#include <kmessagebox.h>

#include <kspell.h>

#include <qtabdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlist.h>
#include <qfont.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qcursor.h>
#include <qmessagebox.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <klocale.h>
#include <ktempfile.h>
#include <fcntl.h>

#if defined CHARSETS
#include <kcharsets.h>
#include "charsetsDlg.h"
#endif

#ifdef KRN
/* start added for KRN */
#include "krnsender.h"
extern KApplication *app;
extern KBusyPtr *kbp;
extern KRNSender *msgSender;
extern KMIdentity *identity;
extern KMAddrBook *addrBook;
typedef QList<QWidget> WindowList;
WindowList* windowList=new WindowList;
#define aboutText "KRN"
/* end added for KRN */
#else
#include "kmglobal.h"
#endif

#include "kmcomposewin.moc"


#define HDR_FROM     0x01
#define HDR_REPLY_TO 0x02
#define HDR_TO       0x04
#define HDR_CC       0x08
#define HDR_BCC      0x10
#define HDR_SUBJECT  0x20
#define HDR_NEWSGROUPS  0x40
#define HDR_FOLLOWUP_TO 0x80
#define HDR_ALL      0xff

#ifdef KRN
#define HDR_STANDARD (HDR_SUBJECT|HDR_NEWSGROUPS)
#else
#define HDR_STANDARD (HDR_SUBJECT|HDR_TO|HDR_CC)
#endif

QString KMComposeWin::mPathAttach = QString::null;

//-----------------------------------------------------------------------------
KMComposeWin::KMComposeWin(KMMessage *aMsg, QString id )
  : KMTopLevelWidget (),
  mMainWidget(this),
  mEdtFrom(this,&mMainWidget), mEdtReplyTo(this,&mMainWidget),
  mEdtTo(this,&mMainWidget),  mEdtCc(this,&mMainWidget),
  mEdtBcc(this,&mMainWidget), mEdtSubject(this,&mMainWidget, "subjectLine"),
  mLblFrom(&mMainWidget), mLblReplyTo(&mMainWidget), mLblTo(&mMainWidget),
  mLblCc(&mMainWidget), mLblBcc(&mMainWidget), mLblSubject(&mMainWidget),
  mBtnTo("...",&mMainWidget), mBtnCc("...",&mMainWidget),
  mBtnBcc("...",&mMainWidget),  mBtnFrom("...",&mMainWidget),
  mBtnReplyTo("...",&mMainWidget),
  mId( id )

#ifdef KRN
  ,mEdtNewsgroups(this,&mMainWidget),mEdtFollowupTo(this,&mMainWidget),
  mLblNewsgroups(&mMainWidget),mLblFollowupTo(&mMainWidget)
#endif
{
  //setWFlags( WType_TopLevel | WStyle_Dialog );

  mDone = false;
  mGrid = NULL;
  mAtmListBox = NULL;
  mAtmList.setAutoDelete(TRUE);
  mAutoDeleteMsg = FALSE;
  mPathAttach = QString::null;
  mEditor = NULL;

  mSpellCheckInProgress=FALSE;

  //setCaption(i18n("KMail Composer"));
  setMinimumSize(200,200);
  
  mBtnTo.setFocusPolicy(QWidget::NoFocus);
  mBtnCc.setFocusPolicy(QWidget::NoFocus);
  mBtnBcc.setFocusPolicy(QWidget::NoFocus);
  mBtnFrom.setFocusPolicy(QWidget::NoFocus);
  mBtnReplyTo.setFocusPolicy(QWidget::NoFocus);

  mAtmListBox = new QListView(&mMainWidget, "mAtmListBox");
  mAtmListBox->setFocusPolicy(QWidget::NoFocus);
  mAtmListBox->addColumn(i18n("Name"), 200);
  mAtmListBox->addColumn(i18n("Size"), 80);
  mAtmListBox->addColumn(i18n("Encoding"), 120);
  mAtmListBox->addColumn(i18n("Type"), 150);
  connect(mAtmListBox,
	  SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
	  SLOT(slotAttachPopupMenu(QListViewItem *, const QPoint &, int)));

  readConfig();
  setupStatusBar();
  setupEditor();
  setupActions();
  if(mShowToolBar)
    toolBar()->show();
  else
    toolBar()->hide();
  
  connect(&mEdtSubject,SIGNAL(textChanged(const QString&)),
	  SLOT(slotUpdWinTitle(const QString&)));
  connect(&mBtnTo,SIGNAL(clicked()),SLOT(slotAddrBookTo()));
  connect(&mBtnCc,SIGNAL(clicked()),SLOT(slotAddrBookCc()));
  connect(&mBtnBcc,SIGNAL(clicked()),SLOT(slotAddrBookBcc()));
  connect(&mBtnReplyTo,SIGNAL(clicked()),SLOT(slotAddrBookReplyTo()));
  connect(&mBtnFrom,SIGNAL(clicked()),SLOT(slotAddrBookFrom()));

  mMainWidget.resize(480,510);
  setView(&mMainWidget, FALSE);
  rethinkFields();

#ifndef KRN
  if (useExtEditor) {
    mEditor->setExternalEditor(true);
    mEditor->setExternalEditorPath(mExtEditor);
  }
#endif

#if defined CHARSETS
  // As family may change with charset, we must save original settings
  mSavedEditorFont=mEditor->font();
#endif

  mMsg = NULL;
  if (aMsg)
    setMsg(aMsg);

  mEdtTo.setFocus();
  mDone = true;
}


//-----------------------------------------------------------------------------
KMComposeWin::~KMComposeWin()
{
  writeConfig();
  if (mAutoDeleteMsg && mMsg) delete mMsg;
}


//-----------------------------------------------------------------------------
void KMComposeWin::readConfig(void)
{
  KConfig *config = kapp->config();
  QString str;
  int w, h;

  config->setGroup("Composer");
  mAutoSign = config->readEntry("signature","manual") == "auto";
  mShowToolBar = config->readNumEntry("show-toolbar", 1);
  mDefEncoding = config->readEntry("encoding", "base64");
  mShowHeaders = config->readNumEntry("headers", HDR_STANDARD);
  mWordWrap = config->readNumEntry("word-wrap", 1);
  mLineBreak = config->readNumEntry("break-at", 78);
  if ((mLineBreak == 0) || (mLineBreak > 78))
    mLineBreak = 78;
  if (mLineBreak < 60)
    mLineBreak = 60;
  mAutoPgpSign = config->readNumEntry("pgp-auto-sign", 0);
#ifndef KRN
  mConfirmSend = config->readBoolEntry("confirm-before-send", false);
#endif

  config->setGroup("Reader");
  QColor c1=QColor(kapp->palette().normal().text());
  QColor c4=QColor(kapp->palette().normal().base());

  if (!config->readBoolEntry("defaultColors",TRUE)) {
    foreColor = config->readColorEntry("ForegroundColor",&c1);
    backColor = config->readColorEntry("BackgroundColor",&c4);
  }
  else {
    foreColor = c1;
    backColor = c4;
  }

  // Color setup
  mPalette = palette().copy();
  QColorGroup cgrp  = mPalette.normal();
  QColorGroup ncgrp(foreColor,cgrp.background(),
		    cgrp.light(),cgrp.dark(), cgrp.mid(), foreColor,
		    backColor);
  mPalette.setNormal(ncgrp);
  mPalette.setDisabled(ncgrp);
  mPalette.setActive(ncgrp);
  mPalette.setInactive(ncgrp);

  mEdtFrom.setPalette(mPalette);
  mEdtReplyTo.setPalette(mPalette);
  mEdtTo.setPalette(mPalette);
  mEdtCc.setPalette(mPalette);
  mEdtBcc.setPalette(mPalette);
  mEdtSubject.setPalette(mPalette);

#ifndef KRN
  config->setGroup("General");
  mExtEditor = config->readEntry("external-editor", DEFAULT_EDITOR_STR);
  useExtEditor = config->readBoolEntry("use-external-editor", FALSE);

  int headerCount = config->readNumEntry("mime-header-count", 0);
  mCustHeaders.clear();
  mCustHeaders.setAutoDelete(true);
  for (int i = 0; i < headerCount; i++) {
    QString thisGroup;
    _StringPair *thisItem = new _StringPair;
    thisGroup.sprintf("Mime #%d", i);
    config->setGroup(thisGroup);
    thisItem->name = config->readEntry("name", "");
    if ((thisItem->name).length() > 0) {
      thisItem->value = config->readEntry("value", "");
      mCustHeaders.append(thisItem);
    } else {
      delete thisItem;
    }
  }
#endif

  config->setGroup("Fonts");
  if (!config->readBoolEntry("defaultFonts",TRUE)) {
    QString mBodyFontStr;
    mBodyFontStr = config->readEntry("body-font", "helvetica-medium-r-12");
    mBodyFont = kstrToFont(mBodyFontStr);
  }
  else
    mBodyFont = KGlobal::generalFont();
  if (mEditor) mEditor->setFont(mBodyFont);

#if defined CHARSETS
  m7BitAscii = config->readNumEntry("7bit-is-ascii",1);
  mQuoteUnknownCharacters = config->readNumEntry("quote-unknown",0);

  str = config->readEntry("default-charset", "");
  if (str.isNull() || str=="default" || !KGlobal::charsets()->isAvailable(str))
      mDefaultCharset="default";
  else
      mDefaultCharset=str;

  debug("Default charset: %s", (const char*)mDefaultCharset);

  str = config->readEntry("composer-charset", "");
  if (str.isNull() || str=="default" || !KGlobal::charsets()->isAvailable(str))
      mDefComposeCharset="default";
  else
      mDefComposeCharset=str;

  debug("Default composer charset: %s", (const char*)mDefComposeCharset);
#endif

  config->setGroup("Geometry");
  str = config->readEntry("composer", "480 510");
  sscanf(str.data(), "%d %d", &w, &h);
  if (w<200) w=200;
  if (h<200) h=200;
  resize(w, h);
}	


//-----------------------------------------------------------------------------
void KMComposeWin::writeConfig(void)
{
  KConfig *config = kapp->config();
  QString str;

  config->setGroup("Composer");
  config->writeEntry("signature", mAutoSign?"auto":"manual");
  config->writeEntry("show-toolbar", mShowToolBar);
  config->writeEntry("encoding", mDefEncoding);
  config->writeEntry("headers", mShowHeaders);
#if defined CHARSETS
  config->writeEntry("7bit-is-ascii",m7BitAscii);
  config->writeEntry("quote-unknown",mQuoteUnknownCharacters);
  config->writeEntry("default-charset",mDefaultCharset);
  config->writeEntry("composer-charset",mDefComposeCharset);
#endif

  config->setGroup("Geometry");
  str.sprintf("%d %d", width(), height());
  config->writeEntry("composer", str);
}


//-----------------------------------------------------------------------------
void KMComposeWin::deadLetter(void)
{
  if (!mMsg) return;

  // This method is called when KMail crashed, so we better use as
  // basic functions as possible here.
  applyChanges();
  QString msgStr = mMsg->asString();
  QString fname = getenv("HOME");
  fname += "/dead.letter";
  // Security: the file is created in the user's home directory, which
  // might be readable by other users. So the file only gets read/write
  // permissions for the user himself. Note that we create the file with
  // correct permissions, we do not set them after creating the file!
  // (dnaber, 2000-02-27):
  int fd = open(fname, O_CREAT|O_APPEND|O_WRONLY, S_IWRITE|S_IREAD);
  if (fd != -1)
  {
    const char* startStr = "From ???@??? Mon Jan 01 00:00:00 1997\n";
    write(fd, startStr, strlen(startStr));
    write(fd, msgStr.latin1(), msgStr.length()); // TODO?: not unicode aware :-(
    write(fd, "\n", 1);
    close(fd);
    fprintf(stderr,"appending message to ~/dead.letter\n");
  }
  else perror("cannot open ~/dead.letter for saving the current message");
}



//-----------------------------------------------------------------------------
void KMComposeWin::slotView(void)
{
  if (!mDone)
    return; // otherwise called from rethinkFields during the construction
            // which is not the intended behavior
  int id;

  //This sucks awfully, but no, I cannot get an activated(int id) from
  // actionContainer()
  if (!sender()->isA("KToggleAction"))
    return;
  KToggleAction *act = (KToggleAction *) sender();
  
  if (act == allFieldsAction)
    id = 0;
  else if (act == fromAction)
    id = HDR_FROM;
  else if (act == replyToAction)
    id = HDR_REPLY_TO;
  else if (act == toAction)
    id = HDR_TO;
  else if (act == ccAction)
    id = HDR_CC;
  else  if (act == bccAction)
    id = HDR_BCC;
  else if (act == subjectAction)
    id = HDR_SUBJECT;
#ifdef KRN
   else if (act == newsgroupsAction)
     id = HDR_NEWSGROUPS;
   else if (act == followupToAction)
     id = HDR_FOLLOWUP_TO;
#endif
   else
   {
     id = 0;
     debug("Something is wrong (Oh, yeah?)");
     return;
   }

  // sanders There's a bug here this logic doesn't work if no
  // fields are shown and then show all fields is selected.
  // Instead of all fields being shown none are.
  if (!act->isChecked())
  {
    // hide header
    if (id > 0) mShowHeaders = mShowHeaders & ~id;
    else mShowHeaders = abs(mShowHeaders);
  }
  else
  {
    // show header
    if (id > 0) mShowHeaders |= id;
    else mShowHeaders = -abs(mShowHeaders);
  }
  rethinkFields(true);

}

void KMComposeWin::rethinkFields(bool fromSlot)
{
  //This sucks even more but again no ids. sorry (sven)
  int mask, row, numRows;
  long showHeaders;

  if (mShowHeaders < 0)
    showHeaders = HDR_ALL;
  else
    showHeaders = mShowHeaders;
  
  for (mask=1,mNumHeaders=0; mask<=showHeaders; mask<<=1)
    if ((showHeaders&mask) != 0) mNumHeaders++;

  numRows = mNumHeaders + 2;

  if (mGrid) delete mGrid;
  mGrid = new QGridLayout(&mMainWidget, numRows, 3, 4, 4);
  mGrid->setColStretch(0, 1);
  mGrid->setColStretch(1, 100);
  mGrid->setColStretch(2, 1);
  mGrid->setRowStretch(mNumHeaders, 100);

  mEdtList.clear();
  row = 0;
  if (!fromSlot) fromAction->setChecked(showHeaders&HDR_FROM);
  rethinkHeaderLine(showHeaders,HDR_FROM, row, i18n("&From:"),
		    &mLblFrom, &mEdtFrom, &mBtnFrom);
  if (!fromSlot) replyToAction->setChecked(showHeaders&HDR_REPLY_TO);
  rethinkHeaderLine(showHeaders,HDR_REPLY_TO,row,i18n("&Reply to:"),
		    &mLblReplyTo, &mEdtReplyTo, &mBtnReplyTo);
  if (!fromSlot) toAction->setChecked(showHeaders&HDR_TO);
  rethinkHeaderLine(showHeaders,HDR_TO, row, i18n("&To:"),
		    &mLblTo, &mEdtTo, &mBtnTo);
  if (!fromSlot) ccAction->setChecked(showHeaders&HDR_CC);
  rethinkHeaderLine(showHeaders,HDR_CC, row, i18n("&Cc:"),
		    &mLblCc, &mEdtCc, &mBtnCc);
  if (!fromSlot) bccAction->setChecked(showHeaders&HDR_BCC);
  rethinkHeaderLine(showHeaders,HDR_BCC, row, i18n("&Bcc:"),
		    &mLblBcc, &mEdtBcc, &mBtnBcc);
  if (!fromSlot) subjectAction->setChecked(showHeaders&HDR_SUBJECT);
  rethinkHeaderLine(showHeaders,HDR_SUBJECT, row, i18n("&Subject:"),
		    &mLblSubject, &mEdtSubject);
#ifdef KRN
  if (!fromSlot) newsgroupsAction->setChecked(showHeaders&HDR_NEWSGROUPS);
  rethinkHeaderLine(showHeaders,HDR_NEWSGROUPS, row, i18n("&Newsgroups:"),
		    &mLblNewsgroups, &mEdtNewsgroups);
  if (!fromSlot) followupToAction->setChecked(showHeaders&HDR_FOLLOWUP_TO);
  rethinkHeaderLine(showHeaders,HDR_FOLLOWUP_TO, row, i18n("&Followup-To:"),
		    &mLblFollowupTo, &mEdtFollowupTo);
#endif
  assert(row<=mNumHeaders);

  mGrid->addMultiCellWidget(mEditor, row, mNumHeaders, 0, 2);
  mGrid->addMultiCellWidget(mAtmListBox, mNumHeaders+1, mNumHeaders+1, 0, 2);

  if (mAtmList.count() > 0) mAtmListBox->show();
  else mAtmListBox->hide();
  resize(this->size());
  repaint();

  mGrid->activate();

  fromAction->setEnabled(!allFieldsAction->isChecked());
  replyToAction->setEnabled(!allFieldsAction->isChecked());
  toAction->setEnabled(!allFieldsAction->isChecked());
  ccAction->setEnabled(!allFieldsAction->isChecked());
  bccAction->setEnabled(!allFieldsAction->isChecked());
  subjectAction->setEnabled(!allFieldsAction->isChecked());
#ifdef KRN
  newsgroupsAction->setEnabled(!allFieldsAction->isChecked());
  followupToAction->setEnabled(!allFieldsAction->isChecked());
#endif
}


//-----------------------------------------------------------------------------
void KMComposeWin::rethinkHeaderLine(int aValue, int aMask, int& aRow,
				     const QString aLabelStr, QLabel* aLbl,
				     QLineEdit* aEdt, QPushButton* aBtn)
{
  if (aValue & aMask)
  {
    //mMnuView->setItemChecked(aMask, TRUE);
    aLbl->setText(aLabelStr);
    aLbl->adjustSize();
    aLbl->resize((int)aLbl->sizeHint().width(),aLbl->sizeHint().height() + 6);
    aLbl->setMinimumSize(aLbl->size());
    aLbl->show();
    aLbl->setBuddy(aEdt);
    mGrid->addWidget(aLbl, aRow, 0);

    aEdt->setBackgroundColor( backColor );
    aEdt->show();
    aEdt->setMinimumSize(100, aLbl->height()+2);
    aEdt->setMaximumSize(1000, aLbl->height()+2);
    //aEdt->setFocusPolicy(QWidget::ClickFocus);
    //aEdt->setFocusPolicy(QWidget::StrongFocus);
    mEdtList.append(aEdt);

    if (aBtn)
    {
      mGrid->addWidget(aEdt, aRow, 1);
      mGrid->addWidget(aBtn, aRow, 2);
      aBtn->setFixedSize(aBtn->sizeHint().width(), aLbl->height());
      aBtn->show();
    }
    else mGrid->addMultiCellWidget(aEdt, aRow, aRow, 1, 2);
    aRow++;
  }
  else
  {
    aLbl->hide();
    aEdt->hide();
    // aEdt->setFocusPolicy(QWidget::NoFocus);
    if (aBtn) aBtn->hide();
  }
}

//-----------------------------------------------------------------------------
void KMComposeWin::setupActions(void)
{
  if (kernel->msgSender()->sendImmediate()) //default == send now?
  {
    //default = send now, alternative = queue
    (void) new KAction (i18n("&Send"), QIconSet(BarIcon("send")), CTRL+Key_Return,
                        this, SLOT(slotSendNow()), actionCollection(),
                        "send_default");
    (void) new KAction (i18n("&Queue"), QIconSet(BarIcon("filemail")), 0,
                        this, SLOT(slotSendLater()),
                        actionCollection(), "send_alternative");
  }
  else //no, default = send later
  {
    //default = queue, alternative = send now
    (void) new KAction (i18n("&Queue"), QIconSet(BarIcon("filemail")),
                        CTRL+Key_Return,
                        this, SLOT(slotSendLater()), actionCollection(),
                        "send_default");
    (void) new KAction (i18n("S&end now"), QIconSet(BarIcon("send")), 0,
                        this, SLOT(slotSendNow()),
                        actionCollection(), "send_alternative");
  }

  (void) new KAction (i18n("&Insert File..."), QIconSet(BarIcon("fileopen")), 0,
                      this,  SLOT(slotInsertFile()),
                      actionCollection(), "insert_file");
  (void) new KAction (i18n("&Addressbook..."), QIconSet(BarIcon("contents")),0,
                      this, SLOT(slotAddrBook()),
                      actionCollection(), "addresbook");
  (void) new KAction (i18n("&New Composer..."), QIconSet(BarIcon("filenew")),
                      KStdAccel::key(KStdAccel::New),
                      this, SLOT(slotNewComposer()),
                      actionCollection(), "new_composer");
#ifndef KRN
  (void) new KAction (i18n("Open Mailreader"), /*QIconSet(BarIcon("kmail")),*/
                      0, this, SLOT(slotNewMailReader()),
                      actionCollection(), "open_mailreader");
#endif
  //KStdAction::save(this, SLOT(), actionCollection(), "save_message");
  KStdAction::print (this, SLOT(slotPrint()), actionCollection());
  KStdAction::close (this, SLOT(slotClose()), actionCollection());

  KStdAction::undo (mEditor, SLOT(undo()), actionCollection());
  KStdAction::redo (mEditor, SLOT(redo()), actionCollection());
  KStdAction::cut (this, SLOT(slotCut()), actionCollection());
  KStdAction::copy (this, SLOT(slotCopy()), actionCollection());
  KStdAction::paste (this, SLOT(slotPaste()), actionCollection());
  KStdAction::selectAll (this, SLOT(slotMarkAll()), actionCollection());
  
  KStdAction::find (this, SLOT(slotFind()), actionCollection());
  KStdAction::replace (this, SLOT(slotReplace()), actionCollection());
  KStdAction::spelling (this, SLOT(slotSpellcheck()), actionCollection(), "spellcheck");

  (void) new KAction (i18n("Cl&ean Spaces"), 0, this, SLOT(slotCleanSpace()),
                      actionCollection(), "clean_spaces");

  //these are checkable!!!
  urgentAction = new KToggleAction (i18n("&Urgent"), 0,
                                    actionCollection(),
                                    "urgent");
  confirmDeliveryAction =  new KToggleAction (i18n("&Confirm delivery"), 0,
                                              actionCollection(),
                                              "confirm_delivery");
  confirmReadAction = new KToggleAction (i18n("&Confirm read"), 0,
                                         actionCollection(), "confirm_read");
  (void) new KAction (i18n("&Spellchecker..."), 0, this, SLOT(slotSpellcheckConfig()),
                      actionCollection(), "setup_spellchecker");
#if defined CHARSETS
  (void) new KAction (i18n("&Charsets..."), 0, this, SLOT(slotConfigureCharsets()),
                      actionCollection(), "charsets");
#endif

  KStdAction::configureToolbars(this, SLOT(slotEditToolbars()), actionCollection());
  //these are checkable!!!
  allFieldsAction = new KToggleAction (i18n("&All Fields"), 0, this,
                                       SLOT(slotView()),
                                       actionCollection(), "show_all_fields");
  fromAction = new KToggleAction (i18n("&From"), 0, this,
                                  SLOT(slotView()),
                                  actionCollection(), "show_from");
  replyToAction = new KToggleAction (i18n("&Reply to"), 0, this,
                                     SLOT(slotView()),
                                     actionCollection(), "show_reply_to");
  toAction = new KToggleAction (i18n("&To"), 0, this,
                                SLOT(slotView()),
                                actionCollection(), "show_to");
  ccAction = new KToggleAction (i18n("&Cc"), 0, this,
                                SLOT(slotView()),
                                actionCollection(), "show_cc");
  bccAction = new KToggleAction (i18n("&Bcc"), 0, this,
                                 SLOT(slotView()),
                                 actionCollection(), "show_bcc");
  subjectAction = new KToggleAction (i18n("&Subject"), 0, this,
                                     SLOT(slotView()),
                                     actionCollection(), "show_subject");
#ifdef KRN
  (void) new KToggleAction (i18n("&Newsgroups"), 0, this,
                            SLOT(slotView()),
                            actionCollection(), "show_newsgroups");
  (void) new KToggleAction (i18n("&Followup-To"), 0, this,
                            SLOT(slotView()),
                            actionCollection(), "show_followup_to");

#endif
  //end of checkable

  (void) new KAction (i18n("Append S&ignature"), 0, this,
                      SLOT(slotAppendSignature()),
                      actionCollection(), "append_signature");
  (void) new KAction (i18n("&Attach..."), QIconSet(UserIcon("attach")),
                      0, this, SLOT(slotAttachFile()),
                      actionCollection(), "attach");
  (void) new KAction (i18n("Attach &Public Key"), 0, this,
                      SLOT(slotInsertPublicKey()),
                      actionCollection(), "attach_public_key");
  KAction *attachMPK = new KAction (i18n("Attach My &Public Key"), 0, this,
                                    SLOT(slotInsertMyPublicKey()),
                                    actionCollection(), "attach_my_public_key");
  KAction *attachPK = new KAction (i18n("&Remove"), 0, this,
                                   SLOT(slotAttachRemove()),
                                   actionCollection(), "remove");
  (void) new KAction (i18n("&Save..."), QIconSet(BarIcon("filesave")),0,
                      this, SLOT(slotAttachSave()),
                      actionCollection(), "attach_save");
  (void) new KAction (i18n("Pr&operties..."), 0, this,
                      SLOT(slotAttachProperties()),
                      actionCollection(), "attach_properties");

  

  
  encryptAction = new KToggleAction (i18n("Encrypt message"),
                                     QIconSet(UserIcon("pub_key_red")), 0,
                                     actionCollection(), "encrypt_message");
  signAction = new KToggleAction (i18n("Sign message"),
                                  QIconSet(UserIcon("feather_white")), 0,
                                  actionCollection(), "sign_message");

  if(!Kpgp::getKpgp()->havePGP())
  {
    attachPK->setEnabled(false);
    attachMPK->setEnabled(false);
    encryptAction->setEnabled(false);
    signAction->setEnabled(false);
  }

  signAction->setChecked(mAutoPgpSign);
  

  createGUI("kmcomposerui.rc");
}

//-----------------------------------------------------------------------------
void KMComposeWin::setupStatusBar(void)
{
  statusBar()->addWidget( new QLabel( statusBar(), "" ), 1 );
  statusBar()->insertItem(QString(i18n(" Column"))+":     ",2,0,true);
  statusBar()->insertItem(QString(i18n(" Line"))+":     ",1,0,true);
  setStatusBar(statusBar());
}


//-----------------------------------------------------------------------------
void KMComposeWin::updateCursorPosition()
{
  int col,line;
  QString temp;
  line = mEditor->currentLine();
  col = mEditor->currentColumn();
  temp = QString(" %1: %2 ").arg(i18n("Line")).arg(line+1);
  statusBar()->changeItem(temp,1);
  temp = QString(" %1: %2 ").arg(i18n("Column")).arg(col+1);
  statusBar()->changeItem(temp,2);
}


//-----------------------------------------------------------------------------
void KMComposeWin::setupEditor(void)
{
  QPopupMenu* menu;
  mEditor = new KMEdit(&mMainWidget, this);
  mEditor->setModified(FALSE);
  //mEditor->setFocusPolicy(QWidget::ClickFocus);

  if (mWordWrap)
  {
    mEditor->setWordWrap( QMultiLineEdit::FixedColumnWidth );
    mEditor->setWrapColumnOrWidth(mLineBreak);
  }
  else
  {
    mEditor->setWordWrap( QMultiLineEdit::NoWrap );
  }

  // Font setup
  mEditor->setFont(mBodyFont);

  // Color setup
  mEditor->setPalette(mPalette);
  mEditor->setBackgroundColor(backColor);

  menu = new QPopupMenu();
  //#ifdef BROKEN
  menu->insertItem(i18n("Undo"),mEditor,
		   SLOT(undo()), KStdAccel::key(KStdAccel::Undo));
  menu->insertItem(i18n("Redo"),mEditor,
		   SLOT(redo()), KStdAccel::key(KStdAccel::Redo));
  menu->insertSeparator();
  //#endif //BROKEN
  menu->insertItem(i18n("Cut"), this, SLOT(slotCut()));
  menu->insertItem(i18n("Copy"), this, SLOT(slotCopy()));
  menu->insertItem(i18n("Paste"), this, SLOT(slotPaste()));
  menu->insertItem(i18n("Mark all"),this, SLOT(slotMarkAll()));
  menu->insertSeparator();
  menu->insertItem(i18n("Find..."), this, SLOT(slotFind()));
  menu->insertItem(i18n("Replace..."), this, SLOT(slotReplace()));
  mEditor->installRBPopup(menu);
  updateCursorPosition();
  connect(mEditor,SIGNAL(CursorPositionChanged()),SLOT(updateCursorPosition()));
  connect(mEditor,SIGNAL(gotUrlDrop(QDropEvent *)),SLOT(slotDropAction(QDropEvent *)));
}

//-----------------------------------------------------------------------------
void KMComposeWin::verifyWordWrapLengthIsAdequate(const QString &body)
{
  int maxLineLength = 0;
  int curPos;
  int oldPos = 0;
  if (mEditor->QMultiLineEdit::wordWrap() == QMultiLineEdit::FixedColumnWidth) {
    for (curPos = 0; curPos < (int)body.length(); ++curPos)
	if (body[curPos] == '\n') {
	  if ((curPos - oldPos) > maxLineLength)
	    maxLineLength = curPos - oldPos;
	  oldPos = curPos;
	}
    if ((curPos - oldPos) > maxLineLength)
      maxLineLength = curPos - oldPos;
    if (mEditor->wrapColumnOrWidth() < maxLineLength) // column
      mEditor->setWrapColumnOrWidth(maxLineLength);
  }
}

//-----------------------------------------------------------------------------
void KMComposeWin::setMsg(KMMessage* newMsg, bool mayAutoSign)
{
  KMMessagePart bodyPart, *msgPart;
  int i, num;

  //assert(newMsg!=NULL);
  if(!newMsg)
    {
      debug("KMComposeWin::setMsg() : newMsg == NULL!\n");
      return;
    }
  mMsg = newMsg;

  mEdtTo.setText(mMsg->to());
  mEdtFrom.setText(mMsg->from());
  mEdtCc.setText(mMsg->cc());
  mEdtSubject.setText(mMsg->subject());
  mEdtReplyTo.setText(mMsg->replyTo());
  mEdtBcc.setText(mMsg->bcc());
#ifdef KRN
  mEdtNewsgroups.setText(mMsg->groups());
  mEdtFollowupTo.setText(mMsg->followup());
#endif

  num = mMsg->numBodyParts();
  if (num > 0)
  {
    QString bodyDecoded;
    mMsg->bodyPart(0, &bodyPart);

#if defined CHARSETS
    mCharset=bodyPart.charset();
    cout<<"Charset: "<<mCharset<<"\n";
    if (mCharset==""){
      mComposeCharset=mDefComposeCharset;
      if (mComposeCharset=="default"
                        && KGlobal::charsets()->isAvailable(mDefaultCharset))
        mComposeCharset=mDefaultCharset;
    }	
    else if (KGlobal::charsets()->isAvailable(mCharset))
      mComposeCharset=mCharset;
    else mComposeCharset=mDefComposeCharset;
    cout<<"Compose charset: "<<mComposeCharset<<"\n";

    // FIXME: We might need a QTextCodec here
#endif
    bodyDecoded = QString(bodyPart.bodyDecoded());
    verifyWordWrapLengthIsAdequate(bodyDecoded);
    mEditor->setText(bodyDecoded);
    mEditor->insertLine("\n", -1);

    for(i=1; i<num; i++)
    {
      msgPart = new KMMessagePart;
      mMsg->bodyPart(i, msgPart);
      addAttach(msgPart);
    }
  }
#if defined CHARSETS
  else{
    mCharset=mMsg->charset();
    cout<<"mCharset: "<<mCharset<<"\n";
    if (mCharset==""){
      mComposeCharset=mDefComposeCharset;
      if (mComposeCharset=="default"
                        && KGlobal::charsets()->isAvailable(mDefaultCharset))
        mComposeCharset=mDefaultCharset;
    }	
    else if (KGlobal::charsets()->isAvailable(mCharset))
      mComposeCharset=mCharset;
    else mComposeCharset=mDefComposeCharset;
    cout<<"Compose charset: "<<mComposeCharset<<"\n";
    // FIXME: QTextCodec needed?
    Editor->setText(QString(mMsg->bodyDecoded()));
  }
#else
  else {
    QString bodyDecoded = QString(mMsg->bodyDecoded());
    verifyWordWrapLengthIsAdequate(bodyDecoded);
    mEditor->setText(bodyDecoded);
  }
#endif

  if (mAutoSign && mayAutoSign) slotAppendSignature();
  mEditor->setModified(FALSE);

#if defined CHARSETS
  setEditCharset();
#endif
}


//-----------------------------------------------------------------------------
bool KMComposeWin::applyChanges(void)
{
  QString str, atmntStr;
  QString temp, replyAddr;
  KMMessagePart bodyPart, *msgPart;

  //assert(mMsg!=NULL);
  if(!mMsg)
  {
    debug("KMComposeWin::applyChanges() : mMsg == NULL!\n");
    return FALSE;
  }

  mMsg->setTo(to());
  mMsg->setFrom(from());
  mMsg->setCc(cc());
  mMsg->setSubject(subject());
  mMsg->setReplyTo(replyTo());
  mMsg->setBcc(bcc());
#ifdef KRN
  mMsg->setFollowup(followupTo());
  mMsg->setGroups(newsgroups());
#endif

  if (!replyTo().isEmpty()) replyAddr = replyTo();
  else replyAddr = from();

  if (confirmDeliveryAction->isChecked())
    mMsg->setHeaderField("Return-Receipt-To", replyAddr);

  if (confirmReadAction->isChecked())
    mMsg->setHeaderField("X-Chameleon-Return-To", replyAddr);

  if (urgentAction->isChecked())
  {
    mMsg->setHeaderField("X-PRIORITY", "2 (High)");
    mMsg->setHeaderField("Priority", "urgent");
  }

#ifndef KRN
  _StringPair *pCH;
  for (pCH  = mCustHeaders.first();
       pCH != NULL;
       pCH  = mCustHeaders.next()) {
    mMsg->setHeaderField(pCH->name, pCH->value);
  }

#endif

  if(mAtmList.count() <= 0)
  {
    mMsg->setAutomaticFields(FALSE);

    // If there are no attachments in the list waiting it is a simple
    // text message.
    if (kernel->msgSender()->sendQuotedPrintable())
    {
      mMsg->setTypeStr("text");
      mMsg->setSubtypeStr("plain");
      mMsg->setCteStr("quoted-printable");
      str = pgpProcessedMsg();
      if (str.isNull()) return FALSE;
#if defined CHARSETS
      convertToSend(str);
      cout<<"Setting charset to: "<<mCharset<<"\n";
      mMsg->setCharset(mCharset);
#endif
      mMsg->setBodyEncoded(str);
    }
    else
    {
      mMsg->setTypeStr("text");
      mMsg->setSubtypeStr("plain");
      mMsg->setCteStr("8bit");
      str = pgpProcessedMsg();
      if (str.isNull()) return FALSE;
#if defined CHARSETS
      convertToSend(str);
      cout<<"Setting charset to: "<<mCharset<<"\n";
      mMsg->setCharset(mCharset);
#endif
      mMsg->setBody(str);
    }
  }
  else
  {
    mMsg->deleteBodyParts();
    mMsg->setAutomaticFields(TRUE);

    // create informative header for those that have no mime-capable
    // email client
    mMsg->setBody("This message is in MIME format.\n\n");

    // create bodyPart for editor text.
    if (kernel->msgSender()->sendQuotedPrintable())
         bodyPart.setCteStr("quoted-printable");
    else bodyPart.setCteStr("8bit");
    bodyPart.setTypeStr("text");
    bodyPart.setSubtypeStr("plain");
    str = pgpProcessedMsg();
    if (str.isNull()) return FALSE;
#if defined CHARSETS
    convertToSend(str);
    cout<<"Setting charset to: "<<mCharset<<"\n";
    mMsg->setCharset(mCharset);
#endif
    str.truncate(str.length()); // to ensure str.size()==str.length()+1
    bodyPart.setBodyEncoded(QCString(str.ascii()));
    mMsg->addBodyPart(&bodyPart);

    // Since there is at least one more attachment create another bodypart
    for (msgPart=mAtmList.first(); msgPart; msgPart=mAtmList.next())
	mMsg->addBodyPart(msgPart);
  }
  if (!mAutoDeleteMsg) mEditor->setModified(FALSE);

  // remove fields that contain no data (e.g. an empty Cc: or Bcc:)
  mMsg->cleanupHeader();
  return TRUE;
}


//-----------------------------------------------------------------------------
bool KMComposeWin::queryClose ()
{
  int rc;

  if(mEditor->isModified())
  {
    rc = KMessageBox::warningContinueCancel(this,
           i18n("Close and discard\nedited message?"),
           i18n("Close message"), i18n("&Discard"));
    if (rc == KMessageBox::Cancel)
      return false;
  }
  return true;
}

bool KMComposeWin::queryExit ()
{
  return true;
}

//-----------------------------------------------------------------------------
const QString KMComposeWin::pgpProcessedMsg(void)
{
  Kpgp *pgp = Kpgp::getKpgp();
  bool doSign = signAction->isChecked();
  bool doEncrypt = encryptAction->isChecked();
  QString _to, receiver;
  int index, lastindex;
  QStrList persons;

  if (!doSign && !doEncrypt) return mEditor->brokenText();

  pgp->setMessage(mEditor->brokenText());

  if (!doEncrypt)
  {
    if(pgp->sign()) return pgp->message();
  }
  else
  {
    // encrypting
    _to = to().copy();
    if(!cc().isEmpty()) _to += "," + cc();
    if(!bcc().isEmpty()) _to += "," + bcc();
    lastindex = -1;
    do
    {
      index = _to.find(",",lastindex+1);
      receiver = _to.mid(lastindex+1, index<0 ? 255 : index-lastindex-1);
      if (!receiver.isEmpty())
      {
	persons.append(receiver);
      }
      lastindex = index;
    }
    while (lastindex > 0);

    if(pgp->encryptFor(persons, doSign))
      return pgp->message();
  }

  // in case of an error we end up here
  //warning(i18n("Error during PGP:") + QString("\n") +
  //	  pgp->lastErrorMsg());

  return QString::null;
}


//-----------------------------------------------------------------------------
void KMComposeWin::addAttach(const QString aUrl)
{
  QString name;
  QByteArray str;
  KMMessagePart* msgPart;
  KMMsgPartDlg dlg;
  int i;

  // load the file
  kernel->kbp()->busy();
  str = kFileToBytes(aUrl,FALSE);
  if (str.isNull())
  {
    kernel->kbp()->idle();
    return;
  }

  // create message part
  i = aUrl.findRev('/');
  name = (i>=0 ? aUrl.mid(i+1, 256) : aUrl);
  msgPart = new KMMessagePart;
  msgPart->setName(name);
  msgPart->setCteStr(mDefEncoding);
  msgPart->setBodyEncoded(str);
  msgPart->magicSetType();
  msgPart->setContentDisposition("attachment; filename=\""+name+"\"");

  // show properties dialog
  kernel->kbp()->idle();
  dlg.setMsgPart(msgPart);
  if (!dlg.exec())
  {
    delete msgPart;
    return;
  }

  // add the new attachment to the list
  addAttach(msgPart);
  rethinkFields(); //work around initial-size bug in Qt-1.32
}


//-----------------------------------------------------------------------------
void KMComposeWin::addAttach(const KMMessagePart* msgPart)
{
  mAtmList.append(msgPart);

  // show the attachment listbox if it does not up to now
  if (mAtmList.count()==1)
  {
    mGrid->setRowStretch(mNumHeaders+1, 50);
    mAtmListBox->setMinimumSize(100, 80);
    mAtmListBox->setMaximumHeight( 100 );
    mAtmListBox->show();
    resize(size());
  }

  // add a line in the attachment listbox
  QListViewItem *lvi = new QListViewItem(mAtmListBox);
  msgPartToItem(msgPart, lvi);
  mAtmItemList.append(lvi);
}


//-----------------------------------------------------------------------------
void KMComposeWin::msgPartToItem(const KMMessagePart* msgPart, 
				 QListViewItem *lvi)
{
  unsigned int len;
  QString lenStr;

  assert(msgPart != NULL);

  len = msgPart->size();
  if (len > 9999) lenStr.sprintf("%uK", (len>>10));
  else lenStr.sprintf("%u", len);

  lvi->setText(0, msgPart->name());
  lvi->setText(1, lenStr);
  lvi->setText(2, msgPart->contentTransferEncodingStr());
  lvi->setText(3, msgPart->typeStr() + "/" + msgPart->subtypeStr());
}


//-----------------------------------------------------------------------------
void KMComposeWin::removeAttach(const QString aUrl)
{
  int idx;
  KMMessagePart* msgPart;
  for(idx=0,msgPart=mAtmList.first(); msgPart;
      msgPart=mAtmList.next(),idx++) {
    if (msgPart->name() == aUrl) {
      removeAttach(idx);
      return;
    }
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::removeAttach(int idx)
{
  mAtmList.remove(idx);
  delete mAtmItemList.take(idx);

  if (mAtmList.count()<=0)
  {
    mAtmListBox->hide();
    mGrid->setRowStretch(mNumHeaders+1, 0);
    mAtmListBox->setMinimumSize(0, 0);
    resize(size());
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::addrBookSelInto(KMLineEdit* aLineEdit)
{
  KMAddrBookSelDlg dlg(kernel->addrBook());
  QString txt;

  //assert(aLineEdit!=NULL);
  if(!aLineEdit)
    {
      debug("KMComposeWin::addrBookSelInto() : aLineEdit == NULL\n");
      return;
    }
  if (dlg.exec()==QDialog::Rejected) return;
  txt = QString(aLineEdit->text()).stripWhiteSpace();
  if (!txt.isEmpty())
  {
    if (txt.right(1).at(0)!=',') txt += ", ";
    else txt += ' ';
  }
  aLineEdit->setText(txt + dlg.address());
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBook()
{
  KMAddrBookEditDlg dlg(kernel->addrBook());
  dlg.exec();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBookFrom()
{
  addrBookSelInto(&mEdtFrom);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBookReplyTo()
{
  addrBookSelInto(&mEdtReplyTo);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBookTo()
{
  addrBookSelInto(&mEdtTo);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBookCc()
{
  addrBookSelInto(&mEdtCc);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAddrBookBcc()
{
  addrBookSelInto(&mEdtBcc);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachFile()
{
  // Create File Dialog and return selected file
  // We will not care about any permissions, existence or whatsoever in
  // this function.
  if (mPathAttach.isEmpty()) mPathAttach = QDir::currentDirPath();

  KFileDialog fdlg(mPathAttach, "*", this, NULL, TRUE);

  fdlg.setCaption(i18n("Attach File"));
  if (!fdlg.exec()) return;

  KURL u = fdlg.selectedURL();

  mPathAttach = u.directory();

  if(u.filename().isEmpty()) return;
  addAttach(u.path());
  mEditor->setModified(TRUE);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotInsertFile()
{
  QString str;
  int col, line;

  if (mPathAttach.isEmpty()) mPathAttach = QDir::currentDirPath();

  KFileDialog fdlg(mPathAttach, "*", this, NULL, TRUE);
  fdlg.setCaption(i18n("Include File"));
  if (!fdlg.exec()) return;

  KURL u = fdlg.selectedURL();

  mPathAttach = u.directory();

  if (u.filename().isEmpty()) return;

  if( !u.isLocalFile() )
  {
    KMessageBox::sorry( 0L, i18n( "Only local files are supported." ) );
    return;
  }

  str = kFileToString(u.path(), TRUE, TRUE);
  if (str.isEmpty()) return;

  mEditor->getCursorPosition(&line, &col);
  mEditor->insertAt(QCString(str), line, col);
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotInsertMyPublicKey()
{
  QString str, name;
  KMMessagePart* msgPart;

  // load the file
  kernel->kbp()->busy();
  str=Kpgp::getKpgp()->getAsciiPublicKey(mMsg->from());
  if (str.isNull())
  {
    kernel->kbp()->idle();
    return;
  }

  // create message part
  msgPart = new KMMessagePart;
  msgPart->setName(i18n("my pgp key"));
  msgPart->setCteStr(mDefEncoding);
  msgPart->setTypeStr("application");
  msgPart->setSubtypeStr("pgp-keys");
  msgPart->setBodyEncoded(QCString(str.ascii()));
  msgPart->setContentDisposition("attachment; filename=public_key.asc");

  // add the new attachment to the list
  addAttach(msgPart);
  rethinkFields(); //work around initial-size bug in Qt-1.32

  kernel->kbp()->idle();
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotInsertPublicKey()
{
  QString str, name;
  KMMessagePart* msgPart;
  Kpgp *pgp;

  pgp=Kpgp::getKpgp();

  str=pgp->getAsciiPublicKey(
         KpgpKey::getKeyName(this, pgp->keys()));

  // create message part
  msgPart = new KMMessagePart;
  msgPart->setName(i18n("pgp key"));
  msgPart->setCteStr(mDefEncoding);
  msgPart->setTypeStr("application");
  msgPart->setSubtypeStr("pgp-keys");
  msgPart->setBodyEncoded(QCString(str.ascii()));
  msgPart->setContentDisposition("attachment; filename=public_key.asc");

  // add the new attachment to the list
  addAttach(msgPart);
  rethinkFields(); //work around initial-size bug in Qt-1.32

}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachPopupMenu(QListViewItem *, const QPoint &, int)
{
  QPopupMenu *menu = new QPopupMenu;

  menu->insertItem(i18n("View..."), this, SLOT(slotAttachView()));
  menu->insertItem(i18n("Save..."), this, SLOT(slotAttachSave()));
  menu->insertItem(i18n("Properties..."),
		   this, SLOT(slotAttachProperties()));
  menu->insertItem(i18n("Remove"), this, SLOT(slotAttachRemove()));
  menu->insertSeparator();
  menu->insertItem(i18n("Attach..."), this, SLOT(slotAttachFile()));
  menu->popup(QCursor::pos());
}

//-----------------------------------------------------------------------------
int KMComposeWin::currentAttachmentNum()
{
  int idx = -1;
  QListIterator<QListViewItem> it(mAtmItemList);
  for ( int i = 0; it.current(); ++it, ++i )
    if (*it == mAtmListBox->currentItem()) {
      idx = i;
      break;
    }

  return idx;
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachProperties()
{
  KMMsgPartDlg dlg;
  KMMessagePart* msgPart;
  int idx = currentAttachmentNum();
  
  if (idx < 0) return;

  msgPart = mAtmList.at(idx);
  dlg.setMsgPart(msgPart);
  if (dlg.exec())
  {
    // values may have changed, so recreate the listbox line
    msgPartToItem(msgPart, mAtmItemList.at(idx));
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachView()
{
  QString str, pname;
  KMMessagePart* msgPart;
  QMultiLineEdit* edt = new QMultiLineEdit;
  int idx = currentAttachmentNum();

  if (idx < 0) return;

  msgPart = mAtmList.at(idx);
  pname = msgPart->name();
  if (pname.isEmpty()) pname=msgPart->contentDescription();
  if (pname.isEmpty()) pname="unnamed";

  kernel->kbp()->busy();
  str = QCString(msgPart->bodyDecoded());

  edt->setCaption(i18n("View Attachment: ") + pname);
  edt->insertLine(str);
  edt->setReadOnly(TRUE);
  edt->show();

  kernel->kbp()->idle();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachSave()
{
  KMMessagePart* msgPart;
  QString fileName, pname;
  int idx = currentAttachmentNum();

  if (idx < 0) return;

  msgPart = mAtmList.at(idx);
  pname = msgPart->name();
  if (pname.isEmpty()) pname="unnamed";

  if (mPathAttach.isEmpty()) mPathAttach = QDir::currentDirPath();

  KURL url = KFileDialog::getSaveURL(mPathAttach, "*", NULL, pname);

  if( url.isEmpty() )
    return;

  if( !url.isLocalFile() )
  {
    KMessageBox::sorry( 0L, i18n( "Only local files supported yet." ) );
    return;
  }

  fileName = url.path();

  kByteArrayToFile(msgPart->bodyDecoded(), fileName, TRUE);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAttachRemove()
{
  int idx = currentAttachmentNum();

  if (idx >= 0)
  {
    removeAttach(idx);
    mEditor->setModified(TRUE);
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotToggleToolBar()
{
  enableToolBar(KToolBar::Toggle);
  mShowToolBar = !mShowToolBar;
  repaint();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotFind()
{
  mEditor->search();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotReplace()
{
  mEditor->replace();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotCut()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("KEdit"))
    ((QMultiLineEdit*)fw)->cut();
  else if (fw->inherits("KMLineEdit"))
    ((KMLineEdit*)fw)->cut();
  else debug("wrong focus widget");
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotCopy()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

#ifdef KeyPress
#undef KeyPress
#endif

  QKeyEvent k(QEvent::KeyPress, Key_C , 0 , ControlButton);
  kapp->notify(fw, &k);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotPaste()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

#ifdef KeyPress
#undef KeyPress
#endif

  QKeyEvent k(QEvent::KeyPress, Key_V , 0 , ControlButton);
  kapp->notify(fw, &k);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotMarkAll()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("QLineEdit"))
      ((QLineEdit*)fw)->selectAll();
  else if (fw->inherits("QMultiLineEdit"))
    ((QMultiLineEdit*)fw)->selectAll();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotClose()
{
  close(FALSE);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotNewComposer()
{
  KMComposeWin* win;
  KMMessage* msg = new KMMessage;

  msg->initHeader();
  win = new KMComposeWin(msg);
  win->show();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotNewMailReader()
{

#ifndef KRN
  KMMainWin *kmmwin = new KMMainWin(NULL);
  kmmwin->show();
  //d->resize(d->size());
#endif

}


//-----------------------------------------------------------------------------
void KMComposeWin::slotToDo()
{
  warning(i18n("Sorry, but this feature\nis still missing"));
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotUpdWinTitle(const QString& text)
{
  if (text.isEmpty())
       setCaption("("+QString(i18n("unnamed"))+")");
  else setCaption(text);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotPrint()
{
  QPrinter printer;
  QPainter paint;
  QString  str;

  if (printer.setup(this))
  {
    paint.begin(&printer);
    str += i18n("To:");
    str += " \n";
    str += to();
    str += "\n";
    str += i18n("Subject:");
    str += " \n";
    str += subject();
    str += i18n("Date:");
    str += " \n\n";
    str += mEditor->brokenText();
    str += "\n";
    //str.replace(QRegExp("\n"),"\n");
    paint.drawText(30,30,str);
    paint.end();
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotDropAction(QDropEvent *e)
{
  QString  file;
  QStringList fileList;
  QStringList::Iterator it;
  if(QUriDrag::canDecode(e) && QUriDrag::decodeLocalFiles( e, fileList )) {
    for (it = fileList.begin(); it != fileList.end(); ++it) {
      (*it).replace(QRegExp("file:"),"");
      addAttach(*it);
    }
  }
  else
    KMessageBox::sorry( 0L, i18n( "Only local files are supported." ) );
}


//----------------------------------------------------------------------------
void KMComposeWin::doSend(int aSendNow)
{
  bool sentOk;

  kernel->kbp()->busy();
  //applyChanges();  // is called twice otherwise. Lars
  mMsg->setDateToday();
  sentOk = (applyChanges() && kernel->msgSender()->send(mMsg, aSendNow));
  kernel->kbp()->idle();

  if (sentOk)
  {
    mAutoDeleteMsg = FALSE;
    close();
  }
}



//----------------------------------------------------------------------------
void KMComposeWin::slotSendLater()
{
  doSend(FALSE);
}


//----------------------------------------------------------------------------
void KMComposeWin::slotSendNow()
{
#ifndef KRN
  if (mConfirmSend) {
    switch(QMessageBox::information(&mMainWidget,
                                    i18n("Send Confirmation"),
                                    i18n("About to send email..."),
                                    i18n("Send &Now"),
                                    i18n("Send &Later"),
                                    i18n("&Cancel"),
                                    0,
                                    2)) {
    case 0:        // send now
        doSend(TRUE);
      break;
    case 1:        // send later
        doSend(FALSE);
      break;
    case 2:        // cancel
      break;
    default:
      ;    // whoa something weird happened here!
    }
    return;
  }
#endif

  doSend(TRUE);
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotAppendSignature()
{
  QString identStr = "unknown";
  if (!mId.isEmpty() && KMIdentity::identities().contains( mId ))
    identStr = mId;

  KMIdentity ident( identStr );
  ident.readConfig();
  QString sigFileName = ident.signatureFile();
  QString sigText;
  bool mod = mEditor->isModified();

  if (sigFileName.isEmpty())
  {
    // open a file dialog and let the user choose manually
#warning KFileDialog misses localfiles only flag.
//    KFileDialog dlg(getenv("HOME"),QString::null,this,0,TRUE,FALSE);
    KFileDialog dlg(getenv("HOME"),QString::null,this,0, TRUE);

    dlg.setCaption(i18n("Choose Signature File"));

    if( !dlg.exec() )
      return;

    KURL url = dlg.selectedURL();

    if( url.isEmpty() )
      return;

    if( !url.isLocalFile() )
    {
      KMessageBox::sorry( 0L, i18n( "Only local files are supported." ) );
      return;
    }

    sigFileName = url.path();
    sigText = kFileToString(sigFileName, TRUE);
    ident.setSignatureFile(sigFileName);
    ident.writeConfig(true);
  }
  else sigText = ident.signature();

  if (!sigText.isEmpty())
  {
    mEditor->insertLine("-- ", -1);
    mEditor->insertLine(sigText, -1);
    mEditor->setModified(mod);
  }
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotHelp()
{
  kapp->invokeHTMLHelp("","");
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotCleanSpace()
{
  if (!mEditor) return;

  mEditor->cleanWhiteSpace();
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotSpellcheck()
{
  if (mSpellCheckInProgress) return;

  mSpellCheckInProgress=TRUE;
  /*
    connect (mEditor, SIGNAL (spellcheck_progress (unsigned)),
    this, SLOT (spell_progress (unsigned)));
    */

  if(mEditor){
    connect (mEditor, SIGNAL (spellcheck_done()),
	     this, SLOT (slotSpellcheckDone ()));
    mEditor->spellcheck();
  }
}
//-----------------------------------------------------------------------------
void KMComposeWin::slotSpellcheckDone()
{
  debug( "spell check complete" );
  mSpellCheckInProgress=FALSE;
  statusBar()->changeItem(i18n("Spellcheck complete."),0);

}

//-----------------------------------------------------------------------------
void KMComposeWin::slotConfigureCharsets()
{
#if defined CHARSETS
   CharsetsDlg *dlg=new CharsetsDlg((const char*)mCharset,
				    (const char*)mComposeCharset,
                                    m7BitAscii,mQuoteUnknownCharacters);
   connect(dlg,SIGNAL( setCharsets(const char *,const char *,bool,bool,bool) )
           ,this,SLOT(slotSetCharsets(const char *,const char    dlg->show();
*,bool,bool,bool)));
   delete dlg;	
#endif
}


//-----------------------------------------------------------------------------
void KMComposeWin::slotSetCharsets(const char *message,const char *composer,
                                   bool ascii,bool quote,bool def)
{
  // prevent warning
  (void)message;
  (void)composer;
  (void)ascii;
  (void)quote;
  (void)def;

#if defined CHARSETS
  mCharset=message;
  m7BitAscii=ascii;
  mComposeCharset=composer;
  mQuoteUnknownCharacters=quote;
  if (def)
  {
    mDefaultCharset=message;
    mDefComposeCharset=composer;
  }
  setEditCharset();
#endif
}


#if defined CHARSETS
//-----------------------------------------------------------------------------
bool KMComposeWin::is8Bit(const QString str)
{
  const char *ptr=str;
  while(*ptr)
  {
    if ( (*ptr)&0x80 ) return TRUE;
    else if ( (*ptr)=='&' && ptr[1]=='#' )
    {
      int code=atoi(ptr+2);
      if (code>0x7f) return TRUE;
    }
    ptr++;
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
void KMComposeWin::convertToSend(const QString str)
{
  cout<<"Converting to send...\n";
  if (m7BitAscii && !is8Bit(str))
  {
    mCharset="us-ascii";
    return;
  }
  if (mCharset=="")
  {
     if (mDefaultCharset=="default")
          mCharset=KGlobal::charsets()->charsetForLocale();
     else mCharset=mDefaultCharset;
  }
  cout<<"mCharset: "<<mCharset<<"\n";
}

//-----------------------------------------------------------------------------
void KMComposeWin::setEditCharset()
{
  QFont fnt=mSavedEditorFont;
  if (mComposeCharset=="default")
    mComposeCharset=KGlobal::charsets()->charsetForLocale();
  cout<<"Setting font to: "<<mComposeCharset<<"\n";
  KGlobal::charsets()->setQFont(fnt, mComposeCharset);
  mEditor->setFont(fnt);
}
#endif //CHARSETS


//-----------------------------------------------------------------------------
void KMComposeWin::focusNextPrevEdit(const QLineEdit* aCur, bool aNext)
{
  QLineEdit* cur;

  if (!aCur)
  {
    cur=mEdtList.last();
  }
  else
  {
    for (cur=mEdtList.first(); aCur!=cur && cur; cur=mEdtList.next())
      ;
    if (!cur) return;
    if (aNext) cur = mEdtList.next();
    else cur = mEdtList.prev();
  }
  if (cur) cur->setFocus();
  else if (aNext) mEditor->setFocus(); //Key up from first doea nothing (sven)
}



//=============================================================================
//
//   Class  KMLineEdit
//
//=============================================================================
KMLineEdit::KMLineEdit(KMComposeWin* composer, QWidget *parent,
		       const char *name): KMLineEditInherited(parent,name)
{
  mComposer = composer;

  installEventFilter(this);

  connect (this, SIGNAL(completion()), this, SLOT(slotCompletion()));
}


//-----------------------------------------------------------------------------
KMLineEdit::~KMLineEdit()
{
  removeEventFilter(this);
}


//-----------------------------------------------------------------------------
bool KMLineEdit::eventFilter(QObject*, QEvent* e)
{
#ifdef KeyPress
#undef KeyPress
#endif

  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent* k = (QKeyEvent*)e;

    if (k->state()==ControlButton && k->key()==Key_T)
    {
      emit completion();
      cursorAtEnd();
      return TRUE;
    }
    // ---sven's Return is same Tab and arrow key navigation start ---
    if (k->key() == Key_Enter || k->key() == Key_Return)
    {
      mComposer->focusNextPrevEdit(this,TRUE);
      return TRUE;
    }
    if (k->state()==ControlButton && k->key() == Key_Right)
    {
      if ((int)strlen(text()) == cursorPosition()) // at End?
      {
        emit completion();
        cursorAtEnd();
        return TRUE;
      }
      return FALSE;
    }
    if (k->key() == Key_Up)
    {
      mComposer->focusNextPrevEdit(this,FALSE); // Go up
      return TRUE;
    }
    if (k->key() == Key_Down)
    {
      mComposer->focusNextPrevEdit(this,TRUE); // Go down
      return TRUE;
    }
    // ---sven's Return is same Tab and arrow key navigation end ---

  }
  else if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = (QMouseEvent*)e;
    if (me->button() == RightButton)
    {
      QPopupMenu* p = new QPopupMenu;
      p->insertItem(i18n("Cut"),this,SLOT(cut()));
      p->insertItem(i18n("Copy"),this,SLOT(copy()));
      p->insertItem(i18n("Paste"),this,SLOT(paste()));
      p->insertItem(i18n("Mark all"),this,SLOT(markAll()));
      setFocus();
      p->popup(QCursor::pos());
      return TRUE;
    }
  }
  return FALSE;
}


//-----------------------------------------------------------------------------
void KMLineEdit::cursorAtEnd()
{
  QKeyEvent ev( QEvent::KeyPress, Key_End, 0, 0 );
  QLineEdit::keyPressEvent( &ev );
}


//-----------------------------------------------------------------------------
void KMLineEdit::copy()
{
  QString t = markedText();
  QApplication::clipboard()->setText(t);
}

//-----------------------------------------------------------------------------
void KMLineEdit::cut()
{
  if(hasMarkedText())
  {
    QString t = markedText();
    QKeyEvent k(QEvent::KeyPress, Key_D, 0, ControlButton);
    keyPressEvent(&k);
    QApplication::clipboard()->setText(t);
  }
}

//-----------------------------------------------------------------------------
void KMLineEdit::paste()
{
  QKeyEvent k(QEvent::KeyPress, Key_V, 0, ControlButton);
  keyPressEvent(&k);
}

//-----------------------------------------------------------------------------
void KMLineEdit::markAll()
{
  selectAll();
}

//-----------------------------------------------------------------------------
void KMLineEdit::slotCompletion()
{
  QString t;
  QString Name(name());

  if (Name == "subjectLine")
  {
    //mComposer->focusNextPrevEdit(this,TRUE); //IMHO, it is useless now (sven)

    return;
  }

  QPopupMenu pop;
  int n;

  KMAddrBook adb;
  adb.readConfig();

  if(adb.load() == IO_FatalError)
    return;

  QString s(text());
  QString prevAddr;
  n = s.findRev(',');
  if (n>=0)
  {
    prevAddr = s.left(n+1) + ' ';
    s = s.mid(n+1,255).stripWhiteSpace();
  }
  //s.append("*");
  //QRegExp regexp(s.data(), FALSE, TRUE);

  n=0;
  for (QString a=adb.first(); a; a=adb.next())
  {
    //t.setStr(a);
    //if (t.contains(regexp))
    if (QString(a).find(s,0,false) >= 0)
    {
      pop.insertItem(a);
      n++;
    }
  }

  if (n > 1)
  {
    int id;
    pop.popup(parentWidget()->mapToGlobal(QPoint(x(), y()+height())));
    pop.setActiveItem(pop.idAt(0));
    id = pop.exec();

    if (id!=-1)
    {
      setText(prevAddr + pop.text(id));
      //mComposer->focusNextPrevEdit(this,TRUE);
    }
  }
  else if (n==1)
  {
    setText(prevAddr + pop.text(pop.idAt(0)));
    //mComposer->focusNextPrevEdit(this,TRUE);
  }

  setFocus();
  cursorAtEnd();
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotSpellcheckConfig()
{

  KWM kwm;
  QTabDialog qtd (this, "tabdialog", true);
  KSpellConfig mKSpellConfig (&qtd);

  qtd.addTab (&mKSpellConfig, "Spellchecker");
  qtd.setCancelButton ();

  kwm.setMiniIcon (qtd.winId(), kapp->miniIcon());

  if (qtd.exec())
    mKSpellConfig.writeGlobalSettings();
}

//-----------------------------------------------------------------------------
void KMComposeWin::slotEditToolbars()
{
  KEditToolbar dlg(actionCollection(), "kmcomposerui.rc");

  if (dlg.exec() == true)
  {
    createGUI("kmcomposerui.rc");
  }
}


//=============================================================================
//
//   Class  KMEdit
//
//=============================================================================
KMEdit::KMEdit(QWidget *parent, KMComposeWin* composer,
	       const char *name):
  KMEditInherited(parent, name)
{
  initMetaObject();
  mComposer = composer;
  installEventFilter(this);

#ifndef KRN
  extEditor = false;     // the default is to use ourself
#endif

  mKSpell = NULL;
}


//-----------------------------------------------------------------------------
KMEdit::~KMEdit()
{

  removeEventFilter(this);

  if (mKSpell) delete mKSpell;

}


//-----------------------------------------------------------------------------
QString KMEdit::brokenText() const
{
    QString temp;

    for (int i = 0; i < numLines(); ++i) {
      temp += *getString(i);
      if (i + 1 < numLines())
	temp += '\n';
    }

    return temp;
}

//-----------------------------------------------------------------------------
bool KMEdit::eventFilter(QObject*, QEvent* e)
{
  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent *k = (QKeyEvent*)e;

#ifndef KRN
    if (extEditor) {
      if (k->key() == Key_Up)
      {
        mComposer->focusNextPrevEdit(0, false); //take me up
        return TRUE;
      }

      QRegExp repFn("\\%f");
      QString sysLine = mExtEditor;
      KTempFile tmpFile;
      QString tmpName = tmpFile.name();

      tmpFile.setAutoDelete(true);

      fprintf(tmpFile.fstream(), "%s", (const char *)text());

      tmpFile.close();
      // replace %f in the system line
      sysLine.replace(repFn, tmpName);
      system((const char *)sysLine);

      setAutoUpdate(false);
      clear();

      // read data back in from file
      insertLine(kFileToString(tmpName, TRUE, FALSE), -1);
//       QFile mailFile(tmpName);

//       while (!mailFile.atEnd()) {
//          QString oneLine;
//          mailFile.readLine(oneLine, 1024);
//          insertLine(oneLine, -1);
//       }

//       setModified(true);
//       mailFile.close();

      setAutoUpdate(true);
      repaint();
      return TRUE;
    } else {
#endif
    if (k->key()==Key_Tab)
    {
      int col, row;
      getCursorPosition(&row, &col);
      insertAt("	", row, col); // insert tab character '\t'
      emit CursorPositionChanged();
      return TRUE;
    }
    // ---sven's Arrow key navigation start ---
    // Key Up in first line takes you to Subject line.
    if (k->key() == Key_Up && currentLine() == 0)
    {
      mComposer->focusNextPrevEdit(0, false); //take me up
      return TRUE;
    }
    // ---sven's Arrow key navigation end ---
#ifndef KRN
    }
#endif
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
void KMEdit::spellcheck()
{
  mKSpell = new KSpell(this, i18n("Spellcheck - KMail"), this,
		       SLOT(slotSpellcheck2(KSpell*)));
  connect (mKSpell, SIGNAL( death()),
          this, SLOT (slotSpellDone()));
  connect (mKSpell, SIGNAL (misspelling (QString, QStringList *, unsigned)),
          this, SLOT (misspelling (QString, QStringList *, unsigned)));
  connect (mKSpell, SIGNAL (corrected (QString, QString, unsigned)),
          this, SLOT (corrected (QString, QString, unsigned)));
  connect (mKSpell, SIGNAL (done(const char *)),
          this, SLOT (slotSpellResult (const char *)));
}


//-----------------------------------------------------------------------------
void KMEdit::slotSpellcheck2(KSpell*)
{
  spellcheck_start();
  //we're going to want to ignore quoted-message lines...
  mKSpell->check(text());
}

//-----------------------------------------------------------------------------
void KMEdit::slotSpellResult(const char *aNewText)
{
  spellcheck_stop();
  if (mKSpell->dlgResult() == 0)
  {
     setText(aNewText);
  }
  mKSpell->cleanUp();
}

//-----------------------------------------------------------------------------
void KMEdit::slotSpellDone()
{
  KSpell::spellStatus status = mKSpell->status();
  delete mKSpell;
  mKSpell = 0;
  if (status == KSpell::Error)
  {
     KMessageBox::sorry(this, i18n("ISpell could not be started.\n Please make sure you have ISpell properly configured and in your PATH."));
  }
  else if (status == KSpell::Crashed)
  {
     spellcheck_stop();
     KMessageBox::sorry(this, i18n("ISpell seems to have crashed."));
  }
  else
  {
     emit spellcheck_done();
  }
}







