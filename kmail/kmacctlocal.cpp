// kmacctlocal.cpp

#include "kmacctlocal.h"
#include "kmfolder.h"
#include "kmmessage.h"
#include "kmacctfolder.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


//-----------------------------------------------------------------------------
KMAcctLocal::KMAcctLocal(KMAcctMgr* aOwner, const char* aAccountName):
  KMAcctLocalInherited(aOwner, aAccountName)
{
}


//-----------------------------------------------------------------------------
KMAcctLocal::~KMAcctLocal()
{
  writeConfig();
  mLocation = NULL;
}


//-----------------------------------------------------------------------------
const char* KMAcctLocal::type(void) const
{
  return "local";
}


//-----------------------------------------------------------------------------
void KMAcctLocal::init(void)
{
  mLocation = "/var/spool/mail/";
  mLocation.detach();
  mLocation += getenv("USER");
}


//-----------------------------------------------------------------------------
bool KMAcctLocal::processNewMail(void)
{
  KMFolder mailFolder(NULL, location());
  long num = 0;
  long i;
  int rc;
  KMMessage* msg;

  if (mFolder==NULL) return FALSE;

  printf("processNewMail: %s\n", (const char*)location());

  mailFolder.setAutoCreateToc(FALSE);
  rc = mailFolder.open();
  if (rc)
  {
    perror("cannot open file "+mailFolder.path()+"/"+mailFolder.name());
    return FALSE;
  }

  mFolder->quiet(TRUE);
  mFolder->open();

  num = mailFolder.numMsgs();
  printf("%d messages in %s\n", num, (const char*)location());

  for (i=1; i<=num; i++)
  {
    debug(QString("processing message ")+i);
    msg = mailFolder.getMsg(i);
    mailFolder.detachMsg(i);
    if (msg) 
    {
      rc = mFolder->addMsg(msg);
      if (rc) perror("failed to add message");
      if (rc) warning("Failed to add message:\n" + QString(strerror(rc)));
    }
  }
  debug("done, closing folders");

  if (mailFolder.expunge())
    warning("Cannot remove mail from\nsystem mail folder.\n");

  mailFolder.close();
  mFolder->close();
  mFolder->quiet(FALSE);

  return (num > 0);
}


//-----------------------------------------------------------------------------
void KMAcctLocal::readConfig(void)
{
  QString defaultPath("/var/spool/mail/");

  defaultPath += getenv("USER");

  mLocation = mConfig->readEntry("location", defaultPath);
}


//-----------------------------------------------------------------------------
void KMAcctLocal::writeConfig(void)
{
  mConfig->writeEntry("type", "local");
  mConfig->writeEntry("location", mLocation);
  mConfig->sync();
}


//-----------------------------------------------------------------------------
void KMAcctLocal::setLocation(const QString& aLocation)
{
  mLocation = aLocation;
}
