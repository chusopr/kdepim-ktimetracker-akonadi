#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or   
    (at your option) any later version.                                 
                                                                        
    This program is distributed in the hope that it will be useful,     
    but WITHOUT ANY WARRANTY; without even the implied warranty of      
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        
    GNU General Public License for more details.                        
                                                                        
    You should have received a copy of the GNU General Public License   
    along with this program; if not, write to the Free Software         
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qptrlist.h>
#include <qobject.h>

#include <kaction.h>

class KXMLGUIClient;
class KAddressBook;
class ViewManager;

/** The ActionManager creates all the actions in KAddressBook. This class
* is shared between the main application and the part so all common
* actions are in one location.
*/
class ActionManager : public QObject
{
  Q_OBJECT
  
  public:
    ActionManager(KXMLGUIClient *client, KAddressBook *widget,
                  bool readWrite, QObject *parent);
    ~ActionManager();
  
    void setReadWrite(bool rw);
  
  public slots:
    void initActionViewList();
    
  protected slots:
    /** Called whenever an addressee is selected or unselected.
    *
    * @param selected True if there is an addressee select, false otherwise
    */
    void addresseeSelected(bool selected);
    
    /** Called whenever the addressbook is modified.
    *
    * @see KAddressBook
    */
    void modified(bool mod);
    
    /** Called whenever the view selection changes.
    */
    void selectViewAction();
    
    /** Called whenever the view configuration changes. This usually means
    * a view was added or deleted.
    */
    void viewConfigChanged(const QString &newActive);

    /** Called whenever the user clicks changes the view policy
    * of a quick tool.
    */
    void quickToolsAction();
    
    void updateEditMenu();
    
  private slots:
    void clipboardDataChanged();

  private:
    /** Create all the read only actions. These are all the actions that
    * cannot modify the addressbook.
    */
    void initReadOnlyActions();
    
    /** Create all the read write actions. These are all the actions that
    * can modify the addressbook.
    */
    void initReadWriteActions();

    /** Destroys all the read write actions.
    */
    void destroyReadWriteActions();
    
    bool mReadWrite;
    
    QString mActiveViewName;
    
    KAddressBook *mWidget;
    ViewManager *mViewManager;
    KXMLGUIClient *mGUIClient;
    KActionCollection *mACollection;
    
    KAction *mActionPaste;
    KAction *mActionCut;
    KAction *mActionDelete;
    KAction *mActionCopy;
    KAction *mActionEditAddressee;
    KAction *mActionMail;
    KAction *mActionUndo;
    KAction *mActionRedo;
    KAction *mActionSave;
    KAction *mActionDeleteView;
    QPtrList<KAction> mActionViewList;
    KToggleAction *mActiveActionView;
    KToggleAction *mActionJumpBar;
    KToggleAction *mActionIncSearch;
    KToggleAction *mActionQuickEdit;
    KToggleAction *mActionFilter;
};

#endif
