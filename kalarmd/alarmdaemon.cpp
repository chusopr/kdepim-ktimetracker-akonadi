/*
    KDE Alarm Daemon.

    This file is part of the KDE alarm daemon.
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>
    Based on the original, (c) 1998, 1999 Preston Brown

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
#include <qfile.h>
#include <qdatetime.h>

#include <kapp.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kio/netaccess.h>
#include <dcopclient.h>

#include <libkcal/calendarlocal.h>

#include "alarmguiiface_stub.h"
#include "alarmapp.h"

#include "alarmdaemon.h"
#include "alarmdaemon.moc"


const int LOGIN_DELAY( 5 );

AlarmDaemon::AlarmDaemon(QObject *parent, const char *name)
  : QObject(parent, name), DCOPObject(name),
    mSessionStartTimer(0L),
    mEnabled( true )
{
  kdDebug(5900) << "AlarmDaemon::AlarmDaemon()" << endl;

  bool splash = kapp->dcopClient()->isApplicationRegistered("ksplash");
  if (splash  ||  static_cast<AlarmApp*>(kapp)->startedAtLogin())
  {
    // Login session is starting up - need to wait for it to complete
    // in order to prevent the daemon starting clients before they are
    // restored by the session (where applicable).
    // If ksplash can be detected as running, start a 1-second timer;
    // otherwise, wait a few seconds.
    kdDebug(5900) << "AlarmDaemon::AlarmDaemon(): session start\n";
    mSessionStartTimer = new QTimer(this);
    connect(mSessionStartTimer, SIGNAL(timeout()), SLOT(checkIfSessionStarted()));
    mSessionStartTimer->start(splash ? 1000 : LOGIN_DELAY * 1000);
  }

  readDaemonData(!!mSessionStartTimer);

  enableAutoStart(true);    // switch autostart on whenever the program is run

  // set up the alarm timer
  mAlarmTimer = new QTimer(this);
  setTimerStatus();
  checkAlarms();
}

AlarmDaemon::~AlarmDaemon()
{
}

/*
 * DCOP call to quit the program.
 */
void AlarmDaemon::quit()
{
  kdDebug(5900) << "AlarmDaemon::quit()" << endl;
  exit(0);
}

void AlarmDaemon::dumpDebug()
{
  kdDebug(5900) << "AlarmDaemon::dumpDebug()" << endl;
  
  for( ADCalendarBase *cal = mCalendars.first(); cal; cal = mCalendars.next() ) {
    cal->dump();
  }
  
  kdDebug(5900) << "AlarmDaemon::dumpDebug() done" << endl;  
}

/*
 * DCOP call to enable or disable monitoring of a calendar.
 */
void AlarmDaemon::enableCal_(const QString& urlString, bool enable)
{
  kdDebug(5900) << "AlarmDaemon::enableCal_(" << urlString << ")" << endl;

  ADCalendarBase* cal = getCalendar(urlString);
  if (cal)
  {
    cal->setEnabled( enable );
    notifyGuiCalStatus(cal);    // notify any other GUI applications
  }
}

/*
 * DCOP call to add a new calendar file to the list of monitored calendars.
 * If the calendar file is already in the list, the request is ignored.
 */
void AlarmDaemon::addCal_(const QCString& appname, const QString& urlString, bool msgCal)
{
  kdDebug(5900) << "AlarmDaemon::addCal_(" << urlString << "): " << (msgCal ? "KALARM" : "KORGANIZER") << endl;

  ADCalendarBase* cal = getCalendar(urlString);
  if (cal)
  {
    // Calendar is already being monitored
    if (!cal->unregistered())
      return;
    if (cal->appName() == appname)
    {
      cal->setUnregistered( false );
      reloadCal_(cal);
      return;
    }
    // The calendar used to belong to another application!
    mCalendars.remove(cal);
  }

  // Load the calendar
  cal = new ADCalendar(urlString, appname, (msgCal ? ADCalendar::KALARM : ADCalendar::KORGANIZER));
  mCalendars.append(cal);

  writeConfigCalendar(appname, cal);

  if (cal->loaded())
    notifyGui((msgCal ? ADD_MSG_CALENDAR : ADD_CALENDAR), cal->urlString(), appname);
  kdDebug(5900) << "AlarmDaemon::addCal_(): calendar added" << endl;

  setTimerStatus();
//  checkAlarms(cal);
}

/*
 * DCOP call to reload the specified calendar.
 * The calendar is first added to the list of monitored calendars if necessary.
 */
