/**
 * kmacctimap.h
 *
 * Copyright (c) 2000 Michael Haeckel <Michael@Haeckel.Net>
 *
 * This file is based on kmacctexppop.h by Don Sanders
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef KMAcctImap_h
#define KMAcctImap_h

#include "kmaccount.h"
#include <kapp.h>
#include <qdialog.h>
#include "kio/job.h"
#include "kio/global.h"
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qcstring.h>
#include <qqueue.h>

class QLineEdit;
class QPushButton;
class KApplication;
class KMMessage;
class QTimer;
class KURL::List;
class QDataStream;
class KMFolderTreeItem;

#define KMAcctImapInherited KMAccount

class KMImapJob : public QObject
{
  Q_OBJECT

public:
  KMImapJob(QList<KMMessage> msgList, KMFolder *destFolder);
  KMImapJob(KMMessage *msg);
  static void killJobsForMessage(KMMessage *msg);
signals:
  void messagesRetrieved(QList<KMMessage>, KMFolder*);
  void messageRetrieved(KMMessage *);
private slots:
  void slotGetMessageResult(KIO::Job * job);
  void slotGetNextMessage();
private:
  enum JobType { tListDirectory, tGetFolder, tCreateFolder, tDeleteMessage,
    tGetMessage, tPutMessage };
  JobType mType;
  QList<KMMessage> mMsgList;
  KMFolder *mDestFolder;
  KIO::Job *mJob;
  bool mSingleMessage;
  QByteArray mData;
  QCString mStrData;
  QStringList mItems;
  KMFolderTreeItem *mFti;
  int mTotal, mDone;
};


//-----------------------------------------------------------------------------
class KMAcctImap: public KMAccount
{
  Q_OBJECT
  friend class KMImapJob;

public:
  virtual ~KMAcctImap();
  virtual void init(void);

  /** Imap user login name */
  const QString& login(void) const { return mLogin; }
  virtual void setLogin(const QString&);

  /** Imap user password */
  const QString passwd(void) const;
  virtual void setPasswd(const QString&, bool storeInConfig=FALSE);

  /** Imap authentificaion method */
  const QString auth(void) const { return mAuth; }
  virtual void setAuth(const QString&);

  /** Will the password be stored in the config file ? */
  bool storePasswd(void) const { return mStorePasswd; }
  virtual void setStorePasswd(bool);

  /** Imap host */
  const QString& host(void) const { return mHost; }
  virtual void setHost(const QString&);

  /** Port on Imap host */
  unsigned short int port(void) { return mPort; }
  virtual void setPort(unsigned short int);

  /** Prefix to the Imap folders */
  const QString& prefix(void) const { return mPrefix; }
  virtual void setPrefix(const QString&);

  /** Show hidden files on the server */
  bool hiddenFolders() { return mHiddenFolders; }
  virtual void setHiddenFolders(bool);

  /** List a directory and add the contents to a KMFolderTreeItem */
  void listDirectory(KMFolderTreeItem * fti, bool secondStep = FALSE);

  /** Retrieve all mails in a folder */
  void getFolder(KMFolderTreeItem * fti);

  /** Get the whole message */
  void getMessage(KMFolder * folder, KMMessage * msg);

  /** Create a new subfolder */
  void createFolder(KMFolderTreeItem * fti, const QString &name);

  /** Kill all jobs related the the specified folder */
  void killJobsForItem(KMFolderTreeItem * fti);

  /** Kill the slave if any jobs are active */
  void killAllJobs();

  /** Delete a message */
  void deleteMessage(KMMessage * msg);

  /** Expunge deleted messages from the folder */
  void expungeFolder(KMFolder * aFolder);

  /** Inherited methods. */
  virtual const char* type(void) const;
  virtual void readConfig(KConfig&);
  virtual void writeConfig(KConfig&);
  virtual void processNewMail(bool) { emit finishedCheck(false); }
  virtual void pseudoAssign(KMAccount*);
  
  struct jobData
  {
    QByteArray data;
    QCString cdata;
    QStringList items;
    KMFolderTreeItem *parent;
    int total, done;
    bool inboxOnly;
  };
  QMap<KIO::Job *, jobData> mapJobData;

  /** Get the URL for the account */
  KURL getUrl();

  /** Update the progress bar */
  void displayProgress();

  /** Get the Slave used for the account */
  KIO::Slave * slave() { return mSlave; }
  void slaveDied() { mSlave = NULL; }

