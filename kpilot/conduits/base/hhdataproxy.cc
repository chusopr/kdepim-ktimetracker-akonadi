/* hhdataproxy.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
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

#include "hhdataproxy.h"

#include <klocalizedstring.h>

#include "pilotDatabase.h"
#include "pilotAppInfo.h"
#include "pilotRecord.h"
#include "options.h"
#include "pilot.h"

#include "hhrecord.h"
#include "hhcategory.h"

HHDataProxy::HHDataProxy( PilotDatabase *db ) : fDatabase( db )
	, fLastUsedUniqueId( 0L ), fUnfiled( 0L )
{
}

void HHDataProxy::syncFinished()
{
	FUNCTIONSETUP;
	
	if( fDatabase && fDatabase->isOpen() )
	{
		fDatabase->cleanup();
		fDatabase->resetSyncFlags();
		saveCategories();
	}
}

QString HHDataProxy::generateUniqueId()
{
	recordid_t id = 0;
	
	QList<QString> ids = fRecords.keys();
	
	for( int i = 0; i < fRecords.size(); i++ )
	{
		if( ids.at( i ).toULong() > id )
		{
			id = ids.at( i ).toULong();
		}
	}
	
	return QString::number( id + 1 );
}

void HHDataProxy::commitCreate( Record *rec )
{
	FUNCTIONSETUP;
	
	if( fDatabase && rec )
	{
		if( HHRecord *hhRec = static_cast<HHRecord*>( rec ) )
		{
			// Set the id to 0 to make sure that the database asigns a valid id to the
			// record.
			hhRec->setId( QString::number( 0 ) );
			fDatabase->writeRecord( hhRec->pilotRecord() );
		}
		else
		{
			DEBUGKPILOT << "Record " << rec->id() << " is not of type HHRecord*.";
		}
	}
}

void HHDataProxy::commitUpdate( Record *rec )
{
	FUNCTIONSETUP;

	if( fDatabase && rec )
	{
		if( HHRecord *hhRec = static_cast<HHRecord*>( rec ) )
		{
			fDatabase->writeRecord( hhRec->pilotRecord() );
		}
		else
		{
			DEBUGKPILOT << "Record " << rec->id() << " is not of type HHRecord*.";
		}
	}
}

void HHDataProxy::commitDelete( Record *rec )
{
	FUNCTIONSETUP;
	
	if( !rec || !fDatabase )
	{
		return;
	}
	else
	{
		if( HHRecord *hhRec = static_cast<HHRecord*>( rec ) )
		{
			fDatabase->deleteRecord( hhRec->pilotRecord()->id() );
		}
		else
		{
			DEBUGKPILOT << "Record " << rec->id() << " is not of type HHRecord*.";
		}
	}
}

bool HHDataProxy::isOpen() const
{
	FUNCTIONSETUP;
	
	if( fDatabase )
	{
		return fDatabase->isOpen();
	}
	else
	{
		return false;
	}
}

void HHDataProxy::loadCategories()
{
	FUNCTIONSETUP;
	
	if( fDatabase && fDatabase->isOpen() )
	{
		CategoryAppInfo *catAppInfo = readCategoryAppInfo();
		
		// NOTE: There's also a lastUniqueID in CategoryAppInfo but it isn't clear
		// to me what it does and how it should be used in the sync proces.
		
		for (unsigned int i = 0; i < Pilot::CATEGORY_COUNT; i++)
		{
			QString name = Pilot::categoryName( catAppInfo, i );
			bool renamed = catAppInfo->renamed[i];
			unsigned char id = catAppInfo->ID[i];
			
			HHCategory *cat = new HHCategory( name, renamed, i, id );
			fCategories.append( cat );
			
			if( i == (unsigned int) Pilot::Unfiled )
			{
				fUnfiled = cat;
			}
		}
	}
}

void HHDataProxy::loadAllRecords()
{
	FUNCTIONSETUP;
	
	if( fDatabase && fDatabase->isOpen() )
	{
		// This should initialize fAppInfo.
		loadCategories();
	
		int index = 0;
		
		PilotRecord *pRec = fDatabase->readRecordByIndex( index );
		
		while( pRec )
		{
			// Create a record object.
			Record *rec = createHHRecord( pRec );
			fRecords.insert( rec->id(), rec );
			
			HHCategory *cat = fCategories[pRec->category()];
			
			if( cat )
			{
				// This essentialy doesn't add a category to the record but just the
				// label for the category which is set in the pilot record.
				
				// FIXME: This is defenitly not the way to go.
				
				//QStringList categoryNames;
				//categoryNames.append( cat->name() );
				
				//rec->setCategoryNames( categoryNames );
			}
			else
			{
				// This shouldn't happen I think....the category id seems to have some
				// bogus value. So we set it to 0, which is Unfiled normaly.
				pRec->setCategory( 0 );
				
				// FIXME: This is defenitly not the way to go.
				
				//but if it does we set the category to Unfiled.
				//QStringList categoryNames;
				//categoryNames.append( i18nc( "Category not set for the record"
				//	, "Unfiled" ) );
				
				//rec->setCategoryNames( categoryNames );
			}
				
			// Read the next one.
			pRec = fDatabase->readRecordByIndex( ++index );
		}
		fCounter.setStartCount( fRecords.count() );
		
		DEBUGKPILOT << "Loaded " << fRecords.count() << " records.";
	}
}

QStringList HHDataProxy::categoryNames() const
{
	QStringList names;
	
	foreach( HHCategory *cat, fCategories )
	{
		names.append( cat->name() );
	}
		
	return names;
}

int HHDataProxy::categoryId( const QString &name ) const
{
	foreach( HHCategory *cat, fCategories )
	{
		if( name == cat->name() )
		{
			return cat->id();
		}
	}
	
	return 0;
}

HHCategory* HHDataProxy::category( const QString &name ) const
{
	foreach( HHCategory *cat, fCategories )
	{
		if( name == cat->name() )
		{
			return cat;
		}
	}
	
	return fUnfiled;
}
