#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <qdir.h>
#include <qstring.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapp.h>
#include <kstddirs.h>

#include <kmailIface.h>

#include "kmkernel.h"
#include "kmmainwin.h"
#include "kmcomposewin.h"
#include "kmmessage.h"
#include "kmfoldermgr.h"
#include "kmfolder.h"
#include "kmfiltermgr.h"
#include "kmreaderwin.h"
#include "kmsender.h"
#include "kmundostack.h"
#include "kmidentity.h"
#include "kmacctmgr.h"
#include "kbusyptr.h"
#include "kmaddrbook.h"

#include <X11/Xlib.h>

KMKernel *KMKernel::mySelf = 0;

KMKernel::KMKernel (QObject *parent, const char *name) :
  QObject(parent, name),  DCOPObject("KMailIface")
{
  //debug ("KMKernel::KMKernel");
  mySelf = this;
}

KMKernel::~KMKernel ()
{
  mySelf = 0;
  debug ("KMKernel::~KMKernel");
}

// DCOP calable stuff, for local stuff look bellow

void KMKernel::checkMail () //might create a new reader but won�t show!!
{
  debug ("KMKernel::checkMail called");
  KMMainWin *mWin = 0;

  if (kapp->mainWidget() && kapp->mainWidget()->isA("KMMainWin"))
    mWin = (KMMainWin *) kapp->mainWidget();
  else
    mWin = new KMMainWin;
  mWin->slotCheckMail();
}

void KMKernel::openReader( KURL /*messageFile*/)
{
#warning Ugly hack! (sven)
  KMMainWin *mWin = 0;
  KTMainWindow *ktmw = 0;
  debug ("KMKernel::openReader called");

  if (KTMainWindow::memberList)
    for (ktmw = KTMainWindow::memberList->first(); ktmw;
         ktmw = KTMainWindow::memberList->next())
      if (ktmw->isA("KMMainWin"))
        break;

  if (ktmw)
    mWin = (KMMainWin *) ktmw;
  else
    mWin = new KMMainWin;
  mWin->show();
  //and "Activate" by kwin?

  //if (!messageFile.isEmpty())
  //  mWin->viewMessage(messageFile);
}

int KMKernel::openComposer (QString to, QString cc,
                            QString bcc, QString subject, int hidden)
{
  debug ("KMKernel::openComposer called");

  KMMessage *msg = new KMMessage;
  msg->initHeader();
  if (!cc.isEmpty()) msg->setCc(cc);
  if (!bcc.isEmpty()) msg->setBcc(bcc);
  if (!subject.isEmpty()) msg->setSubject(subject);
  if (!to.isEmpty()) msg->setTo(to);

  KMComposeWin *cWin = new KMComposeWin(msg);
  if (hidden == 0)
    cWin->show();
  //return cWin->composerId()
  return 1;
}

int KMKernel::setBody (int /*composerId*/, QString /*body*/)
{
  debug ("KMKernel::setBody called");
  return 1;
}

int KMKernel::addAttachment(int /*composerId*/, KURL /*url*/,
                            QString /*comment*/)
{
  debug ("KMKernel::addAttachment called");
  return 1;
}

int KMKernel::send(int /*composerId*/, int /*when*/)
{
  debug ("KMKernel::send called");
  return 1;
}

int KMKernel::ready()
{
  debug ("KMKernel::ready called");
  return 1;
}



// Normal local stuff

void KMKernel::testDir(const char *_name)
{
  DIR *dp;
  QString c = getenv("HOME");
  if(c.isEmpty())
  {
      KMessageBox::sorry(0, i18n("$HOME is not set!\n"
                                 "KMail cannot start without it.\n"));
      exit(-1);
  }
		
  c += _name;
  dp = opendir(c.data());
  if (dp == NULL) ::mkdir(c.data(), S_IRWXU);
  else closedir(dp);
}


//-----------------------------------------------------------------------------
// Open a composer for each message found in ~/dead.letter
//to control
void KMKernel::recoverDeadLetters(void)
{
  KMComposeWin* win;
  KMMessage* msg;
  QDir dir = QDir::home();
  QString fname = dir.path();
  int i, rc, num;

  if (!dir.exists("dead.letter")) return;
  fname += "/dead.letter";
  KMFolder folder(0, fname);

  folder.setAutoCreateIndex(FALSE);
  rc = folder.open();
  if (rc)
  {
    perror("cannot open file "+fname);
    return;
  }

  folder.quiet(TRUE);
  folder.open();

  num = folder.count();
  for (i=0; i<num; i++)
  {
    msg = folder.take(0);
    if (msg)
    {
      win = new KMComposeWin;
      win->setMsg(msg, FALSE);
      win->show();
    }
  }
  folder.close();
  unlink(fname);
}

