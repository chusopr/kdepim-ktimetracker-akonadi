/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <q3listbox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QBoxLayout>

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include "viewconfigurefieldspage.h"

class FieldItem : public Q3ListBoxText
{
  public:
    FieldItem( Q3ListBox *parent, KABC::Field *field )
      : Q3ListBoxText( parent, field->label() ), mField( field ) {}

    FieldItem( Q3ListBox *parent, KABC::Field *field, int index )
      : Q3ListBoxText( parent, field->label(), parent->item( index ) ),
        mField( field ) {}

    KABC::Field *field() { return mField; }

  private:
    KABC::Field *mField;
};


ViewConfigureFieldsPage::ViewConfigureFieldsPage( KABC::AddressBook *ab,
                                                  QWidget *parent,
                                                  const char *name )
  : QWidget( parent ), mAddressBook( ab )
{
   setObjectName( name );
   initGUI();
}

void ViewConfigureFieldsPage::restoreSettings( KConfig *config )
{
  KABC::Field::List fields = KABC::Field::restoreFields( config, "KABCFields" );

  if ( fields.isEmpty() )
    fields = KABC::Field::defaultFields();

  KABC::Field::List::ConstIterator it;
  for ( it = fields.begin(); it != fields.end(); ++it )
    new FieldItem( mSelectedBox, *it );

  slotShowFields( mCategoryCombo->currentIndex() );
}

void ViewConfigureFieldsPage::saveSettings( KConfig *config )
{
  KABC::Field::List fields;

  for ( uint i = 0; i < mSelectedBox->count(); ++i ) {
    FieldItem *fieldItem = static_cast<FieldItem *>( mSelectedBox->item( i ) );
    fields.append( fieldItem->field() );
  }

  KABC::Field::saveFields( config, "KABCFields", fields );
}

void ViewConfigureFieldsPage::slotShowFields( int index )
{
  int currentPos = mUnSelectedBox->currentItem();
  mUnSelectedBox->clear();

  int category;
  if ( index == 0 ) category = KABC::Field::All;
  else category = 1 << ( index - 1 );

  KABC::Field::List allFields = mAddressBook->fields( category );

  KABC::Field::List::ConstIterator it;
  for ( it = allFields.begin(); it != allFields.end(); ++it ) {
    Q3ListBoxItem *item = mSelectedBox->firstItem();
    while( item ) {
      FieldItem *fieldItem = static_cast<FieldItem *>( item );
      if ( (*it)->equals( fieldItem->field() ) )
        break;
      item = item->next();
    }

    if ( !item )
      new FieldItem( mUnSelectedBox, *it );
  }

  mUnSelectedBox->sort();
  mUnSelectedBox->setCurrentItem( currentPos );
}

void ViewConfigureFieldsPage::slotSelect()
{
  // insert selected items in the unselected list to the selected list,
  // directoy under the current item if selected, or at the bottonm if
  // nothing is selected in the selected list
  int where = mSelectedBox->currentItem();
  if ( !(where > -1 && mSelectedBox->item( where )->isSelected()) )
    where = mSelectedBox->count() - 1;

  for ( uint i = 0; i < mUnSelectedBox->count(); ++i )
    if ( mUnSelectedBox->isSelected( mUnSelectedBox->item( i ) ) ) {
      FieldItem *fieldItem = static_cast<FieldItem *>( mUnSelectedBox->item( i ) );
      new FieldItem( mSelectedBox, fieldItem->field(), where );
      where++;
    }

  slotShowFields( mCategoryCombo->currentIndex() );
}

void ViewConfigureFieldsPage::slotUnSelect()
{
  for ( uint i = 0; i < mSelectedBox->count(); ++i )
    if ( mSelectedBox->isSelected( mSelectedBox->item( i ) ) ) {
      mSelectedBox->removeItem( i );
      --i;
    }

  slotShowFields( mCategoryCombo->currentIndex() );
}

void ViewConfigureFieldsPage::slotButtonsEnabled()
{
  bool state = false;
  // add button: enabled if any items are selected in the unselected list
  for ( uint i = 0; i < mUnSelectedBox->count(); ++i )
    if ( mUnSelectedBox->item( i )->isSelected() ) {
      state = true;
      break;
    }
  mAddButton->setEnabled( state );

  int j = mSelectedBox->currentItem();
  state = ( j > -1 && mSelectedBox->isSelected( j ) );

  // up button: enabled if there is a current item > 0 and that is selected
  mUpButton->setEnabled( ( j > 0 && state ) );

  // down button: enabled if there is a current item < count - 2 and that is selected
  mDownButton->setEnabled( ( j > -1 && j < (int)mSelectedBox->count() - 1 && state ) );

  // remove button: enabled if any items are selected in the selected list
  state = false;
  for ( uint i = 0; i < mSelectedBox->count(); ++i )
    if ( mSelectedBox->item( i )->isSelected() ) {
      state = true;
      break;
    }
  mRemoveButton->setEnabled( state );
}

