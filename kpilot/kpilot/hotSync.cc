/* hotSync.cc                           KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines SyncActions, which are used to perform some specific
** task during a HotSync. Conduits are not included here, nor are 
** sync actions requiring user interaction. Those can be found in the
** conduits subdirectory or interactiveSync.h.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

static const char *hotsync_id =
	"$Id$";

#include "options.h"

#include <time.h>
#include <unistd.h>

#include <pi-file.h>

#include <qtimer.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qvaluelist.h>
#include <qregexp.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>

#include "pilotUser.h"

#include "hotSync.moc"

TestLink::TestLink(KPilotDeviceLink * p) :
	SyncAction(p, "testLink")
{
	FUNCTIONSETUP;

	(void) hotsync_id;
}

/* virtual */ bool TestLink::exec()
{
	FUNCTIONSETUP;

	int i;
	int dbindex = 0;
	int count = 0;
	struct DBInfo db;

	addSyncLogEntry(i18n("Testing.\n"));

#ifdef BRUTE_FORCE
	for (i=0; i<32; i++)
#else
	while ((i = fHandle->getNextDatabase(dbindex,&db)) > 0)
#endif
	{
#ifdef BRUTE_FORCE
		if (fHandle->getNextDatabase(i,&db) < 1)
		{
			DEBUGKPILOT << fname << ": No database index " << i << endl;
			continue;
		}
#endif

		count++;
		dbindex = db.index + 1;

#ifdef DEBUG
		DEBUGKPILOT << fname << ": Read database " << db.name << endl;
#endif

		// Let the Pilot User know what's happening
		openConduit();
		// Let the KDE User know what's happening
		emit logMessage(i18n("Syncing database %1...").arg(db.name));

		kapp->processEvents();
	}

	emit logMessage(i18n("HotSync finished."));
	emit syncDone(this);
	return true;
}

BackupAction::BackupAction(KPilotDeviceLink * p) :
	SyncAction(p, "backupAction")
{
	FUNCTIONSETUP;

	fDatabaseDir = KGlobal::dirs()->saveLocation("data",
		QString("kpilot/DBBackup/"));
}

/* virtual */ QString BackupAction::statusString() const
{
	FUNCTIONSETUP;
	QString s("BackupAction=");

	switch (status())
	{
	case Init:
		s.append("Init");
		break;
	case Error:
		s.append("Error");
		break;
	case FullBackup:
		s.append("FullBackup");
		break;
	case BackupEnded:
		s.append("BackupEnded");
		break;
	default:
		s.append("(unknown ");
		s.append(QString::number(status()));
		s.append(")");
	}

	return s;
}


/* virtual */ bool BackupAction::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": This Pilot user's name is \""
		<< fHandle->getPilotUser()->getUserName() << "\"" << endl;
#endif

	addSyncLogEntry(i18n("Full backup started."));

	ASSERT(!fTimer);

	fTimer = new QTimer(this);
	QObject::connect(fTimer, SIGNAL(timeout()),
		this, SLOT(backupOneDB()));

	fDBIndex = 0;
	fStatus = FullBackup;

	fTimer->start(0, false);
	return true;
}

/* slot */ void BackupAction::backupOneDB()
{
	FUNCTIONSETUP;

	struct DBInfo info;

	emit logProgress(QString::null, fDBIndex);

	if (openConduit() < 0)
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": openConduit failed. User cancel?" << endl;
#endif

		addSyncLogEntry(i18n("Exiting on cancel."));
		endBackup();
		fStatus = BackupIncomplete;
		return;
	}

	if (fHandle->getNextDatabase(fDBIndex, &info) < 0)
	{
#ifdef DEBUG
		DEBUGDAEMON << fname << ": Backup complete." << endl;
#endif

		addSyncLogEntry(i18n("Full backup complete."));
		endBackup();
		fStatus = BackupComplete;
		return;
	}

	fDBIndex = info.index + 1;

	QString s = i18n("Backing up: %1").arg(info.name);
	addSyncLogEntry(s);
	emit logMessage(s);

	if (!createLocalDatabase(&info))
	{
		kdError() << k_funcinfo
			<< ": Couldn't create local database for "
			<< info.name << endl;
		addSyncLogEntry(i18n("Backup of %1 failed.\n").arg(info.name));
	}
	else
	{
		addSyncLogEntry(i18n(" .. OK\n"));
	}
}

