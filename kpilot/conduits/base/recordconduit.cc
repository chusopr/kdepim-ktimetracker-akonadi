/* RecordConduit.cc			KPilot
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

#include "recordconduit.h"

#include "options.h"
#include "pilotDatabase.h"
#include "pilotRecord.h"
#include "kpilotSettings.h"

#include "idmapping.h"
#include "cudcounter.h"
#include "dataproxy.h"
#include "hhdataproxy.h"
#include "record.h"
#include "hhrecord.h"
#include "recordconduitSettings.h"

#include <KMessageBox>

RecordConduit::RecordConduit( KPilotLink *o, const QStringList &a
	, const QString &databaseName, const QString &conduitName ) :
	ConduitAction( o, conduitName.toLatin1(), a ),
	fDatabaseName( databaseName ),
	fHHDataProxy( 0L ),
	fBackupDataProxy( 0L ),
	fPCDataProxy( 0L )
	
{
	fConduitName = conduitName;
}

RecordConduit::~RecordConduit()
{
	delete fHHDataProxy;
	delete fBackupDataProxy;
	delete fPCDataProxy;
}

/* virtual */ bool RecordConduit::exec()
{
	FUNCTIONSETUP;
	
	loadSettings();
	
	// Try to open the handheld database and the local backup database.
	bool retrieved = false;
	
	bool hhDatabaseOpen = openDatabases( fDatabaseName, &retrieved );
	// If retrieved is true there was no local backup database so a first sync 
	// should be done then (see 6.3.2).
	bool backupDatabaseOpen = !retrieved;
	setFirstSync( retrieved );
	
	// NOTE: Do not forget that the HHData proxy and the backup proxy must use
	// the opened databases, maybe we should pass them for clearity to this method.
	if( !initDataProxies() )
	{
		DEBUGKPILOT << "One of the dataproxies was not initialized";
		return false;
	}
	
	// For the pc data proxy we can only after initilisation know if it could be
	// opened.
	bool pcDatabaseOpen = fPCDataProxy->isOpen();
	
	// See 6.2
	fMapping = new IDMapping( KPilotSettings::userName(), fConduitName );
	if( !fMapping->isValid( fBackupDataProxy->ids() ) )
	{
		setFirstSync( true );
	}
	
	// Determine syncmode, see 6.3
	// FIXME: Is this the most efficient way, or is it even possible what i do 
	//        here?
	DEBUGKPILOT << "Databases open [hh,pc,bu]: [" << hhDatabaseOpen << "," 
		<< pcDatabaseOpen << "," << backupDatabaseOpen << "]";
	
	if( hhDatabaseOpen && pcDatabaseOpen && backupDatabaseOpen )
	{
		DEBUGKPILOT << "All proxies are initialized and open.";
		// So what are we going to do this time?!
		if( isFirstSync() )
		{
			firstSync();
		}
		else
		{
			switch( syncMode().mode() ) {
				case SyncMode::eHotSync:
					hotOrFullSync();
					break;
				case SyncMode::eFullSync:
					hotOrFullSync();
					break;
				case SyncMode::eCopyPCToHH:
					copyPCToHH();
					break;
				case SyncMode::eCopyHHToPC:
					copyHHToPC();
					break;
				// Backup and restore should not happen here, if so default to hotsync.
				default:
					changeSync( SyncMode::eHotSync );
					hotOrFullSync();
					break;
			}
		}
	}
	else if( hhDatabaseOpen && pcDatabaseOpen && !backupDatabaseOpen )
	{
		DEBUGKPILOT << "HHDatabase open, PCDatabase open, Bakcupdatabase closed.";
		setFirstSync( true ); // 6.3.2
		firstSync();
	}
	else if( hhDatabaseOpen && !pcDatabaseOpen )
	{
		DEBUGKPILOT << "Pc database not open, trying to create an empty datastore.";
		if( fPCDataProxy->createDataStore() )
		{
			changeSync( SyncMode::eCopyHHToPC ); // 6.3.3 and 6.3.4
			copyHHToPC();
		}
		else
		{
			DEBUGKPILOT << "Could not open or create pc data store.";
			return false;
		}
	}
	else if( !hhDatabaseOpen && pcDatabaseOpen )
	{
		DEBUGKPILOT << "HHDatabase closed, PCDatabase open, Bakcupdatabase closed.";
		
		if( fHHDataProxy->createDataStore() )
		{
			changeSync( SyncMode::eCopyPCToHH ); // 6.3.5 and 6.3.6
			copyPCToHH();
		}
		else
		{
			DEBUGKPILOT << "Could not open or create hand held data store.";
			return false;
		}
	}
	else
	{
		emit logError( i18n( "Failed to open the pc database and the handheld "
			"datbase, no data to sync." ) );
		return false; // 6.3.7 and 6.3.8
	}
	
	// Sync finished, set the endcount of the CUD counters
	fHHDataProxy->setEndcount();
	fPCDataProxy->setEndcount();
	
	fMapping->setLastSyncedDate( QDateTime::currentDateTime() );
	if( !fMapping->isValid( fHHDataProxy->ids() ) )
	{
		// TODO: Warn the user.
		return false;
	}
	
	if( !checkVolatility() )
	{
		// volatility bounds are exceeded or the user did not want to proceed.
		return false;
	}
	
	/*
	 * If from this point something goes wrong (which shouldn't because we did our
	 * very best to deliver sane data) some of the data (mapping, hh database or
	 * pc database) will be corrupted.
	 */
	if( !fHHDataProxy->commit() )
	{
		DEBUGKPILOT << "Commit of hh dataproxy failed.";
		fHHDataProxy->rollback();
		// TODO: notify user.
		return false;
	}
	
	if( !fPCDataProxy->commit() )
	{
		DEBUGKPILOT << "Commit of pc dataproxy failed.";
		
		fPCDataProxy->rollback();
		fHHDataProxy->rollback();
		// TODO: notify user.
		return false;
	}
	
	// Fix the ids.
	QMapIterator<QString,QString> it( fHHDataProxy->changedIds() );
	while( it.hasNext() )
	{
		it.next();
		fMapping->changeHHId( it.key(), it.value() );
	}
	
	it = QMapIterator<QString,QString>( fPCDataProxy->changedIds() );
	while( it.hasNext() )
	{
		it.next();
		fMapping->changePCId( it.key(), it.value() );
	}
	
	// Now we can commit the mapping.
	if( !fMapping->commit() )
	{
		DEBUGKPILOT << "Commit of mapping failed.";
		fMapping->rollback();
		fPCDataProxy->rollback();
		fHHDataProxy->rollback();
		return false;
	}
	
	// Make sure the backup database is an exact copy of the pilot database.
	updateBackupDatabase();
	
	// Clean up things like modified flags.
	fHHDataProxy->syncFinished();
	fPCDataProxy->syncFinished();
	
	return delayDone();
}

