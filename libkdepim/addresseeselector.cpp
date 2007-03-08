/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <q3header.h>
#include <QLabel>
#include <QLayout>
#include <QSignalMapper>
#include <QToolButton>
//Added by qt3to4:
#include <QPixmap>
#include <QGridLayout>
#include <QVBoxLayout>

#include <kabc/stdaddressbook.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <k3listview.h>
#include <klocale.h>

#include "addresseeselector.h"

using namespace KPIM;

class AddresseeSelector::AddressBookManager
{
  public:
    QStringList titles() const;

    void addResource( KABC::Resource* );
    void addAddressBook( const QString &title, SelectionItem::List &list );

    void clear();
    bool contains( int index, const SelectionItem& );

  private:
    struct AddressBookEntry {
      QString title;
      SelectionItem::List list;
    };

    QList<KABC::Resource*> mResources;
    QList<AddressBookEntry> mAddressBooks;
};

QStringList AddresseeSelector::AddressBookManager::titles() const
{
  QStringList titles;

  // we've always an 'all' entry
  titles.append( i18n( "All" ) );

  QList<KABC::Resource*>::ConstIterator resIt;
  for ( resIt = mResources.begin(); resIt != mResources.end(); ++resIt )
    titles.append( (*resIt)->resourceName() );

  QList<AddressBookEntry>::ConstIterator abIt;
  for ( abIt = mAddressBooks.begin(); abIt != mAddressBooks.end(); ++abIt )
    titles.append( (*abIt).title );

  return titles;
}

void AddresseeSelector::AddressBookManager::addResource( KABC::Resource *resource )
{
  if ( !mResources.contains( resource )  )
    mResources.append( resource );
}

void AddresseeSelector::AddressBookManager::addAddressBook( const QString &title,
                                                            SelectionItem::List &list  )
{
  AddressBookEntry entry;
  entry.title = title;
  entry.list = list;


  // TODO: check for duplicates
  mAddressBooks.append( entry );
}

void AddresseeSelector::AddressBookManager::clear()
{
  mResources.clear();
  mAddressBooks.clear();
}

bool AddresseeSelector::AddressBookManager::contains( int index, const SelectionItem &item )
{
  if ( index == 0 ) // the 'all' entry
    return true;

  if ( mResources.count() > 0 ) {
    if ( index <= mResources.count() ) {
      index--;
      if ( item.addressee().resource() == mResources[ index ] )
        return true;
      else
        return false;
    }
  }

  index = index - mResources.count();

  if ( mAddressBooks.count() > 0 ) {
    if ( index <= mAddressBooks.count() ) {
      index--;
      AddressBookEntry entry = mAddressBooks[ index ];
      SelectionItem::List::ConstIterator it;
      for ( it = entry.list.begin(); it != entry.list.end(); ++it )
        if ( (*it).addressee() == item.addressee() )
          return true;

      return false;
    }
  }

  return false;
}


SelectionItem::SelectionItem( const KABC::Addressee &addressee, int index )
  : mAddressee( addressee ), mDistributionList( 0 ), mIndex( index )
{
  mField.fill( false, 10 );
}

SelectionItem::SelectionItem( KABC::DistributionList *list, int index )
  : mDistributionList( list ), mIndex( index )
{
  mField.fill( false, 10 );
}

SelectionItem::SelectionItem()
  : mDistributionList( 0 ), mIndex( 0 )
{
  mField.fill( false, 10 );
}

void SelectionItem::addToField( int index )
{
  mField.setBit( index );
}

void SelectionItem::removeFromField( int index )
{
  mField.clearBit( index );
}

bool SelectionItem::isInField( int index )
{
  return mField.testBit( index );
}

KABC::Addressee SelectionItem::addressee() const
{
  return mAddressee;
}

KABC::DistributionList* SelectionItem::distributionList() const
{
  return mDistributionList;
}

int SelectionItem::index() const
{
  return mIndex;
}


class SelectionViewItem : public Q3ListViewItem
{
  public:
    SelectionViewItem( Q3ListView *parent, Selection *selection,
                       SelectionItem *item )
      : Q3ListViewItem( parent, "" ), mSelection( selection ), mItem( item )
    {
      if ( mItem->distributionList() == 0 )
        mIcon = mSelection->itemIcon( mItem->addressee(), mItem->index() );
      else
        mIcon = mSelection->distributionListIcon( mItem->distributionList() );
    }

