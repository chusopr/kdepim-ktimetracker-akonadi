/* JPilotProxy-factory.cc					  KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the JPilotProxy-conduit plugin.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qdir.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "kpilotlink.h"
#include "kconfig.h"
#include "JPilotProxy-factory.moc"
#include "jpilotproxysettings.h"

extern "C"
{

void *init_conduit_JPilotProxy() {
	FUNCTIONSETUP;
	return new JPilotProxyConduitFactory;
}

};


bool JPilotProxyConduitFactory::pluginsloaded=false;

KAboutData *JPilotProxyConduitFactory::fAbout = 0L;
PluginList_t *JPilotProxyConduitFactory::plugins=0L;

JPilotProxyConduitFactory::JPilotProxyConduitFactory(QObject *p, const char *n) :
		KLibFactory(p,n)  {
	FUNCTIONSETUP;
	plugins=new PluginList_t();
	// load the library containing the JPilot API functions. If this fails, any plugin will probably crash KPilot, so just exit!!!
	apilib=KLibLoader::self()->globalLibrary("libJPilotAPI");
		#ifdef DEBUG
	if (!apilib) DEBUGCONDUIT << fname << ": JPilotAPI library could not be loaded\n  error ["<<KLibLoader::self()->lastErrorMessage()<<"]" << endl;
	else DEBUGCONDUIT << fname << ": loaded JPilotAPI library" << endl;
 	#endif
  jp_logf(4, "testing...");

	fInstance = new KInstance(n);
	fAbout = new KAboutData(n,
		I18N_NOOP("JPilotProxy Conduit for KPilot"),
		KPILOT_VERSION,
		I18N_NOOP("Configures the JPilotProxy Conduit for KPilot"),
		KAboutData::License_GPL,
		"(C) 2002, Reinhold F. Kainhofer");
	fAbout->addAuthor("Reinhold Kainhofer", I18N_NOOP("Original author and maintainer of this conduit"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com");
	fAbout->addCredit("Judd Montgomery", I18N_NOOP("Author of JPilot"), 	"judd@engineer.com", "http://www.jpilot.org/");
}

JPilotProxyConduitFactory::~JPilotProxyConduitFactory() {
	FUNCTIONSETUP;

	PluginIterator_t it(*plugins); // iterator for plugin list
	for ( ; it.current(); ++it ) {
		#ifdef DEBUG
		DEBUGCONDUIT<<"unloading library "<< it.current()->info.fullpath<<" ("<<it.current()->info.name<<"), address="<<it.current()->lib<<endl;
		#endif
		it.current()->exit_cleanup();
	}
	if (apilib) KLibLoader::self()->unloadLibrary(apilib->fileName());

	KPILOT_DELETE(fInstance);
	KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *JPilotProxyConduitFactory::createObject( QObject *p,
	const char *n, const char *c, const QStringList &a) {
	FUNCTIONSETUP;

		#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Creating object of class "	<< c << endl;
		#endif

	if (qstrcmp(c,"ConduitConfig")==0) {
		QWidget *w = dynamic_cast<QWidget *>(p);

		if (w) {
			return createSetupWidget(w,n,a);
		} else {
				#ifdef DEBUG
			DEBUGCONDUIT << fname << ": Couldn't cast parent to widget." << endl;
				#endif
			return 0L;
		}
	}

	if (qstrcmp(c,"SyncAction")==0) {
		KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

		if (d) {
			return createConduit(d,n,a);
		} else {
			kdError() << k_funcinfo
				<< ": Couldn't cast to KPilotDeviceLink."
				<< endl;
		}
	}

	return 0L;
}

JPlugin*JPilotProxyConduitFactory::addPlugin( QString path, bool on) {
	FUNCTIONSETUP;
	// TODO: search the plugin list if the plugin was already loaded
	JPlugin*newplugin=new JPlugin( path );
	#ifdef DEBUG
	DEBUGCONDUIT<<"successfully created a JPlugin instance for "<<path<<endl;
	#endif
	if (newplugin->loaded) {
		newplugin->info.sync_on=on;
		#ifdef DEBUG
		DEBUGCONDUIT<<"loading "<<path<<" was successful"<<endl;
		#endif
		// if the plugin was loaded successfully, insert it into the list of plugins
		plugins->append(newplugin);
		jp_startup_info si;
		si.base_dir="/usr/local";
		newplugin->startup(&si);
		return newplugin;
	} else delete newplugin;
	return 0;
}

// This is not yet optimal, but should work for now...
int JPilotProxyConduitFactory::removePlugin( QString path) {
	FUNCTIONSETUP;
	
	JPlugin*plugintodel=NULL;

	PluginIterator_t it(*plugins); // iterator for plugin list
	for ( ; it.current(); ++it ) {
		JPlugin *plug = it.current();
		if (plug->lib && strcmp(plug->info.fullpath, path)) plugintodel=plug;
	}

	if (plugintodel) {
		plugins->take(plugins->find(plugintodel));
		plugintodel->exit_cleanup();
		delete plugintodel;
	}
}

int JPilotProxyConduitFactory::addPluginPath(QString path) {
	FUNCTIONSETUP;
	// find the list of possible plugins in the directory given by path
	QDir dir(path);
	QStringList plugs=dir.entryList("*.so");

	for (QStringList::Iterator it = plugs.begin(); it != plugs.end(); ++it ) {
		#ifdef DEBUG
		DEBUGCONDUIT<<"Load plugin "<<(*it)<<endl;
		#endif
		bool on=false;
		if (fC) {
			on=JPilotProxySettings::config()->readBoolEntry(*it);
		}
		addPlugin(dir.absFilePath(*it), on);
	}
}

int JPilotProxyConduitFactory::loadPlugins() {
	FUNCTIONSETUP;
	
	JPilotProxySettings::self()->loadConfig();
	QStringList pathes( JPilotProxySettings::PluginPathes() );
	for (QStringList::Iterator it = pathes.begin(); it != pathes.end(); ++it ) {
		addPluginPath(*it, fC);
	}
	// now load the individual plugins...
	QStringList plugs( JPilotProxySettigns::LoadedPlugins() );
	for (QStringList::Iterator it = plugs.begin(); it != plugs.end(); ++it ) {
		addPlugin(*it, fC->readBoolEntry(*it));
	}
	pluginsloaded=true;
	
	QStringList loadedplugs;
	// TODO: Write out the plugin list to the config file.
/*XXX	PluginIterator_t it(*plugins); // iterator for plugin list
	for ( ; it.current(); ++it ) {
		loadedplugs.append(it.current()->info.fullpath);
		// TODO:...
//		QStringList pluginfo();
//		// Do something useful here
	}*/
	
}