bool RecordConduit::checkVolatility()
{
	FUNCTIONSETUP;
	
	const CUDCounter *fCtrHH = fHHDataProxy->counter();
	const CUDCounter *fCtrPC = fPCDataProxy->counter();

	addSyncLogEntry(fCtrHH->moo() +'\n',false);
	DEBUGKPILOT <<":" << fCtrHH->moo();
	addSyncLogEntry(fCtrPC->moo() +'\n',false);
	DEBUGKPILOT <<":" << fCtrPC->moo();

	// STEP2 of making sure we don't delete our little user's
	// precious data...
	// sanity checks for handheld...
	int hhVolatility = fCtrHH->percentDeleted() +
		fCtrHH->percentUpdated() + fCtrHH->percentCreated();
	
	int pcVolatility = fCtrPC->percentDeleted() +
		fCtrPC->percentUpdated() + fCtrPC->percentCreated();

	// TODO: allow user to configure this...
	// this is a percentage...
	int allowedVolatility = 70;
		
	QString caption = i18n( "Large Changes Detected" );
	// args are already i18n'd
	KLocalizedString template_query = ki18n( "The %1 conduit has made a "
		"large number of changes to your %2.  Do you want "
		"to allow this change?\nDetails:\n\t%3");

	// Default to allow changes
	int rc = KMessageBox::Yes;

	if (hhVolatility > allowedVolatility)
	{
		QString query = template_query
			.subs( fConduitName )
			.subs( i18n("handheld") )
			.subs( fCtrHH->moo() )
			.toString();
		
		DEBUGKPILOT << "High volatility." << " Check with user: [" << query << ']';
		
		rc = questionYesNo( query, caption, QString(), 0 );
	}
	
	if (pcVolatility > allowedVolatility)
	{
		QString query = template_query
			.subs( fConduitName )
			.subs( i18n( "pc" ) )
			.subs( fCtrPC->moo() )
			.toString();
		
		DEBUGKPILOT << "High volatility." << " Check with user: [" << query << ']';
		
		rc = questionYesNo( query, caption, QString(), 0 );
	}
	
	// Allow the changes
	if( rc == KMessageBox::Yes )
	{
		return true;
	}

	// Disallow the changes
	return false;
}

