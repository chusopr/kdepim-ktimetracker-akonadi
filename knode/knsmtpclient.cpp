/***************************************************************************
                          knsmtpclient.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Gebauer
    email                : gebauer@bigfoot.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>

#include <klocale.h>

#include "knmime.h"
#include "knjobdata.h"
#include "knsmtpclient.h"


KNSmtpClient::KNSmtpClient(int NfdPipeIn, int NfdPipeOut, QObject *parent, const char *name)
: KNProtocolClient(NfdPipeIn,NfdPipeOut,parent,name)
{}


KNSmtpClient::~KNSmtpClient()
{}


// examines the job and calls the suitable handling method
void KNSmtpClient::processJob()
{
  switch (job->type()) {
    case KNJobData::JTmail :
      doMail();
      break;
    default:
#ifndef NDEBUG
      qDebug("knode: KNSmtpClient::processJob(): mismatched job");
#endif
      break;
  }
}
  

void KNSmtpClient::doMail()
{
  KNLocalArticle *art=static_cast<KNLocalArticle*>(job->data());
  
  sendSignal(TSsendMail); 

  QCString cmd = "MAIL FROM:<";
  cmd += art->from()->email();
  cmd += ">";
  if(!sendCommandWCheck(cmd, 250))
    return;
    
  progressValue = 80;

  QStrList emails;
  art->to()->emails(&emails);
  bool rcptOK=false;

  for(char *e=emails.first() ; e; e=emails.next()) {
    cmd="RCPT TO:<" + QCString(e) + ">";
    if(sendCommandWCheck(cmd, 250))
      rcptOK=true;
  }

  if(!rcptOK) // mail has not been accepted by the smtp-host
    return;
    
  progressValue = 90;

  if(!sendCommandWCheck("DATA", 354))
    return;
    
  progressValue = 100;
  
  if(!sendMsg(art->encodedContent(true)))
    return;
    
  if(!checkNextResponse(250))
    return;
}


bool KNSmtpClient::openConnection()
{
  QString oldPrefix = errorPrefix;
  errorPrefix=i18n("Unable to connect.\nThe following error ocurred:\n");

  if (!KNProtocolClient::openConnection())
    return false;
    
  progressValue = 30;
    
  if (!checkNextResponse(220))
    return false;
    
  progressValue = 50;

  char hostName[500];

  QCString cmd = "HELO ";
    
  if (gethostname(hostName,490)==0) {
    cmd += hostName;
    //qDebug("knode: KNSmtpClient::openConnection(): %s",cmd.data());
  } else {
    cmd += "foo";
    //qDebug("knode: KNSmtpClient::openConnection(): can't detect hostname, using foo");
  }

  if (!sendCommandWCheck(cmd,250))
    return false;
    
  progressValue = 70;
  
  errorPrefix = oldPrefix;
  return true;
}



//--------------------------------

#include "knsmtpclient.moc"
