/*
 *  kalarmapp.cpp  -  description
 *  Program:  kalarm
 *
 *  (C) 2001 by David Jarvie  software@astrojar.org.uk
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "kalarm.h"

#include <ctype.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include <dcopclient.h>
#include <kdebug.h>

#include <vcaldrag.h>
#include <vcalformat.h>
#include <icalformat.h>

#include "mainwindow.h"
#include "messagewin.h"
#include "prefsettings.h"
#include "kalarmapp.h"
#include "kalarmapp.moc"

const QString DCOP_OBJECT_NAME("display");
const QString DEFAULT_CALENDAR_FILE("calendar.ics");

KAlarmApp*  KAlarmApp::theInstance = 0L;


/******************************************************************************
* Construct the application.
*/
KAlarmApp::KAlarmApp()
	:	KUniqueApplication(),
		mainWidget(new MainWidget(DCOP_OBJECT_NAME)),
		daemonRegistered(false),
		m_generalSettings(new GeneralSettings(0L))
{
	m_generalSettings->loadSettings();
}

/******************************************************************************
*/
KAlarmApp::~KAlarmApp()
{
	calendar.close();
}

/******************************************************************************
* Return the one and only KAlarmApp instance.
* If it doesn't already exist, it is created first.
*/
KAlarmApp* KAlarmApp::getInstance()
{
	if (!theInstance)
		theInstance = new KAlarmApp;
	return theInstance;
}

