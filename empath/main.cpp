/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// System includes
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

// KDE includes
#include <kuniqueapp.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kcmdlineargs.h>

// Local includes
#include "Empath.h"
#include "EmpathUI.h"

static const char* description=I18N_NOOP("Empath, the KDE Mail Client");
static const char* VERSION="0.0.1";

    int
main(int argc, char ** argv)
{
    KAboutData aboutData( "empath", I18N_NOOP("Empath"),
        VERSION, description, KAboutData::License_GPL,
        "(c) 1999-2000, The Empath Team");
    aboutData.addAuthor("Rik Hemsley",0, "rik@kde.org");
    aboutData.addAuthor("Wilco Greven",0, "j.w.greven@student.utwente.nl");
    KCmdLineArgs::init( argc, argv, &aboutData );
  
    // Don't do anything if we're being run as root.
    if (getuid() == 0 || geteuid() == 0) {
        fprintf(stderr, "DO NOT RUN GUI APPS AS ROOT !\n");
        return 1;
    }    

    // Pick a sensible umask for everything Empath does.
    int prev_umask = umask(077);
    
#ifdef DOSOMEDOSOMEDOSOME

    if (!KUniqueApplication::start())
        exit(0);
    
    KUniqueApplication app;

#else

    KApplication app;

#endif
    
    // Create the kernel.
    Empath::start();
    
    // Create the user interface.
    EmpathUI * ui = new EmpathUI;

    // Attempt to get the UI up and running quickly.
    app.processEvents();

    // Initialise the kernel.
    empath->init();
    
    // Enter the event loop.
    int retval = app.exec();
    
    delete ui;
    ui = 0;
    
    empath->shutdown();

    // Restore umask.
    umask(prev_umask);

    return retval;
}

// vim:ts=4:sw=4:tw=78
