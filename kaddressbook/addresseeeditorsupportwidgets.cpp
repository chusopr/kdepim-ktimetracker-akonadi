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

#include "addresseeeditorsupportwidgets.h"

#include <qbuttongroup.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qstring.h>

#include <kapplication.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klineedit.h>
#include <klistview.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdialog.h>

/////////////////////////////////////
// EmailWidget

EmailEditWidget::EmailEditWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  QGridLayout *topLayout = new QGridLayout(this, 2, 2);

  QLabel *label = new QLabel(i18n("Preferred email address:"), this);
  topLayout->addWidget(label, 0, 0);

  mEmailEdit = new KLineEdit(this);
  connect(mEmailEdit, SIGNAL( textChanged(const QString &) ), SIGNAL( modified() ) );
  topLayout->addWidget(mEmailEdit, 0, 1);

  QPushButton *editButton = new QPushButton(i18n("Edit email addresses..."), this);
  connect(editButton, SIGNAL(clicked()), SLOT(edit()));
  topLayout->addMultiCellWidget(editButton, 1, 1, 0, 1);

  topLayout->activate();
}

EmailEditWidget::~EmailEditWidget()
{
}
    
void EmailEditWidget::setEmails(const QStringList &list)
{
  mEmailList = list;

  if ( list.count() > 0 ) {
    bool blocked = mEmailEdit->signalsBlocked();
    mEmailEdit->blockSignals( true );
    mEmailEdit->setText( list[ 0 ] );
    mEmailEdit->blockSignals( blocked );
  }
}

QStringList EmailEditWidget::emails()
{
  if ( mEmailEdit->text().isEmpty() ) {
    if ( mEmailList.count() > 0 )
      mEmailList.remove( mEmailList.begin() );
  } else {
    if ( mEmailList.count() > 0 )
      mEmailList.remove( mEmailList.begin() );

    mEmailList.prepend( mEmailEdit->text() );
  }

  return mEmailList;
}

void EmailEditWidget::edit()
{
  EmailEditDialog dlg( mEmailList, this );
  
  if ( dlg.exec() ) {
    mEmailList = dlg.emails();
    mEmailEdit->setText( mEmailList[ 0 ] );
    emit modified();
  }
}

/////////////////////////////////////
// EmailEditDialog

EmailEditDialog::EmailEditDialog( const QStringList &list, QWidget *parent, const char *name )
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Email Addresses" ),
                KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                parent, name, true)
{
  QWidget *page = plainPage();

  QGridLayout *topLayout = new QGridLayout( page, 4, 3 );

  QLabel *label = new QLabel(i18n("Email address:"), page);
  topLayout->addWidget(label, 0, 0);

  mEmailEdit = new KLineEdit(page);
  topLayout->addWidget(mEmailEdit, 0, 1);
  connect(mEmailEdit, SIGNAL(returnPressed()), SLOT(add()));

  QPushButton *mAddButton = new QPushButton( i18n("Add"), page );
  connect(mAddButton, SIGNAL(clicked()), SLOT(add()));
  topLayout->addWidget(mAddButton, 0, 2);

  mEmailListBox = new QListBox( page );
  mEmailListBox->insertStringList( list );

  // Make sure there is room for the scrollbar
  mEmailListBox->setMinimumHeight(mEmailListBox->sizeHint().height() + 30);
  connect(mEmailListBox, SIGNAL(highlighted(int)), 
          SLOT(selectionChanged(int)));
  topLayout->addMultiCellWidget(mEmailListBox, 1, 3, 0, 1);
  
  mEditButton = new QPushButton(i18n("Change"), page);
  connect(mEditButton, SIGNAL(clicked()), SLOT(edit()));
  topLayout->addWidget(mEditButton, 1, 2);

  mRemoveButton = new QPushButton(i18n("Remove..."), page);
  connect(mRemoveButton, SIGNAL(clicked()), SLOT(remove()));
  topLayout->addWidget(mRemoveButton, 2, 2);

  mStandardButton = new QPushButton(i18n("Set Standard"), page);
  connect(mStandardButton, SIGNAL(clicked()), SLOT(standard()));
  topLayout->addWidget(mStandardButton, 3, 2);

  topLayout->activate();
  
  // set default state
  selectionChanged(-1);
}

EmailEditDialog::~EmailEditDialog()
{
}
    
QStringList EmailEditDialog::emails() const
{
  QStringList emails;
  
  for (unsigned int i = 0; i < mEmailListBox->count(); ++i)
    emails << mEmailListBox->text(i);
  
  return emails;
}