/******************************************************************************
* Called for a KUniqueApplication when a new instance of the application is
* started.
*/
int KAlarmApp::newInstance()
{
	kdDebug()<<"KAlarmApp::newInstance(): New instance\n";
	static bool restored = false;
	int exitCode = 0;      // default = success
	if (!restored  &&  isRestored())
	{
		// Process is being restored by session management.
		exitCode = !initCheck(false);       // open the calendar file (needed for main windows)
		for (int i = 1;  KMainWindow::canBeRestored(i);  ++i)
		{
			if (KMainWindow::classNameOfToplevel(i) == "KAlarmMainWindow")
				(new KAlarmMainWindow)->restore(i);
			else
				(new MessageWin)->restore(i);
		}
		initCheck();           // register with the alarm daemon
		restored = true;       // make sure we restore only once
	}
	else
	{
		KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
		if (args->isSet("stop"))
		{
			// Stop the alarm daemon
			args->clear();      // free up memory
			if (!stopDaemon())
				exitCode = 1;
		}
		else
		if (args->isSet("reset"))
		{
			// Reset the alarm daemon
			args->clear();      // free up memory
			resetDaemon();
		}
		else
		if (args->isSet("handleEvent")  ||  args->isSet("displayEvent")  ||  args->isSet("cancelEvent")  ||  args->isSet("calendarURL"))
		{
			// Display or delete the message with the specified event ID
			kdDebug()<<"Event ...\n";
			EventFunc function = EVENT_HANDLE;
			int count = 0;
			const char* option = 0;
			if (args->isSet("handleEvent"))  { function = EVENT_HANDLE;  option = "handleEvent";  ++count; }
			if (args->isSet("displayEvent"))  { function = EVENT_DISPLAY;  option = "displayEvent";  ++count; }
			if (args->isSet("cancelEvent"))  { function = EVENT_CANCEL;  option = "cancelEvent";  ++count; }
			if (!count)
				args->usage(i18n("--calendarURL requires --handleEvent, --displayEvent or --cancelEvent"));    // exits program
			if (count > 1)
				args->usage(i18n("--handleEvent, --displayEvent, --cancelEvent mutually exclusive"));    // exits program

			QString eventID = args->getOption(option);
			if (!initCheck(false))
				exitCode = 1;
			else
			if (args->isSet("calendarURL"))
			{
				QString calendarUrl = args->getOption("calendarURL");
				if (KURL(calendarUrl).url() != calendar.urlString())
				{
					kdError() << "KAlarmApp::deleteMessage(): wrong calendar file " << calendarUrl << endl;
					exitCode = 1;
				}
			}
			if (!exitCode)
			{
				args->clear();          // free up memory
				if (!handleMessage(eventID, function))
					exitCode = 1;
			}
		}
		else
		if (args->isSet("file")  ||  args->count())
		{
			// Display a message or file
			bool file = false;
			QCString alMessage;
			if (args->isSet("file"))
			{
				kdDebug()<<"File ...\n";
				if (args->count())
					args->usage(i18n("message incompatible with --file"));      // exits program
				alMessage = args->getOption("file");
				file = true;
			}
			else
			{
				kdDebug()<<"Message ...\n";
				alMessage = args->arg(0);
			}

			long flags = 0;
			QDateTime* alarmTime = 0L;
			QDateTime wakeup;
			QColor bgColour = generalSettings()->defaultBgColour();
			int    repeatCount = 0;
			int    repeatInterval = 0;
			if (args->isSet("colour"))
			{
				// Colour is specified
				QCString colourText = args->getOption("colour");
				if (static_cast<const char*>(colourText)[0] == '0'
				&&  tolower(static_cast<const char*>(colourText)[1]) == 'x')
					colourText.replace(0, 2, "#");
				bgColour.setNamedColor(colourText);
				if (!bgColour.isValid())
				{
					kdError() << "Invalid --colour parameter\n";
					exitCode = 1;
				}
			}

			if (args->isSet("time"))
			{
				QCString dateTime = args->getOption("time");
				if (!convWakeTime(dateTime, wakeup))
				{
					kdError() << "Invalid --time parameter\n";
					exitCode = 1;
				}
				alarmTime = &wakeup;
			}

			if (args->isSet("repeat"))
			{
				// Repeat count is specified
				if (!args->isSet("interval"))
					args->usage(i18n("--repeat requires --interval"));   // exits program
				bool ok;
				repeatCount = args->getOption("repeat").toInt(&ok);
				if (!ok || repeatCount < 0)
				{
					kdError() << "Invalid --repeat parameter\n";
					exitCode = 1;
				}
				repeatInterval = args->getOption("interval").toInt(&ok);
				if (!ok || repeatInterval < 0)
				{
					kdError() << "Invalid --interval parameter\n";
					exitCode = 1;
				}
			}
			else if (args->isSet("interval"))
				args->usage(i18n("--interval requires --repeat"));      // exits program

			if (args->isSet("beep"))
				flags |= MessageEvent::BEEP;
			if (args->isSet("late-cancel"))
				flags |= MessageEvent::LATE_CANCEL;
			args->clear();               // free up memory

			if (!exitCode)
			{
				// Display or schedule the message
				if (!scheduleMessage(alMessage, alarmTime, bgColour, flags, file, repeatCount, repeatInterval))
					exitCode = 1;
			}
		}
		else
		{
			QString invalidOptions;
			if (args->isSet("beep"))
				invalidOptions += "--beep ";
			if (args->isSet("colour"))
				invalidOptions += "--colour ";
			if (args->isSet("late-cancel"))
				invalidOptions += "--late-cancel ";
			if (args->isSet("time"))
				invalidOptions += "--time ";
			if (invalidOptions.length())
				args->usage(invalidOptions + i18n(": option(s) only valid with a message"));    // exits program

			args->clear();               // free up memory
			if (!initCheck())
				exitCode = 1;
			else
			{
				// No arguments - run interactively & display the dialogue
				KAlarmMainWindow* mainWindow = new KAlarmMainWindow(PROGRAM_TITLE);
				mainWindowList.push_back(mainWindow);
				mainWindow->show();
			}
		}
	}

	return exitCode;
}

/******************************************************************************
* Called when a main window is closed to remove it from the main
* window list.
*/
void KAlarmApp::deleteWindow(KAlarmMainWindow* win)
{
	for (vector<KAlarmMainWindow*>::iterator it = mainWindowList.begin();  it != mainWindowList.end();  ++it)
		if (*it == win)
		{
			mainWindowList.erase(it);
			break;
		}
}

/******************************************************************************
* Called in response to a DCOP notification by the alarm daemon that a
* new message should be scheduled.
* Reply = true unless there was an error opening calendar file.
*/
bool KAlarmApp::scheduleMessage(const QString& message, const QDateTime* dateTime, const QColor& bg,
                                int flags, bool file, int repeatCount, int repeatInterval)
{
	kdDebug() << "KAlarmApp::scheduleMessage(): " << message << endl;
	bool display = true;
	QDateTime alarmTime;
	if (dateTime)
	{
		alarmTime = *dateTime;
		QDateTime now = QDateTime::currentDateTime();
		if ((flags & MessageEvent::LATE_CANCEL)  &&  *dateTime < now.addSecs(-65))
			return true;               // alarm time was already expired a minute ago
		display = (alarmTime <= now);
	}
	MessageEvent* event = new MessageEvent(alarmTime, flags, bg, message, file);
	event->setRepetition(repeatInterval, repeatCount);
	if (display)
	{
		// Alarm is due for display already
		kdDebug() << "Displaying message: " << message << "\n";
		(new MessageWin(*event, false))->show();
		delete event;
		return true;
	}
	if (!initCheck())
		return false;
	addMessage(event, 0L);        // event instance will now belong to the calendar
	return true;
}

