// conduitApp.cc
//
// Copyright (C) 1998-2000 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// This is conduitApp.cc for KDE 2 / KPilot 4.
//
//
//

static const char *id=
	"$Id$";

#include "options.h"

#ifdef KDE2
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stream.h>
#include <kwin.h>
#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapp.h>
#include <kdebug.h>

#include "conduitApp.h"
#else
#include <iostream.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <kwm.h>
#include "conduitApp.moc"
#include "kpilot.h"
#endif


// In KDE2 we use command line options in KDE2 style (!)
// as opposed to the GNU style long options used in
// KDE1. This increases conformity.
//
// Help and debug options are standard in KDE2.
//
#ifdef KDE2
static KCmdLineOptions conduitoptions[] =
{
	{ "setup", I18N_NOOP("Start up configuration dialog " 
		"for this conduit"), 0L },
	{ "info", I18N_NOOP("Print which databases are associated "
		"with this conduit"), 0L },
	{ "backup", I18N_NOOP("Backup the databases associated "
		"with this conduit"), 0L },
	{ "hotsync", I18N_NOOP("HotSync the databases associated "
		"with this conduit"), 0L },
	{ "debug <level>", I18N_NOOP("Set debugging level"), "0" },
	{ 0,0,0 }
} ;
#else
static struct option longOptions[]=
{
	{ "setup",0,0L,'s' },
	{ "info",0,0L,'i' },
	{ "backup",0,0L,'b' },
	{ "debug",1,0L,'d' },
	{ "hotsync",0,0L,'h' },	// Included for orthogonality
	{ "help",0,0L,1 },
	{ 0L,0,0L,0 }
} ;
#endif


// Constructors
//
//
#ifdef KDE2
// The constructor for KDE2. In KDE2 a ConduitApp
// HAS a KApplication, it's not a KIND OF KApplication.
//
//
ConduitApp::ConduitApp(
	int& argc, 
	char** argv, 
	const char * rAppName,
	const char *conduitName,
	const char *version) :
	fAbout(0L),
	fApp(0L),
	fCmd(false),
	fMode(BaseConduit::None),
	fArgc(argc),
	fArgv(argv)
{
	fAbout=new KAboutData(rAppName,
		conduitName,
		version,
		I18N_NOOP("Pilot HotSync software for KDE 2"),
		(int) KAboutData::License_GPL,
		"Copyright (c) 1998-2000 Dan Pilone",
		(const char *)0L,
		"http://www.slac.com/pilone/kpilot_home/");

	fAbout->addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com",
		"http://www.slac.com/pilone/kpilot_home/");
#if 0
	fAbout->addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"adridg@cs.kun.nl",
		"http://www.cs.kun.nl/~adridg/kpilot/");                        
#endif
}
#else
// The constructor for KDE1 places more burden on the
// caller, who must compose a decent banner for the
// application.
//
//
ConduitApp::ConduitApp(
	int& argc, 
	char** argv, 
	const QCString& rAppName,
	const char *banner)
  : KApplication(argc, argv, rAppName), fConduit(0L)
{
	fMode=handleOptions(banner,argc,argv);
	if (fMode==BaseConduit::Error) usage(banner,longOptions);
}
#endif




#ifdef KDE2
void ConduitApp::addAuthor(const char *name,
	const char *task,
	const char *email)
{
	fAbout->addAuthor(name,task,email);
}
#endif








// Next are helper functions, which vary considerably
// from KDE1 to KDE2 but have the same purpose: deal
// with the options that can be passed to the conduit.
//
#ifdef KDE2
// Options are added by addOptions(); the actual handling
// of options is done much later by exec()
//
//
void ConduitApp::addOptions(KCmdLineOptions *p)
{
	if (!fCmd)
	{
		KCmdLineArgs::init(fArgc,fArgv,fAbout);
		KCmdLineArgs::addCmdLineOptions(conduitoptions);
		fCmd=true;
	}
	if (p)
	{
		KCmdLineArgs::addCmdLineOptions(p);
	}
}

KCmdLineArgs *ConduitApp::getOptions()
{
	KCmdLineArgs *p;

	if (!fCmd)
	{
		addOptions(0L);
	}
	KApplication::addCmdLineOptions();

	p=KCmdLineArgs::parsedArgs();

	debug_level = atoi(p->getOption("debug"));

	return p;
}
#else