void KMKernel::initFolders(KConfig* cfg)
{
  QString name;

  name = cfg->readEntry("inboxFolder");

  // Currently the folder manager cannot manage folders which are not
  // in the base folder directory.
  //if (name.isEmpty()) name = getenv("MAIL");

  if (name.isEmpty()) name = "inbox";

  the_inboxFolder  = (KMFolder*)the_folderMgr->findOrCreate(name);
  // inboxFolder->open();

  the_outboxFolder = the_folderMgr->findOrCreate(cfg->readEntry("outboxFolder", "outbox"));
  the_outboxFolder->setType("Out");
  the_outboxFolder->setWhoField("To");
  the_outboxFolder->setSystemFolder(TRUE);
  the_outboxFolder->open();

  the_sentFolder = the_folderMgr->findOrCreate(cfg->readEntry("sentFolder", "sent-mail"));
  the_sentFolder->setType("St");
  the_sentFolder->setWhoField("To");
  the_sentFolder->setSystemFolder(TRUE);
  the_sentFolder->open();

  the_trashFolder  = the_folderMgr->findOrCreate(cfg->readEntry("trashFolder", "trash"));
  the_trashFolder->setType("Tr");
  the_trashFolder->setSystemFolder(TRUE);
  the_trashFolder->open();

}


//-----------------------------------------------------------------------------
//to control
void KMKernel::init()
{
  debug ("entering KMKernel::init()");
  QCString  acctPath, foldersPath;
  KConfig* cfg;

  the_checkingMail = false;
  the_shuttingDown = false;
  the_server_is_ready = false;
  
  the_kbp = new KBusyPtr;
  cfg = kapp->config();
  //debug ("1");
  // Stefan: Yes, we really want this message handler. Without it,
  // kmail does not show vital warning() dialogs.
  //qInstallMsgHandler(&kmailMsgHandler);

  QDir dir;
  QString d = locateLocal("data", "kmail/");

  the_identity = new KMIdentity;

  cfg->setGroup("General");
  the_firstStart = cfg->readBoolEntry("first-start", true);
  foldersPath = cfg->readEntry("folders", "");
  acctPath = cfg->readEntry("accounts", foldersPath + "/.kmail-accounts");

  if (foldersPath.isEmpty())
  {
    foldersPath = QDir::homeDirPath() + QString("/Mail");
    transferMail();
  }

  the_folderMgr = new KMFolderMgr(foldersPath);
  the_undoStack = new KMUndoStack(20);
  the_acctMgr   = new KMAcctMgr(acctPath);
  the_filterMgr = new KMFilterMgr;
  the_filterActionDict = new KMFilterActionDict;
  the_addrBook  = new KMAddrBook;

  initFolders(cfg);
  the_acctMgr->readConfig();
  the_filterMgr->readConfig();
  the_addrBook->readConfig();
  if(the_addrBook->load() == IO_FatalError)
  {
      KMessageBox::sorry(0, i18n("The addressbook could not be loaded."));
  }
  KMMessage::readConfig();
  the_msgSender = new KMSender;

  
  the_server_is_ready = true;
  
  // filterMgr->dump(); 
  debug ("exiting KMKernel::init()");
}

bool KMKernel::doSessionManagement()
{

  // Do session management
  if (kapp->isRestored()){
    int n = 1;
    while (KTMainWindow::canBeRestored(n)){
      //only restore main windows! (Matthias);
      if (KTMainWindow::classNameOfToplevel(n) == "KMMainWin")
        (new KMMainWin)->restore(n);
      n++;
    }
    return true; // we were restored by SM
  }
  return false;  // no, we were not restored
}

