/*
 *  alarmcalendar.h  -  KAlarm calendar file access
 *  Program:  kalarm
 *  (C) 2001, 2002 by David Jarvie  software@astrojar.org.uk
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
 */

#ifndef ALARMCALENDAR_H
#define ALARMCALENDAR_H

#include <libkcal/calendarlocal.h>
#include "msgevent.h"
using namespace KCal;


class AlarmCalendar
{
	public:
		AlarmCalendar() :     calendar(0L), kAlarmVersion(-1) { }
		bool                  open();
		int                   load();
		int                   reload();
		bool                  save()                              { return save(localFile); }
		void                  close();
		Event*                event(const QString& uniqueID)      { return calendar->event(uniqueID); }
		QPtrList<Event>       events()                            { return calendar->events(); }
		void                  addEvent(const KAlarmEvent&);
		void                  updateEvent(const KAlarmEvent&);
		void                  deleteEvent(const QString& eventID);
		bool                  isOpen() const                      { return !!calendar; }
		void                  getURL() const;
		const QString         urlString() const                   { getURL();  return url.url(); }
		int                   kalarmVersion() const;
		static const QString& getDefaultTimeZoneID();
	private:
		CalendarLocal*        calendar;
		KURL                  url;           // URL of calendar file
		QString               localFile;     // local name of calendar file
		mutable int           kAlarmVersion; // version of KAlarm which created the loaded calendar file
		bool                  vCal;          // true if calendar file is in VCal format

		bool                  create();
		bool                  save(const QString& tempFile);
};

#endif // ALARMCALENDAR_H