    QString text( int column ) const
    {
      if ( column == 0 ) {
        if ( mItem->distributionList() == 0 )
          return mSelection->itemText( mItem->addressee(), mItem->index() );
        else
          return mSelection->distributionListText( mItem->distributionList() );
      } else
        return QString();
    }

    const QPixmap* pixmap( int column ) const
    {
      if ( column == 0 ) {
        return &mIcon;
      } else
        return 0;
    }

    SelectionItem* item() const { return mItem; }

  private:
    Selection *mSelection;
    SelectionItem *mItem;
    QPixmap mIcon;
};

AddresseeSelector::AddresseeSelector( Selection *selection, QWidget *parent, const char *name )
  : QWidget( parent ), mSelection( selection ), mManager( 0 )
{
  setObjectName(name);
  mMoveMapper = new QSignalMapper( this );
  mRemoveMapper = new QSignalMapper( this );

  mAddressBookManager = new AddressBookManager();

  initGUI();

  init();

  mSelection->setSelector( this );
}

AddresseeSelector::~AddresseeSelector()
{
  delete mManager;
  mManager = 0;

  delete mAddressBookManager;
  mAddressBookManager = 0;
}

void AddresseeSelector::init()
{
  connect( KABC::StdAddressBook::self( true ), SIGNAL( addressBookChanged( AddressBook* ) ),
           this, SLOT( reloadAddressBook() ) );
  connect( mAddresseeFilter, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( updateAddresseeView() ) );
  connect( mAddressBookCombo, SIGNAL( activated( int ) ),
           this, SLOT( updateAddresseeView() ) );

  connect( mMoveMapper, SIGNAL( mapped( int ) ),
           this, SLOT( move( int ) ) );
  connect( mRemoveMapper, SIGNAL( mapped( int ) ),
           this, SLOT( remove( int ) ) );

  reloadAddressBook();
}

void AddresseeSelector::initGUI()
{
  QGridLayout *layout = new QGridLayout( this );
  layout->setMargin( KDialog::marginHint() );
  layout->setSpacing( KDialog::spacingHint() );

  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setMargin(KDialog::marginHint() );

  QLabel *label = new QLabel( i18n( "Address book:" ), this );
  mAddressBookCombo = new KComboBox( false, this );

  topLayout->addWidget( label, 0, 0 );
  topLayout->addWidget( mAddressBookCombo, 0, 1 );

  label = new QLabel( i18n( "Search:" ), this );
  mAddresseeFilter = new KLineEdit( this );

  topLayout->addWidget( label, 1, 0 );
  topLayout->addWidget( mAddresseeFilter, 1, 1 );

  topLayout->setColumnStretch( 1, 1 );

  layout->addLayout( topLayout, 0, 0, 1, 3 );

  int row = 1;

  KIcon moveSet( "find-next", KIconLoader::global() );
  KIcon removeSet( "find-previous", KIconLoader::global() );

  int count = mSelection->fieldCount();
  for ( int i = 0; i < count; ++i, ++row ) {
    K3ListView *listView = new K3ListView( this );
    listView->addColumn( mSelection->fieldTitle( i ) );
    listView->setFullWidth( true );
    mSelectionViews.append( listView );

    connect( listView, SIGNAL( doubleClicked( Q3ListViewItem*, const QPoint&, int ) ),
             mRemoveMapper, SLOT( map() ) );
    mRemoveMapper->setMapping( listView, i );

    QVBoxLayout *buttonLayout = new QVBoxLayout( this );
    buttonLayout->setAlignment( Qt::AlignBottom );
    layout->addLayout( buttonLayout, row, 1 );

    // move button
    QToolButton *moveButton = new QToolButton( this );
    moveButton->setIcon( moveSet );
    moveButton->setFixedSize( 30, 30 );

    connect( moveButton, SIGNAL( clicked() ),
             mMoveMapper, SLOT( map() ) );
    mMoveMapper->setMapping( moveButton, i );

    // remove button
    QToolButton *removeButton = new QToolButton( this );
    removeButton->setIcon( removeSet );
    removeButton->setFixedSize( 30, 30 );

    connect( removeButton, SIGNAL( clicked() ),
             mRemoveMapper, SLOT( map() ) );
    mRemoveMapper->setMapping( removeButton, i );

    buttonLayout->addWidget( moveButton );
    buttonLayout->addWidget( removeButton );

    layout->addWidget( listView, row, 2 );
  }

  mAddresseeView = new K3ListView( this );
  mAddresseeView->addColumn( "" );
  mAddresseeView->header()->hide();
  mAddresseeView->setFullWidth( true );

  layout->addWidget( mAddresseeView, 1, 0, row, 1);
}