bool BackupAction::createLocalDatabase(DBInfo * info)
{
	FUNCTIONSETUP;

	QString fullBackupDir =
		fDatabaseDir + fHandle->getPilotUser()->getUserName() + "/";

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Looking in directory " << fullBackupDir << endl;
#endif

	QFileInfo fi(fullBackupDir);

	if (!(fi.exists() && fi.isDir()))
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Need to create backup directory for user "
			<< fHandle->getPilotUser()->getUserName() << endl;
#endif

		fi = QFileInfo(fDatabaseDir);
		if (!(fi.exists() && fi.isDir()))
		{
			kdError() << k_funcinfo
				<< ": Database backup directory "
				<< "doesn't exist."
				<< endl;
			return false;
		}

		QDir databaseDir(fDatabaseDir);

		if (!databaseDir.mkdir(fullBackupDir, true))
		{
			kdError() << k_funcinfo
				<< ": Can't create backup directory." << endl;
			return false;
		}
	}

	QString databaseName(info->name);

	databaseName.replace(QRegExp("/"), "_");

	QString fullBackupName = fullBackupDir + databaseName;

	if (info->flags & dlpDBFlagResource)
	{
		fullBackupName.append(".prc");
	}
	else
	{
		fullBackupName.append(".pdb");
	}

#ifdef DEBUG
	DEBUGDB << fname
		<< ": Creating local database " << fullBackupName << endl;
#endif

	/* Ensure that DB-open flag is not kept */
	info->flags &= ~dlpDBFlagOpen;

	return fHandle->retrieveDatabase(fullBackupName,info);
}

void BackupAction::endBackup()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fTimer);
	fDBIndex = (-1);
	fStatus = BackupEnded;

	emit syncDone(this);
}

FileInstallAction::FileInstallAction(KPilotDeviceLink * p,
	const QString & d,
	const QStringList & l) :
	SyncAction(p, "fileInstall"),
	fDBIndex(-1),
	fTimer(0L),
	fDir(d),
	fList(l)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": File list has "
		<< fList.  count() << " entries" << endl;

	QStringList::ConstIterator i;

	for (i = fList.begin(); i != fList.end(); ++i)
	{
		DEBUGDAEMON << fname << ": " << *i << endl;
	}
#endif
}

FileInstallAction::~FileInstallAction()
{
	FUNCTIONSETUP;

	// KPILOT_DELETE(fTimer);
}

/* virtual */ bool FileInstallAction::exec()
{
	FUNCTIONSETUP;

	fDBIndex = 0;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Installing " << fList.count() << " files" << endl;
#endif

	// Possibly no files to install?
	if (!fList.count())
	{
		emit logMessage(i18n("No Files to install"));
		emit syncDone(this);

		return true;
	}

	fTimer = new QTimer(this);
	QObject::connect(fTimer, SIGNAL(timeout()),
		this, SLOT(installNextFile()));

	fTimer->start(0, false);

	emit logProgress(i18n("Installing Files"), 0);
	return true;
}

/* slot */ void FileInstallAction::installNextFile()
{
	FUNCTIONSETUP;

	ASSERT(fDBIndex >= 0);
	ASSERT((unsigned) fDBIndex <= fList.count());

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Installing file index "
		<< fDBIndex << " (of " << fList.count() << ")" << endl;
#endif

	if ((!fList.count()) || ((unsigned) fDBIndex >= fList.count()))
	{
#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Peculiar file index, bailing out." << endl;
#endif
		KPILOT_DELETE(fTimer);
		fDBIndex = (-1);
		emit logProgress(i18n("Done Installing Files"), 100);
		emit syncDone(this);
		return;
	}

	const QString filePath = fDir + fList[fDBIndex];
	const QString fileName = fList[fDBIndex];

	fDBIndex++;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Installing file " << filePath << endl;
#endif

	QString m = i18n("Installing %1").arg(fileName);
	emit logProgress(m,(100 * fDBIndex) / (fList.count()+1));
	m+=QString::fromLatin1("\n");
	emit addSyncLogEntry(m,true /* Don't print in KPilot's log. */ );


	struct pi_file *f = 0L;

	f = pi_file_open(const_cast <char *>
		((const char *) QFile::encodeName(filePath)));

	if (!f)
	{
		kdWarning() << k_funcinfo
			<< ": Unable to open file." << endl;

		emit logError(i18n("Unable to open file &quot;%1&quot;!").
			arg(fileName));
		goto nextFile;
	}

	if (pi_file_install(f, pilotSocket(), 0) < 0)
	{
		kdWarning() << k_funcinfo << ": failed to install." << endl;


		emit logError(i18n("Cannot install file &quot;%1&quot;!").
			arg(fileName));
	}
	else
	{
		QFile::remove(filePath);
	}


nextFile:
	if (f) pi_file_close(f);
	if (fDBIndex == -1)
	{
		emit syncDone(this);
	}
}