void AlarmDaemon::reloadCal_(const QCString& appname, const QString& urlString, bool msgCal)
{
  kdDebug(5900) << "AlarmDaemon::reloadCal_(" << urlString << "): " << (msgCal ? "KALARM" : "KORGANIZER") << endl;

  if (!urlString.isEmpty())
  {
    ADCalendarBase* cal = getCalendar(urlString);
    if (cal)
      reloadCal_(cal);
    else
    {
      // Calendar wasn't in the list, so add it
      if (!appname.isEmpty())
        addCal_(appname, urlString, msgCal);
    }
  }
}

/*
 * Reload the specified calendar.
 */
void AlarmDaemon::reloadCal_(ADCalendarBase* cal)
{
  kdDebug(5900) << "AlarmDaemon::reloadCal_(): calendar" << endl;

  if (cal)
  {
    cal->close();
    if (cal->loadFile())
      kdDebug(5900) << "AlarmDaemon::reloadCal_(): calendar reloaded" << endl;
    notifyGuiCalStatus(cal);
    setTimerStatus();
//    checkAlarms(cal);
  }
}

/*
 * DCOP call to reload the specified calendar and reset the data associated with it.
 * The calendar is first added to the list of monitored calendars if necessary.
 */
void AlarmDaemon::resetMsgCal_(const QCString& appname, const QString& urlString)
{
  kdDebug(5900) << "AlarmDaemon::resetMsgCal_(" << urlString << ")\n";

  reloadCal_(appname, urlString, true);
  ADCalendar::clearEventsHandled(urlString);
}

/* Remove a calendar file from the list of monitored calendars */
void AlarmDaemon::removeCal_(const QString& urlString)
{
  kdDebug(5900) << "AlarmDaemon::removeCal_(" << urlString << ")\n";

  ADCalendarBase* cal = getCalendar(urlString);
  if (cal)
  {
    deleteConfigCalendar(cal);
    mCalendars.remove(cal);
    kdDebug(5900) << "AlarmDaemon::removeCal_(): calendar removed" << endl;
    notifyGui(DELETE_CALENDAR, urlString);
    setTimerStatus();
  }
}

/*
 * DCOP call to add an application to the list of client applications,
 * and add it to the config file.
 */
void AlarmDaemon::registerApp(const QCString& appName, const QString& appTitle,
                              const QCString& dcopObject, int notificationType,
                              bool displayCalendarName)
{
  kdDebug(5900) << "AlarmDaemon::registerApp(" << appName << ", " << appTitle << ")\n";
  if (!appName.isEmpty())
  {
    if (KStandardDirs::findExe(appName) == QString::null)
      kdError() << "AlarmDaemon::registerApp(): app not found\n";
    else
    {
      ClientInfo c = getClientInfo(appName);
      if (c.isValid())
      {
        // The application is already in the clients list.
        // Mark all its calendar files as unregistered and remove it from the list.
        for (ADCalendarBase* cal = mCalendars.first();  cal;  cal = mCalendars.next())
        {
          if (cal->appName() == appName)
            cal->setUnregistered( true );
        }
        removeClientInfo(appName);
      }
      ClientInfo cinfo(appName, appTitle, dcopObject, notificationType, displayCalendarName);
      mClients.append(cinfo);

      writeConfigClient(appName, cinfo);

      enableAutoStart(true);
      notifyGui(CHANGE_CLIENT);
      setTimerStatus();
//      checkAlarms(appName);
    }
  }
}

/*
 * DCOP call to set autostart at login on or off.
 */
void AlarmDaemon::enableAutoStart(bool on)
{
  kdDebug(5900) << "AlarmDaemon::enableAutoStart(" << (int)on << ")\n";
  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry("Autostart", on);
  config->sync();
  notifyGui(CHANGE_STATUS);
}

/*
 * Check if any alarms are pending for any enabled calendar, and
 * display the pending alarms.
 * Called by the alarm timer.
 */
void AlarmDaemon::checkAlarmsSlot()
{
  kdDebug(5901) << "AlarmDaemon::checkAlarmsSlot()" << endl;

  if (mAlarmTimerSyncing)
  {
    // We've synced to the minute boundary. Now set timer to 1 minute intervals.
    mAlarmTimer->changeInterval(1000 * 60);
    mAlarmTimerSyncing = false;
  }
  checkAlarms();
}

/*
 * Check if any alarms are pending for any enabled calendar, and
 * display the pending alarms.
 */
