/*
 *  datetime.cpp  -  date/time representation with optional date-only value
 *  Program:  kalarm
 *  (C) 2003 by David Jarvie  software@astrojar.org.uk
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  As a special exception, permission is given to link this program
 *  with any edition of Qt, and distribute the resulting executable,
 *  without including the source code for Qt in the source distribution.
 */

#include <kglobal.h>
#include <klocale.h>

#include "kalarmapp.h"
#include "preferences.h"
#include "datetime.h"


QTime DateTime::time() const
{
	return mDateOnly ? theApp()->preferences()->startOfDay() : mDateTime.time();
}

QDateTime DateTime::dateTime() const
{
	return mDateOnly ? QDateTime(mDateTime.date(), theApp()->preferences()->startOfDay()) : mDateTime;
}

QString DateTime::formatLocale(bool shortFormat) const
{
	if (mDateOnly)
		return KGlobal::locale()->formatDate(mDateTime.date(), shortFormat);
	else
		return KGlobal::locale()->formatDateTime(mDateTime, shortFormat);
}

bool operator==(const DateTime& dt1, const DateTime& dt2)
{
	if (dt1.mDateTime.date() != dt2.mDateTime.date())
		return false;
	if (dt1.mDateOnly && dt2.mDateOnly)
		return true;
	if (!dt1.mDateOnly && !dt2.mDateOnly)
		return dt1.mDateTime.time() == dt2.mDateTime.time();
	return (dt1.mDateOnly ? dt2.mDateTime.time() : dt1.mDateTime.time()) == theApp()->preferences()->startOfDay();
}

bool operator<(const DateTime& dt1, const DateTime& dt2)
{
	if (dt1.mDateTime.date() != dt2.mDateTime.date())
		return dt1.mDateTime.date() < dt2.mDateTime.date();
	if (dt1.mDateOnly && dt2.mDateOnly)
		return false;
	if (!dt1.mDateOnly && !dt2.mDateOnly)
		return dt1.mDateTime.time() < dt2.mDateTime.time();
	QTime t = theApp()->preferences()->startOfDay();
	if (dt1.mDateOnly)
		return t < dt2.mDateTime.time();
	return dt1.mDateTime.time() < t;
}