void RecordConduit::updateBackupDatabase()
{
	FUNCTIONSETUP;
	
	int index = 0;
	PilotRecord *rec = fDatabase->readRecordByIndex( index );
	QList<recordid_t> addedIds;
	
	// Copy all records from the pilot database to the local copy of it.
	while( rec )
	{
		fLocalDatabase->writeRecord( rec );
		addedIds.append( rec->id() );
		rec = fDatabase->readRecordByIndex( ++index );
	}
	
	index = 0;
	rec = fLocalDatabase->readRecordByIndex( index );
	
	// Remove all records that are not explicitly added in the previous loop.
	while( rec )
	{
		if( !addedIds.contains( rec->id() ) )
		{
			fLocalDatabase->deleteRecord( rec->id() );
		}
		
		rec = fLocalDatabase->readRecordByIndex( ++index );
	}
}

// 4.1 || 5.2
void RecordConduit::hotOrFullSync()
{
	FUNCTIONSETUP;
	
	fSyncedPcRecords = new QStringList();
	
	if( syncMode().mode() == SyncMode::eHotSync )
	{
		// A hotsync only does modified records.
		DEBUGKPILOT << "Doing HotSync";
		fHHDataProxy->setIterateMode( DataProxy::Modified );
		fPCDataProxy->setIterateMode( DataProxy::Modified );
	}
	else
	{
		DEBUGKPILOT << "Doing FullSync";
		// Fullsync, all records.
		fHHDataProxy->setIterateMode( DataProxy::All );
		fPCDataProxy->setIterateMode( DataProxy::All );
	}
	
	// Walk through all modified hand held records. The proxy is responsible for
	// serving the right records.
	
	DEBUGKPILOT << "Walking over hh records.";
	
	fHHDataProxy->resetIterator();
	while( fHHDataProxy->hasNext() )
	{
		HHRecord *hhRecord = static_cast<HHRecord*>( fHHDataProxy->next() );
		HHRecord *backupRecord = static_cast<HHRecord*>( 
			fBackupDataProxy->find( hhRecord->id() ) );
		Record *pcRecord = 0L;
		
		QString pcRecordId = fMapping->pcRecordId( hhRecord->id() );
		if( !pcRecordId.isEmpty() ) {
			// There is a mapping.
			pcRecord = fPCDataProxy->find( pcRecordId );
		}
		
		syncRecords( pcRecord, backupRecord, hhRecord );
		
		// Keep track of the pc records that are in sync. Which is needed to avoid
		// strange result when iterating over the pc records. Getting the pc id from
		// the mapping assures that create pc records are taken also.
		QString pcId = fMapping->pcRecordId( hhRecord->id() );
		if( !pcId.isEmpty() )
		{
			fSyncedPcRecords->append( pcId );
		}
	}
	
	// Walk through all modified pc records. The proxy is responsible for
	// serving the right records.
	
	DEBUGKPILOT << "Walking over pc records.";
	
	fPCDataProxy->resetIterator();
	while( fPCDataProxy->hasNext() )
	{
		Record *pcRecord = fPCDataProxy->next();
		
		// Only sync those records which wheren't touched by the iteration over
		// hand held records.
		if( !fSyncedPcRecords->contains( pcRecord->id() ) )
		{
			HHRecord *backupRecord = 0L;
			HHRecord *hhRecord = 0L;
			
			QString hhRecordId = fMapping->hhRecordId( pcRecord->id() );
			
			if( !hhRecordId.isEmpty() ) {
				// There is a mapping.
				backupRecord = static_cast<HHRecord*>( fBackupDataProxy->find( hhRecordId ) );
				hhRecord = static_cast<HHRecord*>( fHHDataProxy->find( hhRecordId ) );
			}
			
			syncRecords( pcRecord, backupRecord, hhRecord );
		}
	}
	
	delete fSyncedPcRecords;
}

