#include <qlayout.h>

#include <klocale.h>
#include <kdebug.h>

#include "stdaddressbook.h"

#include "addresseedialog.h"
#include "addresseedialog.moc"

using namespace KABC;

class AddresseeItem : public QListViewItem
{
  public:
    AddresseeItem( KListView *parent, const Addressee &addressee ) :
      QListViewItem( parent ),
      mAddressee( addressee )
    {
      setText( 0, addressee.realName() );
      setText( 1, addressee.preferredEmail() );
    }
    
    Addressee addressee() const
    {
      return mAddressee;
    }
    
  private:
    Addressee mAddressee;
};


AddresseeDialog::AddresseeDialog( QWidget *parent ) :
  KDialogBase( KDialogBase::Plain, i18n("Select Addressee"),
               Ok|Cancel, Ok, parent )
{
  QWidget *topWidget = plainPage();
  QBoxLayout *topLayout = new QVBoxLayout( topWidget );
  
  mAddresseeList = new KListView( topWidget );
  mAddresseeList->addColumn( i18n("Name") );
  mAddresseeList->addColumn( i18n("Email") );
  mAddresseeList->setAllColumnsShowFocus( true );
  topLayout->addWidget( mAddresseeList );
  connect( mAddresseeList, SIGNAL( doubleClicked( QListViewItem * ) ),
           SLOT( slotOk() ) );
  connect( mAddresseeList, SIGNAL( selectionChanged( QListViewItem * ) ),
           SLOT( updateEdit( QListViewItem * ) ) );
  
  mAddresseeEdit = new KLineEdit( topWidget );
  mAddresseeEdit->setCompletionMode( KGlobalSettings::CompletionAuto );
  connect( mAddresseeEdit->completionObject(), SIGNAL( match( const QString & ) ),
           SLOT( selectItem( const QString & ) ) );
  mAddresseeEdit->setFocus();
  topLayout->addWidget( mAddresseeEdit );

  mAddressBook = StdAddressBook::self();

  loadAddressBook();
}

AddresseeDialog::~AddresseeDialog()
{
}

void AddresseeDialog::loadAddressBook()
{
  mAddresseeList->clear();
  mItemDict.clear();
  mAddresseeEdit->completionObject()->clear();

  AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    AddresseeItem *item = new AddresseeItem( mAddresseeList, (*it) );
    addCompletionItem( (*it).realName(), item );
    addCompletionItem( (*it).preferredEmail(), item );
  }
}

void AddresseeDialog::addCompletionItem( const QString &str, QListViewItem *item )
{
  QString s = str.lower();
  mItemDict.insert( s, item );
  mAddresseeEdit->completionObject()->addItem( s );
}

void AddresseeDialog::selectItem( const QString &str )
{
  if ( str.isEmpty() ) return;

  QListViewItem *item = mItemDict.find( str );
  if ( item ) {
    mAddresseeList->blockSignals( true );
    mAddresseeList->setSelected( item, true );
    mAddresseeList->blockSignals( false );
  }
}

void AddresseeDialog::updateEdit( QListViewItem *item )
{
  mAddresseeEdit->setText( item->text( 0 ) );
  mAddresseeEdit->setSelection( 0, item->text( 0 ).length() );
}

Addressee AddresseeDialog::getAddressee( QWidget *parent )
{
  AddresseeDialog *dlg = new AddresseeDialog( parent );
  int result = dlg->exec();
  if ( result == QDialog::Accepted ) {
    QListViewItem *item = dlg->mAddresseeList->selectedItem();
    AddresseeItem *aItem = dynamic_cast<AddresseeItem *>( item );
    if ( !aItem ) return Addressee(); 
    return aItem->addressee();
  }

  return Addressee();
}
