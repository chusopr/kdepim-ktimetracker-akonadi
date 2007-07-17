#ifndef IDMAPPING_H
#define IDMAPPING_H
/* idmapping.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
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

#include "pi-macros.h"

#include "idmappingxmlsource.h"

#include <QDateTime>
#include <QString>
#include <QMap>
#include <QFile>

class KPILOT_EXPORT IDMapping {
private:
	QString fConduitName;
	IDMappingXmlSource fSource;

public:
	/**
	 * Creates a new mapping object for given user and conduit.
	 */
	IDMapping( const QString &userName, const QString &conduit );

	/**
	 * Deletes any mapping that exists for @p hhRecordId and @p pcRecordId and 
	 * then creates a new mapping between @p hhRecordId and @p pcRecordId.
	 */
	void map( const QString &hhRecordId, const QString &pcRecordId );
	
	/**
	 * Searches for a mapping which contains @p hhRecordId and returns the id to
	 * which it is mapped. Returns QString() if no mapping is found.
	 */
	QString pcRecordId( const QString &hhRecordId );
	
	/**
	 * Searches for a mapping which contains @p pcRecordId and returns the id to
	 * which it is mapped. Returns QString() if no mapping is found.
	 */
	QString hhRecordId( const QString &pcRecordId );
	
	/**
	 * Search for a mapping for which the hh id == @p hhRecordId and if one is
	 * found it will be removed.
	 */
	void removeHHId( const QString &hhRecordId );
	
	/**
	 * Search for a mapping for which the pc id == @p pcRecordId and if one is
	 * found it will be removed.
	 */
	void removePCId( const QString &pcRecordId );

	/**
	 * Method to find out wether or not there is a mapping for hh record with 
	 * @p hhRecordId.
	 */
	bool containsHHId( const QString &hhRecordId );

	/**
	 * Method to find out wether or not there is a mapping for pc record with 
	 * @p pcRecordId.
	 */
	bool containsPCId( const QString &pcRecordId );
	
	/**
	 * Validates the mapping file with given dataproxy. The mapping is considered 
	 * valid if:
	 * 1. The number of mappings matches the number of records in @p ids.
	 * 2. Every id that is in @p recordIds has a mapping.
	 *
	 * NOTE: The list of id's should be a list of handheld ids or a list of pc 
	 * ids. Not a mix of them.
	 */
	bool isValid( const QList<QString> &recordIds );

	/**
	 * Sets the date/time on which the last sync is executed to @p dateTime.
	 */
	void setLastSyncedDate( const QDateTime &dateTime );

	/**
	 * Sets the pc on which the last sync is executed to @p pc.
	 */
	void setLastSyncedPC( const QString &pc );

	/**
	 * Saves the changes to persistent storage.
	 */
	bool commit();
	
	/**
	 * Tries to undo the changes in persistent storage.
	 */
	bool rollback();
};
#endif
