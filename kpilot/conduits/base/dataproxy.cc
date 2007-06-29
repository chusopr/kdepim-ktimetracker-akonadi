/* dataproxy.cc			KPilot
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
#include "dataproxy.h"
#include "cudcounter.h"

DataProxy::DataProxy()
{
}

DataProxy::~DataProxy()
{
}

/**
 * Adds the record to the database and returns the internal id for the added record.
 */
QVariant DataProxy::addRecord()
{
	return QVariant();
}

void DataProxy::deleteRecord()
{
}

void DataProxy::editRecord()
{
}

void DataProxy::syncFinished()
{
}

void DataProxy::setIterateMode( const Mode m )
{
	fMode = m;
}

