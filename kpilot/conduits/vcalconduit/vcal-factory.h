#ifndef _KPILOT_VCAL_FACTORY_H
#define _KPILOT_VCAL_FACTORY_H
/* vcal-factory.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the vcal-conduit plugin.
** It also defines the class for the behavior of the setup dialog.
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

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <klibloader.h>

class KInstance;
class KAboutData;

class VCalConduitFactory : public KLibFactory
{
Q_OBJECT

public:
	VCalConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~VCalConduitFactory();

	static KAboutData *about() { return fAbout; } ;

	static const char * const group;
	static const char * const calendarFile,
		* const firstTime,
		* const deleteOnPilot, 
		*const fullSyncOnPCChange, 
		*const alwaysFullSync;

protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
	static KAboutData *fAbout;
} ;

extern "C"
{

void *init_libvcalconduit();

} ;

// $Log$
// Revision 1.5  2002/04/19 19:34:11  kainhofe
// didn't compile
//
// Revision 1.4  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.3  2001/12/28 12:56:46  adridg
// Added SyncAction, it may actually do something now.
//
// Revision 1.2  2001/12/27 16:43:36  adridg
// Fixup configuration
//
// Revision 1.1  2001/12/13 21:40:40  adridg
// New files for move to .so
//

#endif