void AddresseeSelector::finish()
{
  SelectionItem::List::Iterator it;

  for ( int field = 0; field < mSelection->fieldCount(); ++field ) {
    for ( it = mSelectionItems.begin(); it != mSelectionItems.end(); ++it ) {
      if ( (*it).isInField( field ) ) {
        if ( (*it).distributionList() == 0 )
          mSelection->addSelectedAddressees( field, (*it).addressee(), (*it).index() );
        else
          mSelection->addSelectedDistributionList( field, (*it).distributionList() );
      }
    }
  }
}

void AddresseeSelector::updateAddresseeView()
{
  mAddresseeView->clear();

  int addressBookIndex = mAddressBookCombo->currentIndex();

  SelectionItem::List::Iterator it;
  for ( it = mSelectionItems.begin(); it != mSelectionItems.end(); ++it ) {
    if ( mAddressBookManager->contains( addressBookIndex, *it ) ) {
      if ( (*it).distributionList() == 0 ) {
        if ( mAddresseeFilter->text().isEmpty() ||
             mSelection->itemMatches( (*it).addressee(), (*it).index(),
                                      mAddresseeFilter->text() ) )
          new SelectionViewItem( mAddresseeView, mSelection, &(*it) );
      } else {
        if ( mAddresseeFilter->text().isEmpty() ||
             mSelection->distributionListMatches( (*it).distributionList(),
                                                  mAddresseeFilter->text() ) )
          new SelectionViewItem( mAddresseeView, mSelection, &(*it) );
      }
    }
  }

  updateSelectionViews();
}

void AddresseeSelector::move( int index )
{
  SelectionViewItem *item = dynamic_cast<SelectionViewItem*>( mAddresseeView->selectedItem() );
  if ( item ) {
    item->item()->addToField( index );
    updateSelectionView( index );
  }
}

void AddresseeSelector::remove( int index )
{
  K3ListView *view = mSelectionViews[ index ];

  SelectionViewItem *item = dynamic_cast<SelectionViewItem*>( view->selectedItem() );
  if ( item ) {
    item->item()->removeFromField( index );
    updateSelectionView( index );
  }
}

void AddresseeSelector::setItemSelected( int fieldIndex, const KABC::Addressee &addr, int itemIndex )
{
  bool found = false;

  SelectionItem::List::Iterator it;
  for ( it = mSelectionItems.begin(); it != mSelectionItems.end(); ++it ) {
    if ( (*it).addressee() == addr && (*it).index() == itemIndex ) {
      (*it).addToField( fieldIndex );
      found = true;
    }
  }

  if ( !found ) {
    SelectionItem item( addr, itemIndex );
    item.addToField( fieldIndex );

    mSelectionItems.append( item );
  }

  updateSelectionView( fieldIndex );
}

void AddresseeSelector::setItemSelected( int fieldIndex, const KABC::Addressee &addr,
                                         int itemIndex, const QString &text )
{
  bool found = false;

  SelectionItem::List::Iterator it;
  for ( it = mSelectionItems.begin(); it != mSelectionItems.end(); ++it ) {
    if ( mSelection->itemEquals( (*it).addressee(), (*it).index(), text ) ) {
      (*it).addToField( fieldIndex );
      found = true;
    }
  }

  if ( !found ) {
    SelectionItem item( addr, itemIndex );
    item.addToField( fieldIndex );

    mSelectionItems.append( item );
  }

  updateSelectionView( fieldIndex );
}

