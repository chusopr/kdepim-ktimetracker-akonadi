#ifndef _KPILOT_TODO_FACTORY_H
#define _KPILOT_TODO_FACTORY_H
/* todo-factory.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the todo-conduit plugin.
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
#include "vcal-factorybase.h"

class KInstance;
class KAboutData;

class ToDoConduitFactory : public VCalConduitFactoryBase
{
Q_OBJECT

public:
	ToDoConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~ToDoConduitFactory();

	static KAboutData *about() { return fAbout; } ;

	static const char * const group;

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

void *init_libtodoconduit();

} ;

// $Log$
// Revision 1.2.2.1  2002/04/28 12:58:54  kainhofe
// Calendar conduit now works, no memory leaks, timezone still shifted. Todo conduit mostly works, for my large list it crashes when saving the calendar file.
//
// Revision 1.3  2002/04/22 22:51:51  kainhofe
// Added the first version of the todo conduit, fixed a check for a null pointer in the datebook conduit
//
// Revision 1.2  2001/12/27 16:43:36  adridg
// Fixup configuration
//
// Revision 1.1  2001/12/13 21:40:40  adridg
// New files for move to .so
//

#endif