/******************************************************************************
* Called in response to a DCOP notification by the alarm daemon that a message
* should be handled, i.e. displayed or cancelled.
* Optionally display the event. Delete the event from the calendar file and
* from every main window instance.
*/
void KAlarmApp::handleMessage(const QString& urlString, const QString& eventID, EventFunc function)
{
	kdDebug() << "KAlarmApp::handleMessage(): " << eventID << endl;
	if (KURL(urlString).url() != calendar.urlString())
		kdError() << "KAlarmApp::handleMessage(): wrong calendar file " << urlString << endl;
	else
		handleMessage(eventID, function);
}

/******************************************************************************
* Either:
* a) Display the event and then delete it if it has no outstanding repetitions.
* b) Delete the event.
* c) Reschedule the event for its next repetition. If none remain, delete it.
* If the event is deleted, it is removed from the calendar file and from every
* main window instance.
*/
bool KAlarmApp::handleMessage(const QString& eventID, EventFunc function)
{
	MessageEvent* event = calendar.getEvent(eventID);
	if (!event)
	{
		kdError() << "KAlarmApp::handleMessage(): event ID not found: " << eventID << endl;
		return false;
	}
	if (function == EVENT_HANDLE)
	{
		function = EVENT_DISPLAY;
		if (event->lateCancel())
		{
			// Alarm is to be cancelled if late.
			// Allow it to be just over a minute late before cancelling it.
			QDateTime now = QDateTime::currentDateTime();
			int secs = event->dateTime().secsTo(now);
			if (secs > 65)
			{
				// It's over a minute late.
				// Find the latest repetition time before the current time
				if (event->lastDateTime().secsTo(now) > 65)
					function = EVENT_CANCEL;      // all repetitions have expired
				else if (secs % (event->repeatMinutes() * 60) > 65)
					function = EVENT_RESCHEDULE;  // the latest repetition was over a minute ago
			}
		}
	}
	switch (function)
	{
		case EVENT_DISPLAY:
			(new MessageWin(*event, true))->show();
			break;
		case EVENT_RESCHEDULE:
			rescheduleMessage(event);
			break;
		case EVENT_CANCEL:
			deleteMessage(event, 0L, false);
			break;
		case EVENT_HANDLE:     // filtered out above
			break;
	}
	return true;
}

/******************************************************************************
* Add a new alarm message.
* Save it in the calendar file and add it to every main window instance.
* Parameters:
*    win  = initiating main window instance (which has already been updated)
*/
void KAlarmApp::addMessage(const MessageEvent* event, KAlarmMainWindow* win)
{
	kdDebug() << "KAlarmApp::addMessage(): " << event->message() << endl;

	// Update the window lists
	for (vector<KAlarmMainWindow*>::iterator it = mainWindowList.begin();  it != mainWindowList.end();  ++it)
		if (*it != win)
			(*it)->addMessage(event);

	// Save the message details in the calendar file.
	calendar.addEvent(event);      // the event instance now belongs to the calendar
	calendar.save();

	// Tell the daemon to reread the calendar file
	reloadDaemon();
}

/******************************************************************************
* Modify a message in every main window instance.
* Parameters:
*    win  = initiating main window instance (which has already been updated)
*/
void KAlarmApp::modifyMessage(MessageEvent* oldEvent, const MessageEvent* newEvent, KAlarmMainWindow* win)
{
	kdDebug() << "KAlarmApp::modifyMessage(): '" << oldEvent->message() << "' to: '" << newEvent->message() << "'" << endl;

	// Update the window lists
	for (vector<KAlarmMainWindow*>::iterator it = mainWindowList.begin();  it != mainWindowList.end();  ++it)
		if (*it != win)
			(*it)->modifyMessage(oldEvent, newEvent);

	// Update the event in the calendar file
	calendar.deleteEvent(oldEvent);
	calendar.addEvent(newEvent);      // the event instance now belongs to the calendar
	calendar.save();

	// Tell the daemon to reread the calendar file
	reloadDaemon();
}