/* virtual */ QString FileInstallAction::statusString() const
{
	FUNCTIONSETUP;
	if (fDBIndex < 0)
	{
		return QString("Idle");
	}
	else
	{
		if ((unsigned) fDBIndex >= fList.count())
		{
			return QString("Index out of range");
		}
		else
		{
			return QString("Installing %1").arg(fList[fDBIndex]);
		}
	}
}

CleanupAction::CleanupAction(KPilotDeviceLink *p)  : SyncAction(p,"cleanupAction")
{
	FUNCTIONSETUP;
}

CleanupAction::~CleanupAction()
{
#ifdef DEBUG
	FUNCTIONSETUP;
	DEBUGDAEMON << fname
		<< ": Deleting @" << (int)this << endl;
#endif
}

/* virtual */ bool CleanupAction::exec()
{
	FUNCTIONSETUP;

	fHandle->finishSync();
	emit syncDone(this);
	return true;
}


// $Log$
// Revision 1.19  2002/11/27 21:29:06  adridg
// See larger ChangeLog entry
//
// Revision 1.18  2002/08/30 22:24:55  adridg
// - Improved logging, connected the right signals now
// - Try to handle dlp_ReadUserInfo failures sensibly
// - Trying to sort out failures reading the database list.
//
// Revision 1.17  2002/08/23 22:03:21  adridg
// See ChangeLog - exec() becomes bool, debugging added
//
// Revision 1.16  2002/05/15 17:15:33  gioele
// kapp.h -> kapplication.h
// I have removed KDE_VERSION checks because all that files included "options.h"
// which #includes <kapplication.h> (which is present also in KDE_2).
// BTW you can't have KDE_VERSION defined if you do not include
// - <kapplication.h>: KDE3 + KDE2 compatible
// - <kdeversion.h>: KDE3 only compatible
//
// Revision 1.15  2002/05/14 22:57:40  adridg
// Merge from _BRANCH
//
// Revision 1.14.2.1  2002/04/04 20:28:28  adridg
// Fixing undefined-symbol crash in vcal. Fixed FD leak. Compile fixes
// when using PILOT_VERSION. kpilotTest defaults to list, like the options
// promise. Always do old-style USB sync (also works with serial devices)
// and runs conduits only for HotSync. KPilot now as it should have been
// for the 3.0 release.
//
// Revision 1.14  2002/02/02 11:46:02  adridg
// Abstracting away pilot-link stuff
//
// Revision 1.13  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.12  2002/01/23 02:56:23  mhunter
// CVS_SILENT Removed extra space after full-stop
//
// Revision 1.11  2001/12/29 15:45:02  adridg
// Lots of little changes for the syncstack
//
// Revision 1.10  2001/10/08 22:20:18  adridg
// Changeover to libkpilot, prepare for lib-based conduits
//
// Revision 1.9  2001/09/30 19:51:56  adridg
// Some last-minute layout, compile, and __FUNCTION__ (for Tru64) changes.
//
// Revision 1.8  2001/09/30 17:11:10  adridg
// fname unknown with DEBUG turned off
//
// Revision 1.7  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.6  2001/09/24 22:17:41  adridg
// () Removed lots of commented out code from previous incarnations.
// () Added a cleanup action.
// () Removed a heap-corruption bug caused by using QStringList & and
//    then deleting what it points to in FileInstallAction.
// () Removed deadlock when last file to install couldn't be read.
// () Moved RestoreAction to interactiveSync.{h,cc}, since I feel it
//    needs to ask "Are you sure?" at the very least.
//
