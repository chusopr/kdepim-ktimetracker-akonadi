/* ListMaker-conduit.cc  ListMaker-Conduit for syncing KPilot and KOrganizer
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
** Copyright (C) 2001 Cornelius Schumacher
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file is part of the ListMaker conduit, a conduit for KPilot that
** synchronises the Pilot's ListMaker application with the outside world,
** which currently means KOrganizer.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

#include "options.h"

#if QT_VERSION < 300
#include <qmsgbox.h>
#else
#include <qmessagebox.h>
#endif

#include <kconfig.h>

#if KDE_VERSION < 300
#include <libkcal/todo.h>
#else
#include <todo.h>
#endif

#include "pilotRecord.h"
#include "pilotSerialDatabase.h"
//#include "pilotListMakerEntry.h"

//#include "ListMaker-factory.h"
#include "MultiDB-factory.h"
#include "ListMaker-conduit.h"

using namespace KCal;

static const char *ListMaker_conduit_id = "$Id$";



ListMakerConduit::ListMakerConduit(KPilotDeviceLink *d, const char *n, const QStringList &l, SyncTypeList_t *tps) : OrganizerConduit(d,n,l,tps) {
	FUNCTIONSETUP;
}


// $Log$
// Revision 1.5  2002/04/07 11:56:18  reinhold
// Last version before moving to KDE CVS
//
// Revision 1.4  2002/04/06 00:51:08  reinhold
// Finally the conduit compiles again... Still have a lot of TODOS
//
// Revision 1.3  2002/03/10 23:58:32  reinhold
// Made the conduit compile...
//
// Revision 1.2  2002/03/10 16:06:43  reinhold
// Cleaned up the class hierarchy, implemented some more features (should be quite finished now...)
//
// Revision 1.1  2002/03/09 15:45:48  reinhold
// Moved the files around
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the  project manager / List manager conduit.
//
//
