/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** Databases approached through DLP / Pilot-link look different,
** so this file defines an API for them.
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
#include "options.h"

#include <time.h>
#include <iostream>

#include <pi-dlp.h>

#include <qfile.h>
#include <qtextcodec.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3CString>

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

#include "pilotAppCategory.h"
#include "pilotSerialDatabase.h"

static const char *pilotSerialDatabase_id =
	"$Id$";

PilotSerialDatabase::PilotSerialDatabase(int linksocket,
	const QString &dbName) :
	PilotDatabase(dbName),
	fDBName(QString::null),
	fDBHandle(-1),
	fDBSocket(linksocket)
{
	FUNCTIONSETUP;
	fDBName = dbName;

	openDatabase();

	/* NOTREACHED */
	(void) pilotSerialDatabase_id;
}

PilotSerialDatabase::~PilotSerialDatabase()
{
	FUNCTIONSETUP;
	closeDatabase();
}

QString PilotSerialDatabase::dbPathName() const
{
	QString s = CSL1("Pilot:");
	s.append(fDBName);
	return s;
}

// Reads the application block info
int PilotSerialDatabase::readAppBlock(unsigned char *buffer, int maxLen)
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return -1;
	}
#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	return dlp_ReadAppBlock(fDBSocket, getDBHandle(), 0, (void *) buffer,
		maxLen);
#else
	pi_buffer_t *buf = pi_buffer_new(maxLen);
	int r = dlp_ReadAppBlock(fDBSocket, getDBHandle(), 0 /* offset */, maxLen, buf);
	if (r>=0)
	{
		memcpy(buffer, buf->data, qMax(maxLen, r));
	}
	pi_buffer_free(buf);
	return r;
#endif
}

// Writes the application block info.
int PilotSerialDatabase::writeAppBlock(unsigned char *buffer, int len)
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return -1;
	}
	return dlp_WriteAppBlock(fDBSocket, getDBHandle(), buffer, len);
}

	// returns the number of records in the database
int PilotSerialDatabase::recordCount()
{
	int idlen;
	// dlp_ReadOpenDBInfo returns the number of bytes read and sets idlen to the # of recs
	if (isDBOpen() && dlp_ReadOpenDBInfo(fDBSocket, getDBHandle(), &idlen)>0)
	{
		return idlen;
	}
	else return -1;
}


// Returns a QValueList of all record ids in the database.
Q3ValueList<recordid_t> PilotSerialDatabase::idList()
{
	Q3ValueList<recordid_t> idlist;
	int idlen=recordCount();
	if (idlen<=0) return idlist;

	recordid_t *idarr=new recordid_t[idlen];
	int idlenread;
	int r = dlp_ReadRecordIDList (fDBSocket, getDBHandle(), 0, 0, idlen, idarr, &idlenread);

	if ( (r<0) || (idlenread<1) )
	{
		kdWarning() << k_funcinfo << ": Failed to read ID list from database." << endl;
		return idlist;
	}

	// now create the QValue list from the idarr:
	for (idlen=0; idlen<idlenread; idlen++)
	{
		idlist.append(idarr[idlen]);
	}
	delete[] idarr;
	return idlist;
}


// Reads a record from database by id, returns record length
PilotRecord *PilotSerialDatabase::readRecordById(recordid_t id)
{
	FUNCTIONSETUPL(3);
	int index, attr, category;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return 0L;
	}
	if (id>0xFFFFFF)
	{
		kdError() << k_funcinfo <<  " Encountered an invalid record id "
			<<id<<endl;;
		return 0L;
	}
#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	char buffer[PilotRecord::APP_BUFFER_SIZE];
	PI_SIZE_T size;
	if (dlp_ReadRecordById(fDBSocket, getDBHandle(), id, buffer, &index,
			&size, &attr, &category) >= 0)
		return new PilotRecord(buffer, size, attr, category, id);
