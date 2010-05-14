/*
    Copyright (C) 2010  Bertjan Broeksema b.broeksema@home.nl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "incidencegeneraleditor.h"

#include "categoryconfig.h"
#include "categoryselectdialog.h"
#include "editorconfig.h"
#include "ui_incidencegeneraleditor.h"

using namespace IncidenceEditors;
using namespace IncidenceEditorsNG;


IncidenceGeneralEditor::IncidenceGeneralEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mUi( new Ui::IncidenceGeneralEditor )
{
  mUi->setupUi( this );

  connect( mUi->mSelectCategoriesButton, SIGNAL(clicked()),
           SLOT(selectCategories()));
  connect( mUi->mSummaryEdit, SIGNAL(textChanged(QString)),
           SLOT(checkDirtyStatus()));
  connect( mUi->mLocationEdit, SIGNAL(textChanged(QString)),
           SLOT(checkDirtyStatus()));
}

void IncidenceGeneralEditor::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;
  if ( mLoadedIncidence ) {
    mUi->mSummaryEdit->setText( mLoadedIncidence->summary() );
    mUi->mLocationEdit->setText( mLoadedIncidence->location() );
    setCategories( mLoadedIncidence->categories() );
  } else {
    mUi->mSummaryEdit->clear();
    mUi->mLocationEdit->clear();
    mUi->mCategoriesLabel->clear();
  }
  mWasDirty = false;
}

void IncidenceGeneralEditor::save( KCal::Incidence::Ptr incidence )
{
  Q_ASSERT( incidence );
  incidence->setSummary( mUi->mSummaryEdit->text() );
  incidence->setLocation( mUi->mLocationEdit->text() );
  incidence->setCategories( mSelectedCategories );
}

bool IncidenceGeneralEditor::isDirty() const
{
  if ( mLoadedIncidence ) {
    return ( mUi->mSummaryEdit->text() != mLoadedIncidence->summary() )
      || ( mUi->mLocationEdit->text() != mLoadedIncidence->location() )
      || categoriesChanged();
  } else {
    return mUi->mSummaryEdit->text().isEmpty()
      && mUi->mLocationEdit->text().isEmpty()
      && categoriesChanged();
  }
}

void IncidenceGeneralEditor::selectCategories()
{
  CategoryConfig cc( EditorConfig::instance()->config() );
  QPointer<CategorySelectDialog> categoryDialog =
    new CategorySelectDialog( &cc, mUi->mSelectCategoriesButton );
  categoryDialog->setHelp( "categories-view", "korganizer" );
  categoryDialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Help );
  categoryDialog->setSelected( mSelectedCategories );

  connect( categoryDialog, SIGNAL(editCategories()),
           SIGNAL(openCategoryDialog()) );
  connect( this, SIGNAL(updateCategoryConfig()),
           categoryDialog, SLOT(updateCategoryConfig()) );

  if ( categoryDialog->exec() ) {
    setCategories( categoryDialog->selectedCategories() );
    checkDirtyStatus();
  }
  delete categoryDialog;
}

bool IncidenceGeneralEditor::categoriesChanged() const
{
  // If no Incidence was loaded, mSelectedCategories should be empty.
  bool categoriesEqual = mSelectedCategories.isEmpty();
  
  if ( mLoadedIncidence ) { // There was an Incidence loaded
    bool categoriesEqual = ( mLoadedIncidence->categories().size() == mSelectedCategories.size() );
    if ( categoriesEqual ) {
      QStringListIterator it( mLoadedIncidence->categories() );
      while ( it.hasNext() && categoriesEqual )
        categoriesEqual = mSelectedCategories.contains( it.next() );
    }
  }
  return !categoriesEqual;
}

void IncidenceGeneralEditor::setCategories( const QStringList &categories )
{
  mSelectedCategories = categories;
  mUi->mCategoriesLabel->setText( categories.join( "," ) );
}
