/*
    Calendar access for KDE Alarm Daemon.

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
#ifndef ADCALENDAR_H
#define ADCALENDAR_H

#include "adcalendarbase.h"

// Alarm Daemon calendar access
class ADCalendar : public ADCalendarBase
{
  public:
    ADCalendar(const QString& url, const QString& appname, Type);
    ~ADCalendar()  { }
    ADCalendar *create(const QString& url, const QString& appname, Type);

    void           setEnabled( bool enabled ) { enabled_ = enabled; }
    bool           enabled() const     { return enabled_ && !unregistered(); }

    void           setAvailable( bool ) {}
    bool           available() const   { return loaded() && !unregistered(); }

    bool           eventHandled(const Event*, const QValueList<QDateTime> &);
    void           setEventHandled(const Event*,
                                   const QValueList<QDateTime> &);

    void           setEventPending(const QString& ID);
    bool           getEventPending(QString& ID);

    static void    clearEventsHandled(const QString& calendarURL);

    bool           loadFile()          { return loadFile_(QString()); }

  public:
    bool              available_;
    bool              enabled_;       // events are currently manually enabled
  private:
    QPtrList<QString> eventsPending_; // IDs of pending KALARM type events
};

class ADCalendarFactory : public ADCalendarBaseFactory
{
  public:
    ADCalendar *create(const QString& url, const QString& appname,
                       ADCalendarBase::Type);
};

#endif