// 5.1
void RecordConduit::firstSync()
{
	FUNCTIONSETUP;
	
	// A firstSync iterates over all records.
	fHHDataProxy->setIterateMode( DataProxy::All );
	fPCDataProxy->setIterateMode( DataProxy::All );
	
	DEBUGKPILOT << "Walking over all hh records.";
	
	fHHDataProxy->resetIterator();
	while( fHHDataProxy->hasNext() )
	{
		HHRecord *hhRecord = static_cast<HHRecord*>( fHHDataProxy->next() );
		Record *pcRecord = findMatch( hhRecord );
		
		if( pcRecord )
		{
			// TODO: Make this configurable or something, maybe pcRecord should
			// overide or user should be asked which record should be used. In this
			// case it might even be useful to let the user select per field which
			// record should be used. For now the handheld record overides.
			
			// Overide pcRecord values with hhRecord values.
			//  ( from    , to       );
			copy( hhRecord, pcRecord );
			// Both records are in sync so they are no longer modified.
			hhRecord->synced();
			pcRecord->synced();
			
			fMapping->map( hhRecord->id(), pcRecord->id() );
		}
		else
		{
			DEBUGKPILOT << "No match found for:" << hhRecord->id();
			Record *pcRecord = createPCRecord( hhRecord );
			fPCDataProxy->create( pcRecord );
			fMapping->map( hhRecord->id(), pcRecord->id() );
		}
	}
	
	DEBUGKPILOT << "Walking over all pc records.";
	
	fPCDataProxy->resetIterator();
	while( fPCDataProxy->hasNext() )
	{
		Record *pcRecord = fPCDataProxy->next();
		
		if( !fMapping->containsPCId( pcRecord->id() ) )
		{
			HHRecord *hhRecord = createHHRecord( pcRecord );
			fHHDataProxy->create( hhRecord );
			
			fMapping->map( hhRecord->id(), pcRecord->id() );
		}
	}
}

// 5.3
void RecordConduit::copyHHToPC()
{
	FUNCTIONSETUP;
	
	fHHDataProxy->setIterateMode( DataProxy::All );
	fPCDataProxy->setIterateMode( DataProxy::All );
	
	DEBUGKPILOT << "Walking over all hh records.";
	
	// 5.3.4
	fHHDataProxy->resetIterator();
	while( fHHDataProxy->hasNext() )
	{
		HHRecord *hhRecord = static_cast<HHRecord*>( fHHDataProxy->next() );
		HHRecord *backupRecord = 0L;
		Record *pcRecord = 0L;
		
		QString hhId = hhRecord->id();
		
		if( fMapping->containsHHId( hhId ) )
		{
			DEBUGKPILOT << "Mapping exists, syncing records.";
			backupRecord = static_cast<HHRecord*>( fBackupDataProxy->find( hhId ) );
			pcRecord = fPCDataProxy->find( fMapping->pcRecordId( hhId ) );
			syncRecords( pcRecord, backupRecord, hhRecord );
		}
		else
		{
			DEBUGKPILOT << "Mapping does not exists, copy hh to pc.";
			
			Record *pcRecord = createPCRecord( hhRecord );
			fPCDataProxy->create( pcRecord );
			fMapping->map( hhRecord->id(), pcRecord->id() );
		}
	}
	
	DEBUGKPILOT << "Walking over all pc records.";
	
	fPCDataProxy->resetIterator();
	// 5.3.5
	while( fPCDataProxy->hasNext() )
	{
		Record *pcRecord = fPCDataProxy->next();
		
		if( !fMapping->containsPCId( pcRecord->id() ) )
		{
			// 5.3.5.1
			fPCDataProxy->remove( pcRecord->id() );
		}
		else
		{
			QString hhId = fMapping->hhRecordId( pcRecord->id() );
			
			// Remove the pc record if there is no record on the handheld anymore.
			if( !fHHDataProxy->find( hhId ) )
			{
				// 5.3.5.2
				fPCDataProxy->remove( pcRecord->id() );
				fMapping->removePCId( pcRecord->id() );
			}
		}
	}
}

