/* KMail account for pop mail folders
 *
 */
#ifndef kmacctpop_h
#define kmacctpop_h

#include <qobject.h>
#include <qwidget.h>
#include <qpushbt.h>
#include <qlined.h>
#include <qlabel.h>
#include "kmaccount.h"

#define KMAcctPopInherited KMAccount

class KMAcctPop: public KMAccount
{
Q_OBJECT
public:
  virtual ~KMAcctPop();
  virtual void init(void);

  /** Pop user login name */
  const QString& login(void) const { return mLogin; }
  virtual void setLogin(const QString&);

  /** Pop user password */
  const QString& passwd(void) const { return mPasswd; }
  virtual void setPasswd(const QString&, bool storeInConfig=FALSE);

  /** Will the password be stored in the config file ? */
  bool storePasswd(void) const { return mStorePasswd; }

  /** Pop host */
  const QString& host(void) const { return mHost; }
  virtual void setHost(const QString&);

  /** Port on pop host */
  int port(void) { return mPort; }
  virtual void setPort(int);

  /** Pop protocol: shall be 2 or 3 */
  short protocol(void) { return mProtocol; }
  virtual void setProtocol(short);

  virtual const char* type(void) const;
  virtual void readConfig(void);
  virtual void writeConfig(void);
  virtual bool processNewMail(void);

private:
  /** This function is called when either username or the passwd
      was incorrect. It pops up a dialog asking for a new username
      and passwd */
  void passwdError();
  QWidget *newWidget;
  QLineEdit *usernameLEdit;
  QLineEdit *passwdLEdit;
  QPushButton *ok;
  QPushButton *cancel;

private slots:
  void slotOkPressed();
  void slotCancelPressed(); 
  
protected:
  KMAcctPop(KMAcctMgr* owner, const char* accountName);
  friend class KMAcctMgr;


  QString mLogin, mPasswd;
  QString mHost;
  int     mPort;
  short   mProtocol;
  bool    mStorePasswd;
};

#endif /*kmacctpop_h*/