/******************************************************************************
* Delete a message from every main window instance.
* Parameters:
*    win  = initiating main window instance (which has already been updated)
*/
void KAlarmApp::deleteMessage(MessageEvent* event, KAlarmMainWindow* win, bool tellDaemon)
{
	kdDebug() << "KAlarmApp::deleteMessage(): " << event->message() << endl;

	// Update the window lists
	for (vector<KAlarmMainWindow*>::iterator it = mainWindowList.begin();  it != mainWindowList.end();  ++it)
		if (*it != win)
			(*it)->deleteMessage(event);

	// Delete the event from the calendar file
	calendar.deleteEvent(event);
	calendar.save();

	// Tell the daemon to reread the calendar file
	if (tellDaemon)
		reloadDaemon();
}

/******************************************************************************
* Reschedule the specified event for its next repetition. If no repetitions
* remain, cancel it.
* Rescheduling is necessary in order to distinguish between alarms which have
* been displayed at their last repetition, and those which haven't.
* Reply = true if event still exists, false if cancelled.
*/
bool KAlarmApp::rescheduleMessage(MessageEvent* event)
{
	int secs = event->dateTime().secsTo(QDateTime::currentDateTime());
	if (secs >= 0)
	{
		// The event is due by now
		int repeatSecs = event->repeatMinutes() * 60;
		int n = secs / repeatSecs + 1;
		int remainingCount = event->repeatCount() - n;
		if (remainingCount >= 0)
		{
			// Repetitions still remain, so rewrite the event
//			MessageEvent* newEvent = new MessageEvent(event->dateTime().addSecs(n * repeatSecs), event->flags(), event->colour(), event->message());
//			newEvent->setRepetition(event->repeatMinutes(), event->initialRepeatCount(), remainingCount);
//			modifyMessage(event, newEvent, 0L);
// Should just call calendar update, but it doesn't seem to work
			event->updateRepetition(event->dateTime().addSecs(n * repeatSecs), remainingCount);

			// Update the window lists
			for (vector<KAlarmMainWindow*>::iterator it = mainWindowList.begin();  it != mainWindowList.end();  ++it)
				(*it)->modifyMessage(event);

			// Update the event in the calendar file
			calendar.updateEvent(event);
			calendar.save();

			// Tell the daemon to reread the calendar file
			reloadDaemon();
		}
		else
		{
			deleteMessage(event, 0L, false);
			return false;
		}
	}
	return true;
}

/******************************************************************************
* If this is the first time through, open the calendar file and optionally
* start the alarm daemon.
*/
bool KAlarmApp::initCheck(bool daemon)
{
	if (!calendar.isOpen())
	{
		kdDebug() << "KAlarmApp::initCheck(): initialising" << endl;

		// First time through. Open the calendar file.
		if (!calendar.open())
			return false;

		if (daemon)
		{
			// Make sure the alarm daemon is running
			startDaemon();
		}
	}
	else if (!daemonRegistered)
		startDaemon();
	return true;
}

/******************************************************************************
* Start the alarm daemon if necessary, and register this application with it.
*/
void KAlarmApp::startDaemon()
{
	kdDebug() << "KAlarmApp::startDaemon()" << endl;
	calendar.getURL();    // check that the calendar file name is OK - program exit if not
	if (!dcopClient()->isApplicationRegistered("kalarmd"))
	{
		// Start the alarm daemon. It is a KUniqueApplication, which means that
		// there is automatically only one instance of the alarm daemon running.
		QString execStr = locate("exe","kalarmd");
		system(execStr.latin1());
		kdDebug() << "Alarm daemon started" << endl;
	}

	// Register this application with the alarm daemon
	{
		QByteArray data;
		QDataStream arg(data, IO_WriteOnly);
		arg << QString(PROGRAM_NAME) << QString(PROGRAM_TITLE) << DCOP_OBJECT_NAME << (Q_INT8)1 << (Q_INT8)0;
		if (!dcopClient()->send("kalarmd","ad","registerApp(QString,QString,QString,bool,bool)", data))
			kdDebug() << "KAlarmApp::startDaemon(): registerApp dcop send failed" << endl;
	}

	// Tell alarm daemon to load the calendar
	{
		QByteArray data;
		QDataStream arg(data, IO_WriteOnly);
		arg << QString(PROGRAM_NAME) << calendar.urlString();
		if (!dcopClient()->send("kalarmd","ad","addMsgCal(QString,QString)", data))
			kdDebug() << "KAlarmApp::startDaemon(): addCal dcop send failed" << endl;
	}

	daemonRegistered = true;
	kdDebug() << "KAlarmApp::startDaemon(): started daemon" << endl;
}

