/*
    Copyright (c) 2007 Bruno Virlet <bruno.virlet@gmail.com>
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

#include "entityfilterproxymodel.h"

#include "entitytreemodel.h"

#include <kdebug.h>
#include <kmimetype.h>

#include <QtCore/QString>
#include <QtCore/QStringList>

using namespace Akonadi;

/**
 * @internal
 */
class EntityFilterProxyModel::Private
{
  public:
    Private( EntityFilterProxyModel *parent )
      : mParent( parent )
    {
    }

    EntityFilterProxyModel *mParent;
    QStringList includedMimeTypes;
    QStringList excludedMimeTypes;

    QPersistentModelIndex m_rootIndex;
};

EntityFilterProxyModel::EntityFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent ),
    d( new Private( this ) )
{
  // TODO: Override setSourceModel and do this there?
  setSupportedDragActions( Qt::CopyAction | Qt::MoveAction );
}

EntityFilterProxyModel::~EntityFilterProxyModel()
{
  delete d;
}

void EntityFilterProxyModel::addMimeTypeInclusionFilters(const QStringList &typeList)
{
  d->includedMimeTypes << typeList;
  invalidateFilter();
}

void EntityFilterProxyModel::addMimeTypeExclusionFilters(const QStringList &typeList)
{
  d->excludedMimeTypes << typeList;
  invalidateFilter();
}

void EntityFilterProxyModel::addMimeTypeInclusionFilter(const QString &type)
{
  d->includedMimeTypes << type;
  invalidateFilter();
}

void EntityFilterProxyModel::addMimeTypeExclusionFilter(const QString &type)
{
  d->excludedMimeTypes << type;
  invalidateFilter();
}

bool EntityFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent) const
{
  // All rows that are not below m_rootIndex are unfiltered.
  QModelIndex _parent = sourceParent.parent();
  bool found = false;
  while (_parent.isValid())
  {
    if (_parent == d->m_rootIndex)
    {
      found = true;
    }
    _parent = _parent.parent();
  }

  if (!found)
  {
    return true;
  }

  QString rowMimetype = sourceModel()->data(
          sourceModel()->index(sourceRow, 0, sourceParent), EntityTreeModel::MimeTypeRole ).toString();
  if ( d->excludedMimeTypes.contains( rowMimetype ) )
    return false;
  if ( d->includedMimeTypes.isEmpty() || d->includedMimeTypes.contains( rowMimetype ) )
    return true;

  return false;
}

QStringList EntityFilterProxyModel::mimeTypeInclusionFilters() const
{
  return d->includedMimeTypes;
}

QStringList EntityFilterProxyModel::mimeTypeExclusionFilters() const
{
  return d->excludedMimeTypes;
}

void EntityFilterProxyModel::clearFilters()
{
  d->includedMimeTypes.clear();
  d->excludedMimeTypes.clear();
  invalidateFilter();
}

void EntityFilterProxyModel::setRootIndex(const QModelIndex &srcIndex)
{
  d->m_rootIndex = srcIndex;
  reset();
}

#include "entityfilterproxymodel.moc"
