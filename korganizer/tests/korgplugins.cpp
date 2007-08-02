/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <calendar/plugin.h>

#include "kocore.h"

int main(int argc,char **argv)
{
  KAboutData aboutData("korgplugins", 0,ki18n("KOrgPlugins"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;
  
  KService::List plugins = KOCore::self()->availablePlugins();
  KService::List::ConstIterator it;
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kDebug(5850) <<"Plugin:" << (*it)->desktopEntryName() <<" ("
              << (*it)->name() << ")" << endl;
    KOrg::Plugin *p = KOCore::self()->loadPlugin(*it);
    if (!p) {
      kDebug(5850) <<"Plugin loading failed.";
    } else {
      kDebug(5850) <<"PLUGIN INFO:" << p->info();
    }
  }
  
  plugins = KOCore::self()->availablePrintPlugins();
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kDebug(5850) <<"Print plugin:" << (*it)->desktopEntryName() <<" ("
              << (*it)->name() << ")" << endl;
    KOrg::PrintPlugin *p = KOCore::self()->loadPrintPlugin(*it);
    if (!p) {
      kDebug(5850) <<"Print plugin loading failed.";
    } else {
      kDebug(5850) <<"PRINT PLUGIN INFO:" << p->info();
    }
  }
  
  plugins = KOCore::self()->availableParts();
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kDebug(5850) <<"Part:" << (*it)->desktopEntryName() <<" ("
              << (*it)->name() << ")" << endl;
    KOrg::Part *p = KOCore::self()->loadPart(*it,0);
    if (!p) {
      kDebug(5850) <<"Part loading failed.";
    } else {
      kDebug(5850) <<"PART INFO:" << p->info();
    }
  }
  
  plugins = KOCore::self()->availableCalendarDecorations();
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kDebug(5850) <<"CalendarDecoration:" << (*it)->desktopEntryName() <<" ("
              << (*it)->name() << ")" << endl;
    KOrg::CalendarDecoration::Decoration *p = KOCore::self()->loadCalendarDecoration(*it);
    if (!p) {
      kDebug(5850) <<"Calendar decoration loading failed.";
    } else {
      kDebug(5850) <<"CALENDAR DECORATION INFO:" << p->info();
    }
  }

}