/******************************************************************************
* Stop the alarm daemon if it is running.
*/
bool KAlarmApp::stopDaemon()
{
	kdDebug() << "KAlarmApp::stopDaemon()" << endl;
	if (dcopClient()->isApplicationRegistered("kalarmd"))
	{
		QByteArray data;
		if (!dcopClient()->send("kalarmd","ad","quit()", data))
		{
			kdError() << "KAlarmApp::restartDaemon(): quit dcop send failed" << endl;
			return false;
		}
	}
	return true;
}

/******************************************************************************
* Reset the alarm daemon. If it is not already running, start it.
*/
void KAlarmApp::resetDaemon()
{
	kdDebug() << "KAlarmApp::resetDaemon()" << endl;
	if (!dcopClient()->isApplicationRegistered("kalarmd"))
		startDaemon();
	else
	{
		QByteArray data;
		QDataStream arg(data, IO_WriteOnly);
		arg << QString(PROGRAM_NAME) << calendar.urlString();
		if (!dcopClient()->send("kalarmd","ad","resetMsgCal(QString,QString)", data))
			kdDebug() << "KAlarmApp::resetDaemon(): addCal dcop send failed" << endl;
	}
}

/******************************************************************************
* Tell the alarm daemon to reread the calendar file.
*/
void KAlarmApp::reloadDaemon()
{
	QByteArray data;
	QDataStream arg(data, IO_WriteOnly);
	arg << QString(PROGRAM_NAME) << calendar.urlString();
	if (!kapp->dcopClient()->send("kalarmd","ad","reloadMsgCal(QString,QString)", data))
		kdDebug() << "KAlarmApp::reloadDaemon(): dcop send failed" << endl;
}

/******************************************************************************
*  Read the size for the specified window from the config file, for the
*  current screen resolution.
*  Reply = window size.
*/
QSize KAlarmApp::readConfigWindowSize(const char* window, const QSize& defaultSize)
{
	KConfig* config = KGlobal::config();
	config->setGroup(QString::fromLatin1(window));
	QWidget* desktop = KApplication::desktop();
	return QSize(config->readNumEntry(QString::fromLatin1("Width %1").arg(desktop->width()), defaultSize.width()),
	             config->readNumEntry(QString::fromLatin1("Height %1").arg(desktop->height()), defaultSize.height()));
}

/******************************************************************************
*  Write the size for the specified window to the config file, for the
*  current screen resolution.
*/
void KAlarmApp::writeConfigWindowSize(const char* window, const QSize& size)
{
	KConfig* config = KGlobal::config();
	config->setGroup(QString::fromLatin1(window));
#warning "Should the next line be here? - for session resoration???"
	config->writeEntry("Size", size);
	QWidget* desktop = KApplication::desktop();
	config->writeEntry(QString::fromLatin1("Width %1").arg(desktop->width()), size.width());
	config->writeEntry(QString::fromLatin1("Height %1").arg(desktop->height()), size.height());
}