// 5.4
void RecordConduit::copyPCToHH()
{
	FUNCTIONSETUP;
	
	fHHDataProxy->setIterateMode( DataProxy::All );
	fPCDataProxy->setIterateMode( DataProxy::All );
	
	DEBUGKPILOT << "Walking over all pc records.";
	
	// 5.4.4
	fPCDataProxy->resetIterator();
	while( fPCDataProxy->hasNext() )
	{
		Record *pcRecord = fPCDataProxy->next();
		HHRecord *backupRecord = 0L;
		HHRecord *hhRecord = 0L;
		
		QString pcId = pcRecord->id();
		
		if( fMapping->containsPCId( pcId ) )
		{
			DEBUGKPILOT << "Mapping exists, syncing records.";
			
			QString hhId = fMapping->hhRecordId( pcId );
			
			backupRecord = static_cast<HHRecord*>( fBackupDataProxy->find( hhId ) );
			hhRecord = static_cast<HHRecord*>( fHHDataProxy->find( hhId ) );
			syncRecords( pcRecord, backupRecord, hhRecord );
		}
		else
		{
			DEBUGKPILOT << "Mapping does not exists, copy pc to hh.";
			
			HHRecord *hhRecord = createHHRecord( pcRecord );
			fHHDataProxy->create( hhRecord );
			fMapping->map( hhRecord->id(), pcRecord->id() );
		}
	}
	
	DEBUGKPILOT << "Walking over all hh records.";
	
	fHHDataProxy->resetIterator();
	// 5.4.5
	while( fHHDataProxy->hasNext() )
	{
		Record *hhRecord = fHHDataProxy->next();
		
		if( !fMapping->containsHHId( hhRecord->id() ) )
		{
			// 5.4.5.1
			fHHDataProxy->remove( hhRecord->id() );
		}
		else
		{
			QString pcId = fMapping->pcRecordId( hhRecord->id() );
			
			// Remove the hh record if there is no record on the pc anymore.
			if( !fPCDataProxy->find( pcId ) )
			{
				// 5.4.5.2
				fHHDataProxy->remove( hhRecord->id() );
				fMapping->removeHHId( hhRecord->id() );
			}
		}
	}
}

Record* RecordConduit::findMatch( HHRecord *hhRec )
{
	FUNCTIONSETUP;
	
	fPCDataProxy->resetIterator();
	
	while( fPCDataProxy->hasNext() )
	{
		Record *pcRec = fPCDataProxy->next();
		
		if( pcRec->equal( hhRec ) )
		{
			return pcRec;
		}
	}
	
	return 0L;
}

