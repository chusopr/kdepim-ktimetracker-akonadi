/*
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef CONTACTEDITORVIEW_H
#define CONTACTEDITORVIEW_H

#include "kdeclarativefullscreenview.h"

namespace Akonadi
{
  class CollectionComboBox;
  class ContactEditor;
  class Item;
}

class ContactCreateWidget;
class ContactEditWidget;

class ContactEditorView : public KDeclarativeFullScreenView
{
  Q_OBJECT
 
  public:
    enum Mode {
      Create,
      Edit
    };
   
    explicit ContactEditorView( Mode mode, QWidget *parent = 0 );

    ~ContactEditorView();

    void setCollectionSelector( Akonadi::CollectionComboBox *comboBox );

    void setCreatorWidget( ContactCreateWidget *editor );
    void setEditorWidget( ContactEditWidget *editor );

    void loadContact( const Akonadi::Item &item );

  public Q_SLOTS:
    void save();
    void cancel();
    
  private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void saveFinished() );
    Q_PRIVATE_SLOT( d, void saveFailed( const QString& ) );
};

#endif
