// $Id$

#include <qlayout.h>
#include <qvbox.h>
#include <qlistbox.h>
#include <qwidget.h>
#include <qfile.h>
#include <qimage.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qevent.h>
#include <qurl.h>

#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kabc/addressbook.h>

#include "kaddressbooktableview.h"
#include "undocmds.h"
#include "contactlistview.h"
#include "kabprefs.h"

KAddressBookTableView::KAddressBookTableView( KABC::AddressBook *doc,
		      QWidget *parent,
		      const char *name )
  : KAddressBookView(doc, parent, name)
{
  mainLayout = new QVBoxLayout( viewWidget(), 2 );
  
  // The list view will be created when the config is read.
  mListView = 0;
}

KAddressBookTableView::~KAddressBookTableView()
{
}

void KAddressBookTableView::reconstructListView()
{
    if (mListView)
    {
        disconnect(mListView, SIGNAL(selectionChanged()),
                   this, SLOT(addresseeSelected()));
        disconnect(mListView, SIGNAL(executed(QListViewItem*)),
                   this, SLOT(addresseeExecuted(QListViewItem*)));
        disconnect(mListView, SIGNAL(doubleClicked(QListViewItem*)),
                   this, SLOT(addresseeExecuted(QListViewItem*)));
        disconnect(mListView, SIGNAL(startAddresseeDrag()), this,
                   SIGNAL(startDrag()));
        disconnect(mListView, SIGNAL(addresseeDropped(QDropEvent*)), this,
                   SIGNAL(dropped(QDropEvent*)));
        delete mListView;
    }

  mListView = new ContactListView( this, addressBook(), viewWidget() );
  
  // Add the columns
  KABC::Field::List fieldList = fields();
  KABC::Field::List::ConstIterator it;

  int c = 0;
  for( it = fieldList.begin(); it != fieldList.end(); ++it ) {
      mListView->addColumn( (*it)->label() );
      mListView->setColumnWidthMode(c++, QListView::Manual);
  }
 
  connect(mListView, SIGNAL(selectionChanged()),
          this, SLOT(addresseeSelected()));
  connect(mListView, SIGNAL(startAddresseeDrag()), this,
          SIGNAL(startDrag()));
  connect(mListView, SIGNAL(addresseeDropped(QDropEvent*)), this,
          SIGNAL(dropped(QDropEvent*)));
  if (KABPrefs::instance()->mHonorSingleClick)
    connect(mListView, SIGNAL(executed(QListViewItem*)),
          this, SLOT(addresseeExecuted(QListViewItem*)));
  else
    connect(mListView, SIGNAL(doubleClicked(QListViewItem*)),
          this, SLOT(addresseeExecuted(QListViewItem*)));
          
  refresh();
  
  mListView->setSorting( 0, true );
  mainLayout->addWidget( mListView );
  mainLayout->activate();
  mListView->show();
}

void KAddressBookTableView::writeConfig(KConfig *config)
{
  KAddressBookView::writeConfig(config);

  mListView->saveLayout(config, config->group());
}

void KAddressBookTableView::readConfig(KConfig *config)
{
  QString group = config->group();

  KAddressBookView::readConfig(config);
  
  // The config could have changed the fields, so we need to reconstruct
  // the listview.
  reconstructListView();

  // reconstructListView modifies config-group
  config->setGroup( group );
  
  // Set the list view options
  mListView->setAlternateBackgroundEnabled(config->readBoolEntry("ABackground",
                                                                 true));
  mListView->setSingleLineEnabled(config->readBoolEntry("SingleLine", false));
  mListView->setToolTipsEnabled(config->readBoolEntry("ToolTips", true));
  
  if (config->readBoolEntry("Background", false))
    mListView->setBackgroundPixmap(config->readEntry("BackgroundName"));
  
  // Restore the layout of the listview
  mListView->restoreLayout(config, config->group());
}