void EmailEditDialog::add()
{
  if (!mEmailEdit->text().isEmpty())
    mEmailListBox->insertItem(mEmailEdit->text());

  mEmailEdit->clear();
  mEmailEdit->setFocus();
}

void EmailEditDialog::edit()
{
  mEmailEdit->setText(mEmailListBox->currentText());
  mEmailEdit->setFocus();
}

void EmailEditDialog::remove()
{
  QString address = mEmailListBox->currentText();
  
  QString text = i18n("Are you sure that you want to remove the email address \"%1\"?").arg( address );
  
  QString caption = i18n("Confirm Remove");
  
  if (KMessageBox::questionYesNo(this, text, caption) == KMessageBox::Yes)
    mEmailListBox->removeItem(mEmailListBox->currentItem());
}

void EmailEditDialog::standard()
{
  QString text = mEmailListBox->currentText();
  mEmailListBox->removeItem(mEmailListBox->currentItem());
  mEmailListBox->insertItem(text, 0);
  mEmailListBox->setSelected(0, true);
}

void EmailEditDialog::selectionChanged(int index)
{
  bool value = (index >= 0); // An item is selected

  mRemoveButton->setEnabled(value);
  mEditButton->setEnabled(value);
  mStandardButton->setEnabled(value);
}

/////////////////////////////////////
// NameEditDialog

NameEditDialog::NameEditDialog(const QString &familyName, 
                               const QString &givenName,
                               const QString &prefix, const QString &suffix,
                               const QString &additionalName, 
                               QWidget *parent, const char *name)
  : KDialogBase(KDialogBase::Plain, i18n("Edit Contact Name"),
                KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                parent, name, true)
{
  QWidget *page = plainPage();
  QGridLayout *layout = new QGridLayout(page);
  layout->setSpacing(spacingHint());
  layout->setMargin(marginHint());
  layout->addColSpacing(2, 50);
  QLabel *label;
  
  label = new QLabel(i18n("Honorific prefixes:"), page);
  layout->addWidget(label, 0, 0);
  mPrefixCombo = new KComboBox(page, "mPrefixCombo");
  mPrefixCombo->setDuplicatesEnabled(false);
  mPrefixCombo->setEditable(true);
  layout->addWidget(mPrefixCombo, 0, 1);
  
  label = new QLabel(i18n("Given name:"), page);
  layout->addWidget(label, 1, 0);
  mGivenNameEdit = new KLineEdit(page, "mGivenNameEdit");
  layout->addMultiCellWidget(mGivenNameEdit, 1, 1, 1, 2);

  label = new QLabel(i18n("Additional names:"), page);
  layout->addWidget(label, 2, 0);
  mAdditionalNameEdit = new KLineEdit(page, "mAdditionalNameEdit");
  layout->addMultiCellWidget(mAdditionalNameEdit, 2, 2, 1, 2);
  
  label = new QLabel(i18n("Family names:"), page);
  layout->addWidget(label, 3, 0);
  mFamilyNameEdit = new KLineEdit(page, "mFamilyNameEdit");
  layout->addMultiCellWidget(mFamilyNameEdit, 3, 3, 1, 2);
  
  label = new QLabel(i18n("Honorific suffixes:"), page);
  layout->addWidget(label, 4, 0);
  mSuffixCombo = new KComboBox(page, "mSuffixCombo");
  mSuffixCombo->setDuplicatesEnabled(false);
  mSuffixCombo->setEditable(true);
  layout->addWidget(mSuffixCombo, 4, 1);

  mParseBox = new QCheckBox( i18n( "Parse name automatically" ), page );
  connect( mParseBox, SIGNAL( toggled(bool) ), SLOT( parseBoxChanged(bool) ) );
  layout->addMultiCellWidget( mParseBox, 5, 5, 0, 1 );
  
  // Fill in the values
  mFamilyNameEdit->setText(familyName);
  mGivenNameEdit->setText(givenName);
  mAdditionalNameEdit->setText(additionalName);
  
  // Prefix and suffix combos
  QStringList sTitle;
  QStringList sSuffix;

  sTitle += i18n( "Dr." );
  sTitle += i18n( "Miss" );
  sTitle += i18n( "Mr." );
  sTitle += i18n( "Mrs." );
  sTitle += i18n( "Ms." );
  sTitle += i18n( "Prof." );
  sTitle.sort();

  sSuffix += i18n( "I" );
  sSuffix += i18n( "II" );
  sSuffix += i18n( "III" );
  sSuffix += i18n( "Jr." );
  sSuffix += i18n( "Sr." );
  sSuffix.sort();
  
  mPrefixCombo->insertStringList(sTitle);
  mSuffixCombo->insertStringList(sSuffix);
  
  mPrefixCombo->setCurrentText(prefix);
  mSuffixCombo->setCurrentText(suffix);

  KConfig *config = kapp->config();
  config->setGroup( "General" );
  mParseBox->setChecked( config->readBoolEntry( "AutomaticNameParsing", true ) );
}
    