void AlarmDaemon::checkAlarms()
{
  kdDebug(5901) << "AlarmDaemon::checkAlarms()" << endl;

  if ( !mEnabled ) return;

  QDateTime now = QDateTime::currentDateTime();

  ADCalendarBase *cal;
  for( cal = mCalendars.first(); cal; cal = mCalendars.next() ) {
    checkAlarms( cal, mLastCheck.addSecs( 1 ), now );
  }
  
  mLastCheck = now;
}

/*
 * Check if any alarms are pending for any enabled calendar
 * belonging to a specified client, and display the pending alarms.
 */
void AlarmDaemon::checkAlarms(const QCString& appName)
{
  if ( !mEnabled ) return;

  for (ADCalendarBase* cal = mCalendars.first();  cal;  cal = mCalendars.next())
  {
    if (cal->appName() == appName)
      checkAlarms( cal, mLastCheck.addSecs( 1 ), QDateTime::currentDateTime() );
  }
}

/*
 * Check if any alarms are pending for a specified calendar, and
 * display the pending alarms.
 * Reply = true if there were any KORGANIZER type alarms.
 */
void AlarmDaemon::checkAlarms( ADCalendarBase* cal, const QDateTime &from, const QDateTime &to)
{
  kdDebug(5901) << "AlarmDaemons::checkAlarms(): '" << cal->urlString() << "'" << endl;
  
  if ( !mEnabled  ||  !cal->loaded()  ||  !cal->enabled() ) return;

  kdDebug(5901) << "  From: " << from.toString() << "  To: " << to.toString() << endl;

  QPtrList<Event> alarmEvents;
  Alarm::List alarms;
  Alarm::List::ConstIterator it;
  switch ( cal->actionType() ) {
    case ADCalendar::KORGANIZER:
      alarms = cal->alarms( from, to );
      for ( it = alarms.begin();  it != alarms.end();  ++it ) {
        kdDebug(5901) << "AlarmDaemon::checkAlarms(): KORGANIZER event "
                      << (*it)->parent()->VUID() << endl;
        notifyEvent(cal, (*it)->parent()->VUID());
      }
      break;

    case ADCalendar::KALARM:
      alarms = cal->alarmsTo( QDateTime::currentDateTime() );
      if (alarms.count()) {
        kdDebug(5901) << "Kalarm alarms=" << alarms.count() << endl;
        for ( it = alarms.begin(); it != alarms.end(); ++it ) {
          Event *event = dynamic_cast<Event *>( (*it)->parent() );
          if ( event ) {
            const QString& eventID = event->VUID();
            kdDebug(5901) << "AlarmDaemon::checkAlarms(): KALARM event " << eventID  << endl;
            QValueList<QDateTime> alarmtimes;
            checkEventAlarms(*event, alarmtimes);
            if (!cal->eventHandled(event, alarmtimes)) {
              if (notifyEvent(cal, eventID))
                cal->setEventHandled(event, alarmtimes);
              else
                cal->setEventPending(eventID);
            }
          }
        }
      }
      break;
  }
}

/*
 * Check which of the alarms for the given event are due.
 * The times in 'alarmtimes' corresponding to due alarms are set.
 */
void AlarmDaemon::checkEventAlarms(const Event& event, QValueList<QDateTime>& alarmtimes)
{
  alarmtimes.clear();
  const Alarm* alarm;
  QDateTime now = QDateTime::currentDateTime();
  for (QPtrListIterator<Alarm> it(event.alarms());  (alarm = it.current()) != 0;  ++it) {
    alarmtimes.append((alarm->enabled()  &&  alarm->time() <= now) ? alarm->time() : QDateTime());
  }
}

/*
 * Send a DCOP message to a client application telling it that an alarm
 * should now be handled.
 * Reply = false if the event should be held pending until the client
 *         application can be started.
 */