void RecordConduit::syncRecords( Record *pcRecord, HHRecord *backupRecord,
	HHRecord *hhRecord )
{
	FUNCTIONSETUP;
	
	// Two records for which we seem to have a mapping.
	if( hhRecord && pcRecord )
	{
		if( hhRecord->isModified() )
		{
			if( pcRecord->isModified() )
			{
				if( pcRecord->isDeleted() && hhRecord->isDeleted() )
				{
					// Case: 6.5.12
					DEBUGKPILOT << "Case 6.5.12";
					deleteRecords( pcRecord, hhRecord );
				}
				else
				{
					// Case: 6.5.9, 6.5.10 or 6.5.11
					DEBUGKPILOT << "Case 6.5.9, 6.5.10 or 6.5.11";
					solveConflict( pcRecord, hhRecord );
				}
			}
			else
			{
				if( hhRecord->isDeleted() )
				{
					// Case 6.5.4
					DEBUGKPILOT << "Case 6.5.4";
					deleteRecords( pcRecord, hhRecord );
				}
				else
				{
					// Case 6.5.3 or 6.5.1 (fullSync)
					DEBUGKPILOT << "Case 6.5.3 or 6.5.1 (fullSync)";

					// Keep hhRecord values.
					copy( hhRecord, pcRecord );
					fPCDataProxy->update( pcRecord->id(), pcRecord );
					// Both records are in sync so they are no longer modified.
					hhRecord->synced();
					pcRecord->synced();
				}
			}
		}
		else if( pcRecord->isModified() )
		{
			if( pcRecord->isDeleted() )
			{
				//  Case: 6.5.7
				DEBUGKPILOT << "Case 6.5.7";
				deleteRecords( pcRecord, hhRecord );
			}
			else
			{
				// Case: 6.5.6
				DEBUGKPILOT << "Case 6.5.6";
				// Keep pc record values.
				copy( pcRecord, hhRecord );
				fHHDataProxy->update( hhRecord->id(), hhRecord );
				// Both records are in sync so they are no longer modified.
				hhRecord->synced();
				pcRecord->synced();
			}
		} 
		else if( isFullSync() )
		{
			if( !equal( pcRecord, backupRecord ) )
			{
				// Two handheld records.
				if( !hhRecord->equal( backupRecord ) )
				{
					solveConflict( pcRecord, hhRecord );
				}
				else
				{
					//  ( from    , to       )
					copy( pcRecord, hhRecord );
					// Both records are in sync so they are no longer modified.
					fHHDataProxy->update( hhRecord->id(), hhRecord );
					hhRecord->synced();
					pcRecord->synced();
				}
			}
			else if( !hhRecord->equal( backupRecord ) )
			{
				//  ( from    , to       )
				copy( hhRecord, pcRecord );
				// Both records are in sync so they are no longer modified.
				fPCDataProxy->update( pcRecord->id(), pcRecord );
				hhRecord->synced();
				pcRecord->synced();
			}
		}
		else
		{
			// Case: 6.5.1 (hotSync)
			DEBUGKPILOT << "Case 6.5.1";
		}
	}
	else if( hhRecord )
	{
		// Case: 6.5.2 or 6.5.8
		// Warning id is a temporary id. Only after commit we know what id is
		// assigned to the record. So on commit the proxy should get the mapping
		// so that it can change the mapping.
		DEBUGKPILOT << "Case 6.5.2 and 6.5.8";
		pcRecord = createPCRecord( hhRecord );
		QString id = fPCDataProxy->create( pcRecord );
		fMapping->map( hhRecord->id(), id );
		
		pcRecord->synced();
		hhRecord->synced();
	}
	else if( pcRecord )
	{
		if( fMapping->containsPCId( pcRecord->id() ) && pcRecord->isDeleted() )
		{
			DEBUGKPILOT << "Case 6.5.17 - pc:" << pcRecord->id();
			fMapping->removePCId( pcRecord->id() );
			fPCDataProxy->remove( pcRecord->id() );
		}
		else
		{
			// Case: 6.5.5 or 6.5.8
			DEBUGKPILOT << "Case 6.5.5 or 6.5.8";
			hhRecord = createHHRecord( pcRecord );
			QString id = fHHDataProxy->create( hhRecord );
			fMapping->map( id, pcRecord->id() );
			
			pcRecord->synced();
			hhRecord->synced();
		}
	}
	else
	{
		DEBUGKPILOT << "This should not happen.";
	}
}