NameEditDialog::~NameEditDialog() 
{
}
    
QString NameEditDialog::familyName() const
{
  return mFamilyNameEdit->text();
}
    
QString NameEditDialog::givenName() const
{
  return mGivenNameEdit->text();
}
    
QString NameEditDialog::prefix() const
{
  return mPrefixCombo->currentText();
}
    
QString NameEditDialog::suffix() const
{
  return mSuffixCombo->currentText();
}
    
QString NameEditDialog::additionalName() const
{
  return mAdditionalNameEdit->text();
}

void NameEditDialog::parseBoxChanged( bool value )
{
  KConfig *config = kapp->config();
  config->setGroup( "General" );
  config->writeEntry( "AutomaticNameParsing", value );
}

///////////////////////////
// AddressEditWidget

class AddressItem : public QListBoxText
{
public:
  AddressItem( QListBox *parent, const KABC::Address::List &list, const KABC::Address &addr )
    : QListBoxText( parent, QString::null ), mAddress( addr )
  {
    KABC::Address::List::ConstIterator it;
    int occurances = 0;
    for ( it = list.begin(); it != list.end(); ++it ) {
      if ( (*it).id() == mAddress.id() ) {
        QString text = mAddress.typeLabel();
        if ( occurances > 0 )
          text += " " + QString::number( occurances + 1 );
        setText( text );
        break;
      }

      if ( ( (*it).type() & ~KABC::Address::Pref ) == ( mAddress.type() & ~KABC::Address::Pref ) )
        occurances++;
    }
  }

  KABC::Address address() const { return mAddress; }


private:
  KABC::Address mAddress;
};

AddressEditWidget::AddressEditWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *layout = new QGridLayout( this, 4, 2 );
  layout->setSpacing( KDialog::spacingHint() );

  mTypeCombo = new KComboBox(this);
  connect( mTypeCombo, SIGNAL( highlighted(int) ), SLOT( typeHighlighted(int) ) );
  layout->addWidget(mTypeCombo, 0, 0);

  mAddressTextEdit = new QTextEdit( this );
  mAddressTextEdit->setReadOnly( true );
  layout->addMultiCellWidget( mAddressTextEdit, 1, 3, 0, 0 );
  
  QPushButton *addButton = new QPushButton( i18n( "&Add" ), this );
  layout->addWidget( addButton, 1, 1 );
  connect( addButton, SIGNAL( clicked() ), SLOT( addAddress() ) );

  mEditButton = new QPushButton( i18n( "&Edit..." ), this );
  mEditButton->setEnabled( false );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( editAddress() ) );
  layout->addWidget( mEditButton, 2, 1 );

  mRemoveButton = new QPushButton( i18n( "&Remove" ), this );
  mRemoveButton->setEnabled( false );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeAddress() ) );
  layout->addWidget( mRemoveButton, 3, 1 );

  mIndex = 0;
}

AddressEditWidget::~AddressEditWidget()
{
}
    
const KABC::Address::List &AddressEditWidget::addresses()
{
  return mAddressList;
}

void AddressEditWidget::setAddresses(const KABC::Address::List &list)
{
  mAddressList = list;
  
  mAddressTextEdit->setText("");

  updateTypeCombo( mAddressList, mTypeCombo );

  typeHighlighted( mIndex );
}

void AddressEditWidget::addressChanged()
{
  bool state = ( mTypeCombo->count() != 0 );

  mEditButton->setEnabled( state );
  mRemoveButton->setEnabled( state );
}

void AddressEditWidget::addAddress()
{
  KABC::Address a;
  AddressEditDialog dialog( a, this );
  if ( dialog.exec() ) {
    mAddressList.append( dialog.address() );
    updateTypeCombo( mAddressList, mTypeCombo );
    emit modified();
  }
  
  typeHighlighted( mIndex );
}

void AddressEditWidget::editAddress()
{
  AddressEditDialog dialog( currentAddress( mTypeCombo, mTypeCombo->currentItem() ) , this );
  if ( dialog.exec() ) {
    KABC::Address addr = dialog.address();

    KABC::Address::List::Iterator it;
    for ( it = mAddressList.begin(); it != mAddressList.end(); ++it )
      if ( (*it).id() == addr.id() )
        (*it) = addr;

    updateTypeCombo( mAddressList, mTypeCombo );
    emit modified();
  }

  typeHighlighted( mIndex );
}

