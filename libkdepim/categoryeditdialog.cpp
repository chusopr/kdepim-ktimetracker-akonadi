/*
  This file is part of libkdepim.

  Copyright (c) 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "categoryeditdialog.h"
#include "ui_categoryeditdialog_base.h"
#include "categoryhierarchyreader.h"
#include "kpimprefs.h"

#include <KLocale>

#include <QLayout>
#include <QStringList>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QList>
#include <QHeaderView>

using namespace KPIM;

CategoryEditDialog::CategoryEditDialog( KPimPrefs *prefs, QWidget *parent, bool modal )
  : KDialog( parent ), mPrefs( prefs )
{
  setCaption( i18n( "Edit Categories" ) );
  setModal( modal );
  setButtons( Ok|Apply|Cancel|Help );
  mWidgets = new Ui::CategoryEditDialog_base();
  QWidget *widget = new QWidget( this );
  widget->setObjectName( "CategoryEdit" );
  mWidgets->setupUi( widget );

  mWidgets->mCategories->header()->hide();
  mWidgets->mButtonAdd->setIcon( KIcon( "list-add" ) );
  mWidgets->mButtonAddSubcategory->setIcon( KIcon( "list-add" ) );
  mWidgets->mButtonRemove->setIcon( KIcon( "list-remove" ) );

  setMainWidget( widget );

  fillList();

  connect( mWidgets->mCategories,
           SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * )),
           SLOT( editItem( QTreeWidgetItem * )) );
  connect( mWidgets->mCategories, SIGNAL( itemSelectionChanged() ),
           SLOT( slotSelectionChanged() ) );
  connect( mWidgets->mCategories, SIGNAL( itemCollapsed( QTreeWidgetItem * ) ),
           SLOT( expandIfToplevel( QTreeWidgetItem * ) ) );
  connect( mWidgets->mEdit, SIGNAL( textChanged( const QString & )),
           this, SLOT( slotTextChanged( const QString & )));
  connect( mWidgets->mButtonAdd, SIGNAL( clicked() ),
           this, SLOT( add() ) );
  connect( mWidgets->mButtonAddSubcategory, SIGNAL( clicked() ),
           this, SLOT( addSubcategory() ) );
  connect( mWidgets->mButtonRemove, SIGNAL( clicked() ),
           this, SLOT( remove() ) );
  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
  connect( this, SIGNAL( cancelClicked() ), this, SLOT( slotCancel() ) );
  connect( this, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
CategoryEditDialog::~CategoryEditDialog()
{
  delete mWidgets;
}

void CategoryEditDialog::fillList()
{
  CategoryHierarchyReaderQTreeWidget( mWidgets->mCategories ).read( mPrefs->mCustomCategories );

  mWidgets->mButtonRemove->setEnabled( mWidgets->mCategories->topLevelItemCount() > 0 );
  mWidgets->mButtonAddSubcategory->setEnabled( mWidgets->mCategories->topLevelItemCount() > 0 );
}

void CategoryEditDialog::slotTextChanged( const QString &text )
{
  QTreeWidgetItem *item = mWidgets->mCategories->currentItem();
  if ( item ) {
    item->setText( 0, text );
  }
}

void CategoryEditDialog::slotSelectionChanged()
{
  QTreeWidgetItemIterator it( mWidgets->mCategories, QTreeWidgetItemIterator::Selected );
  mWidgets->mButtonRemove->setEnabled( *it );
}

void CategoryEditDialog::add()
{
  if ( !mWidgets->mEdit->text().isEmpty() ) {
    QTreeWidgetItem *newItem =
      new QTreeWidgetItem( mWidgets->mCategories, QStringList( "" ) );
    // FIXME: Use a better string once string changes are allowed again
//                                                i18n("New category") );
    newItem->setExpanded( true );
    mWidgets->mCategories->setCurrentItem( newItem );
    mWidgets->mCategories->clearSelection();
    newItem->setSelected( true );
    mWidgets->mCategories->scrollToItem( newItem );
    mWidgets->mButtonRemove->setEnabled( mWidgets->mCategories->topLevelItemCount() > 0 );
    mWidgets->mButtonAddSubcategory->setEnabled( mWidgets->mCategories->topLevelItemCount() > 0 );
    mWidgets->mEdit->setFocus();
  }
}

void CategoryEditDialog::addSubcategory()
{
  if ( !mWidgets->mEdit->text().isEmpty() ) {
    QTreeWidgetItem *newItem =
      new QTreeWidgetItem( mWidgets->mCategories->currentItem(), QStringList( "" ) );
    // FIXME: Use a better string once string changes are allowed again
//                                                i18n("New category") );
    newItem->setExpanded( true );
    mWidgets->mCategories->setCurrentItem( newItem );
    mWidgets->mCategories->clearSelection();
    newItem->setSelected( true );
    mWidgets->mCategories->scrollToItem( newItem );
    mWidgets->mEdit->setFocus();
  }
}

void CategoryEditDialog::remove()
{
  QList<QTreeWidgetItem*> to_remove = mWidgets->mCategories->selectedItems();
  while ( !to_remove.isEmpty() ) {
    deleteItem( to_remove.takeFirst(), to_remove );
  }

  mWidgets->mButtonRemove->setEnabled( mWidgets->mCategories->topLevelItemCount() > 0 );
  mWidgets->mButtonAddSubcategory->setEnabled( mWidgets->mCategories->topLevelItemCount() > 0 );
  if ( mWidgets->mCategories->currentItem() ) {
    mWidgets->mCategories->currentItem()->setSelected( true );
  }
}

void CategoryEditDialog::deleteItem( QTreeWidgetItem *item, QList<QTreeWidgetItem *> &to_remove )
{
  if ( !item ) {
    return;
  }

  for ( int i = item->childCount() - 1; i >= 0; i-- ) {
    QTreeWidgetItem *child = item->child( i );
    to_remove.removeAll( child );
    deleteItem( child, to_remove );
  }
  delete item;
}

void CategoryEditDialog::slotOk()
{
  slotApply();
  accept();
}

void CategoryEditDialog::slotApply()
{
  mPrefs->mCustomCategories.clear();

  QStringList path;
  QTreeWidgetItemIterator it( mWidgets->mCategories );
  while ( *it ) {
    path = mWidgets->mCategories->pathByItem( *it++ );
    path.replaceInStrings(
      KPimPrefs::categorySeparator, QString( "\\" ) + KPimPrefs::categorySeparator );
    mPrefs->mCustomCategories.append( path.join( KPimPrefs::categorySeparator ) );
  }

  mPrefs->writeConfig();

  emit categoryConfigChanged();
}

void CategoryEditDialog::slotCancel()
{
  reload();
}

void CategoryEditDialog::editItem( QTreeWidgetItem *item )
{
  if ( item ) {
    mWidgets->mEdit->setText( item->text( 0 ) );
  }
}

void CategoryEditDialog::reload()
{
  fillList();
}

void CategoryEditDialog::show()
{
  QTreeWidgetItem *first = 0;
  if ( mWidgets->mCategories->topLevelItemCount() ) {
    first = mWidgets->mCategories->topLevelItem( 0 );
    mWidgets->mCategories->setCurrentItem( first );
  }
  mWidgets->mCategories->clearSelection();
  if ( first ) {
    first->setSelected( true );
    editItem( first );
  }
  KDialog::show();
}

void CategoryEditDialog::expandIfToplevel( QTreeWidgetItem *item )
{
  if ( !item->parent() ) {
    item->setExpanded( true );
  }
}

#include "categoryeditdialog.moc"