void RecordConduit::syncConflictedRecords( Record *pcRecord, HHRecord *hhRecord
	, bool pcOverides )
{
	FUNCTIONSETUP;
	
	if( pcOverides )
	{
		if( pcRecord->isDeleted() )
		{
			deleteRecords( pcRecord, hhRecord );
		}
		else
		{
			// Keep pcRecord. The hhRecord is changed so undo that changes.
			copy( pcRecord, hhRecord );
			fHHDataProxy->update( hhRecord->id(), hhRecord );
			// Both records are in sync so they are no longer modified.
			hhRecord->synced();
			pcRecord->synced();
		}
	}
	else
	{
		if( hhRecord->isDeleted() )
		{
			if( pcRecord->isModified() && hhRecord->isArchived() )
			{
				DEBUGKPILOT << "Case 6.5.16";
				// Keep hhRecordValues.
				copy( hhRecord, pcRecord );
				fPCDataProxy->update( pcRecord->id(), pcRecord );
				// Both records are in sync so they are no longer modified.
				hhRecord->synced();
				pcRecord->synced();
			}
			// else { DEBUGKPILOT << "Case 6.5.15"; }
			deleteRecords( pcRecord, hhRecord );
		}
		else
		{
			// Keep hhRecord. The pcRecord is changed so undo that changes.
			copy( hhRecord, pcRecord );
			fPCDataProxy->update( pcRecord->id(), pcRecord );
			// Both records are in sync so they are no longer modified.
			hhRecord->synced();
			pcRecord->synced();
		}
	}
}

void RecordConduit::deleteRecords( Record *pcRecord, HHRecord *hhRecord )
{
	FUNCTIONSETUP;
	
	fHHDataProxy->remove( hhRecord->id() );
	
	if( !hhRecord->isArchived() )
	{
		DEBUGKPILOT << "record not archived:" << hhRecord->id();
		fPCDataProxy->remove( pcRecord->id() );
		fMapping->removePCId( pcRecord->id() );
	}
	else
	{
		DEBUGKPILOT << "record archived:" << hhRecord->id();
		fMapping->archiveRecord( hhRecord->id() );
	}
}

void RecordConduit::solveConflict( Record *pcRecord, HHRecord *hhRecord )
{
	FUNCTIONSETUP;
	
	// NOTE: One of the two records might be 0L, which means that it's deleted.
	
	DEBUGKPILOT << "Solving conflict for pc " << pcRecord->id() 
		<< " and hh " << hhRecord->id();
	
	int res = getConflictResolution();
	if ( res == SyncAction::eAskUser )
	{
		// TODO: Make this nicer, like the abbrowser conduit had.
		QString query = i18n( "The following item was modified "
			"both on the Handheld and on your PC:\nPC entry:\n\t" );
		if( pcRecord )
		{
			query += pcRecord->toString();
		}
		else
		{
			query += i18nc( "The pc record is deleted.", "deleted" );
		}
		query += i18n( "\nHandheld entry:\n\t" );
		if( hhRecord )
		{
			query += hhRecord->toString();
		}
		else
		{
			query += i18nc( "The pilot record is deleted.", "deleted" );
		}
		query += i18n( "\n\nWhich entry do you want to keep? It will "
			"overwrite the other entry." );

		if( KMessageBox::No == questionYesNo(
			query,
			i18n( "Conflicting Entries" ),
			QString(),
			0 /* Never timeout */,
			i18n( "Handheld" ), i18n( "PC" )) )
		{
			// Keep PC record
			syncConflictedRecords( pcRecord, hhRecord, true );
		}
		else
		{
			// Keep Handheld record
			syncConflictedRecords( pcRecord, hhRecord, false );
		}
	}
	else if( res == eHHOverrides )
	{
		// Keep Handheld record
		syncConflictedRecords( pcRecord, hhRecord, false );
	}
	else if( res == ePCOverrides )
	{
		// Keep PC record
		syncConflictedRecords( pcRecord, hhRecord, true );
	}
	else if( res == eDuplicate )
	{
		/*
		 * break the previous relationship and create a new one on both sides, 
		 * duplicating bothrecords
		 */
		fMapping->removePCId( pcRecord->id() );
		
		HHRecord *hhRec = createHHRecord( pcRecord );
		QString id = fHHDataProxy->create( hhRec );
		fMapping->map( id, pcRecord->id() );
		
		Record *pcRec = createPCRecord( hhRecord );
		id = fPCDataProxy->create( pcRec );
		fMapping->map( id, pcRecord->id() );
	}
	else if( res == ePreviousSyncOverrides )
	{
		// FIXME: Implement.
		
	}
	
	// else: eDoNothing
	//return true;
}

