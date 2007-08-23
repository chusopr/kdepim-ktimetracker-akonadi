/* recordlistmodel.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "recordlistmodel.h"

RecordListModel::RecordListModel( QObject *parent )
	: QAbstractListModel( parent )
{
}

void RecordListModel::addItem( recordid_t id, const QString &item )
{
	fRecords.insert( item, id );
}

int RecordListModel::rowCount( const QModelIndex &parent ) const
{
	Q_UNUSED( parent );
	return fRecords.size();
}

QVariant RecordListModel::data ( const QModelIndex & index, int role ) const
{
	if( !index.isValid() )
	{
		return QVariant();
	}
	
	if( index.row() >= fRecords.size() )
	{
		return QVariant();
	}
	
	if( role == Qt::DisplayRole )
	{
		return fRecords.keys().at( index.row() );
	}
	
	return QVariant();
}

QVariant RecordListModel::headerData( int section, Qt::Orientation orientation
	, int role ) const
{
	Q_UNUSED( section );
	
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
		return QString("Record name");
	else
		return QVariant();
}
