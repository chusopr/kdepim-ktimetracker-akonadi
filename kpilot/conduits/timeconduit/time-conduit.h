#ifndef _Time_CONDUIT_H
#define _Time_CONDUIT_H
/* time-conduit.h                           KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <plugin.h>

class TimeConduit : public ConduitAction
{
Q_OBJECT
public:
	TimeConduit(
		KPilotDeviceLink *o,
		const char *n = 0L,
		const QStringList &a = QStringList() );
	virtual ~TimeConduit();
	virtual bool exec();

public slots:
	void syncHHfromPC();
	void syncPCfromHH();

protected:
	void readConfig();
} ;

#endif