void AddresseeSelector::updateSelectionView( int index )
{
  K3ListView *view = mSelectionViews[ index ];
  view->clear();

  SelectionItem::List::Iterator it;
  for ( it = mSelectionItems.begin(); it != mSelectionItems.end(); ++it ) {
    if ( (*it).isInField( index ) )
      new SelectionViewItem( view, mSelection, &(*it) );
  }
}

void AddresseeSelector::updateSelectionViews()
{
  for ( int i = 0; i < mSelection->fieldCount(); ++i )
    updateSelectionView( i );
}

void AddresseeSelector::reloadAddressBook()
{
  // load contacts
  KABC::Addressee::List list = KABC::StdAddressBook::self( true )->allAddressees();
  KABC::Addressee::List::Iterator it;

  SelectionItem::List selectedItems;

  SelectionItem::List::Iterator itemIt;
  for ( itemIt = mSelectionItems.begin(); itemIt != mSelectionItems.end(); ++itemIt ) {
    bool isSelected = false;
    for ( int i = 0; i < mSelection->fieldCount(); ++i ) {
      if ( (*itemIt).isInField( i ) ) {
        isSelected = true;
        break;
      }
    }

    // we don't save distribution lists, since this leads to crashes
    if ( isSelected && (*itemIt).distributionList() == 0 ) {
      selectedItems.append( *itemIt );
    }
  }

  mSelectionItems.clear();
  mSelectionItems = selectedItems;

  for ( it = list.begin(); it != list.end(); ++it ) {
    int itemCount = mSelection->itemCount( *it );
    for ( int index = 0; index < itemCount; ++index ) {
      bool available = false;
      for ( itemIt = mSelectionItems.begin(); itemIt != mSelectionItems.end(); ++itemIt ) {
        if ( (*itemIt).addressee() == (*it) && (*itemIt).index() == index ) {
          available = true;
          break;
        }
      }

      if ( !available ) {
        SelectionItem item( *it, index );
        mSelectionItems.append( item );
      }
    }
  }

  // load distribution lists
  delete mManager;
  mManager = new KABC::DistributionListManager( KABC::StdAddressBook::self( true ) );

  mManager->load();

  QStringList lists = mManager->listNames();

  QStringList::Iterator listIt;
  for ( listIt = lists.begin(); listIt != lists.end(); ++listIt ) {
    KABC::DistributionList *list = mManager->list( *listIt );
    SelectionItem item( list, 0 );
    mSelectionItems.append( item );
  }

  mAddressBookManager->clear();

  // update address book combo
  mAddressBookCombo->clear();

  QList<KABC::Resource*> resources = KABC::StdAddressBook::self( true )->resources();
  QListIterator<KABC::Resource*> resIt( resources );
  while ( resIt.hasNext() ) {
    KABC::Resource *res = resIt.next();
    if ( res->isActive() )
      mAddressBookManager->addResource( res );
  }

  for ( int i = 0; i < mSelection->addressBookCount(); ++i ) {
    SelectionItem::List itemList;

    KABC::Addressee::List addrList = mSelection->addressBookContent( i );
    for ( it = addrList.begin(); it != addrList.end(); ++it ) {
      int itemCount = mSelection->itemCount( *it );
      for ( int index = 0; index < itemCount; ++index ) {
        SelectionItem item( *it, index );
        mSelectionItems.append( item );
        itemList.append( item );
      }
    }

    mAddressBookManager->addAddressBook( mSelection->addressBookTitle( i ),
                                         itemList );
  }

  mAddressBookCombo->addItems( mAddressBookManager->titles() );

  updateAddresseeView();
}


AddresseeSelectorDialog::AddresseeSelectorDialog( Selection *selection,
                                                  QWidget *parent )
  : KDialog( parent )
{
  setCaption( "" );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );
  setModal( true );
  QFrame *frame = new QFrame( this );
  setMainWidget( frame );
  QVBoxLayout *layout = new QVBoxLayout( frame );
  mSelector = new KPIM::AddresseeSelector( selection, frame );
  layout->addWidget( mSelector );

  resize( 500, 490 );
}

void AddresseeSelectorDialog::accept()
{
  mSelector->finish();
  QDialog::accept();
}

#include "addresseeselector.moc"
