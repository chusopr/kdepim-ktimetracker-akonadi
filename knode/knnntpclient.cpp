/*
    knnntpclient.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <stdlib.h>
#include <qstrlist.h>
#include <klocale.h>
#include <qtextcodec.h>

#include "knmime.h"
#include "kngroup.h"
#include "kngroupmanager.h"
#include "knjobdata.h"
#include "knnntpclient.h"
#include "utilities.h"


KNNntpClient::KNNntpClient(int NfdPipeIn, int NfdPipeOut, pthread_mutex_t *nntpMutex, QObject *parent, const char *name)
: KNProtocolClient(NfdPipeIn,NfdPipeOut,parent,name)
{
  mutex = nntpMutex;
}


KNNntpClient::~KNNntpClient()
{}
  

// examines the job and calls the suitable handling method
void KNNntpClient::processJob()
{
  switch (job->type()) {
    case KNJobData::JTLoadGroups :
      doLoadGroups();
      break;
    case KNJobData::JTFetchGroups :
      doFetchGroups();
      break;      
    case KNJobData::JTCheckNewGroups :
      doCheckNewGroups();
      break;      
    case KNJobData::JTfetchNewHeaders :
      doFetchNewHeaders();
      break;
    case KNJobData::JTfetchArticle :
      doFetchArticle();
      break;
    case KNJobData::JTpostArticle :
      doPostArticle();
      break;
   case KNJobData::JTfetchSource :
      doFetchSource();
      break;
    default:
#ifndef NDEBUG
      qDebug("knode: KNNntpClient::processJob(): mismatched job");
#endif
      break;
  }   
}
  

void KNNntpClient::doLoadGroups()
{
  KNGroupListData *target = static_cast<KNGroupListData *>(job->data());
  sendSignal(TSloadGrouplist);

  if (!target->readIn(this))
    job->setErrorString(i18n("Unable to read the group list file"));
}


void KNNntpClient::doFetchGroups()
{
  KNGroupListData *target = static_cast<KNGroupListData *>(job->data());
  
  sendSignal(TSdownloadGrouplist);
  errorPrefix = i18n("The group list could not be retrieved.\nThe following error occured:\n");
  
  progressValue = 100;
  predictedLines = 30000;     // rule of thumb ;-)
  
  if (!sendCommandWCheck("LIST",215))       // 215 list of newsgroups follows
    return;
    
  char *s, *line;
  QString name;
  KNGroup::Status status;
  bool subscribed;

  while (getNextLine()) {
    line = getCurrentLine();
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double period into one
      else
        if (line[1]==0)
          break;   // message complete
    }
    s = strchr(line,' ');
    if(!s) {
#ifndef NDEBUG
      qDebug("knode: retrieved broken group-line - ignoring");
#endif
    } else {
      s[0] = 0;    // cut string

      name = QString::fromUtf8(line);

      if (target->subscribed.contains(name)) {
        target->subscribed.remove(name);    // group names are unique, we wont find it again anyway...
        subscribed = true;
      } else
        subscribed = false;

      while (s[1]!=0) s++;   // the last character determines the moderation status
      switch (s[0]) {
        case 'n' : status = KNGroup::readOnly;
                   break;
        case 'y' : status = KNGroup::postingAllowed;
                   break;
        case 'm' : status = KNGroup::moderated;
                   break;
        default  : status = KNGroup::unknown;
      }

      target->groups->append(new KNGroupInfo(name,QString::null,false,subscribed,status));
    }
    doneLines++;
  }

  if (!job->success() || job->canceled())
    return;     // stopped...

  QSortedVector<KNGroupInfo> tempVector;
  target->groups->toVector(&tempVector);
  tempVector.sort();

  if (target->getDescriptions) {
    errorPrefix = i18n("The group descriptions could not be retrieved.\nThe following error occured:\n");
    progressValue = 100;
    doneLines = 0;
    predictedLines = target->groups->count();

    sendSignal(TSdownloadDesc);
    sendSignal(TSprogressUpdate);

    if (!sendCommandWCheck("LIST NEWSGROUPS",215))       // 215 informations follows
      return;

    QString description;
    KNGroupInfo info;
    int pos;

    while (getNextLine()) {
      line = getCurrentLine();
      if (line[0]=='.') {
        if (line[1]=='.')
          line++;        // collapse double period into one
        else
          if (line[1]==0)
            break;   // message complete
      }
      s = line;
      while (*s != '\0' && *s != '\t' && *s != ' ') s++;
      if (*s == '\0') {
#ifndef NDEBUG
        qDebug("knode: retrieved broken group-description - ignoring");
#endif
      } else {
        s[0] = 0;         // terminate groupname
        s++;
        while (*s == ' ' || *s == '\t') s++;    // go on to the description

        name = QString::fromUtf8(line);
        if (target->codecForDescriptions)          // some countries use local 8 bit characters in the tag line
          description = target->codecForDescriptions->toUnicode(s);
        else
          description = QString::fromLocal8Bit(s);
        info.name = name;

        if ((pos=tempVector.bsearch(&info))!=-1)
          tempVector[pos]->description = description;
      }
      doneLines++;
    }
    if (!job->success() || job->canceled())
      return;     // stopped...
  }

  target->groups->setAutoDelete(false);
  tempVector.toList(target->groups);
  target->groups->setAutoDelete(true);

  sendSignal(TSwriteGrouplist);
  if (!target->writeOut())
    job->setErrorString(i18n("Unable to write the group list file"));

}


void KNNntpClient::doCheckNewGroups()
{
  KNGroupListData *target = static_cast<KNGroupListData *>(job->data());
  
  sendSignal(TSdownloadNewGroups);
  errorPrefix = i18n("New groups could not be retrieved.\nThe following error occured:\n");
  
  progressValue = 100;
  predictedLines = 30;     // rule of thumb ;-)
  
  QCString cmd;
  cmd.sprintf("NEWGROUPS %.2d%.2d%.2d 000000",target->fetchSince.year()%100,target->fetchSince.month(),target->fetchSince.day());
  if (!sendCommandWCheck(cmd,231))      // 231 list of new newsgroups follows
    return;

  char *s, *line;
  QString name;
  KNGroup::Status status;
  QSortedList<KNGroupInfo> tmpList;
  tmpList.setAutoDelete(true);

  while (getNextLine()) {
    line = getCurrentLine();
    if (line[0]=='.') {
      if (line[1]=='.')
        line++;        // collapse double period into one
      else
        if (line[1]==0)
          break;   // message complete
    }
    s = strchr(line,' ');
    if(!s) {
#ifndef NDEBUG
      qDebug("knode: retrieved broken group-line - ignoring");
#endif
    } else {
      s[0] = 0;    // cut string
      name = QString::fromUtf8(line);

      while (s[1]!=0) s++;   // the last character determines the moderation status
      switch (s[0]) {
        case 'n' : status = KNGroup::readOnly;
                   break;
        case 'y' : status = KNGroup::postingAllowed;
                   break;
        case 'm' : status = KNGroup::moderated;
                   break;
        default  : status = KNGroup::unknown;
      }

      tmpList.append(new KNGroupInfo(name,QString::null,true,false,status));
    }
    doneLines++;
  }

  if (!job->success() || job->canceled())
    return;     // stopped...

  if (target->getDescriptions) {
    errorPrefix = i18n("The group descriptions could not be retrieved.\nThe following error occured:\n");
    progressValue = 100;
    doneLines = 0;
    predictedLines = tmpList.count()*3;

    sendSignal(TSdownloadDesc);
    sendSignal(TSprogressUpdate);

    cmd = "LIST NEWSGROUPS ";
    QStrList desList;
    char *s;

    for (KNGroupInfo *group=tmpList.first(); group; group=tmpList.next()) {
      if (!sendCommandWCheck(cmd+group->name.utf8(),215))       // 215 informations follows
        return;
      desList.clear();
      if (!getMsg(desList))
        return;

      if (desList.count()>0) {        // group has a description
        s = desList.first();
        while (*s !=- '\0' && *s != '\t' && *s != ' ') s++;
        if (*s == '\0') {
#ifndef NDEBUG
          qDebug("knode: retrieved broken group-description - ignoring");
#endif
        } else {
          while (*s == ' ' || *s == '\t') s++;    // go on to the description
          if (target->codecForDescriptions)          // some countries use local 8 bit characters in the tag line
            group->description = target->codecForDescriptions->toUnicode(s);
          else
            group->description = QString::fromLocal8Bit(s);
        }
      }
    }
  }

  sendSignal(TSloadGrouplist);

  if (!target->readIn()) {
    job->setErrorString(i18n("Unable to read the group list file"));
    return;
  }
  target->merge(&tmpList);
  sendSignal(TSwriteGrouplist);
  if (!target->writeOut()) {
    job->setErrorString(i18n("Unable to write the group list file"));
    return;
  }
}


void KNNntpClient::doFetchNewHeaders()
{
  KNGroup* target=static_cast<KNGroup*>(job->data());
  char* s;
  int first, last, oldlast, toFetch;
  QCString cmd;
  
  sendSignal(TSdownloadNew);
  errorPrefix=i18n("No new articles could have been retrieved for\n%1/%2!\nThe following error ocurred:\n")
  						.arg(account.server()).arg(target->groupname());
  
  cmd="GROUP ";
  cmd+=target->groupname().utf8();
  if (!sendCommandWCheck(cmd,211))       // 211 n f l s group selected
    return;
    
  progressValue = 90; 
    
  s = strchr(getCurrentLine(),' '); 
  if (s) {
    s++;
    s = strchr(s,' ');  
  }
  if (s) {
    s++;
    first=atoi(s);
    s = strchr(s,' ');
  }
  if (s) {
    last=atoi(s);
  } else {
    QString tmp=i18n("No new articles could have been retrieved!\nThe server sent a malformated response:\n");
    tmp+=getCurrentLine();
    job->setErrorString(tmp);
    closeConnection();
    return;
  }
  
  if(target->lastNr()==0) {   //first fetch
    if(first>0)
      oldlast=first-1;
    else
      oldlast=first;
  } else
    oldlast=target->lastNr();
    
  toFetch=last-oldlast;
  //qDebug("knode: last %d  oldlast %d  toFetch %d\n",last,oldlast,toFetch);
    
  if(toFetch<=0) {
    //qDebug("knode: No new Articles in group\n");
    target->setLastNr(last);     // don't get stuck when the article numbers wrap
    return;
  }
  
  if(toFetch>target->maxFetch()) {
    toFetch=target->maxFetch();
    //qDebug("knode: Fetching only %d articles\n",toFetch);
  }

  progressValue = 100;  
  predictedLines = toFetch;
    
  //qDebug("knode: KNNntpClient::doFetchNewHeaders() : xover %d-%d", last-toFetch+1, last);
  cmd.sprintf("xover %d-%d",last-toFetch+1,last);
  if (!sendCommandWCheck(cmd,224))       // 224 success
    return;
    
  QStrList headers;
  if (!getMsg(headers))
    return;
  
  progressValue = 1000;   
  sendSignal(TSprogressUpdate);
    
  sendSignal(TSsortNew);

  if (0==pthread_mutex_lock(mutex)) {
    target->insortNewHeaders(&headers, this);
    target->setLastNr(last);
    if (0!=pthread_mutex_unlock(mutex)) {
#ifndef NDEBUG
      qDebug("knode: failed to unlock nntp mutex");
#endif
    }
  } else {
#ifndef NDEBUG
    qDebug("knode: failed to lock nntp mutex");
#endif
  }
}


void KNNntpClient::doFetchArticle()
{
  KNRemoteArticle *target = static_cast<KNRemoteArticle*>(job->data());
  
  sendSignal(TSdownloadArticle);
  errorPrefix = i18n("Article could not been retrieved.\nThe following error occured:\n");

  progressValue = 100;
  predictedLines = target->lines()->numberOfLines()+10;
    
  QCString cmd = "ARTICLE " + target->messageID()->as7BitString(false);
  if (!sendCommandWCheck(cmd,220)) {      // 220 n <a> article retrieved - head and body follow
    int code = atoi(getCurrentLine());
    if (code == 430)  // 430 no such article found
      job->setErrorString(
             errorPrefix + getCurrentLine() +
             i18n("<br><br>The article you requested isn't available on your newsserver.<br>You can try to get it from <a href=\"http://groups.google.com/groups?q=msgid:%1&ic=1\">groups.google.com</a>.")
                  .arg(target->messageID()->as7BitString(false)));
    return;
  }
  
  QStrList msg;
  if (!getMsg(msg))
    return;
    
  progressValue = 1000;   
  sendSignal(TSprogressUpdate);
    
  target->setContent(&msg);
  target->parse();
}


void KNNntpClient::doPostArticle()
{
  KNLocalArticle *art=static_cast<KNLocalArticle*>(job->data());
  
  sendSignal(TSsendArticle);

  if (art->messageID(false)!=0) {
    int rep;
    if (!sendCommand(QCString("STAT ")+art->messageID(false)->as7BitString(false),rep))
      return;

    if (rep==223) {   // 223 n <a> article retrieved - request text separately
      #ifndef NDEBUG
      qDebug("knode: STAT successfull, we have probably already sent this article.");
      #endif
      return;       // the article is already on the server, lets put it silently into the send folder
    }
  }

  if(!sendCommandWCheck("POST", 340))       // 340 send article to be posted. End with <CR-LF>.<CR-LF>
    return;

  if (art->messageID(false)==0) {  // article has no message ID => search for a ID in the response
    QCString s = getCurrentLine();
    int start = s.findRev(QRegExp("<[^\\s]*@[^\\s]*>"));
    if (start != -1) {        // post response includes a recommended id
      int end = s.find('>',start);
      art->messageID()->from7BitString(s.mid(start,end-start+1));
      art->assemble();
      #ifndef NDEBUG
      qDebug("knode: using the message-id recommended by the server: %s",s.mid(start,end-start+1).data());
      #endif
    }
  }

  if (!sendMsg(art->encodedContent(true)))
    return;
    
  if (!checkNextResponse(240))            // 240 article posted ok
    return;
}


void KNNntpClient::doFetchSource()
{
  KNRemoteArticle *target = static_cast<KNRemoteArticle*>(job->data());

  sendSignal(TSdownloadArticle);
  errorPrefix = i18n("Article could not been retrieved.\nThe following error occured:\n");

  progressValue = 100;
  predictedLines = target->lines()->numberOfLines()+10;

  QCString cmd = "ARTICLE " + target->messageID()->as7BitString(false);
  if (!sendCommandWCheck(cmd,220))      // 220 n <a> article retrieved - head and body follow
    return;

  QStrList msg;
  if (!getMsg(msg))
    return;

  progressValue = 1000;
  sendSignal(TSprogressUpdate);

  target->setContent(&msg);
}


bool KNNntpClient::openConnection()
{
  QString oldPrefix = errorPrefix;
  errorPrefix=i18n("Unable to connect.\nThe following error ocurred:\n");

  if (!KNProtocolClient::openConnection())
    return false;
    
  progressValue = 30;
    
  int rep;
  if (!getNextResponse(rep))
    return false;
    
  if ((rep!=200)&&(rep!=201)) {  // 200 server ready - posting allowed
    handleErrors();              // 201 server ready - no posting allowed
    return false;
  }
  
  progressValue = 50;

  if (!sendCommand("MODE READER",rep))
    return false;

  if (rep==500) {
#ifndef NDEBUG
    qDebug("knode: \"MODE READER\" command not recognized.");
#endif
  } else
    if ((rep!=200)&&(rep!=201)) { // 200 Hello, you can post
      handleErrors();             // 201 Hello, you can't post
      return false;
    }

  progressValue = 60;

  // logon now, some newsserver send a incomplete group list otherwise
  if (account.needsLogon() && !account.user().isEmpty()) {
    //qDebug("knode: user: %s",account.user().latin1());

    QCString command = "AUTHINFO USER ";
    command += account.user().local8Bit();
    if (!KNProtocolClient::sendCommand(command,rep))
      return false;

    if (rep==381) {          // 381 PASS required
      //qDebug("knode: Password required");

      if (!account.pass().length()) {
        job->setErrorString(i18n("Authentication failed!\nCheck your username and password."));
        job->setAuthError(true);
        return false;
      }

      //qDebug("knode: pass: %s",account.pass().latin1());

      command = "AUTHINFO PASS ";
      command += account.pass().local8Bit();
      if (!KNProtocolClient::sendCommand(command,rep))
        return false;

      if (rep==281) {         // 281 authorization success
        #ifndef NDEBUG
        qDebug("knode: Authorization successful");
        #endif
      } else {
        #ifndef NDEBUG
        qDebug("knode: Authorization failed");
        #endif
        job->setErrorString(i18n("Authentication failed!\nCheck your username and password.\n\n%1").arg(getCurrentLine()));
        job->setAuthError(true);
        closeConnection();
        return false;
      }
    } else {
      if (rep==281) {         // 281 authorization success
        #ifndef NDEBUG
        qDebug("knode: Authorization successful");
        #endif
      } else {
        if ((rep==482)||(rep==500)) {   //482 Authentication rejected
          #ifndef NDEBUG
          qDebug("knode: Authorization failed");    // we don't care, the server can refuse the info
          #endif
        } else {
          handleErrors();
          return false;
        }
      }
    }
  }
  
  progressValue = 70;
  
  errorPrefix = oldPrefix;
  return true;
}


// authentication on demand
bool KNNntpClient::sendCommand(const QCString &cmd, int &rep)
{
  if (!KNProtocolClient::sendCommand(cmd,rep))
    return false;
  
  if (rep==480) {            // 480 requesting authorization
    //qDebug("knode: Authorization requested");
    
    if (!account.user().length()) {
      job->setErrorString(i18n("Authentication failed!\nCheck your username and password."));
      job->setAuthError(true);
      closeConnection();
      return false;
    }

    //qDebug("knode: user: %s",account.user().data());
        
    QCString command = "AUTHINFO USER ";
    command += account.user().local8Bit();
    if (!KNProtocolClient::sendCommand(command,rep))
      return false;
    
    if (rep==381) {          // 381 PASS required
      //qDebug("knode: Password required");
      
      if (!account.pass().length()) {
        job->setErrorString(i18n("Authentication failed!\nCheck your username and password.\n\n%1").arg(getCurrentLine()));
        job->setAuthError(true);
        closeConnection();
        return false;
      } 
          
      //qDebug("knode: pass: %s",account.pass().data());
      
      command = "AUTHINFO PASS ";
      command += account.pass().local8Bit();
      if (!KNProtocolClient::sendCommand(command,rep))
        return false; 
    }
    
    if (rep==281) {         // 281 authorization success
      #ifndef NDEBUG
      qDebug("knode: Authorization successful");
      #endif
      if (!KNProtocolClient::sendCommand(cmd,rep))    // retry the original command
        return false;
    } else {
      job->setErrorString(i18n("Authentication failed!\nCheck your username and password.\n\n%1").arg(getCurrentLine()));
      job->setAuthError(true);
      closeConnection();
      return false;
    }
  }
  return true;      
}


void KNNntpClient::handleErrors()
{
  if (errorPrefix.isEmpty())
    job->setErrorString(i18n("An error occured:\n%1").arg(getCurrentLine()));
  else
    job->setErrorString(errorPrefix + getCurrentLine());

  int code = atoi(getCurrentLine());

  // close the connection only when necessary:
  if ((code != 430)&&(code != 411))  // 430 no such article found / 411 no such news group
    closeConnection();
}


//--------------------------------

#include "knnntpclient.moc"
