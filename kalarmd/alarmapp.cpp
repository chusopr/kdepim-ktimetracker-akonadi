/*
    This file is part of the KDE alarm daemon.
    Copyright (c) 1997-1999 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>

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

// $Id$

#include <qstring.h>

#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kstartupinfo.h>

#include "alarmdaemon.h"

#include "alarmapp.h"
#include "alarmapp.moc"


AlarmApp::AlarmApp() :
  KUniqueApplication(/*false,false*/),
  mAd(0L)
{
}

AlarmApp::~AlarmApp()
{
}

int AlarmApp::newInstance()
{
  kdDebug(5900) << "kalarmd:AlarmApp::newInstance()" << endl;

  KStartupInfo::appStarted();

  // Check if we already have a running alarm daemon widget
  if (mAd) return 0;

  // Check if we are starting up at session startup
  static bool restored = false;
  if (!restored  &&  isRestored()) {
    mStartedAtLogin = true;
    restored = true;       // make sure we restore only once
  }
  else {
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    mStartedAtLogin = args->isSet("login");
    args->clear();      // free up memory
  }

  mAd = new AlarmDaemon(0L, "ad");

  return 0;
}
