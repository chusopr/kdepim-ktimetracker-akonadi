/* plugin.cc                            KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the base class of all KPilot conduit plugins configuration
** dialogs. This is necessary so that we have a fixed API to talk to from
** inside KPilot.
**
** The factories used by KPilot plugins are also documented here.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <stdlib.h>

#include <qstringlist.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"

#include "plugin.moc"

ConduitConfigBase::ConduitConfigBase(QWidget *parent,
	const char *name) :
	QObject(parent,name),
	fModified(false),
	fWidget(0L),
	fConduitName(i18n("Unnamed"))
{
	FUNCTIONSETUP;
}

ConduitConfigBase::~ConduitConfigBase()
{
	FUNCTIONSETUP;
}

/* slot */ void ConduitConfigBase::modified()
{
	fModified=true;
}

#if 0
void ConduitConfigBase::commit(KConfig *)
{
	FUNCTIONSETUP;
}

void ConduitConfigBase::load(KConfig *)
{
	FUNCTIONSETUP;
}
#endif

/* virtual */ QString ConduitConfigBase::maybeSaveText() const
{
	FUNCTIONSETUP;

	return i18n("<qt>The <i>%1</i> conduit's settings have been changed. Do you "
		"want to save the changes before continuing?</qt>").arg(this->conduitName());
}

/* virtual */ bool ConduitConfigBase::maybeSave(KConfig *c)
{
	FUNCTIONSETUP;

	if (!isModified()) return true;

	int r = KMessageBox::questionYesNoCancel(fWidget,
		maybeSaveText(),
		i18n("%1 Conduit").arg(this->conduitName()));
	if (r == KMessageBox::Cancel) return false;
	if (r == KMessageBox::Yes) commit(c);
	return true;
}

ConduitConfig::ConduitConfig(QWidget *parent,
	const char *name,
	const QStringList &args) :
	UIDialog(parent,name,PluginUtility::isModal(args)),
	fConfig(0L),
	fConduitName(i18n("Unnamed"))
{
	FUNCTIONSETUP;
}


/* virtual */ ConduitConfig::~ConduitConfig()
{
	FUNCTIONSETUP;
}

ConduitAction::ConduitAction(KPilotDeviceLink *p,
	const char *name,
	const QStringList &args) :
	SyncAction(p,name),
	fConfig(0L),
	fDatabase(0L),
	fLocalDatabase(0L),
	fTest(args.contains(CSL1("--test"))),
	fBackup(args.contains(CSL1("--backup"))),
	fLocal(args.contains(CSL1("--local"))),
	fConflictResolution(SyncAction::eAskUser)
{
	FUNCTIONSETUP;
	if (args.contains(CSL1("--copyPCToHH"))) fSyncDirection=SyncAction::eCopyPCToHH;
	else if (args.contains(CSL1("--copyHHToPC"))) fSyncDirection=SyncAction::eCopyHHToPC;
	else if (args.contains(CSL1("--full"))) fSyncDirection=SyncAction::eFullSync;
	else fSyncDirection=SyncAction::eFastSync;

	QString cResolution(args.grep(QRegExp("--conflictResolution \\d*")).first());
	if (cResolution.isEmpty())
	{
		fConflictResolution=(SyncAction::eConflictResolution)
			cResolution.replace(QRegExp("--conflictResolution (\\d*)"), "\\1").toInt();
	}

#ifdef DEBUG
	for (QStringList::ConstIterator it = args.begin();
		it != args.end();
		++it)
	{
		DEBUGCONDUIT << fname << ": " << *it << endl;
	}
#endif
}

/* virtual */ ConduitAction::~ConduitAction()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(fDatabase);
	KPILOT_DELETE(fLocalDatabase);
}

bool ConduitAction::openDatabases_(const QString &name, bool *retrieved)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Trying to open database "
		<< name << endl;
#endif

	fLocalDatabase = new PilotLocalDatabase(name);

	if (!fLocalDatabase)
	{
		kdWarning() << k_funcinfo
			<< ": Could not initialize object for local copy of database \""
			<< name
			<< "\"" << endl;
			return false;
	}

	// if there is no backup db yet, fetch it from the palm, open it and set the full sync flag.
	if (!fLocalDatabase->isDBOpen() )
	{
		QString dbpath(dynamic_cast<PilotLocalDatabase*>(fLocalDatabase)->dbPathName());
		KPILOT_DELETE(fLocalDatabase);
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Backup database "<< dbpath <<" could not be opened. Will fetch a copy from the palm and do a full sync"<<endl;
#endif
		struct DBInfo dbinfo;
		if (fHandle->findDatabase(name.latin1(), &dbinfo)<0 )
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< ": Could not get DBInfo for "<<name<<"! "<<endl;
#endif
			return false;
		}
#ifdef DEBUG
			DEBUGCONDUIT << fname
					<< ": Found Palm database: "<<dbinfo.name<<endl
					<<"type = "<< dbinfo.type<<endl
					<<"creator = "<< dbinfo.creator<<endl
					<<"version = "<< dbinfo.version<<endl
					<<"index = "<< dbinfo.index<<endl;
