/* interactiveSync.cc                   KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file specializes SyncAction to a kind that can have interaction
** with the user without the Sync timing out.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

#include <config.h>

static const char *interactivesync_id =
	"$Id$";

#include "options.h"

#include <time.h>

#include <pi-socket.h>
#include <pi-file.h>

#include <qtimer.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtl.h>
#include <qstyle.h>
#include <qtextcodec.h>

#include <kdialogbase.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <kapplication.h>

#include "pilotUser.h"
#include "pilotAppCategory.h"
#include "pilotLocalDatabase.h"
#include "kpilotConfig.h"
#include "kpilotlink.h"

#include "interactiveSync.moc"


CheckUser::CheckUser(KPilotDeviceLink * p, QWidget * vp):
	InteractiveAction(p, vp, "userCheck")
{
	FUNCTIONSETUP;
	
	(void) interactivesync_id;
}

CheckUser::~CheckUser()
{
	FUNCTIONSETUP;
}

/* virtual */ bool CheckUser::exec()
{
	FUNCTIONSETUP;

	KPilotConfigSettings & config = KPilotConfig::getConfig();
	config.resetGroup();

	QString guiUserName = config.getUser();
	QString pilotUserName = PilotAppCategory::codec()->
		toUnicode(fHandle->getPilotUser()->getUserName());
	bool pilotUserEmpty = pilotUserName.isEmpty();
	// 4 cases to handle:
	//    guiUserName empty / not empty
	//    pilotUserName empty / not empty
	//
	//
	if (guiUserName.isEmpty())
	{
		if (pilotUserEmpty)
		{
			QString defaultUserName =
				i18n("A common name", "John Doe");

			QString q = i18n("<qt>Neither KPilot nor the "
				"Pilot have a user name set. "
				"They <i>should</i> be set. "
				"Should KPilot set them to a default value "
				"(<i>%1</i>)?</qt>").arg(defaultUserName);

			if (questionYesNo(q, i18n("User Unknown") /* ,"askUserNone" */) ==
				KDialogBase::Yes)
			{
				config.setUser(defaultUserName);
				fHandle->getPilotUser()->
					setUserName(PilotAppCategory::codec()->fromUnicode(defaultUserName));
			}

		}
		else
		{
			QString q = i18n("<qt>The Pilot has a user name set "
				"(<i>%1</i>) but KPilot does not. Should "
				"KPilot use this user name in future?").
				arg(pilotUserName);

			if (questionYesNo(q, i18n("User Unknown") /* ,"askUserSome" */ ) ==
				KDialogBase::Yes)
			{
				config.setUser(pilotUserName);
			}
		}
	}
	else
	{
		if (pilotUserEmpty)
		{
			QString q = i18n("<qt>KPilot has a user name set "
				"(<i>%1</i>) but the Pilot does not. "
				"Should KPilot's user name be set in the "
				"Pilot as well?").arg(guiUserName);

			if (questionYesNo(q, i18n("User Unknown") /* ,"askUserSome" */) ==
				KDialogBase::Yes)
			{
#ifdef DEBUG
				DEBUGDAEMON << fname
					<< ": Setting user name in pilot"
					<< endl;
#endif

				const char *l1 = guiUserName.latin1();

#ifdef DEBUG
				DEBUGDAEMON << fname
					<< ": Setting to " << l1 << endl;
#endif

				fHandle->getPilotUser()->setUserName(l1);
			}
		}
		else
		{
			if (guiUserName != pilotUserName)
			{
				QString q = i18n("<qt>The Pilot thinks that "
					"the user name is %1, "
					"however KPilot says you are %2."
					"Should I assume the Pilot is right "
					"and set the user name "
					"for KPilot to %1?").
					arg(pilotUserName).arg(pilotUserName).
					arg(guiUserName);

				int r = questionYesNo(q,
					i18n("User Mismatch") /* ,"askUserDiff" */);
				if (r == KDialogBase::Yes)
				{
					config.setUser(pilotUserName);
				}
			}
		}
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": User name set to <"
		<< guiUserName
		<< ">"
		<< endl;
#endif

	config.sync();

	// Now we've established which user will be used,
	// fix the database location for local databases.
	//
	//
	QString pathName = KGlobal::dirs()->saveLocation("data",
		CSL1("kpilot/DBBackup/"));
	if (!guiUserName.isEmpty())
	{
		pathName.append(guiUserName);
		pathName.append(CSL1("/"));
	}
	PilotLocalDatabase::setDBPath(pathName);

	emit syncDone(this);
	return true;
}

class RestoreAction::RestoreActionPrivate
{
public:
	QString fDatabaseDir;
	QValueList < struct db >fDBList;
	QTimer fTimer;
	int fDBIndex;
};


RestoreAction::RestoreAction(KPilotDeviceLink * p, QWidget * visible ) :
	InteractiveAction(p, visible, "restoreAction")
{
	FUNCTIONSETUP;

	fP = new RestoreActionPrivate;
	fP->fDatabaseDir = KGlobal::dirs()->saveLocation("data",
		CSL1("kpilot/DBBackup/"));
}

/* virtual */ bool RestoreAction::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Restoring from base directory "
		<< fP->fDatabaseDir << endl;
#endif

	QString dirname = fP->fDatabaseDir +
		PilotAppCategory::codec()->toUnicode(fHandle->getPilotUser()->getUserName()) + 
		CSL1("/");

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Restoring user " << dirname << endl;
#endif

	if (questionYesNo(i18n("<qt>Are you sure you want to completely "
				"restore your Pilot from the backup directory "
				"(<i>%1</i>)? This will erase any information "
				"you currently have on your Pilot.</qt>").
			arg(dirname),
			i18n("Restore Pilot")) != KDialogBase::Yes)
	{
		emit logError(i18n("Restore <i>not</i> performed."));

		addSyncLogEntry(i18n("Restore not performed."));
		emit syncDone(this);

		return true;
	}

	QDir dir(dirname, QString::null, QDir::Name,
		QDir::Files | QDir::Readable | QDir::NoSymLinks);

	if (!dir.exists())
	{
		kdWarning() << k_funcinfo
			<< ": Restore directory "
			<< dirname << " does not exist." << endl;
		fStatus = Error;
		return false;
	}

	emit logProgress(i18n("Restoring %1...").arg(QString::null),1);
	
	for (unsigned int i = 0; i < dir.count(); i++)
	{
		QString s;
		struct db dbi;
		struct DBInfo info;
		struct pi_file *f;

		s = dir[i];

#ifdef DEBUG
		DEBUGDAEMON << fname
			<< ": Adding " << s << " to restore list." << endl;
#endif

#if KDE_VERSION < 306  /* 305 ok? */
		strncpy(dbi.name, QFile::encodeName(dirname + s), sizeof(dbi.name) - 1);
		dbi.name[(sizeof(dbi.name) - 1)] = '\0';
#else
		strlcpy(dbi.name, QFile::encodeName(dirname + s), sizeof(dbi.name));
#endif

		f = pi_file_open(dbi.name);
		if (!f)
		{
			kdWarning() << k_funcinfo
				<< ": Can't open " << dbi.name << endl;
			continue;
		}

		if (!pi_file_get_info(f, &info))
		{
			dbi.creator = info.creator;
			dbi.type = info.type;
			dbi.flags = info.flags;
			dbi.maxblock = 0;

			fP->fDBList.append(dbi);
		}
		else
		{
			kdWarning() << k_funcinfo
				<< ": Can't open " << dbi.name << endl;
		}

		pi_file_close(f);
		f = 0L;
	}

	fP->fDBIndex = 0;
	fStatus = GettingFileInfo;

	QObject::connect(&(fP->fTimer), SIGNAL(timeout()),
		this, SLOT(getNextFileInfo()));

	fP->fTimer.start(0, false);
	return true;
}

