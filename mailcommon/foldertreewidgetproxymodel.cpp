/*
    Copyright (c) 2009, 2010, 2011 Laurent Montel <montel@kde.org>


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

#include "foldertreewidgetproxymodel.h"
#include "foldercollection.h"
#include "mailutil.h"
#include "mailkernel.h"

#include <akonadi/collection.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>
#include <kdebug.h>

#include <QtGui/QApplication>
#include <QtGui/QPalette>

namespace MailCommon {

class FolderTreeWidgetProxyModel::Private
{
public:
  Private()
    : enableCheck( false ),
      hideVirtualFolder( false ),
      hideSpecificFolder( false ),
      hideOutboxFolder( false )
    {
    }
  QString filterStr;
  bool enableCheck;
  bool hideVirtualFolder;
  bool hideSpecificFolder;
  bool hideOutboxFolder;
};

FolderTreeWidgetProxyModel::FolderTreeWidgetProxyModel( QObject *parent, FolderTreeWidgetProxyModelOptions option )
  : Akonadi::EntityRightsFilterModel( parent ),
    d( new Private )
{
  setDynamicSortFilter( true );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  
  if ( option & HideVirtualFolder ) {
    d->hideVirtualFolder = true;
  }
  if ( option & HideSpecificFolder ) {
    d->hideSpecificFolder = true;
  }
  if ( option & HideOutboxFolder ) {
    d->hideOutboxFolder = true;
  }
}

FolderTreeWidgetProxyModel::~FolderTreeWidgetProxyModel()
{
  delete d;
}


Qt::ItemFlags FolderTreeWidgetProxyModel::flags( const QModelIndex & index ) const
{
  if ( !d->filterStr.isEmpty() )
  {
    if ( !index.data().toString().contains( d->filterStr, Qt::CaseInsensitive ) )
      return KRecursiveFilterProxyModel::flags( index ) & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  }
  
  if ( d->enableCheck )
    return Akonadi::EntityRightsFilterModel::flags( index );

  return QSortFilterProxyModel::flags( index );
}

void FolderTreeWidgetProxyModel::setEnabledCheck( bool enable )
{
  if ( d->enableCheck == enable )
    return;
  d->enableCheck = enable;
  if ( enable ) {
    setAccessRights( Akonadi::Collection::CanCreateItem );
  }
}

bool FolderTreeWidgetProxyModel::enabledCheck() const
{
  return d->enableCheck;
}

void FolderTreeWidgetProxyModel::setHideVirtualFolder( bool exclude )
{
  d->hideVirtualFolder = exclude;
  invalidate();
}

bool FolderTreeWidgetProxyModel::hideVirtualFolder() const
{
  return d->hideVirtualFolder;
}

void FolderTreeWidgetProxyModel::setHideSpecificFolder( bool hide )
{
  d->hideSpecificFolder = hide;
  invalidate();
}

bool FolderTreeWidgetProxyModel::hideSpecificFolder() const
{
  return d->hideSpecificFolder;
}

void FolderTreeWidgetProxyModel::setHideOutboxFolder( bool hide )
{
  d->hideOutboxFolder = hide;
  invalidate();
}

bool FolderTreeWidgetProxyModel::hideOutboxFolder() const
{
  return d->hideOutboxFolder;
}

bool FolderTreeWidgetProxyModel::acceptRow( int sourceRow, const QModelIndex &sourceParent) const
{
  const QModelIndex modelIndex = sourceModel()->index( sourceRow, 0, sourceParent );

  const Akonadi::Collection collection = sourceModel()->data( modelIndex, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
  if ( d->hideVirtualFolder ) {
    if ( Util::isVirtualCollection( collection ) )
      return false;
  }
  if ( d->hideSpecificFolder ) {
    const QSharedPointer<FolderCollection> col = FolderCollection::forCollection( collection, false );
    if ( col && col->hideInSelectionDialog() )
      return false;
  }

  if ( d->hideOutboxFolder ) {
    if ( collection == Kernel::self()->outboxCollectionFolder() )
      return false;
  }
  if ( d->filterStr.isEmpty() )
    return Akonadi::EntityRightsFilterModel::acceptRow( sourceRow, sourceParent );
  return KRecursiveFilterProxyModel::acceptRow( sourceRow, sourceParent );
}

void FolderTreeWidgetProxyModel::setFilterFolder( const QString& filter )
{
  d->filterStr = filter;
  setFilterWildcard( filter );
}

QVariant FolderTreeWidgetProxyModel::data( const QModelIndex & index, int role) const
{
  if ( role == Qt::TextColorRole ) {
    const QModelIndex sourceIndex = mapToSource( index );
    const QModelIndex rowIndex = sourceIndex.sibling( sourceIndex.row(), 0 );
    const Akonadi::Collection collection = sourceModel()->data( rowIndex, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( !MailCommon::Util::isVirtualCollection(collection) )
    {

      const Akonadi::AgentInstance instance = Akonadi::AgentManager::self()->instance( collection.resource() );
      //TODO configurate it in 4.9 (we are in string freeze now)
      if ( !instance.isOnline() ) {
        return Qt::red;
      } else if ( instance.status() == Akonadi::AgentInstance::Broken ) {
        return Qt::gray;
      }
    }
  }
  return  Akonadi::EntityRightsFilterModel::data( index, role );
}

}


#include "foldertreewidgetproxymodel.moc"