void AddressEditWidget::removeAddress()
{
  mAddressList.remove( currentAddress( mTypeCombo, mTypeCombo->currentItem() ) );
  updateTypeCombo( mAddressList, mTypeCombo );
  emit modified();

  typeHighlighted( mIndex );
}

void AddressEditWidget::typeHighlighted(int index)
{
  // First try to find the type
  mIndex = index;

  KABC::Address a = currentAddress( mTypeCombo, mIndex );

  bool block = signalsBlocked();
  blockSignals(true);
          
  if ( !a.isEmpty() ) {
    QString text;
    text += a.street() + "\n";
    if ( !a.postOfficeBox().isEmpty() )
      text += a.postOfficeBox() + "\n";
    text += a.locality() + QString(" ") + a.region() + QString(", ");
    text += a.postalCode() + "\n";
    text += a.country() + "\n";
    text += a.extended();
    mAddressTextEdit->setText(text);
  } else
    mAddressTextEdit->setText("");
  
  blockSignals(block);

  addressChanged();
}

void AddressEditWidget::updateTypeCombo( const KABC::Address::List &list, KComboBox *combo )
{
  combo->clear();

  QListBox *lb = combo->listBox();

  KABC::Address::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    new AddressItem( lb, list, *it );

  lb->sort();
}

KABC::Address AddressEditWidget::currentAddress( KComboBox *combo, int index )
{
  if ( index < 0 ) index = 0;
  if ( index >= combo->count() ) index = combo->count() - 1;

  QListBox *lb = combo->listBox();
  AddressItem *item = dynamic_cast<AddressItem*>( lb->item( index ) );
  if ( item )
    return item->address();
  else
    return KABC::Address();
}

///////////////////////////////////////////
// PhoneEditDialog

PhoneEditDialog::PhoneEditDialog( const KABC::PhoneNumber &phoneNumber,
                               QWidget *parent, const char *name)
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Phone Number" ),
                KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                parent, name, true), mPhoneNumber( phoneNumber )
{
  QWidget *page = plainPage();
  QLabel *label = 0;
  QGridLayout *layout = new QGridLayout( page, 2, 2, marginHint(), spacingHint() );

  label = new QLabel( i18n( "Number:" ), page );
  layout->addWidget( label, 0, 0 );
  mNumber = new KLineEdit( page );
  layout->addWidget( mNumber, 0, 1 );

  mGroup = new QButtonGroup( 2, Horizontal, i18n( "Types" ), page );
  layout->addMultiCellWidget( mGroup, 1, 1, 0, 1 );

  // fill widgets
  mNumber->setText( mPhoneNumber.number() );

  mTypeList = KABC::PhoneNumber::typeList();
  KABC::PhoneNumber::TypeList::Iterator it;

  for ( it = mTypeList.begin(); it != mTypeList.end(); ++it )
    new QCheckBox( KABC::PhoneNumber::typeLabel( *it ), mGroup );

  for ( int i = 0; i < mGroup->count(); ++i ) {
    int type = mPhoneNumber.type();
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    box->setChecked( type & mTypeList[ i ] );
  }
}

KABC::PhoneNumber PhoneEditDialog::phoneNumber()
{
  mPhoneNumber.setNumber( mNumber->text() );

  int type = 0;
  for ( int i = 0; i < mGroup->count(); ++i ) {
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    if ( box->isChecked() )
      type += mTypeList[ i ];
  }

  mPhoneNumber.setType( type );

  return mPhoneNumber;
}
  
///////////////////////////////////////////
// PhoneEditWidget

class PhoneItem : public QListViewItem
{
public:
  PhoneItem( QListView *parent, const KABC::PhoneNumber &number );

  void setPhoneNumber( const KABC::PhoneNumber &number )
  {
    mPhoneNumber = number;
    makeText();
  }

  QString key() { return mPhoneNumber.id(); }
  QString country() { return ""; }
  QString region() { return ""; }
  QString number() { return ""; }

  KABC::PhoneNumber phoneNumber() { return mPhoneNumber; }

private:
  void makeText();

  KABC::PhoneNumber mPhoneNumber;
};

PhoneItem::PhoneItem( QListView *parent, const KABC::PhoneNumber &number )
  : QListViewItem( parent ), mPhoneNumber( number )
{
  makeText();
}