bool AlarmDaemon::notifyEvent(const ADCalendarBase* calendar, const QString& eventID)
{
  kdDebug(5900) << "AlarmDaemon::notifyEvent(" << eventID << ")\n";
  if (calendar)
  {
    ClientInfo client = getClientInfo(calendar->appName());
kdDebug(5900)<<"Notification type="<<client.notificationType<<": "<<calendar->appName()<<endl;
    if (!client.isValid())
      kdDebug(5900) << "AlarmDaemon::notifyEvent(): unknown client" << endl;
    else
    {
      if (client.waitForRegistration)
      {
        // Don't start the client application if the session manager is still
        // starting the session, since if we start the client before the
        // session manager does, a KUniqueApplication client will not then be
        // able to restore its session.
        // And don't contact a client which was started by the login session
        // until it's ready to handle DCOP calls.
        kdDebug(5900) << "AlarmDaemon::notifyEvent(): wait for session startup" << endl;
        return false;
      }
      if (!kapp->dcopClient()->isApplicationRegistered(static_cast<const char*>(calendar->appName())))
      {
        // The client application is not running
        if (client.notificationType == ClientInfo::NO_START_NOTIFY)
        {
          kdDebug(5900) << "AlarmDaemon::notifyEvent(): don't start client\n";
          return true;
        }

        // Start the client application
        QString execStr = locate("exe", calendar->appName());
        if (execStr.isEmpty())
        {
          kdDebug(5900) << "AlarmDaemon::notifyEvent(): '" << calendar->appName() << "' not found\n";
          return true;
        }
        if (client.notificationType == ClientInfo::COMMAND_LINE_NOTIFY)
        {
          // Use the command line to tell the client about the alarm
          execStr += " --handleEvent ";
          execStr += eventID;
          execStr += " --calendarURL ";
          execStr += calendar->urlString();
          system(QFile::encodeName(execStr));
          kdDebug(5900) << "AlarmDaemon::notifyEvent(): used command line\n";
          return true;
        }
        system(QFile::encodeName(execStr));
        kdDebug(5900) << "AlarmDaemon::notifyEvent(): started " << QFile::encodeName(execStr) << endl;
      }
      
      AlarmGuiIface_stub stub( calendar->appName(), client.dcopObject );
      stub.handleEvent( calendar->urlString(), eventID );
      if ( !stub.ok() ) {
        kdDebug(5900) << "AlarmDaemon::notifyEvent(): dcop send failed" << endl;
      }
    }
  }
  return true;
}

/* Notify the specified client of any pending alarms */
void AlarmDaemon::notifyPendingEvents(const QCString& appname)
{
  kdDebug(5900) << "AlarmDaemon::notifyPendingEvents(" << appname << ")\n";
  for (ADCalendarBase* cal = mCalendars.first(); cal; cal = mCalendars.next())
  {
    if (cal->appName() == appname
    &&  cal->actionType() == ADCalendar::KALARM)
    {
      QString eventID;
      while (cal->getEventPending(eventID))
      {
        notifyEvent(cal, eventID);
        const Event* event = cal->getEvent(eventID);
        QValueList<QDateTime> alarmtimes;
        checkEventAlarms(*event, alarmtimes);
        cal->setEventHandled(event, alarmtimes);
      }
    }
  }
}

/*
 * Called by the timer to check whether session startup is complete.
 * If so, it checks which clients are already running and notifies
 * any which have registered of any pending alarms.
 * (Ideally checking for session startup would be done using a signal
 * from ksmserver, but until such a signal is available, we can check
 * whether ksplash is still running.)
 */
void AlarmDaemon::checkIfSessionStarted()
{
  if (!kapp->dcopClient()->isApplicationRegistered("ksplash"))
  {
    // Session startup has now presumably completed. Cancel the timer.
    kdDebug(5900) << "AlarmDaemon::checkIfSessionStarted(): startup complete\n";
    delete mSessionStartTimer;

    // Notify clients which are not yet running of pending alarms
    for (ClientList::Iterator client = mClients.begin();  client != mClients.end();  ++client)
    {
      if (!kapp->dcopClient()->isApplicationRegistered(static_cast<const char*>((*client).appName)))
      {
        (*client).waitForRegistration = false;
        notifyPendingEvents((*client).appName);
      }
    }

    mSessionStartTimer = 0L;    // indicate that session startup is complete
  }
}

/*
 * Starts or stops the alarm timer as necessary after a calendar is enabled/disabled.
 */
void AlarmDaemon::setTimerStatus()
{
  // Count the number of currently loaded calendars whose names should be displayed
  int nLoaded = 0;
  for (ADCalendarBase* cal = mCalendars.first();  cal;  cal = mCalendars.next()) {
    if (cal->loaded())
      ++nLoaded;
  }

  // Start or stop the alarm timer if necessary
  if (!mAlarmTimer->isActive() && nLoaded)
  {
    // Timeout every minute.
    // But first synchronise to one second after the minute boundary.
    int firstInterval = 61 - QTime::currentTime().second();
    mAlarmTimer->start(1000 * firstInterval);
    mAlarmTimerSyncing = (firstInterval != 60);
    connect(mAlarmTimer, SIGNAL(timeout()), SLOT(checkAlarmsSlot()));
    kdDebug(5900) << "Started alarm timer" << endl;
  }
  else if (mAlarmTimer->isActive() && !nLoaded)
  {
    mAlarmTimer->disconnect();
    mAlarmTimer->stop();
    kdDebug(5900) << "Stopped alarm timer" << endl;
  }
}