/* slot */ void RestoreAction::getNextFileInfo()
{
	FUNCTIONSETUP;

	ASSERT(fStatus == GettingFileInfo);
	ASSERT((unsigned) fP->fDBIndex < fP->fDBList.count());

	struct db &dbi = fP->fDBList[fP->fDBIndex];
	pi_file *f;

	fP->fDBIndex++;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Getting info on " << dbi.name << endl;
#endif

	f = pi_file_open(dbi.name);
	if (!f)
	{
		kdWarning() << k_funcinfo
			<< ": Can't open " << dbi.name << endl;
		goto nextFile;
	}

	int max;

	pi_file_get_entries(f, &max);

	for (int i = 0; i < max; i++)
	{
		int size;

		if (dbi.flags & dlpDBFlagResource)
		{
			pi_file_read_resource(f, i, 0, &size, 0, 0);
		}
		else
		{
			pi_file_read_record(f, i, 0, &size, 0, 0, 0);
		}

		if (size > dbi.maxblock)
		{
			dbi.maxblock = size;
		}
	}

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Read " << max << " entries for this database." << endl;
#endif

nextFile:
	if (f)
		pi_file_close(f);

	if ((unsigned) fP->fDBIndex >= fP->fDBList.count())
	{
		QObject::disconnect(&(fP->fTimer), SIGNAL(timeout()),
			this, SLOT(getNextFileInfo()));
		fP->fTimer.stop();

		qBubbleSort(fP->fDBList);

		fP->fDBIndex = 0;
		fStatus = InstallingFiles;

		QObject::connect(&(fP->fTimer), SIGNAL(timeout()),
			this, SLOT(installNextFile()));
		fP->fTimer.start(0, false);
	}
}