/******************************************************************************
*  Convert the --time parameter string into a date/time value.
*  The parameter is in the form [[[yyyy-]mm-]dd-]hh:mm
*  Reply = true if successful.
*/
bool KAlarmApp::convWakeTime(const QCString timeParam, QDateTime& dateTime)
{
	if (timeParam.length() > 19)
		return false;
	char timeStr[20];
	strcpy(timeStr, timeParam);
	int dt[5] = { -1, -1, -1, -1, -1 };
	char* s;
	char* end;
	// Get the minute value
	if ((s = strchr(timeStr, ':')) == 0L)
		return false;
	*s++ = 0;
	dt[4] = strtoul(s, &end, 10);
	if (end == s  ||  *end  ||  dt[4] >= 60)
		return false;
	// Get the hour value
	if ((s = strrchr(timeStr, '-')) == 0L)
		s = timeStr;
	else
		*s++ = 0;
	dt[3] = strtoul(s, &end, 10);
	if (end == s  ||  *end  ||  dt[3] >= 24)
		return false;

	bool dateSet = false;
	if (s != timeStr)
	{
		dateSet = true;
		// Get the day value
		if ((s = strrchr(timeStr, '-')) == 0L)
			s = timeStr;
		else
			*s++ = 0;
		dt[2] = strtoul(s, &end, 10);
		if (end == s  ||  *end  ||  dt[2] == 0  ||  dt[2] > 31)
			return false;
		if (s != timeStr)
		{
			// Get the month value
			if ((s = strrchr(timeStr, '-')) == 0L)
				s = timeStr;
			else
				*s++ = 0;
			dt[1] = strtoul(s, &end, 10);
			if (end == s  ||  *end  ||  dt[1] == 0  ||  dt[1] > 12)
				return false;
			if (s != timeStr)
			{
				// Get the year value
				dt[0] = strtoul(timeStr, &end, 10);
				if (end == timeStr  ||  *end)
					return false;
			}
		}
	}

	// Compile the values into a date/time structure
	QDateTime now = QDateTime::currentDateTime();
	QDate date((dt[0] < 0 ? now.date().year() : dt[0]),
	           (dt[1] < 0 ? now.date().month() : dt[1]),
	           (dt[2] < 0 ? now.date().day() : dt[2]));
	if (!date.isValid())
		return false;
	QTime time(dt[3], dt[4], 0);
	if (!dateSet  &&  time < now.time())
		date = date.addDays(1);
	dateTime.setDate(date);
	dateTime.setTime(time);
	return true;
}


/******************************************************************************
* Read the calendar file URL from the config file (or use the default).
* If there is an error, the program exits.
*/
void AlarmCalendar::getURL() const
{
	if (!url.isValid())
	{
		KConfig* config = kapp->config();
		config->setGroup("General");
		*const_cast<KURL*>(&url) = config->readEntry("Calendar", locateLocal("appdata", DEFAULT_CALENDAR_FILE));
		if (!url.isValid())
		{
			kdDebug() << "AlarmCalendar::getURL(): invalid name: " << url.prettyURL() << endl;
			KMessageBox::error(0L, i18n("Invalid calendar file name: %1").arg(url.prettyURL()));
			kapp->exit(1);
		}
	}
}

/******************************************************************************
* Open the calendar file and load it into memory.
*/
bool AlarmCalendar::open()
{
	getURL();
	calendar = new AlarmCalendarLocal;
	calendar->showDialogs(FALSE);

	// Find out whether the calendar is ICal or VCal format
	QString ext = url.filename().right(4);
	vCal = (ext == ".vcs");

	if (!KIO::NetAccess::exists(url))
	{
		// Create the calendar file
		KTempFile* tmpFile = 0L;
		QString filename;
		if (url.isLocalFile())
			filename = url.path();
		else
		{
			tmpFile = new KTempFile;
			filename = tmpFile->name();
		}
		if (!save(filename))
		{
			delete tmpFile;
			return false;
		}
		delete tmpFile;
	}

	// Load the calendar file
	return load();
}

/******************************************************************************
* Load the calendar file into memory.
*/
bool AlarmCalendar::load()
{
	getURL();
	kdDebug() << "AlarmCalendar::load(): " << url.prettyURL() << endl;
	QString tmpFile;
	if (!KIO::NetAccess::download(url, tmpFile))
	{
		kdDebug() << "Load failure" << endl;
		KMessageBox::error(0L, i18n("Cannot open calendar %1").arg(url.prettyURL()));
		return false;
	}
	kdDebug() << "--- Downloaded to " << tmpFile << endl;
	if (!calendar->load(tmpFile))
	{
		KIO::NetAccess::removeTempFile(tmpFile);
		kdDebug() << "Error loading calendar file '" << tmpFile << "'" << endl;
		KMessageBox::error(0L, i18n("Error loading calendar %1").arg(url.prettyURL()));
		return false;
	}
	if (!localFile.isEmpty())
		KIO::NetAccess::removeTempFile(localFile);
	localFile = tmpFile;
	return true;
}

