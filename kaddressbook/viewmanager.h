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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <qdict.h>
#include <qstringlist.h>
#include <qwidget.h>

#include <kabc/field.h>
#include <ktrader.h>

#include "filter.h"
#include "kaddressbookview.h"

class QComboBox;
class QDropEvent;
class QHBox;
class QLineEdit;
class QResizeEvent;
class QSplitter;
class QTabWidget;
class QWidgetStack;

class KConfig;

namespace KABC { class AddressBook; }

class AddresseeEditorWidget;
class ExtensionWidget;
class IncSearchWidget;
class JumpButtonBar;
class ViewContainer;

/**
  The view manager manages the views and everything related to them. The
  manager will load the views at startup and display a view when told to
  make one active.

  The view manager will also create and manage all dialogs directly related to
  views (ie: AddView, ConfigureView, DeleteView, etc).
 */
class ViewManager : public QWidget
{
  Q_OBJECT

  public:
    ViewManager( KABC::AddressBook *doc, KConfig *config,
                 QWidget *parent = 0, const char *name = 0 );
    ~ViewManager();

    /**
      Sets the given view active. This usually means raising the
      view to the top of the widget stack and telling it
      to refresh.
     */
    void setActiveView( const QString &name );

    /**
      Destroys all the currently loaded views. It is important that
      after calling this method you call setActiveView before
      the user has a chance to interact with the gui, since no
      views will be loaded.
     */
    void unloadViews();

    /**
      Returns a list of all the defined views. This list is guaranteed
      to always contain at least one view. This list is the 'defined' views,
      not necessarily the loaded views. However the view will be loaded if
      it becomes active.
     */
    const QStringList &viewNames();

    /**
      Return a list of all the uids of selected contacts.
     */
    QStringList selectedUids() const;

    /**
      Used to enable or disable the jump button bar.

      @param visible True for the widget to be visible, false otherwise
     */
    void setJumpButtonBarVisible( bool visible );

    /**
      Used to enable or disable the details widget.

      @param visible True for the widget to be visible, false otherwise
     */
    void setDetailsVisible( bool visible );

    /**
      Return if the quick edit currently is shown or not.
     */
    bool isQuickEditVisible();

    /**
      Set the list of filters, which are defined for the application.
     */
    void setFilters( const Filter::List &list );

    /**
      @return The list of filters defined for the application.
     */
    const Filter::List &filters() const;

    /**
      @return The name list of all filters defined for the application.
     */
    QStringList filterNames();

    /**
      @return The name list of all registered extension widgets.
     */
    QStringList extensionNames();

    /**
      Returns the address book.
     */
    KABC::AddressBook *addressBook();

    /**
      Returns the current global search field.
     */
    KABC::Field *currentSearchField();

    /**
      @return KConfig for kaddressbookrc, useful for when KAddressBook
      is used as a KPart
     */
    static KConfig *config();

  public slots:
    /**
      Reads the config file.
     */
    void readConfig();

    /**
      Writes the config file.
     */
    void writeConfig();

    /**
      Sends an email to all the selected addressees. This is done by
      asking the view for a string of "To:'s" and then asking
      KDE to open the mailer with the information.
     */
    virtual void sendMail();

    /**
      Open a composer with a message to this person.
     */
    void sendMail( const QString& email );

    /** Open a composer with the selected contact's vcard attached. */
    void mailVCard();

    /** Open a composer with those contacts' vcards attached. */
    void mailVCard(const QStringList& uids);

    /**
      Open a browser window displaying the URL given.
     */
    void browse( const QString& url );

    /**
      This slot will delete all the selected entries. This method should
      be called just 'delete' to be consistant with the other edit methods,
      but good 'ol C++ wouldn't like that -mpilone
     */
    void deleteAddressees();

    /**
      Copy a contact from the view into the clipboard. This method will
      copy all selected contacts into the clipboard at once.
     */
    void copy();

    /**
      Cut a contact from the view into the clipboard. This method will
      cut all selected contacts into the clpboard at once.
     */
    void cut();

