/*
    KDE Alarm Daemon.

    This file is part of the KDE alarm daemon.
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

#include <unistd.h>
#include <stdlib.h>

#include <qtimer.h>
#include <qdatetime.h>

#include <kapp.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kio/netaccess.h>
#include <dcopclient.h>

#include <libkcal/event.h>

#include "adcalendar.h"

ADCalendar::ADCalendar(const QString& url, const QCString& appname, Type type)
  : ADCalendarBase(url, appname, type),
    available_( false ),
    enabled_(true)
{
  loadFile();
}

ADCalendar *ADCalendarFactory::create(const QString& url, const QCString& appname,
                                      ADCalendarBase::Type type)
{
  return new ADCalendar(url, appname, type);
}

/*
 * Check whether all the alarms for the event with the given ID have already
 * been handled.
 */
bool ADCalendar::eventHandled(const Event* event, const QValueList<QDateTime>& alarmtimes)
{
  EventsMap::ConstIterator it = eventsHandled_.find(event->VUID());
  if (it == eventsHandled_.end())
    return false;

  int oldCount = it.data().alarmTimes.count();
  int count = alarmtimes.count();
  for (int i = 0;  i < count;  ++i) {
    if (alarmtimes[i].isValid()) {
      if (i >= oldCount                              // is it an additional alarm?
      ||  !it.data().alarmTimes[i].isValid()         // or has it just become due?
      ||  it.data().alarmTimes[i].isValid()          // or has it changed?
       && alarmtimes[i] != it.data().alarmTimes[i])
        return false;     // this alarm has changed
    }
  }
  return true;
}

/*
 * Remember that the specified alarms for the event with the given ID have been
 * handled.
 */
void ADCalendar::setEventHandled(const Event* event, const QValueList<QDateTime>& alarmtimes)
{
  if (event)
  {
    kdDebug(5900) << "ADCalendar::setEventHandled(" << event->VUID() << ")\n";
    EventsMap::Iterator it = eventsHandled_.find(event->VUID());
    if (it != eventsHandled_.end())
    {
      // Update the existing entry for the event
      it.data().alarmTimes = alarmtimes;
      it.data().eventSequence = event->revision();
    }
    else
      eventsHandled_.insert(event->VUID(),EventItem(urlString(),
                                                    event->revision(),
                                                    alarmtimes));
  }
}

/*
 * Clear all memory of events handled for the specified calendar.
 */
void ADCalendar::clearEventsHandled(const QString& calendarURL)
{
  for (EventsMap::Iterator it = eventsHandled_.begin();  it != eventsHandled_.end();  )
  {
    if (it.data().calendarURL == calendarURL)
    {
      EventsMap::Iterator i = it;
      ++it;                      // prevent iterator becoming invalid with remove()
      eventsHandled_.remove(i);
    }
    else
      ++it;
  }
}

/*
 * Note that the event with the given ID is pending
 * (i.e. waiting until the client can be notified).
 */
void ADCalendar::setEventPending(const QString& ID)
{
  if (actionType() == KALARM  &&  !eventsPending_.containsRef(&ID))
  {
    eventsPending_.append(&ID);
    kdDebug(5900) << "ADCalendar::setEventPending(): " << ID << endl;
  }
}

/*
 * Get an event from the pending list, and remove it from the list.
 */
bool ADCalendar::getEventPending(QString& ID)
{
  if (actionType() == KALARM)
  {
    QString* eventID = eventsPending_.getFirst();
    if (eventID)
    {
      eventsPending_.removeFirst();
      ID = *eventID;
      return true;
    }
  }
  return false;
}
