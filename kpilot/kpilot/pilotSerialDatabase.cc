// pilotSerialDatabase.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// This is pilotSerialDatabase.cc for KDE 2 / KPilot 4.
//
// TODO:
//
//

#include "options.h"

#include <time.h>
#include <stream.h>
#include <pi-dlp.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include "pilotSerialDatabase.h"
#include "kpilotlink.h"

PilotSerialDatabase::PilotSerialDatabase(KPilotLink* pilotLink, const char* dbName)
  : PilotDatabase(), fDBName(0L), fDBHandle(-1), fPilotLink(pilotLink)
    {
    fDBName = new char[strlen(dbName) + 1];
    strcpy(fDBName, dbName);
    openDatabase();
    }

PilotSerialDatabase::~PilotSerialDatabase()
    {
    closeDatabase();
    delete [] fDBName;
    }

// Reads the application block info
int PilotSerialDatabase::readAppBlock(unsigned char* buffer, int maxLen)
    {
	EFUNCTIONSETUP;
    if(isDBOpen() == false)
	{
	kdError() << fname << ": DB not open" << endl;
	return -1;
	}
    return dlp_ReadAppBlock(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), 0, (void *)buffer, maxLen);
    }

// Writes the application block info.
int PilotSerialDatabase::writeAppBlock(unsigned char* buffer, int len)
{
	EFUNCTIONSETUP;
  if(isDBOpen() == false)
    {
      kdError() << fname << ": DB not open" << endl;
      return -1;
    }
  return dlp_WriteAppBlock(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), buffer, len);
}


// Reads a record from database by id, returns record length
PilotRecord* PilotSerialDatabase::readRecordById(recordid_t id)
    {
	EFUNCTIONSETUP;
    char* buffer[0xffff];
    int index, size, attr, category;

    if(isDBOpen() == false)
	{
	kdError() << fname << ": DB not open" << endl;
	return 0L;
	}
    if(dlp_ReadRecordById(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), id, buffer, &index,
			      &size, &attr, &category) >= 0)
	return new PilotRecord(buffer, size, attr, category, id);
    return 0L;
    }

// Reads a record from database, returns the record length
PilotRecord* PilotSerialDatabase::readRecordByIndex(int index)
    {
	EFUNCTIONSETUP;
    char* buffer[0xffff];
    int size, attr, category;
    recordid_t id;

    if(isDBOpen() == false)
	{
	kdError() << fname << ": DB not open" << endl;
	return 0L;
	}
    if(dlp_ReadRecordByIndex(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), index, 
				 (void*)buffer, &id, &size, &attr, &category) >= 0)
	return new PilotRecord(buffer, size, attr, category, id);
    return 0L;	
    }

// Reads the next record from database in category 'category'
PilotRecord* PilotSerialDatabase::readNextRecInCategory(int category)
    {
	EFUNCTIONSETUP;
    char* buffer[0xffff];
    int index, size, attr;
    recordid_t id;

    if(isDBOpen() == false)
	{
	kdError() << fname << ": DB not open" << endl;
	return 0L;
	}
    if(dlp_ReadNextRecInCategory (getPilotLink()->getCurrentPilotSocket(), getDBHandle(), 
				      category, buffer, &id, &index, &size, &attr) >= 0)
	return new PilotRecord(buffer, size, attr, category, id);
    return 0L;
    }

// Reads the next record from database that has the dirty flag set.
PilotRecord* PilotSerialDatabase::readNextModifiedRec()
    {
	EFUNCTIONSETUP;
    char* buffer[0xffff];
    int index, size, attr, category;
    recordid_t id;

    if(isDBOpen() == false)
	{
	kdError() << fname << ": DB not open" << endl;
	return 0L;
	}
    if(dlp_ReadNextModifiedRec(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), (void*)buffer, 
				   &id, &index, &size, &attr, &category) >= 0)
	return new PilotRecord(buffer, size, attr, category, id);
    return 0L;
    }

// Writes a new record to database (if 'id' == 0, one will be assigned and returned in 'newid')
int PilotSerialDatabase::writeRecord(PilotRecord* newRecord)
    {
	EFUNCTIONSETUP;
    recordid_t newid;
    int success;

    if(isDBOpen() == false)
	{
	kdError() << fname << ": DB not open" << endl;
	return -1;
	}
    success = dlp_WriteRecord(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), newRecord->getAttrib(), 
			      newRecord->getID(), newRecord->getCat(), newRecord->getData(),
			      newRecord->getLen(), &newid);
    if(newRecord->getID() == 0)
	newRecord->setID(newid);
    return success;
    }

// Resets all records in the database to not dirty.
int PilotSerialDatabase::resetSyncFlags()
    {
	EFUNCTIONSETUP;
    if(isDBOpen() == false)
	{
	kdError() << fname << ": DB not open" << endl;
	return -1;
	}
    return dlp_ResetSyncFlags(getPilotLink()->getCurrentPilotSocket(), getDBHandle()); 
    }

// Resets next record index to beginning
int PilotSerialDatabase::resetDBIndex()
    {
	EFUNCTIONSETUP;
    if(isDBOpen() == false)
	{
	kdError() << fname << ": DB not open" << endl;
	return -1;
	}
    return dlp_ResetDBIndex(getPilotLink()->getCurrentPilotSocket(), getDBHandle()); 
    }

// Purges all Archived/Deleted records from Palm Pilot database
int PilotSerialDatabase::cleanUpDatabase()
    {
	EFUNCTIONSETUP;
    if(isDBOpen() == false)
	{
	kdError() << fname << ": DB not open" << endl;
	return -1;
	}
    return dlp_CleanUpDatabase(getPilotLink()->getCurrentPilotSocket(), getDBHandle()); 
    }

void PilotSerialDatabase::openDatabase()
    {
    int db;

    if(dlp_OpenDB(getPilotLink()->getCurrentPilotSocket(), 0, dlpOpenReadWrite, getDBName(), &db) < 0)
	{
	KMessageBox::error(getPilotLink()->getOwningWidget(),
		i18n("Cannot open database"),
		i18n("Pilot database error"));
	return;
	}
    setDBHandle(db);
    setDBOpen(true);
    }

void PilotSerialDatabase::closeDatabase()
    {
    if(isDBOpen() == false)
	return;
    dlp_CloseDB(getPilotLink()->getCurrentPilotSocket(), getDBHandle());
    }