    /**
      Paste a contact into the addressbook from the clipboard.
     */
    void paste();

    /**
      Selects the given addressee or all addressees if uid == QString::null
     */
    void setSelected( const QString &uid = QString::null, bool selected = true );

    /**
      Refreshes the active view.

      @param uid Only refresh the selected uid. If it is QStrign::null, the
                 entire view will be refreshed.
     */
    void refresh( const QString &uid = QString::null );

    /**
      Launches the ConfigureView dialog for the active view.
     */
    void modifyView();

    /**
      Deletes the current view and makes another view active.
     */
    void deleteView();

    /**
      Displays the add view dialog. If the user confirms, the view
      will be added.
     */
    void addView();

    /**
      Called whenever a filter is activated.
     */
    void filterActivated( int index );

    /**
      The address book has been modified and needs to be saved.
     */
    void slotModified();

    /**
      Set the active widget of the extension bar.

      @param id 0: hide extension bar, otherwise an extension widget is shown
                   according to the list that is returned by
                   @ref extensionNames()
     */
    void setActiveExtension( int id );

    /**
      Set users contact.
     */
    void setUsersContact();

  protected slots:
    /**
      Handle events on the incremental search widget.
     */
    void incSearch( const QString& text );

    /**
      Called whenever the user drops something in the active view.
      This method will try to decode what was dropped, and if it was
      a valid addressee, add it to the addressbook.
     */
    void dropped( QDropEvent* );

    /**
      Called whenever the user attempts to start a drag in the view.
      This method will convert all the selected addressees into text (vcard)
      and create a drag object.
     */
    void startDrag();

    /**
      Called whenever an addressee is selected in the view. This method
      should update the quick edit. The selected() signal will already
      be emitted, so it does not have to be re-emitted from this method.
     */
    void addresseeSelected( const QString &uid );

    /**
      Called whenever the currently displayed extension bar widget is modified.
      This method will emit the modified signal and then tell the view to
      refresh.
     */
    void extensionWidgetModified( KABC::Addressee::List );

  signals:
    /**
      Emitted whenever the user selects an entry in the view.
     */
    void selected( const QString &uid );

    /**
      Emitted whenever the user activates an entry in the view.
     */
    void executed( const QString &uid );

    /**
      Emitted whenever the address book is modified in some way.
     */
    void modified();

    /**
      Emitted whenever the view configuration changes. This can happen
      if a user adds a new view or removes a view.

      @param newActive This is the view that should be made active. If this
                       is QString::null, than the current active can remain
                       that way.
     */
    void viewConfigChanged( const QString &newActive );

    /**
      Emitted whenever the filter combo bar should reread the filter name
      list from the viewmanager.
     */
    void filtersEdited();

    /**
      Emitted whenever the filter combo bar should change its
      current filter.

      @param name Is the name of the new filter.
                  name.isEmpty() for no filter.
     */
    void currentFilterChanged( const QString &name );

    /**
      Import a VCard that has been dragged.
     */
    void importVCard( const KURL&, bool );

    /**
      Emitted whenever the extension names need to be updated
      in the action manager.
     */
    void extensionsReloaded();

  private:
    void loadExtensions();

    void createViewFactories();
    void initGUI();

    Filter mCurrentFilter;
    Filter::List mFilterList;

    KABC::AddressBook *mAddressBook;

    KConfig *mConfig;

    QDict<KAddressBookView> mViewDict;
    QDict<ViewFactory> mViewFactoryDict;
    QStringList mViewNameList;
    QWidgetStack *mViewWidgetStack;

    ExtensionWidget *mCurrentExtensionWidget;
    JumpButtonBar *mJumpButtonBar;
    IncSearchWidget *mIncSearchWidget;
    KAddressBookView *mActiveView;
    ViewContainer *mDetails;
    QHBox *mExtensionBar;
    QPtrList<ExtensionWidget> mExtensionWidgetList;
    QSplitter *mDetailsSplitter;
    QSplitter *mExtensionBarSplitter;
};

#endif