void PhoneItem::makeText()
{
  /**
   * Will be used in future versions of kaddressbook/libkabc

    setText( 0, mPhoneNumber.country() );
    setText( 1, mPhoneNumber.region() );
    setText( 2, mPhoneNumber.number() );
    setText( 3, mPhoneNumber.typeLabel() );
   */

  setText( 0, mPhoneNumber.number() );
  setText( 1, mPhoneNumber.typeLabel() );
}

PhoneEditWidget::PhoneEditWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  QGridLayout *layout = new QGridLayout( this, 2, 2 );
  layout->setSpacing( KDialogBase::spacingHint() );

  mTypeBox = new KComboBox( this );
  mTypeBox->insertItem( i18n( "All" ) );

  mTypeList = KABC::PhoneNumber::typeList();
  KABC::PhoneNumber::TypeList::Iterator it;
  for ( it = mTypeList.begin(); it != mTypeList.end(); ++it )
    mTypeBox->insertItem( KABC::PhoneNumber::typeLabel( *it ) );

  mListView = new KListView( this );
  mListView->setAllColumnsShowFocus( true );
  mListView->addColumn( i18n( "Number" ) );
  mListView->addColumn( i18n( "Type" ) );
  
  KButtonBox *buttonBox = new KButtonBox( this, Vertical );

  mAddButton = buttonBox->addButton( i18n( "&Add..." ), this, SLOT( slotAddPhoneNumber() ) );
  mRemoveButton = buttonBox->addButton( i18n( "&Remove" ), this, SLOT( slotRemovePhoneNumber() ) );
  mRemoveButton->setEnabled( false );
  mEditButton = buttonBox->addButton( i18n( "&Edit..." ), this, SLOT( slotEditPhoneNumber() ) );
  mEditButton->setEnabled( false );
  buttonBox->layout();

  layout->addWidget( mTypeBox, 0, 0 );
  layout->addWidget( mListView, 1, 0 );
  layout->addWidget( buttonBox, 1, 1 );

  connect( mListView, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()) );
  connect( mTypeBox, SIGNAL(activated(int)), SLOT(slotTypeChanged(int)) );
}

PhoneEditWidget::~PhoneEditWidget()
{
}

void PhoneEditWidget::slotAddPhoneNumber()
{
  KABC::PhoneNumber tmp( "", 0 );
  PhoneEditDialog dlg( tmp, this );
  
  if ( dlg.exec() ) {
    KABC::PhoneNumber phoneNumber = dlg.phoneNumber();
    mPhoneNumberList.append( phoneNumber );
    new PhoneItem( mListView, phoneNumber );
    emit modified();
  }
}

void PhoneEditWidget::slotRemovePhoneNumber()
{
  PhoneItem *item = dynamic_cast<PhoneItem*>( mListView->currentItem() );
  if ( !item )
    return;

  mPhoneNumberList.remove( item->phoneNumber() );
  QListViewItem *currItem = mListView->currentItem();
  mListView->takeItem( currItem );
  delete currItem;

  emit modified();
}

void PhoneEditWidget::slotEditPhoneNumber()
{
  PhoneItem *item = dynamic_cast<PhoneItem*>( mListView->currentItem() );
  if ( !item )
    return;

  PhoneEditDialog dlg( item->phoneNumber(), this );
  
  if ( dlg.exec() ) {
    slotRemovePhoneNumber();
    KABC::PhoneNumber phoneNumber = dlg.phoneNumber();
    mPhoneNumberList.append( phoneNumber );
    new PhoneItem( mListView, phoneNumber );
    emit modified();
  }
}

void PhoneEditWidget::slotSelectionChanged()
{
  bool state = ( mListView->currentItem() != 0 );

  mRemoveButton->setEnabled( state );
  mEditButton->setEnabled( state );
}

void PhoneEditWidget::slotTypeChanged( int pos )
{
  int type = 0;
  
  if ( pos != 0 )
    type = mTypeList[ pos - 1 ];

  mListView->clear();
  KABC::PhoneNumber::List::Iterator it;
  for ( it = mPhoneNumberList.begin(); it != mPhoneNumberList.end(); ++it ) {
    if ( ((*it).type() & type) || pos == 0 )
      new PhoneItem( mListView, *it );
  }
}

void PhoneEditWidget::setPhoneNumbers(const KABC::PhoneNumber::List &list)
{
  mPhoneNumberList = list;

  mListView->clear();

  KABC::PhoneNumber::List::Iterator it;
  for ( it = mPhoneNumberList.begin(); it != mPhoneNumberList.end(); ++it ) {
    new PhoneItem( mListView, *it );
  }
}

