/*
    This file is part of KOrganizer.
    Copyright (c) 1997-1999 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <stdlib.h>

#include <qdir.h>

#include <kstandarddirs.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "koapp.h"
#include "aboutdata.h"

static const KCmdLineOptions options[] =
{
  {"l", 0, 0},
  {"list", I18N_NOOP("List the events for the current day"), 0},
  {"s", 0, 0},
  {"show <numdays>", I18N_NOOP("Show a list of all events for the next <numdays>"),"1"},
  {"+[calendar]", I18N_NOOP("A calendar file to load"), 0},
  {0,0,0}
};

int main (int argc, char **argv)
{
  KOrg::AboutData aboutData;

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  if (!KOrganizerApp::start())
    exit(0);

  KOrganizerApp app;

  KGlobal::locale()->insertCatalogue("libkcal");
  KGlobal::locale()->insertCatalogue("libkdepim");
  // This is a workaround for a session management problem with KUniqueApplication
  // The session ID gets reset before the restoration is called. This line makes
  // sure that the config object is created right away  (with the correct config
  // file name). Thanks to Lubos Lunak.
  app.sessionConfig();

//  kdDebug(5850) << "app.exec" << endl;
  return app.exec();
//  kdDebug(5850) << "~app.exec" << endl;
}