protected:
  enum Stage { Idle, List, Uidl, Retr, Dele, Quit };
  friend class KMAcctMgr;
  friend class KMPasswdDialog;
  KMAcctImap(KMAcctMgr* owner, const char* accountName);

  /** Very primitive en/de-cryption so that the password is not
      readable in the config file. But still very easy breakable. */
  const QString encryptStr(const QString inStr) const;
  const QString decryptStr(const QString inStr) const;

  /** Start a KIO Job to get a list of messages on the pop server */
  void startJob();

  /** Connect up the standard signals/slots for the KIO Jobs */
  void connectJob();

  /** Process any queued messages and save the list of seen uids
      for this user/server */
  void processRemainingQueuedMessagesAndSaveUidList();

  /** Connect to the IMAP server, if no connection is active */
  bool makeConnection();

  QString mLogin, mPasswd;
  QString mHost, mAuth;
  QString mPrefix;
  unsigned short int mPort;
  bool    mStorePasswd;
  bool    mHiddenFolders;
  bool    gotMsgs;
  bool    mProgressEnabled;

  KIO::Job *job;
  KIO::Slave *mSlave;

  QStringList idsOfMsgsPendingDownload;
  QValueList<int> lensOfMsgsPendingDownload;

  QStringList idsOfMsgs;
  QValueList<int> lensOfMsgs;
  QStringList uidsOfMsgs;
  QStringList uidsOfSeenMsgs;
  QStringList uidsOfNextSeenMsgs;
  KURL::List idsOfMsgsToDelete;
  int indexOfCurrentMsg;

  QValueList<KMMessage*> msgsAwaitingProcessing;
  QStringList msgIdsAwaitingProcessing;
  QStringList msgUidsAwaitingProcessing;

  QByteArray curMsgData;
  QDataStream *curMsgStrm;

  int curMsgLen;
  int stage;
  int processingDelay;
  int numMsgs, numBytes, numBytesRead, numMsgBytesRead;
  bool interactive;
  bool mProcessing;

  QList<KMImapJob> mJobList;

  struct statusData
  {
    KURL url;
    bool Delete;
  };
  QQueue<statusData> mStatusQueue;

protected slots:
  /** Kills all jobs */
  void slotAbortRequested();

  /** Add the imap folders to the folder tree */
  void slotListEntries(KIO::Job * job, const KIO::UDSEntryList & uds);

  /** Free the resources */
  void slotListResult(KIO::Job * job);

  /** Retrieve the next message */
  void getNextMessage(jobData & jd);

  /** For listing the contents of a folder */
  void slotListFolderResult(KIO::Job * job);
  void slotListFolderEntries(KIO::Job * job, const KIO::UDSEntryList & uds);

  /** For retrieving a message digest */
  void slotGetMessagesResult(KIO::Job * job); 
  void slotGetMessagesData(KIO::Job * job, const QByteArray & data);

  /** For creating a new subfolder */
  void slotCreateFolderResult(KIO::Job * job);

  /** For deleting messages and changing the status */
  void nextStatusAction();
  void slotStatusResult(KIO::Job * job);

  /** Expunge has finished */
  void slotExpungeResult(KIO::Job * job);

  /** Display an error message, that connecting failed */
  void slotSlaveError(KIO::Slave *aSlave, int, const QString &errorMsg);

public slots:
  /** Add the data a KIO::Job retrieves to the buffer */
  void slotGetMessageData(KIO::Job * job, const QByteArray & data);
};


//-----------------------------------------------------------------------------
class KMImapPasswdDialog : public QDialog
{
  Q_OBJECT

public:
  KMImapPasswdDialog(QWidget *parent = 0,const char *name= 0,
                     KMAcctImap *act=0, const QString caption=QString::null,
                     const char *login=0, QString passwd=QString::null);

private:
  QLineEdit *usernameLEdit;
  QLineEdit *passwdLEdit;
  QPushButton *ok;
  QPushButton *cancel;
  KMAcctImap *act;

private slots:
  void slotOkPressed();
  void slotCancelPressed();

};
#endif /*KMAcctImap_h*/
