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

#ifndef SELECTIONPROXYMODEL_H
#define SELECTIONPROXYMODEL_H

#include <QAbstractProxyModel>

#include "kdeui_next_export.h"

class QItemSelectionModel;

class KSelectionProxyModelPrivate;

/**
@brief A Proxy Model which presents a subset of its source model to observers.

The KSelectionProxyModel is most useful as a convenience for displaying the selection in one view in another view.

For example, when a user clicks a mail folder in one view in an email applcation, the contained emails should be displayed in another view.

@code

MyModel *sourceModel = new MyModel(this);
QTreeView *leftView = new QTreeView(this);
leftView->setModel(sourceModel);

KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(leftView->selectionModel(), this);

QTreeView *rightView = new QTreeView(this);
rightView->setModel(selectionProxy);
@endcode

This takes away the need for the developer to handle the selection between the views, including all the mapToSource, mapFromSource and setRootIndex calls.

Additionally, this class can be used to programmatically choose some items from the source model to display in the view. For example,
this is how KMails Favourite folder View works, and how the AmazingCompleter works.

*/
class KDEUI_NEXT_EXPORT KSelectionProxyModel : public QAbstractProxyModel
{
  Q_OBJECT
public:
  /**
  ctor.

  @p selectionModel The selection model used to filter what is presented by the proxy.
  */
  
  explicit KSelectionProxyModel(QItemSelectionModel *selectionModel, QObject *parent = 0 );

  /**
  dtor
  */
  virtual ~KSelectionProxyModel();

  /**
  reimp.
  */
  virtual void setSourceModel ( QAbstractItemModel * sourceModel );

  QItemSelectionModel *selectionModel() const;

  enum Behaviour
  {
    OmitChildren,
    OmitDescendants,
    StartWithChildTrees,
    IncludeAllSelected
  };
  Q_DECLARE_FLAGS(Behaviours, Behaviour)  

  void setBehaviours(Behaviours behaviours);
  Behaviours behaviours() const;

  /**
  Do not include the children of selected items in the model.

  This will normally be used with includeAllSelected to get the following effect:

  @code
  A
  B
  - C
  - - D
  - - - E
  - F
  G
  @endcode

  If B, C, E and G are selected, you get a flat list of the selected items.

  @code
  B
  C
  E
  G
  @endcode

  */
  void setOmitChildren(bool omit);

  /**
    @code
    A
    B
    - C
    - - D
    - - - E
    - F
    G
    @endcode


    If B is selected,
      If omit is true: the proxy shows two levels of items.

      @code
      B
      - C
      @endcode

      else it shows all available levels:

      @code
      B
      - C
      - - D
      - - - E
      @endcode

  */
  void setOmitDescendants(bool omit);

  /**
    @code
    A
    B
    - C
    - - D
    - - - E
    - F
    G
    @endcode

    If startWithChildTrees is true,
      If B is selected, shows:

      @code
      C
      - D
      - - E
      F
      @endcode

    else show:
    @code
      B
      - C
      - - D
      - - - E
      - F
      @endcode
  */
  void setStartWithChildTrees(bool startWithChildTrees);

  /**
    @code
    A
    B
    - C
    - - D
    - - - E
    - F
    G
    @endcode

    Normally, if B and C are selected, only B and descendants appear in the proxy model because C (as a descendant)
    is already part of the model.

    However, if startWithChildTrees is true and omitDescendants is true, @p includeAllSelected can be set to true
    to process all selected indexes.

    If B and D are selected, it shows:

    @code
    C
    E
    F
    @endcode

    This method has no effect if either startWithChildTrees or omitDescendants are false.
  */
  void setIncludeAllSelected(bool includeAllSelected);

  QModelIndex mapFromSource ( const QModelIndex & sourceIndex ) const;
  QModelIndex mapToSource ( const QModelIndex & proxyIndex ) const;

  virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
  QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;
  virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
  virtual QModelIndex index(int, int, const QModelIndex& = QModelIndex() ) const;
  virtual QModelIndex parent(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex& = QModelIndex() ) const;

private:
  Q_DECLARE_PRIVATE(KSelectionProxyModel)
  //@cond PRIVATE
  KSelectionProxyModelPrivate *d_ptr;

  Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeInserted(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsInserted(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeRemoved(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsRemoved(const QModelIndex &, int, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int))
  Q_PRIVATE_SLOT(d_func(), void sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int))
  Q_PRIVATE_SLOT(d_func(), void sourceModelAboutToBeReset())
  Q_PRIVATE_SLOT(d_func(), void sourceModelReset())
  Q_PRIVATE_SLOT(d_func(), void sourceLayoutAboutToBeChanged())
  Q_PRIVATE_SLOT(d_func(), void sourceLayoutChanged())
  Q_PRIVATE_SLOT(d_func(), void sourceDataChanged(const QModelIndex &, const QModelIndex &))
  Q_PRIVATE_SLOT(d_func(), void selectionChanged( const QItemSelection & selected, const QItemSelection & deselected ) )

  //@endcond

};

Q_DECLARE_OPERATORS_FOR_FLAGS(KSelectionProxyModel::Behaviours)


#endif