/*
 * DCOP call to add an application to the list of GUI applications,
 * and add it to the config file.
 */
void AlarmDaemon::registerGui(const QCString& appName, const QCString& dcopObject)
{
  kdDebug(5900) << "AlarmDaemon::registerGui(" << appName << ")\n";
  if (!appName.isEmpty())
  {
    const GuiInfo* g = getGuiInfo(appName);
    if (g)
      mGuis.remove(appName);   // the application is already in the GUI list
    mGuis.insert(appName, GuiInfo(dcopObject));

    writeConfigClientGui(appName, dcopObject);
  }
}

/*
 * Send a DCOP message to all GUI interface applications, notifying them of a change
 * in calendar status.
 */
void AlarmDaemon::notifyGuiCalStatus(const ADCalendarBase* cal)
{
   notifyGui((cal->available() ? (cal->enabled() ? ENABLE_CALENDAR : DISABLE_CALENDAR) : CALENDAR_UNAVAILABLE),
             cal->urlString());
}

/*
 * Send a DCOP message to all GUI interface applications, notifying them of a change.
 */
void AlarmDaemon::notifyGui(GuiChangeType change, const QString& calendarURL)
{
  notifyGui( change, calendarURL, "" );
}

void AlarmDaemon::notifyGui(GuiChangeType change, const QString& calendarURL, const QCString& appName)
{
  QString changeType;
  switch (change)
  {
    case CHANGE_STATUS:         changeType = "STATUS";               break;
    case CHANGE_CLIENT:         changeType = "CLIENT";               break;
    case CHANGE_GUI:            changeType = "GUI";                  break;
    case ADD_CALENDAR:          changeType = "ADD_CALENDAR";         break;
    case ADD_MSG_CALENDAR:      changeType = "ADD_MSG_CALENDAR";     break;
    case DELETE_CALENDAR:       changeType = "DELETE_CALENDAR";      break;
    case ENABLE_CALENDAR:       changeType = "ENABLE_CALENDAR";      break;
    case DISABLE_CALENDAR:      changeType = "DISABLE_CALENDAR";     break;
    case CALENDAR_UNAVAILABLE:  changeType = "CALENDAR_UNAVAILABLE"; break;
    default:
      kdError() << "AlarmDaemon::guiNotify(): " << change << endl;
      return;
  }
  kdDebug(5900) << "AlarmDaemon::notifyGui(" << changeType << ")\n";

  for (GuiMap::ConstIterator g = mGuis.begin();  g != mGuis.end();  ++g)
  {
    const QString& dcopObject = g.data().dcopObject;
    if (kapp->dcopClient()->isApplicationRegistered(static_cast<const char*>(g.key())))
    {
      kdDebug(5900)<<"AlarmDaemon::notifyGui() sending:" << g.key()<<" ->" << dcopObject <<endl;
      QByteArray data;
      QDataStream arg(data, IO_WriteOnly);
      arg << changeType << calendarURL << appName;
      if (!kapp->dcopClient()->send(static_cast<const char*>(g.key()),
                                    static_cast<const char*>( dcopObject),
                                    "alarmDaemonUpdate(const QString&,const QString&,const QString&)",
                                    data))
        kdDebug(5900) << "AlarmDaemon::guiNotify(): dcop send failed:" << g.key() << endl;
    }
  }
}

/* Return the GuiInfo structure for the specified GUI application */
const AlarmDaemon::GuiInfo* AlarmDaemon::getGuiInfo(const QCString& appName) const
{
  if (!appName.isEmpty())
  {
    GuiMap::ConstIterator g = mGuis.find(appName);
    if (g != mGuis.end())
      return &g.data();
  }
  return 0L;
}

void AlarmDaemon::dumpAlarms()
{
  QDateTime start = QDateTime( QDateTime::currentDateTime().date(),
                               QTime( 0, 0 ) );
  QDateTime end = start.addDays( 1 ).addSecs( -1 );

  kdDebug(5900) << "AlarmDeamon::dumpAlarms() from " << start.toString()
            << " to " << end.toString() << endl;

  CalendarList cals = calendars();
  ADCalendarBase *cal;
  for( cal = cals.first(); cal; cal = cals.next() ) {
    kdDebug(5900) << "  Cal: " << cal->urlString() << endl;
    Alarm::List alarms = cal->alarms( start, end );
    Alarm::List::ConstIterator it;
    for( it = alarms.begin(); it != alarms.end(); ++it ) {
      Alarm *a = *it;
      kdDebug(5900) << "    " << a->parent()->summary() << " ("
                << a->time().toString() << ")" << endl;
    }
  }
}
