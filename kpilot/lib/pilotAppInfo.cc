/* pilotAppInfo.cc		KPilot
**
** Copyright (C) 2005-2006 Adriaan de Groot <groot@kde.org>
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#include "pilotAppInfo.h"

#include <stdio.h>

#include "options.h"

PilotAppInfoBase::PilotAppInfoBase(PilotDatabase *d) :
	fC( 0L ),
	fLen(0),
	fOwn(true)
{
	FUNCTIONSETUP;
	int appLen = Pilot::MAX_APPINFO_SIZE;
	unsigned char buffer[Pilot::MAX_APPINFO_SIZE];

	if (!d || !d->isOpen())
	{
		WARNINGKPILOT << "Bad database pointer.";
		fLen = 0;
		KPILOT_DELETE( fC );
		return;
	}

	fC = new struct CategoryAppInfo;
	fLen = appLen = d->readAppBlock(buffer,appLen);
	unpack_CategoryAppInfo(fC, buffer, appLen);
}

PilotAppInfoBase::~PilotAppInfoBase()
{
	if (fOwn)
	{
		delete fC;
	}
}

bool PilotAppInfoBase::setCategoryName(unsigned int i, const QString &s)
{
	if ( (i>=Pilot::CATEGORY_COUNT) || // bad category number
		(!categoryInfo())) // Nowhere to write to
	{
		return false;
	}

	categoryInfo()->ID[i] = i;
	// first, clean up target
	memset( categoryInfo()->name[i], 0, Pilot::CATEGORY_SIZE );
	// now make sure we only put in 15 characters
	QString name=s.left(Pilot::CATEGORY_SIZE - 1);
	// now set the category info, using only the length of our string
	(void) Pilot::toPilot(s, categoryInfo()->name[i], name.length());
	return true;
}


