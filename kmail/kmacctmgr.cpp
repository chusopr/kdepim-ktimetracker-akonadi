// KMail Account Manager

#include "kmacctmgr.h"
#include "kmacctlocal.h"
#include "kmacctpop.h"
#include "kmglobal.h"
#include "kbusyptr.h"

#include <assert.h>
#include <kconfig.h>
#include <kapp.h>
#include <kapp.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

extern KBusyPtr *kbp;
//-----------------------------------------------------------------------------
KMAcctMgr::KMAcctMgr(const char* aBasePath): KMAcctMgrInherited()
{
  assert(aBasePath != NULL);
  mAcctList.setAutoDelete(TRUE);
  setBasePath(aBasePath);
}


//-----------------------------------------------------------------------------
KMAcctMgr::~KMAcctMgr()
{
  writeConfig(FALSE);
  mAcctList.clear();
}


//-----------------------------------------------------------------------------
void KMAcctMgr::setBasePath(const char* aBasePath)
{
  assert(aBasePath != NULL);

  if (aBasePath[0] == '~')
  {
    mBasePath = QDir::homeDirPath();
    mBasePath.append("/");
    mBasePath.append(aBasePath+1);
  }
  else mBasePath = aBasePath;

  mBasePath.detach();
}


//-----------------------------------------------------------------------------
void KMAcctMgr::writeConfig(bool withSync)
{
  debug("writing config");
  KConfig* config = app->getConfig();
  KMAccount* acct;
  QString groupName(256);
  int i;

  config->setGroup("General");
  config->writeEntry("accounts", mAcctList.count());

  for (i=1,acct=mAcctList.first(); acct; acct=mAcctList.next(),i++)
  {
    groupName.sprintf("Account %d", i);
    config->setGroup(groupName);
    acct->writeConfig(*config);
  }
  if (withSync) config->sync();
}


//-----------------------------------------------------------------------------
void KMAcctMgr::readConfig(void)
{
  debug("Read config called");
  KConfig* config = app->getConfig();
  KMAccount* acct;
  QString groupName(256), acctType, acctName;
  int i, num;

  mAcctList.clear();

  config->setGroup("General");
  num = config->readNumEntry("accounts", 0);

  for (i=1; i<=num; i++)
  {
    groupName.sprintf("Account %d", i);
    config->setGroup(groupName);
    acctType = config->readEntry("Type");
    acctName = config->readEntry("Name");
    acct = create(acctType, acctName);
    if (!acct) continue;
    acct->readConfig(*config);
  }
}


//-----------------------------------------------------------------------------
bool KMAcctMgr::singleCheckMail(KMAccount *account)
{
  bool hasNewMail = FALSE;
  debug("singleCheckMail called!");
  //kbp->busy();
  KMIOStatusWdg *wid = new KMIOStatusWdg(0L,0L,KMIOStatus::RETRIEVE);
  wid->show();

  if (account->processNewMail(wid))
  {
    hasNewMail = TRUE;
    emit newMail(account);
  }
  delete wid;
  kbp->idle();
  return hasNewMail;
}


//-----------------------------------------------------------------------------
KMAccount* KMAcctMgr::create(const QString aType, const QString aName) 
{
  KMAccount* act = NULL;

  if (stricmp(aType,"local")==0) 
    act = new KMAcctLocal(this, aName);

  else if (stricmp(aType,"pop")==0) 
    act = new KMAcctPop(this, aName);

  if (act) 
  {
    mAcctList.append(act);
    act->setFolder(inboxFolder);
  }

  return act;
}


//-----------------------------------------------------------------------------
KMAccount* KMAcctMgr::find(const QString aName) 
{
  KMAccount* cur;

  if (aName.isEmpty()) return NULL;

  for (cur=mAcctList.first(); cur; cur=mAcctList.next())
  {
    if (cur->name() == aName) return cur;
  }

  return NULL;
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
bool KMAcctMgr::remove(KMAccount* acct)
{
  assert(acct != NULL);
  mAcctList.remove(acct);
  return TRUE;
}


//-----------------------------------------------------------------------------
bool KMAcctMgr::checkMail(void)
{
  KMAccount* cur;
  bool hasNewMail = FALSE;

  if (mAcctList.isEmpty())
  {
    warning(i18n("You need to add an account in the network\n"
		 "section of the settings in order to\n"
		 "receive mail."));
    return FALSE;
  }


  KMIOStatusWdg *wid = new KMIOStatusWdg(0L,0L,KMIOStatus::RETRIEVE);
  wid->show();
  
  for (cur=mAcctList.first(); cur; cur=mAcctList.next())
  {
    if (cur->processNewMail(wid))
    {
      hasNewMail = TRUE;
      emit newMail(cur);
    }
  }
  delete wid;
  return hasNewMail;
}


QStrList  KMAcctMgr::getAccounts() {
  
  KMAccount *cur;
  QStrList strList;
  for (cur=mAcctList.first(); cur; cur=mAcctList.next()) {
    strList.append(cur->name());
  }

  return strList;

}

bool KMAcctMgr::intCheckMail(int item) {

  KMAccount* cur;
  bool hasNewMail = FALSE;

  if (mAcctList.isEmpty())
  {
    warning(i18n("You need to add an account in the network\n"
		 "section of the settings in order to\n"
		 "receive mail."));
    return FALSE;
  }

  KMIOStatusWdg *wid = new KMIOStatusWdg(0L,0L,KMIOStatus::RETRIEVE);
  wid->show();
  
  printf("Item: %i\n" ,item);
  int x = 0;
  cur = mAcctList.first();
  for(x=0; x < item; x++)
    cur=mAcctList.next();

  debug(cur->name());

  if (cur->processNewMail(wid))
    {
      hasNewMail = TRUE;
      emit newMail(cur);
    }

  delete wid;
  return hasNewMail;

}


//-----------------------------------------------------------------------------
#include "kmacctmgr.moc"
