#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include "version.h"
#include "top.h"


static const char *description = 
	I18N_NOOP("KDE Time tracker tool.");


int main( int argc, char *argv[] )
{
	KAboutData aboutData( "karm", I18N_NOOP("KArm"),
		KARM_VERSION, description, KAboutData::License_GPL,
		"(c) 1997-2000, Sirtaj Singh Kang, Espen Sand, Jesper Pedersen,\nKalle Dalheimer, Klarälvdalens Datakonsult AB, Tomas Pospisek, Sourcepole");
	aboutData.addAuthor( "Tomas Pospisek",	  I18N_NOOP("Current Maintainer"),
												 "tpo_deb@sourcepole.ch" );
	aboutData.addAuthor( "Sirtaj Singh Kang", I18N_NOOP("Original Author"),
												 "taj@kde.org" );
	aboutData.addAuthor( "Espen Sand",        0, "espen@kde.org");
	aboutData.addAuthor( "Kalle Dalheimer",   0, "kalle@kde.org" );
	aboutData.addAuthor( "Jesper Pedersen",   0, "blackie@kde.org" );
	aboutData.addAuthor( "Gioele Barabucci",  0, "gioele@gioelebarabucci.com");
	aboutData.addAuthor( "Jan Schaumann",     0, "jschauma@netmeister.org");
	aboutData.addAuthor( "Scott Monachello",  0, "smonach@cox.net");
	
	KCmdLineArgs::init( argc, argv, &aboutData );
	KApplication myApp;

	KarmWindow *karm = new KarmWindow;

	myApp.setMainWidget( karm );
	karm->show();
	
	int ret = myApp.exec();

	return ret;
}
