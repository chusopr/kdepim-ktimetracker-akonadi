// KMail Account Manager

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kmacctmgr.h"

#include "kmacctmaildir.h"
#include "kmacctlocal.h"
#include "kmacctexppop.h"
#include "kmacctimap.h"
#include "networkaccount.h"
using KMail::NetworkAccount;
#include "kmacctcachedimap.h"
#include "kmbroadcaststatus.h"
#include "kmfiltermgr.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>

#include <qregexp.h>
#include <qvaluelist.h>

//-----------------------------------------------------------------------------
KMAcctMgr::KMAcctMgr(): QObject()
{
  mAcctList.setAutoDelete(TRUE);
  mAcctChecking.clear();
  mAcctTodo.clear();
  mTotalNewMailsArrived=0;
}


//-----------------------------------------------------------------------------
KMAcctMgr::~KMAcctMgr()
{
  writeConfig(FALSE);
}


//-----------------------------------------------------------------------------
void KMAcctMgr::writeConfig(bool withSync)
{
  KConfig* config = KMKernel::config();
  QString groupName;

  KConfigGroupSaver saver(config, "General");
  config->writeEntry("accounts", mAcctList.count());

  // first delete all account groups in the config file:
  QStringList accountGroups =
    config->groupList().grep( QRegExp( "Account \\d+" ) );
  for ( QStringList::Iterator it = accountGroups.begin() ;
	it != accountGroups.end() ; ++it )
    config->deleteGroup( *it );

  // now write new account groups:
  int i = 1;
  for ( QPtrListIterator<KMAccount> it(mAcctList) ;
	it.current() ; ++it, ++i ) {
    groupName.sprintf("Account %d", i);
    KConfigGroupSaver saver(config, groupName);
    (*it)->writeConfig(*config);
  }
  if (withSync) config->sync();
}


//-----------------------------------------------------------------------------
void KMAcctMgr::readConfig(void)
{
  KConfig* config = KMKernel::config();
  KMAccount* acct;
  QString acctType, acctName;
  QCString groupName;
  int i, num;
  uint id;

  mAcctList.clear();

  KConfigGroup general(config, "General");
  num = general.readNumEntry("accounts", 0);

  for (i=1; i<=num; i++)
  {
    groupName.sprintf("Account %d", i);
    KConfigGroupSaver saver(config, groupName);
    acctType = config->readEntry("Type");
    // Provide backwards compatibility
    if (acctType == "advanced pop" || acctType == "experimental pop")
      acctType = "pop";
    acctName = config->readEntry("Name");
    id = config->readUnsignedNumEntry("Id", 0);
    if (acctName.isEmpty()) acctName = i18n("Account %1").arg(i);
    acct = create(acctType, acctName, id);
    if (!acct) continue;
    add(acct);
    acct->readConfig(*config);
  }
}


//-----------------------------------------------------------------------------
void KMAcctMgr::singleCheckMail(KMAccount *account, bool _interactive)
{
  newMailArrived = false;
  interactive = _interactive;

  // queue the account
  mAcctTodo.append(account);

  if (account->checkingMail())
  {
    kdDebug(5006) << "account " << account->name() << " busy, queuing" << endl;
    return;
  }

  processNextCheck(false);
}

//-----------------------------------------------------------------------------
void KMAcctMgr::processNextCheck(bool _newMail)
{
  kdDebug(5006) << "processNextCheck, remaining " << mAcctTodo.count() << endl;
  KMAccount *curAccount = 0;
  newMailArrived |= _newMail;

  KMAccount* acct;
  for ( acct = mAcctChecking.first(); acct; acct = mAcctChecking.next() )
  {
    if ( !acct->checkingMail() )
    {
      // check done
      kdDebug(5006) << "account " << acct->name() << " finished check" << endl;
      mAcctChecking.removeRef( acct );
      kmkernel->filterMgr()->deref();
      disconnect( acct, SIGNAL(finishedCheck(bool)),
                  this, SLOT(processNextCheck(bool)) );
      emit checkedMail(newMailArrived, interactive);
    }
  }
  if (mAcctChecking.isEmpty())
  {
    // all checks finished, display summary
    KMBroadcastStatus::instance()->setStatusMsgTransmissionCompleted(
        mTotalNewMailsArrived );
    mTotalNewMailsArrived = 0;
  }
  if (mAcctTodo.isEmpty()) return;
  curAccount = mAcctTodo.take(0);

  if (curAccount->type() != "imap" && curAccount->type() != "cachedimap" &&
      curAccount->folder() == 0)
  {
    QString tmp = i18n("Account %1 has no mailbox defined:\n"
        "mail checking aborted;\n"
        "check your account settings.")
      .arg(curAccount->name());
    KMessageBox::information(0,tmp);
    emit checkedMail(false, interactive);
    mTotalNewMailsArrived = 0;
    return;
  }

  connect( curAccount, SIGNAL(finishedCheck(bool)),
	   this, SLOT(processNextCheck(bool)) );

  KMBroadcastStatus::instance()->setStatusMsg(
      i18n("Checking account %1 for new mail").arg(curAccount->name()));

  NetworkAccount *nacct = dynamic_cast<NetworkAccount*>( curAccount );
  //If it a NetworkAccount we want to show the user whether the
  //connection is encrypted. If it's a local account we don't want
  //to put a useless icon in the statusbar and ignore it.
  if ( nacct ) {
    KMBroadcastStatus::instance()->setUsingSSL( nacct->useSSL() ||
                                                nacct->useTLS() );
  }
  kdDebug(5006) << "processing next mail check for " << curAccount->name() << endl;

  curAccount->setCheckingMail(true);
  mAcctChecking.append(curAccount);
  kmkernel->filterMgr()->ref();
  curAccount->processNewMail(interactive);
}