void KMKernel::cleanup(void)
{
  the_shuttingDown = TRUE;
  KConfig* config =  kapp->config();

  if (the_trashFolder) {
    the_trashFolder->close(TRUE);
    config->setGroup("General");
    if (config->readNumEntry("empty-trash-on-exit", 0))
      the_trashFolder->expunge();
  }

  if (the_folderMgr) {
    if (config->readNumEntry("compact-all-on-exit", 0))
      the_folderMgr->compactAll(); // I can compact for ages in peace now!
  }

  if (the_inboxFolder) the_inboxFolder->close(TRUE);
  if (the_outboxFolder) the_outboxFolder->close(TRUE);
  if (the_sentFolder) the_sentFolder->close(TRUE);

  if (the_msgSender) delete the_msgSender;
  if (the_addrBook) delete the_addrBook;
  if (the_filterMgr) delete the_filterMgr;
  if (the_acctMgr) delete the_acctMgr;
  if (the_folderMgr) delete the_folderMgr;
  if (the_kbp) delete the_kbp;

  //qInstallMsgHandler(oldMsgHandler);
  kapp->config()->sync();
  //--- Sven's save attachments to /tmp start ---
  //debug ("cleaned");
  QString cmd;
  // This is a dir with attachments and it is not critical if they are
  // left behind.
  if (!KMReaderWin::attachDir().isEmpty())
  {
    cmd.sprintf("rm -rf '%s'", (const char*)KMReaderWin::attachDir());
    system (cmd.data()); // delete your owns only
  }
  //--- Sven's save attachments to /tmp end ---
}

//Isn�t this obsolete? (sven)
void KMKernel::transferMail(void)
{
  QDir dir = QDir::home();
  int rc;

  // Stefan: This function is for all the whiners who think that KMail is
  // broken because they cannot read mail with pine and do not
  // know how to fix this problem with a simple symbolic link  =;-)
  // Markus: lol ;-)
  if (!dir.cd("KMail")) return;

  rc = KMessageBox::questionYesNo(0, 
         i18n(
	    "The directory ~/KMail exists. From now on, KMail uses the\n"
	    "directory ~/Mail for its messages.\n"
	    "KMail can move the contents of the directory ~/KMail into\n"
	    "~/Mail, but this will replace existing files with the same\n"
	    "name in the directory ~/Mail (e.g. inbox).\n\n"
	    "Shall KMail move the mail folders now ?"));

  if (rc == KMessageBox::No) return;

  dir.cd("/");  // otherwise we lock the directory
  testDir("/Mail");
  system("mv -f ~/KMail/* ~/Mail");
  system("mv -f ~/KMail/.??* ~/Mail");
  system("rmdir ~/KMail");
}


void KMKernel::ungrabPtrKb(void)
{
  if(!KTMainWindow::memberList) return;
  QWidget* widg = KTMainWindow::memberList->first();
  Display* dpy;

  if (!widg) return;
  dpy = widg->x11Display();
  XUngrabKeyboard(dpy, CurrentTime);
  XUngrabPointer(dpy, CurrentTime);
}


//-----------------------------------------------------------------------------
// Message handler
void KMKernel::kmailMsgHandler(QtMsgType aType, const char* aMsg)
{
  QString appName = kapp->caption();
  QString msg = aMsg;
  static int recurse=-1;

  recurse++;

  switch (aType)
  {
  case QtDebugMsg:
    kDebugInfo(0, msg);
    break;

  case QtWarningMsg:
    fprintf(stderr, "%s: %s\n", (const char*)kapp->name(), msg.data());
    kDebugInfo(0, msg);
    break;

  case QtFatalMsg:
    ungrabPtrKb();
    fprintf(stderr, appName+" "+i18n("fatal error")+": %s\n", msg.data());
    KMessageBox::error(0, aMsg);
    abort();
  }

  recurse--;
}
void KMKernel::dumpDeadLetters()
{
  QWidget *win;
  
  while (KTMainWindow::memberList->first() != 0)
  {
    win = KTMainWindow::memberList->take();
    if (win->inherits("KMComposeWin")) ((KMComposeWin*)win)->deadLetter();
    delete win;
  }
  cleanup();
  ::exit (1);
}



void KMKernel::action(bool mailto, bool check, QString to, QString cc,
                      QString bcc, QString subj, KURL messageFile)
{
  
  if (mailto)
    openComposer (to, cc, bcc, subj, 0);
  else
    openReader(messageFile);

  if (check)
    checkMail();
  //Anything else?
}

#include "kmkernel.moc"