void RecordConduit::copy( const Record *from, HHRecord *to )
{
	FUNCTIONSETUP;
	
	copyCategory( from, to );
	
	// Let implementing classess do the rest of the copying.
	_copy( from, to );
}
	
void RecordConduit::copy( const HHRecord *from, Record *to  )
{
	FUNCTIONSETUP;
	
	copyCategory( from, to );
	
	// Let implementing classess do the rest of the copying.
	_copy( from, to );
}

void RecordConduit::copyCategory( const Record *from, HHRecord *to )
{
	FUNCTIONSETUP;
	
	DEBUGKPILOT << "(Record *from, HHRecord *to)";
		
	if( !from || !to )
	{
		return;
	}

	QStringList pcCategories = to->categoryNames();
	if( pcCategories.size() < 1 )
	{
		// The pc record has no categories.
		to->setCategory( Pilot::Unfiled, i18nc( "No category set for this record"
			, "Unfiled" ) );
		return;
	}

	// Quick check: does the record (not unfiled) have an entry
	// in the categories list? If so, use that.
	if( to->categoryId() != Pilot::Unfiled )
	{
		QString hhCat = to->categoryName();
		
		if( pcCategories.contains( hhCat ) )
		{
			// Found, so leave the category unchanged.
			return;
		}
	}

	QStringList availableHandheldCategories = fHHDataProxy->categoryNames();

	// Either the record is unfiled, and should be filed, or
	// it has a category set which is not in the list of
	// categories that the event has. So go looking for
	// a category that is available both for the event
	// and on the handheld.
	for ( QStringList::ConstIterator it = pcCategories.begin();
		it != pcCategories.end(); ++it )
	{
		// Odd, an empty category string.
		if( (*it).isEmpty() )
		{
			continue;
		}

		if( availableHandheldCategories.contains( *it ) )
		{
			// Since the string is in the list of available categories,
			// this *can't* fail.
			int c = fHHDataProxy->categoryId( *it );
			Q_ASSERT( Pilot::validCategory(c) );
			to->setCategory( c, *it );
			return;
		}
	}

	to->setCategory( Pilot::Unfiled, i18nc( "No category set for this record"
			, "Unfiled" ) );
}

void RecordConduit::copyCategory( const HHRecord *from, Record *to  )
{
	FUNCTIONSETUP;
	
	DEBUGKPILOT << "(HHRecord *from, Record  *to)";
	
	if( !from || !to )
	{
		return;
	}

	QStringList pcCategories = to->categoryNames();
	QString hhCategory = from->categoryName();
	int cat = from->categoryId();

	DEBUGKPILOT << "HH category id " << cat << " label: [" << hhCategory << ']';

	if( Pilot::validCategory( cat ) && ( cat != Pilot::Unfiled ) )
	{
		if( !pcCategories.contains( hhCategory ) )
		{
			// if this event only has one category associated with it, then we can
			// safely assume that what we should be doing here is changing it to match
			// the palm. if there's already more than one category in the event,
			// however, we won't cause data loss--we'll just append what the palm has
			// to the event's categories.
			if( pcCategories.count() <= 1 )
			{
				pcCategories.clear();
			}

			pcCategories.append( hhCategory );
			to->setCategoryNames( pcCategories );
		}
	}

	DEBUGKPILOT << "PC categories now: [" << pcCategories.join( "," ) << ']';
}
