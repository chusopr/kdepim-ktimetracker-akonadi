/*
    This file is part of KContactManager.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef CONTACTGROUPEDITORDIALOG_H
#define CONTACTGROUPEDITORDIALOG_H

#include <kdialog.h>

namespace Akonadi {
  class Item;
  class ContactGroupEditor;
}

class QAbstractItemModel;

class ContactGroupEditorDialog : public KDialog
{
  Q_OBJECT

  public:
    enum Mode
    {
      CreateMode, ///< Creates a new contact group
      EditMode    ///< Edits an existing contact group
    };

    /**
     * Creates a new contact group editor dialog.
     *
     * @param mode The mode of the dialog.
     * @param collectionModel The collection model.
     * @param parent The parent widget of the dialog.
     */
    ContactGroupEditorDialog( Mode mode, QAbstractItemModel *collectionModel, QWidget *parent = 0 );

    /**
     * Destroys the contact group editor dialog.
     */
    ~ContactGroupEditorDialog();

    /**
     * Sets the contact @p group to edit when in EditMode.
     */
    void setContactGroup( const Akonadi::Item &group );

  Q_SIGNALS:
    /**
     * This signal is emitted whenever a contact group was updated or stored.
     *
     * @param group The contact group.
     */
    void contactGroupStored( const Akonadi::Item &group );

  private Q_SLOTS:
    void slotOkClicked();
    void slotCancelClicked();

  private:
    Akonadi::ContactGroupEditor *mEditor;
};

#endif
