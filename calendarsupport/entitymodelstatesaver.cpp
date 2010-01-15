/*
    Copyright (c) 2009 KDAB
    Author: Frank Osterfeld <frank@kdab.net>

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

#include "entitymodelstatesaver.h"

#include <Akonadi/Collection>
#include <Akonadi/EntityTreeModel>

#include <QModelIndex>
#include <QString>

using namespace Akonadi;

EntityModelStateSaver::EntityModelStateSaver( QAbstractItemModel* model, QObject* parent ) : ModelStateSaver( model, parent ), d( 0 ) {
}

EntityModelStateSaver::~EntityModelStateSaver() {
}


QString EntityModelStateSaver::key( const QModelIndex &index ) const
{
    if ( !index.isValid() )
        return QLatin1String( "x-1" );
    const Collection c = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
    if ( c.isValid() )
        return QString::fromLatin1( "c%1" ).arg( c.id() );
    return QString::fromLatin1( "i%1" ).arg( index.data( EntityTreeModel::ItemIdRole ).value<Entity::Id>() );
}

