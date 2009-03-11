/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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


#ifndef DESCENDANTENTITIESPROXYMODEL_H
#define DESCENDANTENTITIESPROXYMODEL_H

#include <QAbstractProxyModel>

// #include "abstractproxymodel.h"

namespace Akonadi
{
class DescendantEntitiesProxyModelPrivate;

class DescendantEntitiesProxyModel : public QAbstractProxyModel
{
  Q_OBJECT
public:

    DescendantEntitiesProxyModel( QObject *parent = 0 );

    virtual ~DescendantEntitiesProxyModel();


    virtual void setSourceModel ( QAbstractItemModel * sourceModel );

    /**
    Sets the root index to @p index. This is the root of the proxy model.
    @param index The root index in the *source* model which will be shown in this model.
    if the index is invalid, the model is empty.
    */
    void setRootIndex( const QModelIndex &index);

    /**
    Set whether to show ancestor data in the model. If @p display is true, then
    a source model which is displayed as

    @code
    -> "Item 0-0" (this is row-depth)
    -> -> "Item 0-1"
    -> -> "Item 1-1"
    -> -> -> "Item 0-2"
    -> -> -> "Item 1-2"
    -> "Item 1-0"
    @endcode

    will be displayed as

    @code
    -> *Item 0-0"
    -> "Item 0-0 / Item 0-1"
    -> "Item 0-0 / Item 1-1"
    -> "Item 0-0 / Item 1-1 / Item 0-2"
    -> "Item 0-0 / Item 1-1 / Item 1-2"
    -> "Item 1-0"
    @endcode

    If @p display is false, the proxy will show

    @code
    -> *Item 0-0"
    -> "Item 0-1"
    -> "Item 1-1"
    -> "Item 0-2"
    -> "Item 1-2"
    -> "Item 1-0"
    @endcode

    Default is false.

    */
    void setDisplayAncestorData(bool display, const QString &sep = QString(" / "));

    /**
    Whether ancestor data will be displayed.
    */
    bool displayAncestorData() const;

    /**
    Separator used between data of ancestors.
    Returns a null QString() if displayAncestorData is false.
    */
    QString ancestorSeparator() const;

    QModelIndex mapFromSource ( const QModelIndex & sourceIndex ) const;
    QModelIndex mapToSource ( const QModelIndex & proxyIndex ) const;
    int descendantCount(const QModelIndex &index);

    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;

    virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex index(int, int, const QModelIndex&) const;
    virtual QModelIndex parent(const QModelIndex&) const;
    virtual int columnCount(const QModelIndex&) const;

private:
  Q_DECLARE_PRIVATE( DescendantEntitiesProxyModel )
  //@cond PRIVATE
  DescendantEntitiesProxyModelPrivate *d_ptr;

  Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeInserted(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsInserted(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeRemoved(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsRemoved(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceModelAboutToBeReset())
  Q_PRIVATE_SLOT(d_func(), void sourceModelReset())
  Q_PRIVATE_SLOT(d_func(), void sourceLayoutAboutToBeChanged())
  Q_PRIVATE_SLOT(d_func(), void sourceLayoutChanged())
  Q_PRIVATE_SLOT(d_func(), void sourceDataChanged(const QModelIndex &, const QModelIndex &))

  // Make these private, they shouldn't be called by applications
//   virtual bool insertRows(int , int, const QModelIndex & = QModelIndex());
//   virtual bool insertColumns(int, int, const QModelIndex & = QModelIndex());
//   virtual bool removeRows(int, int, const QModelIndex & = QModelIndex());
//   virtual bool removeColumns(int, int, const QModelIndex & = QModelIndex());


  //@endcond
};

}

#endif
