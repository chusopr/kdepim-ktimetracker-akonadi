// kmacctlocal.cpp

#include "kmacctlocal.h"
#include "kmfolder.h"
#include "kmmessage.h"
#include "kmacctfolder.h"
#include "kmglobal.h"

#include <kapp.h>
#include <kconfig.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


//-----------------------------------------------------------------------------
KMAcctLocal::KMAcctLocal(KMAcctMgr* aOwner, const char* aAccountName):
  KMAcctLocalInherited(aOwner, aAccountName)
{
  initMetaObject();
}


//-----------------------------------------------------------------------------
KMAcctLocal::~KMAcctLocal()
{
  mLocation = "";
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
bool KMAcctLocal::processNewMail(KMIOStatus *statusWdg)
{
  KMFolder mailFolder(NULL, location());
  long num = 0;
  long i;
  int rc;
  KMMessage* msg;

  if (mFolder==NULL) return FALSE;

  printf("processNewMail: %s\n", (const char*)location());

  statusWdg->prepareTransmission(location(), KMIOStatus::RETRIEVE);
  app->processEvents();
  mailFolder.setAutoCreateIndex(FALSE);
  rc = mailFolder.open();
  if (rc)
  {
    perror("cannot open file "+mailFolder.path()+"/"+mailFolder.name());
    return FALSE;
  }

  mFolder->quiet(TRUE);
  mFolder->open();

  num = mailFolder.count();

  for (i=0; i<num; i++)
  {
    //if(statusWdg->abortRequested())
    //break;
    statusWdg->updateProgressBar(i,num);
    msg = mailFolder.take(0);
    msg->setStatus(msg->headerField("Status"), msg->headerField("X-Status"));
    if (msg) processNewMsg(msg);
    app->processEvents();
  }

  rc = mailFolder.expunge();
  if (rc)
    warning(i18n("Cannot remove mail from\nmailbox `%s':\n%s"),
	    (const char*)mailFolder.location(), strerror(rc));

  mailFolder.close();
  mFolder->close();
  mFolder->quiet(FALSE);
  statusWdg->transmissionCompleted();

  return (num > 0);
}


//-----------------------------------------------------------------------------
void KMAcctLocal::readConfig(KConfig& config)
{
  QString defaultPath("/var/spool/mail/");
  defaultPath += getenv("USER");

  KMAcctLocalInherited::readConfig(config);
  mLocation = config.readEntry("Location", defaultPath);
}


//-----------------------------------------------------------------------------
void KMAcctLocal::writeConfig(KConfig& config)
{
  KMAcctLocalInherited::writeConfig(config);

  config.writeEntry("Location", mLocation);
}


//-----------------------------------------------------------------------------
void KMAcctLocal::setLocation(const QString& aLocation)
{
  mLocation = aLocation;
}