#else
	pi_buffer_t *b = pi_buffer_new(InitialBufferSize);
	if (dlp_ReadRecordById(fDBSocket,getDBHandle(),id,b,&index,&attr,&category) >= 0)
	{
		return new PilotRecord(b, attr, category, id);
	}
#endif
	return 0L;
}

// Reads a record from database, returns the record length
PilotRecord *PilotSerialDatabase::readRecordByIndex(int index)
{
	FUNCTIONSETUPL(3);

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return 0L;
	}

	int attr, category;
	recordid_t id;
	PilotRecord *rec = 0L;

#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	char buffer[PilotRecord::APP_BUFFER_SIZE];
	PI_SIZE_T size;
	if (dlp_ReadRecordByIndex(fDBSocket, getDBHandle(), index,
			buffer, &id, &size, &attr, &category) >= 0)
	{
		rec = new PilotRecord(buffer, size, attr, category, id);
	}
#else
	pi_buffer_t *b = pi_buffer_new(InitialBufferSize);
	if (dlp_ReadRecordByIndex(fDBSocket, getDBHandle(), index,
		b, &id, &attr, &category) >= 0)
	{
		rec = new PilotRecord(b, attr, category, id);
	}
#endif


	return rec;
}

// Reads the next record from database in category 'category'
PilotRecord *PilotSerialDatabase::readNextRecInCategory(int category)
{
	FUNCTIONSETUP;
	int index, attr;
	recordid_t id;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return 0L;
	}
#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	char buffer[PilotRecord::APP_BUFFER_SIZE];
	PI_SIZE_T size;
	if (dlp_ReadNextRecInCategory(fDBSocket, getDBHandle(),
			category, buffer, &id, &index, &size, &attr) >= 0)
		return new PilotRecord(buffer, size, attr, category, id);
#else
	pi_buffer_t *b = pi_buffer_new(InitialBufferSize);
	if (dlp_ReadNextRecInCategory(fDBSocket, getDBHandle(),
		category,b,&id,&index,&attr) >= 0)
		return new PilotRecord(b, attr, category, id);
#endif
	return 0L;
}

// Reads the next record from database that has the dirty flag set.
PilotRecord *PilotSerialDatabase::readNextModifiedRec(int *ind)
{
	FUNCTIONSETUP;
	int index, attr, category;
	recordid_t id;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return 0L;
	}
#if PILOT_LINK_NUMBER < PILOT_LINK_0_12_0
	char buffer[PilotRecord::APP_BUFFER_SIZE];
	PI_SIZE_T size;
	if (dlp_ReadNextModifiedRec(fDBSocket, getDBHandle(), (void *) buffer,
			&id, &index, &size, &attr, &category) >= 0)
	{
		if (ind) *ind=index;
		return new PilotRecord(buffer, size, attr, category, id);
	}
#else
	pi_buffer_t *b = pi_buffer_new(InitialBufferSize);
	if (dlp_ReadNextModifiedRec(fDBSocket, getDBHandle(), b, &id, &index, &attr, &category) >= 0)
	{
		if (ind) *ind=index;
		return new PilotRecord(b, attr, category, id);
	}
#endif
	return 0L;
}

// Writes a new record to database (if 'id' == 0 or id>0xFFFFFF, one will be assigned and returned in 'newid')
recordid_t PilotSerialDatabase::writeRecord(PilotRecord * newRecord)
{
	FUNCTIONSETUP;
	recordid_t newid;
	int success;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return 0;
	}
	// Do some sanity checking to prevent invalid UniqueIDs from being written
	// to the handheld (RecordIDs are only 3 bytes!!!). Under normal conditions
	// this check should never yield true, so write out an error to indicate
	// someone messed up full time...
	if (newRecord->id()>0xFFFFFF)
	{
		kdError() << k_funcinfo << "Encountered an invalid record id "
			<<newRecord->id()<<", resetting it to zero.";
		newRecord->setID(0);
	}
	success =
		dlp_WriteRecord(fDBSocket, getDBHandle(),
		newRecord->attributes(), newRecord->id(),
		newRecord->category(), newRecord->data(),
		newRecord->size(), &newid);
	if ( (newRecord->id() != newid) && (newid!=0) )
		newRecord->setID(newid);
	return newid;
}

