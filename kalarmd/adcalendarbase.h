/*
    Calendar access for KDE Alarm Daemon and KDE Alarm Daemon GUI.

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

#ifndef ADCALENDARBASE_H
#define ADCALENDARBASE_H

#include <libkcal/calendarlocal.h>

#include "compat.h"

using namespace KCal;

// Base class for Alarm Daemon calendar access
class ADCalendarBase : public CalendarLocal
{
  public:
    enum Type { KORGANIZER = 0, KALARM = 1 };
    ADCalendarBase(const QString& url, const QString& appname, Type);
    ~ADCalendarBase()  { }

    const QString&  urlString() const   { return urlString_; }
    const QString&  appName() const     { return appName_; }
    bool            loaded() const      { return loaded_; }
    Type            actionType() const  { return actionType_; }

    virtual void setEnabled( bool ) = 0;
    virtual bool enabled() const = 0;   

    virtual void setAvailable( bool ) = 0;
    virtual bool available() const = 0;
  
    // client has registered since calendar was
    // constructed, but has not since added the
    // calendar. Monitoring is disabled.
    void setUnregistered( bool u ) { mUnregistered = u; }
    bool unregistered() const { return mUnregistered; }

    virtual bool loadFile() = 0;

    virtual void setEventHandled(const Event*, const QValueList<QDateTime> &) = 0;
    virtual bool eventHandled(const Event*, const QValueList<QDateTime> &) = 0;

    virtual void setEventPending(const QString& ID) = 0;
    virtual bool getEventPending(QString& ID) = 0;

  protected:
    bool            loadFile_(const QString& appNamebool);

  private:
    ADCalendarBase(const ADCalendarBase&);             // prohibit copying
    ADCalendarBase& operator=(const ADCalendarBase&);  // prohibit copying

  protected:
    struct EventItem
    {
      EventItem() : eventSequence(0) { }
      EventItem(const QString& url, int seqno, const QValueList<QDateTime>& alarmtimes)
        : calendarURL(url), eventSequence(seqno), alarmTimes(alarmtimes) {}
      
      QString   calendarURL;
      int       eventSequence;
      QValueList<QDateTime> alarmTimes;
    };
    
    typedef QMap<QString, EventItem>  EventsMap;   // event ID, calendar URL/event sequence num
    static EventsMap  eventsHandled_; // IDs of displayed KALARM type events

  private:
    QString           urlString_;     // calendar file URL
    QString           appName_;       // name of application owning this calendar
    Type              actionType_;    // action to take on event
    bool              loaded_;        // true if calendar file is currently loaded

    bool mUnregistered;
};

typedef QPtrList<ADCalendarBase> CalendarList;

class ADCalendarBaseFactory
{
  public:
    virtual ADCalendarBase *create(const QString& url, const QString& appname,
                                   ADCalendarBase::Type) = 0;
};


#endif
