/*
    This file is part of the KDE alarm daemon GUI.
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$

#include <qstring.h>

#include <kcmdlineargs.h>
#include <kdebug.h>

#include "alarmgui.h"

#include "guiapp.h"
#include "guiapp.moc"


AlarmGuiApp::AlarmGuiApp() :
  KUniqueApplication(),
  mAdg(0L)
{
}

AlarmGuiApp::~AlarmGuiApp()
{
}

int AlarmGuiApp::newInstance()
{
  kdDebug() << "kalarmdgui:AlarmGuiApp::newInstance()" << endl;

  // Check if we already have a running alarm daemon widget
  if (mAdg) return 0;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  mAdg = new AlarmGui(0L, DCOP_OBJECT_NAME);

  return 0;
}