// Deletes a record with the given recordid_t from the database, or all records, if all is set to true. The recordid_t will be ignored in this case
int PilotSerialDatabase::deleteRecord(recordid_t id, bool all)
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo <<": DB not open"<<endl;
		return -1;
	}
	return dlp_DeleteRecord(fDBSocket, getDBHandle(), all?1:0, id);
}


// Resets all records in the database to not dirty.
int PilotSerialDatabase::resetSyncFlags()
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return -1;
	}
	return dlp_ResetSyncFlags(fDBSocket, getDBHandle());
}

// Resets next record index to beginning
int PilotSerialDatabase::resetDBIndex()
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return -1;
	}
	return dlp_ResetDBIndex(fDBSocket, getDBHandle());
}

// Purges all Archived/Deleted records from Palm Pilot database
int PilotSerialDatabase::cleanup()
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return -1;
	}
	return dlp_CleanUpDatabase(fDBSocket, getDBHandle());
}

void PilotSerialDatabase::openDatabase()
{
	FUNCTIONSETUP;
	int db;

	setDBOpen(false);

	QString s = getDBName();
	if (s.isEmpty())
	{
		kdError() << k_funcinfo << ": Bad DB name, "
			<< (s.isNull() ? "null" : "empty")
			<< " string given."
			<< endl;
		return;
	}

	Q3CString encodedName = QFile::encodeName(s);
	if (encodedName.isEmpty())
	{
		kdError() << k_funcinfo << ": Bad DB name, "
			<< (encodedName.isNull() ? "null" : "empty")
			<< " string given."
			<< endl;
		return;
	}

	char encodedNameBuffer[PATH_MAX];
	strlcpy(encodedNameBuffer,(const char *)encodedName,PATH_MAX);

	if (dlp_OpenDB(fDBSocket, 0, dlpOpenReadWrite,
		encodedNameBuffer, &db) < 0)
	{
		kdError() << k_funcinfo
			<< i18n("Cannot open database")
			<< i18n("Pilot database error") << endl;
		return;
	}
	setDBHandle(db);
	setDBOpen(true);
}

bool PilotSerialDatabase::createDatabase(long creator, long type, int cardno, int flags, int version)
{
	FUNCTIONSETUP;
	int db;

	// if the database is already open, we cannot create it again. How about completely resetting it? (i.e. deleting it and the createing it again)
	if (isDBOpen()) return true;
	// The latin1 seems ok, database names are latin1.
	int res=dlp_CreateDB(fDBSocket,
		creator, type, cardno, flags, version,
		PilotAppCategory::codec()->fromUnicode(getDBName()), &db);
	if (res<0) {
		kdError() <<k_funcinfo
			<< i18n("Cannot create database %1 on the handheld").arg(getDBName())<<endl;
		return false;
	}
	// TODO: Do I have to open it explicitly???
	setDBHandle(db);
	setDBOpen(true);
	return true;
}

void PilotSerialDatabase::closeDatabase()
{
	FUNCTIONSETUP;
	if (!isDBOpen() ) return;

	dlp_CloseDB(fDBSocket, getDBHandle());
	setDBOpen(false);
}

int PilotSerialDatabase::deleteDatabase()
{
	FUNCTIONSETUP;

	if (isDBOpen()) closeDatabase();

	return dlp_DeleteDB(fDBSocket, 0, PilotAppCategory::codec()->fromUnicode(fDBName));
}



/* virtual */ PilotDatabase::DBType PilotSerialDatabase::dbType() const
{
	return eSerialDB;
}

