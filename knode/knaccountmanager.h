/***************************************************************************
                          knaccountmanager.h  -  description
                             -------------------
    
    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KNACCOUNTMANAGER_H
#define KNACCOUNTMANAGER_H

#include <qlist.h>


class KNGroupManager;
class KNListView;
class KNNntpAccount;
class KNServerInfo;

class KNAccountManager : public QObject
{
  Q_OBJECT
  
  public:
    KNAccountManager(KNGroupManager *gm, KNListView *v, QObject * parent=0, const char * name=0);
    ~KNAccountManager();
    
    void setCurrentAccount(KNNntpAccount *a);
    
    bool newAccount(KNNntpAccount *a);       // a is new account allocated and configured by the caller
    void applySettings(KNNntpAccount *a);    // commit changes on a the caller made
    void removeAccount(KNNntpAccount *a=0);  // a==0: remove current account
    void editProperties(KNNntpAccount *a=0);

    bool hasCurrentAccount()              { return (c_urrentAccount!=0); }
    KNNntpAccount* currentAccount()       { return c_urrentAccount; }
    KNServerInfo* smtp()                  { return s_mtp; }
    KNNntpAccount* first()                { return accList->first(); }
    KNNntpAccount* next()                 { return accList->next(); }   
    KNNntpAccount* account(int i);  


  protected:
    void loadAccounts();
    KNGroupManager *gManager;
    QList<KNNntpAccount> *accList;
    KNNntpAccount *c_urrentAccount;
    KNServerInfo *s_mtp;
    KNListView *view;


  signals:
    void accountAdded(KNNntpAccount *a);
    void accountModified(KNNntpAccount *a);
    void accountRemoved(KNNntpAccount *a);   // don't do anything with a, it will be deleted soon
};

#endif