void ViewConfigureFieldsPage::slotMoveUp()
{
  int i = mSelectedBox->currentItem();
  if ( i > 0 ) {
    Q3ListBoxItem *item = mSelectedBox->item( i );
    mSelectedBox->takeItem( item );
    mSelectedBox->insertItem( item, i - 1 );
    mSelectedBox->setCurrentItem( item );
    mSelectedBox->setSelected( i - 1, true );
  }
}

void ViewConfigureFieldsPage::slotMoveDown()
{
  int i = mSelectedBox->currentItem();
  if ( i > -1 && i < (int)mSelectedBox->count() - 1 ) {
    Q3ListBoxItem *item = mSelectedBox->item( i );
    mSelectedBox->takeItem( item );
    mSelectedBox->insertItem( item, i + 1 );
    mSelectedBox->setCurrentItem( item );
    mSelectedBox->setSelected( i + 1, true );
  }
}

void ViewConfigureFieldsPage::initGUI()
{
  setWindowTitle( i18n( "Select Fields to Display" ) );

  QGridLayout *gl = new QGridLayout( this  );
  gl->setSpacing( KDialog::spacingHint() );
  gl->setMargin( 0 );

  mCategoryCombo = new KComboBox( false, this );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::All ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::Frequent ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::Address ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::Email ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::Personal ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::Organization ) );
  mCategoryCombo->addItem( KABC::Field::categoryLabel( KABC::Field::CustomCategory ) );
  connect( mCategoryCombo, SIGNAL( activated(int) ), SLOT( slotShowFields(int) ) );
  gl->addWidget( mCategoryCombo, 0, 0 );

  QLabel *label = new QLabel( i18n( "&Selected fields:" ), this );
  gl->addWidget( label, 0, 2 );

  mUnSelectedBox = new Q3ListBox( this );
  mUnSelectedBox->setSelectionMode( Q3ListBox::Extended );
  mUnSelectedBox->setMinimumHeight( 100 );
  gl->addWidget( mUnSelectedBox, 1, 0 );

  mSelectedBox = new Q3ListBox( this );
  mSelectedBox->setSelectionMode( Q3ListBox::Extended );
  label->setBuddy( mSelectedBox );
  gl->addWidget( mSelectedBox, 1, 2 );

  QBoxLayout *vb1 = new QVBoxLayout();
  vb1->setSpacing( KDialog::spacingHint() );
  vb1->addStretch();

  mAddButton = new QToolButton( this );
  mAddButton->setIcon( QApplication::isRightToLeft() ? SmallIconSet( "1leftarrow" ) : SmallIconSet( "1rightarrow" ) );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( slotSelect() ) );
  vb1->addWidget( mAddButton );

  mRemoveButton = new QToolButton( this );
  mRemoveButton->setIcon( QApplication::isRightToLeft() ? SmallIconSet( "1rightarrow" ) : SmallIconSet( "1leftarrow" ) );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( slotUnSelect() ) );
  vb1->addWidget( mRemoveButton );

  vb1->addStretch();
  gl->addLayout( vb1, 1, 1 );

  QBoxLayout *vb2 = new QVBoxLayout();
  vb2->setSpacing( KDialog::spacingHint() );
  vb2->addStretch();

  mUpButton = new QToolButton( this );
  mUpButton->setIcon( SmallIconSet( "1uparrow" ) );
  connect( mUpButton, SIGNAL( clicked() ), SLOT( slotMoveUp() ) );
  vb2->addWidget( mUpButton );

  mDownButton = new QToolButton( this );
  mDownButton->setIcon( SmallIconSet( "1downarrow" ) );
  connect( mDownButton, SIGNAL( clicked() ), SLOT( slotMoveDown() ) );
  vb2->addWidget( mDownButton );

  vb2->addStretch();
  gl->addLayout( vb2, 1, 3 );

  QSize sizeHint = mUnSelectedBox->sizeHint();

  // make sure we fill the list with all items, so that we can
  // get the maxItemWidth we need to not truncate the view
  slotShowFields( 0 );

  sizeHint = sizeHint.expandedTo( mSelectedBox->sizeHint() );
  sizeHint.setWidth( mUnSelectedBox->maxItemWidth() );
  mUnSelectedBox->setMinimumSize( sizeHint );
  mSelectedBox->setMinimumSize( sizeHint );

  gl->activate();

  connect( mUnSelectedBox, SIGNAL( selectionChanged() ), SLOT( slotButtonsEnabled() ) );
  connect( mSelectedBox, SIGNAL( selectionChanged() ), SLOT( slotButtonsEnabled() ) );
  connect( mSelectedBox, SIGNAL( currentChanged( Q3ListBoxItem * ) ), SLOT( slotButtonsEnabled() ) );

  slotButtonsEnabled();
}

#include "viewconfigurefieldspage.moc"
