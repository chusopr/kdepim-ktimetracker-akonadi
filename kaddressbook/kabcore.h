/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KABCORE_H
#define KABCORE_H

#include <kabc/field.h>

#include <qdict.h>
#include <qwidget.h>

#include "core.h"

namespace KABC {
class AddressBook;
class Ticket;
}

class KAboutData;
class KAction;
class KActionCollection;
class KCMultiDialog;
class KConfig;
class KToggleAction;
class KXMLGUIClient;

class QSplitter;

class AddresseeEditorDialog;
class ExtensionManager;
class FilterSelectionWidget;
class IncSearchWidget;
class JumpButtonBar;
class KAddressBookService;
class LDAPSearchDialog;
class ViewContainer;
class ViewManager;
class XXPortManager;

typedef struct {
  KABC::Ticket *ticket;
  int counter;
} ResourceMapEntry;


class KABCore : public KAB::Core
{
  Q_OBJECT

  public:
    KABCore( KXMLGUIClient *client, bool readWrite, QWidget *parent,
             const char *name = 0 );
    ~KABCore();

    /**
      Restores the global settings.
     */
    void restoreSettings();

    /**
      Saves the global settings.
     */
    void saveSettings();

    /**
      Returns a pointer to the StdAddressBook of the application.
     */
    KABC::AddressBook *addressBook() const;

    /**
      Returns a pointer to the KConfig object of the application.
     */
    KConfig *config() const;

    /**
      Returns a pointer to the global KActionCollection object. So
      other classes can register their actions easily.
     */
    KActionCollection *actionCollection() const;

    /**
      Returns the current search field of the Incremental Search Widget.
     */
    KABC::Field *currentSearchField() const;

    /**
      Returns the uid list of the currently selected contacts.
     */
    QStringList selectedUIDs() const;

    /**
      Displays the ResourceSelectDialog and returns the selected
      resource or a null pointer if no resource was selected by
      the user.
     */
    KABC::Resource *requestResource( QWidget *parent );

    /**
      Returns the parent widget.
     */
    QWidget *widget() const;

    static KAboutData *createAboutData();

  public slots:
    /**
      Is called whenever a contact is selected in the view.
     */
    void setContactSelected( const QString &uid );

    /**
      Opens the preferred mail composer with all selected contacts as
      arguments.
     */
    void sendMail();

    /**
      Opens the preferred mail composer with the given contacts as
      arguments.
     */
    void sendMail( const QString& email );


    void mailVCard();
    void mailVCard(const QStringList& uids);

    /**
      Starts the preferred web browser with the given URL as argument.
     */
    void browse( const QString& url );

    /**
      Select all contacts in the view.
     */
    void selectAllContacts();

    /**
      Deletes all selected contacts from the address book.
     */
    void deleteContacts();

    /**
      Deletes given contacts from the address book.

      @param uids The uids of the contacts, which shall be deleted.
     */
    void deleteContacts( const QStringList &uids );

    /**
      Copys the selected contacts into clipboard for later pasting.
     */
    void copyContacts();

    /**
      Cuts the selected contacts and stores them for later pasting.
     */
    void cutContacts();

    /**
      Paste contacts from clipboard into the address book.
     */
    void pasteContacts();

    /**
      Paste given contacts into the address book.

      @param list The list of addressee, which shall be pasted.
     */
    void pasteContacts( KABC::Addressee::List &list );

    /**
      Sets the whoAmI contact, that is used by many other programs to
      get personal information about the current user.
     */
    void setWhoAmI();

    /**
      Displays the category dialog and applies the result to all
      selected contacts.
     */
    void setCategories();

    /**
      Sets the field list of the Incremental Search Widget.
     */
    void setSearchFields( const KABC::Field::List &fields );

    /**
      Search with the current search field for a contact, that matches
      the given text, and selects it in the view.
     */
    void incrementalSearch( const QString& text );

    /**
      Marks the address book as modified.
     */
    void setModified();

    /**
      Marks the address book as modified concerning the argument.
     */
    void setModified( bool modified );

    /**
      Returns whether the address book is modified.
     */
    bool modified() const;

    /**
      Called whenever an contact is modified in the contact editor
      dialog or the quick edit.
     */
    void contactModified( const KABC::Addressee &addr );

    /**
      DCOP METHOD: Adds the given email address to address book.
     */
    virtual void addEmail( const QString& addr );

    /**
      DCOP METHOD: Imports the vCard, located at the given url.
     */
    virtual void importVCard( const KURL& url );

    /**
      DCOP METHOD: Imports the given vCard.
     */
    virtual void importVCard( const QString& vCard );

    /**
      DCOP METHOD: Opens contact editor to input a new contact.
     */
    virtual void newContact();

    /**
      DCOP METHOD: Returns the name of the contact, that matches the given 
                   phone number.
     */
    virtual QString getNameByPhone( const QString& phone );

    /**
      Saves the contents of the AddressBook back to disk.
     */
    void save();

    /**
      Undos the last command using the undo stack.
     */
    void undo();

    /**
      Redos the last command that was undone, using the redo stack.
     */
    void redo();

    /**
      Shows the edit dialog for the given uid. If the uid is QString::null,
      the method will try to find a selected addressee in the view.
     */
    void editContact( const QString &uid = QString::null );

    /**
      Launches the configuration dialog.
     */
    void openConfigDialog();

    /**
      Launches the ldap search dialog.
     */
    void openLDAPDialog();

    /**
      Creates a KAddressBookPrinter, which will display the print
      dialog and do the printing.
     */
    void print();

  signals:
    void contactSelected( const QString &name );
    void contactSelected( const QPixmap &pixmap );

  private slots:
    void setJumpButtonBarVisible( bool visible );
    void setDetailsVisible( bool visible );

    void extensionModified( const KABC::Addressee::List &list );
    void clipboardDataChanged();
    void updateActionMenu();
    void configureKeyBindings();

    void slotEditorDestroyed( const QString &uid );
    void configurationChanged();
    void addressBookChanged();

  private:
    void initGUI();
    void initActions();

    AddresseeEditorDialog *createAddresseeEditorDialog( QWidget *parent,
                                                        const char *name = 0 );

    QWidget *mWidget;
    KABC::AddressBook *mAddressBook;

    ViewManager *mViewManager;
    ExtensionManager *mExtensionManager;
    XXPortManager *mXXPortManager;

    JumpButtonBar *mJumpButtonBar;
    FilterSelectionWidget *mFilterSelectionWidget;
    IncSearchWidget *mIncSearchWidget;
    ViewContainer *mDetails;
    QSplitter *mDetailsSplitter;
    QSplitter *mExtensionBarSplitter;

    KCMultiDialog *mConfigureDialog;
    LDAPSearchDialog *mLdapSearchDialog;
    QDict<AddresseeEditorDialog> mEditorDict;
    QMap<KABC::Resource*, ResourceMapEntry> mResourceMap;

    bool mReadWrite;
    bool mModified;
    bool mIsPart;

    KAction *mActionPaste;
    KAction *mActionCut;
    KAction *mActionDelete;
    KAction *mActionCopy;
    KAction *mActionEditAddressee;
    KAction *mActionMail;
    KAction *mActionMailVCard;
    KAction *mActionUndo;
    KAction *mActionRedo;
    KAction *mActionSave;
    KAction *mActionDeleteView;
    KAction *mActionWhoAmI;
    KAction *mActionCategories;
    KToggleAction *mActionJumpBar;
    KToggleAction *mActionDetails;

    KAddressBookService *mAddressBookService;

    class KABCorePrivate;
    KABCorePrivate *d;
};

#endif
