/*
    This file is part of Akonadi.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef COLLECTIONWIDGET_H
#define COLLECTIONWIDGET_H

#include <QtGui/QWidget>

namespace PIM {
class CollectionView;
class MessageCollectionModel;
}

class CollectionWidget : public QWidget
{
  Q_OBJECT

  public:
    CollectionWidget( QWidget *parent = 0 );

  private:
    PIM::MessageCollectionModel *mModel;
    PIM::CollectionView *mView;
};

#endif