/******************************************************************************
* Save the calendar from memory to file.
*/
bool AlarmCalendar::save(const QString& filename)
{
	kdDebug() << "AlarmCalendar::save(): " << filename << endl;
	CalFormat* format = (vCal ? static_cast<CalFormat*>(new VCalFormat(calendar)) : static_cast<CalFormat*>(new ICalFormat(calendar)));
	bool success = calendar->save(filename, format);
	delete format;
	if (!success)
	{
		kdDebug() << "AlarmCalendar::save(): calendar save failed." << endl;
		return false;
	}

	getURL();
	if (!url.isLocalFile())
	{
		if (!KIO::NetAccess::upload(filename, url))
		{
			KMessageBox::error(0L, i18n("Cannot upload calendar to '%1'").arg(url.prettyURL()));
			return false;
		}
	}

	// Tell the alarm daemon to reload the calendar
	QByteArray data;
	QDataStream arg(data, IO_WriteOnly);
	arg << QString(PROGRAM_NAME) << url.url();
	if (!kapp->dcopClient()->send("kalarmd","ad","reloadMsgCal(QString,QString)", data))
		kdDebug() << "AlarmCalendar::save(): addCal dcop send failed" << endl;
	return true;
}

/******************************************************************************
* Delete any temporary file at program exit.
*/
void AlarmCalendar::close()
{
	if (!localFile.isEmpty())
		KIO::NetAccess::removeTempFile(localFile);
}


/******************************************************************************
* This class's function is simply to act as a receiver for DCOP requests.
*/
MainWidget::MainWidget(const char* dcopObject)
	: QWidget(),
	  DCOPObject(dcopObject)
{
}

/******************************************************************************
* Process a DCOP request.
*/
bool MainWidget::process(const QCString& func, const QByteArray& data, QCString& replyType, QByteArray&)
{
	kdDebug() << "MainWidget::process(): " << func << endl;
	enum { ERR, HANDLE, CANCEL, DISPLAY, SCHEDULE, SCHEDULE_n, SCHEDULE_FILE, SCHEDULE_FILE_n };
	int function;
	if (func == "handleEvent(const QString&,const QString&)")
		function = HANDLE;
	else if (func == "cancelMessage(const QString&,const QString&)")
		function = CANCEL;
	else if (func == "displayMessage(const QString&,const QString&)")
		function = DISPLAY;
	else if (func == "scheduleMessage(const QString&,const QDateTime&,QColor,Q_UINT32)")
		function = SCHEDULE;
	else if (func == "scheduleMessage(const QString&,const QDateTime&,QColor,Q_UINT32,Q_INT32,Q_INT32)")
		function = SCHEDULE_n;
	else if (func == "scheduleFile(const QString&,const QDateTime&,QColor,Q_UINT32)")
		function = SCHEDULE_FILE;
	else if (func == "scheduleFile(const QString&,const QDateTime&,QColor,Q_UINT32,Q_INT32,Q_INT32)")
		function = SCHEDULE_FILE_n;
	else
	{
		kdDebug() << "MainWidget::process(): unknown DCOP function" << endl;
		return false;
	}

	bool file = false;
	switch (function)
	{
		case HANDLE:        // display or cancel message with specified ID from calendar file
		case CANCEL:        // cancel message with specified ID from calendar file
		case DISPLAY:       // display message with specified ID in calendar file
		{

			QDataStream arg(data, IO_ReadOnly);
			QString urlString, vuid;
			arg >> urlString >> vuid;
			replyType = "void";
			switch (function)
			{
				case HANDLE:
					theApp()->handleMessage(urlString, vuid);
					break;
				case CANCEL:
					theApp()->deleteMessage(urlString, vuid);
					break;
				case DISPLAY:
					theApp()->displayMessage(urlString, vuid);
					break;
			}
			break;
		}
		case SCHEDULE_FILE:    // schedule the display of a file's contents
		case SCHEDULE_FILE_n:  // schedule the repeating display of a file's contents
			file = true;
			// fall through to 'SCHEDULE'
		case SCHEDULE:         // schedule a new message
		case SCHEDULE_n:       // schedule a new repeating message
		{
			QDataStream arg(data, IO_ReadOnly);
			QString message;
			QDateTime dateTime;
			QColor bgColour;
			Q_UINT32 flags;
			Q_INT32 repeatCount = 0;
			Q_INT32 repeatInterval = 0;
			arg >> message;
			arg.readRawBytes((char*)&dateTime, sizeof(dateTime));
			arg.readRawBytes((char*)&bgColour, sizeof(bgColour));
			arg >> flags;
			if (function == SCHEDULE_n  ||  function == SCHEDULE_FILE_n)
				arg >> repeatCount >> repeatInterval;
			theApp()->scheduleMessage(message, &dateTime, bgColour, flags, file, repeatCount, repeatInterval);
			replyType = "void";
			break;
		}
	}
	return true;
}