BaseConduit::eConduitMode ConduitApp::handleOptions(const char *banner,
	int& argc,char **argv)
{
	FUNCTIONSETUP;

	int c,li;
	BaseConduit::eConduitMode rc=BaseConduit::HotSync;

	while ((c=getopt_long(argc,argv,"sibd:vh?",longOptions,&li))>0)
	{
		switch(c)
		{
		case 's' : rc=BaseConduit::Setup; break;
		case 'i' : rc=BaseConduit::DBInfo; break;
		case 'b' : rc=BaseConduit::Backup; break;
		case 'h' : rc=BaseConduit::HotSync; break;
		case 'd' : debug_level=atoi(optarg); 
			if (debug_level)
			{
				cerr << fname << ": Debug level set to "
					<< debug_level << endl;
			}
			break;
		case 1 : usage(banner,longOptions); exit(0);
		default : usage(banner,longOptions); exit(1);
		}
	}

	return rc;
}


#endif


// In KDE1, setConduit() performs some actions based
// on the mode (already determined by handleOptions()).
// In KDE2, this is left to exec(). Therefore, much
// of setConduit() is commented out in KDE2.
//
void
ConduitApp::setConduit(BaseConduit* conduit)
{
	FUNCTIONSETUP;

#ifndef KDE2
	if (fMode==BaseConduit::Error)
	{
		cerr << fname << ": ConduitApp has state \"Error\".\n";
		return;
	}
#endif
	fConduit = conduit;

#ifndef KDE2
	switch(fMode)
	{
	case BaseConduit::DBInfo : cout << conduit->dbInfo(); break;
	case BaseConduit::HotSync : conduit->doSync(); break;
	case BaseConduit::Backup : conduit->doBackup(); break;
	case BaseConduit::Setup : break;
	default :
		cerr << fname << ": ConduitApp has state " 
			<< (int) fMode  << endl 
			<< fname << ": where it is strange to call me."
			<< endl;
	}
#endif
}

// exec() actually runs the conduit. This isn't strictly
// true since under KDE1 setConduit() does a lot of work
// which in KDE2 is transferred here.
//
//
#ifdef KDE2
#define CheckArg(s,m)	if (args->isSet(s)) \
		{ if (fMode==BaseConduit::None) \
		{ fMode=BaseConduit::m; } else \
		{ cerr << fname << ": More than one mode given (mode now " \
			<< (int)fMode << ')' << endl; \
			fMode=BaseConduit::Error; } }

BaseConduit::eConduitMode ConduitApp::getMode()
{
	FUNCTIONSETUP;

	if (fMode!=BaseConduit::None) return fMode;

	KCmdLineArgs *args=getOptions();
	CheckArg("info",DBInfo);
	CheckArg("setup",Setup);
	CheckArg("hotsync",HotSync);
	CheckArg("backup",Backup);

	if (fMode==BaseConduit::None)
	{
		cerr << fname 
			<< ": You must specify a mode for the conduit."
			<< endl;
		fMode=BaseConduit::Error;
	}
	else
	{
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname 
				<< ": Set mode to "
				<< (int) fMode
				<< endl;
		}
	}

	return fMode;
}

int ConduitApp::exec()
{
	FUNCTIONSETUP;

	QWidget *widget = 0L;
	QPixmap *icon= 0L;

	(void) getMode();

	// Note that both styles and GUI are only in effect
	// if we are going to do the setup dialog.
	//
	fApp=new KApplication(fMode==BaseConduit::Setup,
		fMode==BaseConduit::Setup);

	kdDebug() << fname << ": App mode="
		<< (int) fMode
		<< " conduit mode="
		<< (int) fConduit->getMode()
		<< endl;

	switch(fMode)
	{
	case BaseConduit::DBInfo : cout << fConduit->dbInfo(); break;
	case BaseConduit::HotSync : fConduit->doSync(); break;
	case BaseConduit::Backup : fConduit->doBackup(); break;
	case BaseConduit::Setup :
		widget = fConduit->aboutAndSetup();
		icon= fConduit->icon();

		fApp->setMainWidget(widget);
		KWin::setIcons(widget->winId(),*icon,*icon);

		widget->show();

		return fApp->exec();
	case BaseConduit::Error :
		cerr << fname << ": ConduitApp is in Error state."
			<< endl;
		break;
	default :
		cerr << fname << ": ConduitApp has state " 
			<< (int) fMode  << endl 
			<< fname << ": where it is strange to call me."
			<< endl;
	}

	// Info, HotSync and Backup can't really fail?
	//
	//
	return 0;
}
#else
int
ConduitApp::exec()
{
	FUNCTIONSETUP;

	if(fMode == BaseConduit::Setup)
	{
		if (debug_level & UI_MAJOR)
		{
				cerr << fname 
					<< ": Running setup widget"
					<< endl ;
		}
		QWidget* widget = fConduit->aboutAndSetup();
		QPixmap *icon= fConduit->icon();

		KApplication::setMainWidget(widget);
		KWin::setIcons(widget->winId(),*icon,*icon);
		widget->show();
		return KApplication::exec();
	}
	return 0;
}
#endif


// $Log:$
