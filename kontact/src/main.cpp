/*
 * Copyright (C) 2001 Matthias H�lzer-Kl�pfel <mhk@caldera.de>
 */

#include <kapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>


#include "core.h"


static const char *description =
    I18N_NOOP("A KDE PIM Framework");

    
static const char *version = "0.2";


/*
static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    { 0, 0, 0 }
};
*/


int main(int argc, char **argv)
{
    KAboutData about("kaplan", I18N_NOOP("Kaplan"), version, description,
                     KAboutData::License_GPL, "(C) 2001 Matthias H�lzer-Kl�pfel", 0, 0, "mhk@kde.org");
    about.addAuthor( "Matthias Hoelzer-Kluepfel", 0, "mhk@kde.org" );
    about.addAuthor( "Daniel Molkentin", 0, "molkentin@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);
//    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    // register ourselves as a dcop client
    app.dcopClient()->registerAs(app.name(), false);

    // see if we are starting with session management
    if (app.isRestored())
        RESTORE(Core)
    else
    {
        // no session.. just start up normally
        Core *widget = new Core;
        widget->show();
    }

    return app.exec();
}
