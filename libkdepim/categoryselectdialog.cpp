/*
    This file is part of libkdepim.
    Copyright (c) 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <qlistview.h>
#include <qpushbutton.h>
#include <qheader.h>

#include "categoryselectdialog.h"

#include "kpimprefs.h"

using namespace KPIM;

CategorySelectDialog::CategorySelectDialog( KPimPrefs *prefs, QWidget* parent,
                                            const char* name, 
                                            bool modal, WFlags fl )
  : CategorySelectDialog_base( parent, name, modal, fl ),
    mPrefs( prefs )
{
  mCategories->header()->hide();

  setCategories();
 
  connect(mButtonEdit,SIGNAL(clicked()),SIGNAL(editCategories()));
}

void CategorySelectDialog::setCategories()
{
  mCategories->clear();

  QStringList::Iterator it;

  for (it = mPrefs->mCustomCategories.begin();
       it != mPrefs->mCustomCategories.end(); ++it ) {
    new QCheckListItem(mCategories,*it,QCheckListItem::CheckBox);
  }
}

CategorySelectDialog::~CategorySelectDialog()
{
}

void CategorySelectDialog::setSelected(const QStringList &selList)
{
  clear();

  QStringList::ConstIterator it;
  for (it=selList.begin();it!=selList.end();++it) {
    QCheckListItem *item = (QCheckListItem *)mCategories->firstChild();
    while (item) {
      if (item->text() == *it) {
        item->setOn(true);
        break;
      }
      item = (QCheckListItem *)item->nextSibling();
    }
  }
}

void CategorySelectDialog::slotApply()
{
  QStringList categories;
  QCheckListItem *item = (QCheckListItem *)mCategories->firstChild();
  while (item) {
    if (item->isOn()) {
      categories.append(item->text());
    }
    item = (QCheckListItem *)item->nextSibling();
  }
  
  QString categoriesStr = categories.join(", ");

  emit categoriesSelected(categories);
  emit categoriesSelected(categoriesStr);
}

void CategorySelectDialog::slotOk()
{
  slotApply();
  accept();
}

void CategorySelectDialog::clear()
{
  QCheckListItem *item = (QCheckListItem *)mCategories->firstChild();
  while (item) {
    item->setOn(false);
    item = (QCheckListItem *)item->nextSibling();
  }  
}

void CategorySelectDialog::updateCategoryConfig()
{
  QStringList selected;
  QCheckListItem *item = (QCheckListItem *)mCategories->firstChild();
  while (item) {
    if (item->isOn()) {
      selected.append(item->text());
    }
    item = (QCheckListItem *)item->nextSibling();
  }

  setCategories();
  
  setSelected(selected);
}

#include "categoryselectdialog.moc"