#endif
		dbinfo.flags &= ~dlpDBFlagOpen;

		// make sure the dir for the backup db really exists!
		QFileInfo fi(dbpath);
		if (!fi.exists()) {
			QDir d(fi.dir(TRUE));
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": Creating backup directory "<<d.absPath()<<endl;
#endif
			d.mkdir(d.absPath());
		}
		if (!fHandle->retrieveDatabase(dbpath, &dbinfo) )
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": Could not retrieve database "<<name<<" from the handheld."<<endl;
#endif
			return false;
		}
		fLocalDatabase = new PilotLocalDatabase(name);
		if (!fLocalDatabase || !fLocalDatabase->isDBOpen())
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": local backup of database "<<name<<" could not be initialized."<<endl;
#endif
			return false;
		}
		if (retrieved) *retrieved=true;
	}

	// These latin1()'s are ok, since database names are latin1-coded.
	fDatabase = new PilotSerialDatabase(pilotSocket(), name /* On pilot */,
		this, name.latin1() /* QObject name */);

	if (!fDatabase)
	{
		kdWarning() << k_funcinfo
			<< ": Could not open database \""
			<< name
			<< "\" on the pilot."
			<< endl;
	}

	return (fDatabase && fDatabase->isDBOpen() &&
	        fLocalDatabase && fLocalDatabase->isDBOpen() );
}

// This whole function is for debugging purposes only.
bool ConduitAction::openDatabases_(const QString &dbName,const QString &localPath)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Doing local test mode for " << dbName << endl;
#endif
	if (localPath.isNull())
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": local mode test for one database only."
			<< endl;
#endif
		fDatabase = new PilotLocalDatabase(dbName);
		fLocalDatabase = 0L;
		return false;
	}

	fDatabase = new PilotLocalDatabase(localPath,dbName);
	fLocalDatabase= new PilotLocalDatabase(dbName); // From default
	if (!fLocalDatabase || !fDatabase)
	{
#ifdef DEBUG
		const QString *where2 = PilotLocalDatabase::getDBPath();

		QString none = CSL1("<null>");
		DEBUGCONDUIT << fname
			<< ": Could not open both local copies of \""
			<< dbName
			<< "\"" << endl
			<< "Using \""
			<< (where2 ? *where2 : none)
			<< "\" and \""
			<< (localPath.isEmpty() ? localPath : none)
			<< "\""
			<< endl;
#endif
	}
#ifdef DEBUG
	if (fLocalDatabase)
	{
		DEBUGCONDUIT << fname
			<< ": Opened local database "
			<< fLocalDatabase->dbPathName()
			<< (fLocalDatabase->isDBOpen() ? " OK" : "")
			<< endl;
	}
	if (fDatabase)
	{
		DEBUGCONDUIT << fname
			<< ": Opened database "
			<< fDatabase->dbPathName()
			<< (fDatabase->isDBOpen() ? " OK" : "")
			<< endl;
	}
#endif

	return (fDatabase && fLocalDatabase);
}

bool ConduitAction::openDatabases(const QString &dbName, bool*retrieved)
{
	FUNCTIONSETUP;

	/*
	** We should look into the --local flag passed
	** to the conduit and act accordingly, but until
	** that is implemented ..
	*/
#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Mode="
		<< (isTest() ? "test " : "")
		<< (isLocal() ? "local " : "")
		<< endl ;
#endif

	if (isTest() && isLocal())
	{
		return openDatabases_(dbName,CSL1("/tmp/"));
	}
	else
	{
		return openDatabases_(dbName, retrieved);
	}
}

int PluginUtility::findHandle(const QStringList &a)
{
	FUNCTIONSETUP;

	int handle = -1;
	for (QStringList::ConstIterator i = a.begin();
		i != a.end(); ++i)
	{
		if ((*i).left(7) == CSL1("handle="))
		{
			QString s = (*i).mid(7);
			if (s.isEmpty()) continue;

			handle = s.toInt();
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< ": Got handle "
				<< handle
				<< endl;
#endif
			if (handle<1)
			{
				kdWarning() << k_funcinfo
					<< ": Improbable handle value found."
					<< endl;
			}
			return handle;
		}
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": No handle= parameter found."
		<< endl;
#endif

	return -1;
}

bool PluginUtility::isModal(const QStringList &a)
{
	return a.contains(CSL1("modal"));
}

/* static */ bool PluginUtility::isRunning(const QCString &n)
{
	FUNCTIONSETUP;

	DCOPClient *dcop = KApplication::kApplication()->dcopClient();
	QCStringList apps = dcop->registeredApplications();
	return apps.contains(n);
}

ConduitConfigImplementation::ConduitConfigImplementation(QWidget *w, const char *n,
	const QStringList &a,
	ConduitConfigBase *(*f)(QWidget *, const char *)) :
	ConduitConfig(w,n,a)
{
	FUNCTIONSETUP;
	fConfigWidget=f(widget(),n);
	fConfigWidget->widget()->adjustSize();
	fConfigWidget->widget()->show();
	fConduitName=fConfigWidget->conduitName();
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Created conduit " << fConduitName << endl;
#endif
	widget()->adjustSize();
	adjustSize();
}

ConduitConfigImplementation::~ConduitConfigImplementation()
{
	FUNCTIONSETUP;
}

void ConduitConfigImplementation::readSettings()
{
	if (fConfig) fConfigWidget->load(fConfig);
}

void ConduitConfigImplementation::commitChanges()
{
	if (fConfig) fConfigWidget->commit(fConfig);
}