/* slot */ void RestoreAction::installNextFile()
{
	FUNCTIONSETUP;

	ASSERT(fStatus == InstallingFiles);
	ASSERT((unsigned) fP->fDBIndex < fP->fDBList.count());

	struct db &dbi = fP->fDBList[fP->fDBIndex];

	fP->fDBIndex++;

#ifdef DEBUG
	DEBUGDAEMON << fname << ": Trying to install " << dbi.name << endl;
#endif

	if ((unsigned) fP->fDBIndex >= fP->fDBList.count() - 1)
	{
		QObject::disconnect(&(fP->fTimer), SIGNAL(timeout()),
			this, SLOT(getNextFileInfo()));
		fP->fTimer.stop();

		fStatus = Done;
	}

	if (openConduit() < 0)
	{
		kdWarning() << k_funcinfo
			<< ": Restore apparently canceled." << endl;
		fStatus = Done;
		emit syncDone(this);

		return;
	}

	QFileInfo databaseInfo(QString::fromLatin1(dbi.name));
	addSyncLogEntry(databaseInfo.fileName());
	emit logProgress(i18n("Restoring %1...").arg(databaseInfo.fileName()),
		(100*fP->fDBIndex) / (fP->fDBList.count()+1)) ;
	
	pi_file *f =
		pi_file_open( /* const_cast <
		char *>((const char *)QFile::encodeName */ (dbi.name));
	if (!f)
	{
		kdWarning() << k_funcinfo
			<< ": Can't open "
			<< dbi.name << " for restore." << endl;
		return;
	}

	if (pi_file_install(f, pilotSocket(), 0) < 0)
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't  restore " << dbi.name << endl;
	}

	pi_file_close(f);


	if (fStatus == Done)
	{
		addSyncLogEntry(i18n("OK."));
		emit syncDone(this);
	}
}

/* virtual */ QString RestoreAction::statusString() const
{
	FUNCTIONSETUP;
	QString s;

	switch (status())
	{
	case InstallingFiles:
		s.append(CSL1("Installing Files ("));
		s.append(QString::number(fP->fDBIndex));
		s.append(CSL1(")"));
		break;
	case GettingFileInfo:
		s.append(CSL1("Getting File Info ("));
		s.append(QString::number(fP->fDBIndex));
		s.append(CSL1(")"));
		break;
	default:
		return SyncAction::statusString();
	}

	return s;
}