const KABC::PhoneNumber::List &PhoneEditWidget::phoneNumbers()
{
  return mPhoneNumberList;
}
 

///////////////////////////////////////////
// AddressEditDialog
AddressEditDialog::AddressEditDialog( const KABC::Address &addr, QWidget *parent, const char *name )
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Address" ),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, name )
{
  mAddress = addr;

  QWidget *page = plainPage();
  
  QGridLayout *topLayout = new QGridLayout(page, 8, 2);
  topLayout->setSpacing(spacingHint());

  QPushButton *editTypeButton = new QPushButton( i18n( "&Edit Type..." ), page );
  connect( editTypeButton, SIGNAL( clicked() ), SLOT( editType() ) );

  topLayout->addMultiCellWidget( editTypeButton, 0, 0, 0, 1 );

  QLabel *label = new QLabel(i18n("Street:"), page);
  label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  topLayout->addWidget(label, 1, 0);
  mStreetTextEdit = new QTextEdit(page, "mStreetTextEdit");
  topLayout->addWidget(mStreetTextEdit, 1, 1);

  label = new QLabel(i18n("Post office box:"), page);
  topLayout->addWidget(label, 2 , 0);
  mPOBoxEdit = new KLineEdit(page, "mPOBoxEdit");
  topLayout->addWidget(mPOBoxEdit, 2, 1);

  label = new QLabel(i18n("Locality:"), page);
  topLayout->addWidget(label, 3, 0);
  mLocalityEdit = new KLineEdit(page, "mLocalityEdit");
  topLayout->addWidget(mLocalityEdit, 3, 1);

  label = new QLabel(i18n("Region:"), page);
  topLayout->addWidget(label, 4, 0);
  mRegionEdit = new KLineEdit(page, "mRegionEdit");
  topLayout->addWidget(mRegionEdit, 4, 1);

  label = new QLabel(i18n("Postal code:"), page);
  topLayout->addWidget(label, 5, 0);
  mPostalCodeEdit = new KLineEdit(page, "mPostalCodeEdit");
  topLayout->addWidget(mPostalCodeEdit, 5, 1);

  label = new QLabel(i18n("Country:"), page);
  topLayout->addWidget(label, 6, 0);
  mCountryCombo = new KComboBox(page, "mCountryCombo");
  mCountryCombo->setEditable(true);
  mCountryCombo->setDuplicatesEnabled(false);
  mCountryCombo->setAutoCompletion(true);
  topLayout->addWidget(mCountryCombo, 6, 1);
  
  mPreferredCheckBox = new QCheckBox( i18n( "This is the preferred address" ), page );
  topLayout->addMultiCellWidget( mPreferredCheckBox, 7, 7, 0, 1 );
  
  fillCombo(mCountryCombo);
  
  // Fill in the values
  mStreetTextEdit->setText(mAddress.street());
  mStreetTextEdit->setFocus();
  mRegionEdit->setText(mAddress.region());
  mLocalityEdit->setText(mAddress.locality());
  mPostalCodeEdit->setText(mAddress.postalCode());
  mPOBoxEdit->setText(mAddress.postOfficeBox());
  mCountryCombo->setCurrentText(mAddress.country());
  mPreferredCheckBox->setChecked( mAddress.type() & KABC::Address::Pref );

  // initialize the layout
  topLayout->activate();

  // set default type
  if ( mAddress.type() == 0 )
    mAddress.setType( KABC::Address::Home );
}

AddressEditDialog::~AddressEditDialog()
{
}
    
const KABC::Address &AddressEditDialog::address()
{
  mAddress.setLocality(mLocalityEdit->text());
  mAddress.setRegion(mRegionEdit->text());
  mAddress.setPostalCode(mPostalCodeEdit->text());
  mAddress.setCountry(mCountryCombo->currentText());
  mAddress.setPostOfficeBox(mPOBoxEdit->text());
  mAddress.setStreet(mStreetTextEdit->text());

  if ( mPreferredCheckBox->isChecked() )
    mAddress.setType( mAddress.type() | KABC::Address::Pref );
  else
    mAddress.setType( mAddress.type() & ~KABC::Address::Pref );

  return mAddress;
}

void AddressEditDialog::editType()
{
  AddressTypeDialog dlg( mAddress.type(), this );
  if ( dlg.exec() )
    mAddress.setType( dlg.type() );
}