void KAddressBookTableView::refresh(QString uid)
{
  // For now just repopulate. In reality this method should 
  // check the value of uid, and if valid iterate through
  // the listview to find the entry, then tell it to refresh.

  if (uid == QString::null) {
    // Clear the list view
    mListView->clear();
    
    KABC::Addressee::List addresseeList = addressees();
    KABC::Addressee::List::Iterator it;
    for (it = addresseeList.begin(); it != addresseeList.end(); ++it ) {
      new ContactListViewItem(*it, mListView, addressBook(), fields());
    }
        
    // Sometimes the background pixmap gets messed up when we add lots
    // of items.
    mListView->repaint();
  } else {
    // Only need to update on entry. Iterate through and try to find it
    ContactListViewItem *ceItem;
    QListViewItemIterator it( mListView );
    while ( it.current() ) {
      ceItem = dynamic_cast<ContactListViewItem*>( it.current() );
      if ( ceItem && ceItem->addressee().uid() == uid ) {
        ceItem->refresh();
        return;
      }
      ++it;
    }

    refresh( QString::null );
  }
}

QStringList KAddressBookTableView::selectedUids()
{
    QStringList uidList;
    QListViewItem *item;
    ContactListViewItem *ceItem;
    
    for(item = mListView->firstChild(); item; item = item->itemBelow()) 
    {
        if (mListView->isSelected( item ))
        {
            ceItem = dynamic_cast<ContactListViewItem*>(item);
            if (ceItem != 0L)
                uidList << ceItem->addressee().uid();
        }
    }
    
    return uidList;
}

void KAddressBookTableView::setSelected(QString uid, bool selected)
{
    QListViewItem *item;
    ContactListViewItem *ceItem;
    
    if (uid == QString::null)
    {
        mListView->selectAll(selected);
    }
    else
    {
        for(item = mListView->firstChild(); item; item = item->itemBelow())
        {
            ceItem = dynamic_cast<ContactListViewItem*>(item);
            if ((ceItem != 0L) && (ceItem->addressee().uid() == uid))
            {
                mListView->setSelected(item, selected);
                
                if (selected)
                    mListView->ensureItemVisible(item);
            }
        }
    }
}

void KAddressBookTableView::incrementalSearch(const QString &value, 
                                              KABC::Field *field)
{
    if ( value.isEmpty() ) {
      mListView->clearSelection();
      return;
    }

    KABC::Field::List fieldList = fields();
    KABC::Field::List::ConstIterator it;
    int column = 0;
    for( it = fieldList.begin(); it != fieldList.end(); ++it ) {
      if ( (*it)->equals( field ) ) break;
      ++column;
    }
    
    if ( it == fieldList.end() ) column = 0;

    // Now do the inc search
    mListView->setCurrentItem( mListView->firstChild() );
    QListViewItem *item = mListView->findItem(value, column, Qt::BeginsWith);
    
    if (item)
    {
        // We have a match. Deselect all the others and select this one

        // avoid flickering in details page
        bool blocked = signalsBlocked();
        blockSignals( true );
        mListView->clearSelection();
        blockSignals( blocked );

        mListView->setSelected(item, true);
        mListView->ensureItemVisible(item);
    }
}

void KAddressBookTableView::addresseeSelected()
{
    // We need to try to find the first selected item. This might not be the
    // last selected item, but when QListView is in multiselection mode,
    // there is no way to figure out which one was
    // selected last.
    QListViewItem *item;
    bool found =false;
    for (item = mListView->firstChild(); item && !found; 
         item = item->nextSibling())
    {
        if (item->isSelected())
        {
            found = true;
            ContactListViewItem *ceItem 
                 = dynamic_cast<ContactListViewItem*>(item);
             emit selected(ceItem->addressee().uid());
        }
    }
    
    if (!found)
        emit selected(QString::null);
}

void KAddressBookTableView::addresseeExecuted(QListViewItem *item)
{
    if (item)
    {
        ContactListViewItem *ceItem 
               = dynamic_cast<ContactListViewItem*>(item);
               
       if (ceItem)
       {
           emit executed(ceItem->addressee().uid());
       }
    }
    else
    {
        emit executed(QString::null);
    }
}

#include "kaddressbooktableview.moc"