//-----------------------------------------------------------------------------
KMAccount* KMAcctMgr::create(const QString &aType, const QString &aName, uint id)
{
  KMAccount* act = 0;
  if (id == 0)
    id = createId();

  if (aType == "local")
    act = new KMAcctLocal(this, aName, id);

  if (aType == "maildir")
    act = new KMAcctMaildir(this, aName, id);

  else if (aType == "pop")
    act = new KMAcctExpPop(this, aName, id);

  else if (aType == "imap")
    act = new KMAcctImap(this, aName, id);

  else if (aType == "cachedimap")
    act = new KMAcctCachedImap(this, aName, id);

  if (act)
  {
    if (aType != "imap" && aType != "cachedimap")
      act->setFolder(kmkernel->inboxFolder());
    connect( act, SIGNAL(newMailsProcessed(int)),
	this, SLOT(addToTotalNewMailCount(int)) );
  }

  return act;
}


//-----------------------------------------------------------------------------
void KMAcctMgr::add(KMAccount *account)
{
  if (account)
    mAcctList.append(account);
}


//-----------------------------------------------------------------------------
KMAccount* KMAcctMgr::findByName(const QString &aName)
{
  if (aName.isEmpty()) return 0;

  for ( QPtrListIterator<KMAccount> it(mAcctList) ; it.current() ; ++it )
  {
    if ((*it)->name() == aName) return (*it);
  }

  return 0;
}


//-----------------------------------------------------------------------------
KMAccount* KMAcctMgr::find(const uint id)
{
  if (id == 0) return 0;

  for ( QPtrListIterator<KMAccount> it(mAcctList) ; it.current() ; ++it )
  {
    if ((*it)->id() == id) return (*it);
  }

  return 0;
}


//-----------------------------------------------------------------------------
KMAccount* KMAcctMgr::first(void)
{
  return mAcctList.first();
}


//-----------------------------------------------------------------------------
KMAccount* KMAcctMgr::next(void)
{
  return mAcctList.next();
}


//-----------------------------------------------------------------------------
bool KMAcctMgr::remove( KMAccount* acct )
{
  if( !acct )
    return false;
  mAcctList.removeRef( acct );
  emit accountRemoved( acct );
  return true;
}

//-----------------------------------------------------------------------------
void KMAcctMgr::checkMail(bool _interactive)
{
  newMailArrived = false;

  if (mAcctList.isEmpty())
  {
    KMessageBox::information(0,i18n("You need to add an account in the network "
				    "section of the settings in order to "
				    "receive mail."));
    return;
  }

  mTotalNewMailsArrived=0;

  for ( QPtrListIterator<KMAccount> it(mAcctList) ;
	it.current() ; ++it )
  {
    if (!it.current()->checkExclude())
      singleCheckMail(it.current(), _interactive);
  }
}


//-----------------------------------------------------------------------------
void KMAcctMgr::singleInvalidateIMAPFolders(KMAccount *account) {
  account->invalidateIMAPFolders();
}


void KMAcctMgr::invalidateIMAPFolders()
{
  if (mAcctList.isEmpty()) {
    KMessageBox::information(0,i18n("You need to add an account in the network "
                                   "section of the settings in order to "
                                   "receive mail."));
    return;
  }

  for ( QPtrListIterator<KMAccount> it(mAcctList) ; it.current() ; ++it )
    singleInvalidateIMAPFolders(it.current());
}


//-----------------------------------------------------------------------------
QStringList  KMAcctMgr::getAccounts(bool noImap) {

  KMAccount *cur;
  QStringList strList;
  for (cur=mAcctList.first(); cur; cur=mAcctList.next()) {
    if (!noImap || cur->type() != "imap") strList.append(cur->name());
  }

  return strList;

}

//-----------------------------------------------------------------------------
void KMAcctMgr::intCheckMail(int item, bool _interactive)
{
  KMAccount* cur;
  newMailArrived = false;

  mTotalNewMailsArrived = 0;
  int x = 0;
  cur = mAcctList.first();
  while (cur)
  {
    x++;
    if (x > item) break;
    cur=mAcctList.next();
  }

  singleCheckMail(cur, _interactive);
}


//-----------------------------------------------------------------------------
void KMAcctMgr::addToTotalNewMailCount(int newmails)
{
  if ( newmails == -1 ) mTotalNewMailsArrived = -1;
  if ( mTotalNewMailsArrived == -1 ) return;
  mTotalNewMailsArrived += newmails;
}

//-----------------------------------------------------------------------------
uint KMAcctMgr::createId()
{
  QValueList<uint> usedIds;
  for ( QPtrListIterator<KMAccount> it(mAcctList) ; it.current() ; ++it )
    usedIds << it.current()->id();

  usedIds << 0; // 0 is default for unknown
  int newId;
  do
  {
    newId = kapp->random();
  } while ( usedIds.find(newId) != usedIds.end() );

  return newId;
}

//-----------------------------------------------------------------------------
void KMAcctMgr::cancelMailCheck()
{
  for ( QPtrListIterator<KMAccount> it(mAcctList) ;
	it.current() ; ++it ) {
    it.current()->cancelMailCheck();
  }
}

#include "kmacctmgr.moc"