void AddressEditDialog::fillCombo(KComboBox *combo)
{
  QString sCountry[] = {
    i18n( "Afghanistan" ), i18n( "Albania" ), i18n( "Algeria" ),
    i18n( "American Samoa" ), i18n( "Andorra" ), i18n( "Angola" ),
    i18n( "Anguilla" ), i18n( "Antarctica" ), i18n( "Antigua and Barbuda" ),
    i18n( "Argentina" ), i18n( "Armenia" ), i18n( "Aruba" ),
    i18n( "Ashmore and Cartier Islands" ), i18n( "Australia" ),
    i18n( "Austria" ), i18n( "Azerbaijan" ), i18n( "Bahamas" ),
    i18n( "Bahrain" ), i18n( "Bangladesh" ), i18n( "Barbados" ),
    i18n( "Belarus" ), i18n( "Belgium" ), i18n( "Belize" ),
    i18n( "Benin" ), i18n( "Bermuda" ), i18n( "Bhutan" ),
    i18n( "Bolivia" ), i18n( "Bosnia and Herzegovina" ), i18n( "Botswana" ),
    i18n( "Brazil" ), i18n( "Brunei" ), i18n( "Bulgaria" ),
    i18n( "Burkina Faso" ), i18n( "Burundi" ), i18n( "Cambodia" ),
    i18n( "Cameroon" ), i18n( "Canada" ), i18n( "Cape Verde" ),
    i18n( "Cayman Islands" ), i18n( "Central African Republic" ),
    i18n( "Chad" ), i18n( "Chile" ), i18n( "China" ), i18n( "Colombia" ),
    i18n( "Comoros" ), i18n( "Congo" ), i18n( "Congo, Dem. Rep." ),
    i18n( "Costa Rica" ), i18n( "Croatia" ),
    i18n( "Cuba" ), i18n( "Cyprus" ), i18n( "Czech Republic" ),
    i18n( "Denmark" ), i18n( "Djibouti" ),
    i18n( "Dominica" ), i18n( "Dominican Republic" ), i18n( "Ecuador" ),
    i18n( "Egypt" ), i18n( "El Salvador" ), i18n( "Equatorial Guinea" ),
    i18n( "Eritrea" ), i18n( "Estonia" ), i18n( "England" ),
    i18n( "Ethiopia" ), i18n( "European Union" ), i18n( "Faroe Islands" ),
    i18n( "Fiji" ), i18n( "Finland" ), i18n( "France" ),
    i18n( "French Polynesia" ), i18n( "Gabon" ), i18n( "Gambia" ),
    i18n( "Georgia" ), i18n( "Germany" ), i18n( "Ghana" ),
    i18n( "Greece" ), i18n( "Greenland" ), i18n( "Grenada" ),
    i18n( "Guam" ), i18n( "Guatemala" ), i18n( "Guinea" ),
    i18n( "Guinea-Bissau" ), i18n( "Guyana" ), i18n( "Haiti" ),
    i18n( "Honduras" ), i18n( "Hong Kong" ), i18n( "Hungary" ), 
    i18n( "Iceland" ), i18n( "India" ), i18n( "Indonesia" ),
    i18n( "Iran" ), i18n( "Iraq" ), i18n( "Ireland" ),
    i18n( "Israel" ), i18n( "Italy" ), i18n( "Ivory Coast" ),
    i18n( "Jamaica" ), i18n( "Japan" ), i18n( "Jordan" ),
    i18n( "Kazakhstan" ), i18n( "Kenya" ), i18n( "Kiribati" ),
    i18n( "Korea, North" ), i18n( "Korea, South" ),
    i18n( "Kuwait" ), i18n( "Kyrgyzstan" ), i18n( "Laos" ),
    i18n( "Latvia" ), i18n( "Lebanon" ), i18n( "Lesotho" ),
    i18n( "Liberia" ), i18n( "Libya" ), i18n( "Liechtenstein" ),
    i18n( "Lithuania" ), i18n( "Luxembourg" ), i18n( "Macau" ),
    i18n( "Madagascar" ), i18n( "Malawi" ), i18n( "Malaysia" ),
    i18n( "Maldives" ), i18n( "Mali" ), i18n( "Malta" ),
    i18n( "Marshall Islands" ), i18n( "Martinique" ), i18n( "Mauritania" ),
    i18n( "Mauritius" ), i18n( "Mexico" ),
    i18n( "Micronesia, Federated States Of" ), i18n( "Moldova" ),
    i18n( "Monaco" ), i18n( "Mongolia" ), i18n( "Montserrat" ),
    i18n( "Morocco" ), i18n( "Mozambique" ), i18n( "Myanmar" ),
    i18n( "Namibia" ),
    i18n( "Nauru" ), i18n( "Nepal" ), i18n( "Netherlands" ),
    i18n( "Netherlands Antilles" ), i18n( "New Caledonia" ),
    i18n( "New Zealand" ), i18n( "Nicaragua" ), i18n( "Niger" ),
    i18n( "Nigeria" ), i18n( "Niue" ), i18n( "North Korea" ),
    i18n( "Northern Ireland" ), i18n( "Northern Mariana Islands" ),
    i18n( "Norway" ), i18n( "Oman" ), i18n( "Pakistan" ), i18n( "Palau" ),
    i18n( "Palestinian" ), i18n( "Panama" ), i18n( "Papua New Guinea" ),
    i18n( "Paraguay" ), i18n( "Peru" ), i18n( "Philippines" ),
    i18n( "Poland" ), i18n( "Portugal" ), i18n( "Puerto Rico" ),
    i18n( "Qatar" ), i18n( "Romania" ), i18n( "Russia" ), i18n( "Rwanda" ),
    i18n( "St. Kitts and Nevis" ), i18n( "St. Lucia" ),
    i18n( "St. Vincent and the Grenadines" ), i18n( "San Marino" ),
    i18n( "Sao Tome and Principe" ), i18n( "Saudi Arabia" ),
    i18n( "Senegal" ), i18n( "Serbia & Montenegro" ), i18n( "Seychelles" ),
    i18n( "Sierra Leone" ), i18n( "Singapore" ), i18n( "Slovakia" ),
    i18n( "Slovenia" ), i18n( "Solomon Islands" ), i18n( "Somalia" ),
    i18n( "South Africa" ), i18n( "South Korea" ), i18n( "Spain" ),
    i18n( "Sri Lanka" ), i18n( "St. Kitts and Nevis" ), i18n( "Sudan" ),
    i18n( "Suriname" ), i18n( "Swaziland" ), i18n( "Sweden" ),
    i18n( "Switzerland" ), i18n( "Syria" ), i18n( "Taiwan" ),
    i18n( "Tajikistan" ), i18n( "Tanzania" ), i18n( "Thailand" ),
    i18n( "Tibet" ), i18n( "Togo" ), i18n( "Tonga" ),
    i18n( "Trinidad and Tobago" ), i18n( "Tunisia" ), i18n( "Turkey" ),
    i18n( "Turkmenistan" ), i18n( "Turks and Caicos Islands" ),
    i18n( "Tuvalu" ), i18n( "Uganda " ), i18n( "Ukraine" ),
    i18n( "United Arab Emirates" ), i18n( "United Kingdom" ),
    i18n( "United States" ), i18n( "Uruguay" ), i18n( "Uzbekistan" ),
    i18n( "Vanuatu" ), i18n( "Vatican City" ), i18n( "Venezuela" ),
    i18n( "Vietnam" ), i18n( "Western Samoa" ), i18n( "Yemen" ),
    i18n( "Yugoslavia" ), i18n( "Zaire" ), i18n( "Zambia" ),
    i18n( "Zimbabwe" ),
    ""
  };
  
  QStringList countries;
  for (int i =0; sCountry[i] != ""; ++i )
    countries.append( sCountry[i] );

  countries.sort();
  
  combo->insertStringList( countries );
}

AddressTypeDialog::AddressTypeDialog( int type, QWidget *parent )
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Address Type" ),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                 parent, "AddressTypeDialog" )
{
  QWidget *page = plainPage();
  QVBoxLayout *layout = new QVBoxLayout( page );

  mGroup = new QButtonGroup( 2, Horizontal, i18n( "Address Types" ), page );
  layout->addWidget( mGroup );

  mTypeList = KABC::Address::typeList();
  mTypeList.remove( KABC::Address::Pref );

  KABC::Address::TypeList::Iterator it;
  for ( it = mTypeList.begin(); it != mTypeList.end(); ++it )
    new QCheckBox( KABC::Address::typeLabel( *it ), mGroup );

  for ( int i = 0; i < mGroup->count(); ++i ) {
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    box->setChecked( type & mTypeList[ i ] );
  }
}    

AddressTypeDialog::~AddressTypeDialog()
{
}

int AddressTypeDialog::type()
{
  int type = 0;
  for ( int i = 0; i < mGroup->count(); ++i ) {
    QCheckBox *box = (QCheckBox*)mGroup->find( i );
    if ( box->isChecked() )
      type += mTypeList[ i ];
  }

  return type;
}

#include "addresseeeditorsupportwidgets.moc"
