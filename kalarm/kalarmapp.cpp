/*
 *  kalarmapp.cpp  -  the KAlarm application object
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

#include "kalarm.h"

#include <stdlib.h>
#include <ctype.h>
#include <iostream>

#include <qfile.h>

#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kaboutdata.h>
#include <dcopclient.h>
#include <kprocess.h>
#include <kfileitem.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kwinmodule.h>
#include <kdebug.h>

#include <kalarmd/clientinfo.h>

#include "alarmcalendar.h"
#include "mainwindow.h"
#include "messagewin.h"
#include "daemongui.h"
#include "traywindow.h"
#include "prefsettings.h"
#include "prefdlg.h"
#include "kalarmapp.moc"

extern QCString execArguments;

#define     DAEMON_APP_NAME_DEF       "kalarmd"
const char* DCOP_OBJECT_NAME        = "display";
const char* GUI_DCOP_OBJECT_NAME    = "tray";
const char* DAEMON_APP_NAME         = DAEMON_APP_NAME_DEF;
const char* DAEMON_DCOP_OBJECT      = "ad";

KAlarmApp*  KAlarmApp::theInstance = 0L;
int         KAlarmApp::activeCount = 0;


/******************************************************************************
* Construct the application.
*/
KAlarmApp::KAlarmApp()
	: KUniqueApplication(),
	  mDcopHandler(0L),
	  mDaemonGuiHandler(0L),
	  mTrayWindow(0L),
	  mCalendar(new AlarmCalendar),
	  mSettings(new Settings(0L)),
	  mDaemonCheckInterval(0),
	  mDaemonRegistered(false),
	  mDaemonRunning(false),
	  mSessionClosingDown(false)
{
	mSettings->loadSettings();
	connect(mSettings, SIGNAL(settingsChanged()), this, SLOT(slotSettingsChanged()));
	CalFormat::setApplication(aboutData()->programName(),
	                          QString::fromLatin1("-//K Desktop Environment//NONSGML %1 " VERSION "//EN")
	                                       .arg(aboutData()->programName()));
	readDaemonCheckInterval();
	KWinModule wm;
	mKDEDesktop             = wm.systemTrayWindows().count();
	mOldRunInSystemTray     = mKDEDesktop && mSettings->runInSystemTray();
	mDisableAlarmsIfStopped = mOldRunInSystemTray && mSettings->disableAlarmsIfStopped();

	// Set up actions used by more than one menu
	KActionCollection* actions = new KActionCollection(this);
	mActionAlarmEnable = new ActionAlarmsEnabled(Qt::CTRL+Qt::Key_E, this, SLOT(toggleAlarmsEnabled()), this);
	mActionPrefs       = KStdAction::preferences(this, SLOT(slotPreferences()), actions);
	mActionDaemonPrefs = new KAction(i18n("Configure Alarm &Daemon..."), mActionPrefs->iconSet(),
	                                 0, this, SLOT(slotDaemonPreferences()), this);
}

/******************************************************************************
*/
KAlarmApp::~KAlarmApp()
{
	mCalendar->close();
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
	kdDebug(5950)<<"KAlarmApp::newInstance(): New instance\n";
	++activeCount;
	static bool restored = false;
	int exitCode = 0;               // default = success
	QString usage;
	if (!restored  &&  isRestored())
	{
		// Process is being restored by session management.
		kdDebug(5950)<<"KAlarmApp::newInstance(): Restoring session\n";
		exitCode = !initCheck(true);     // open the calendar file (needed for main windows)
		KAlarmMainWindow* trayParent = 0L;
		for (int i = 1;  KMainWindow::canBeRestored(i);  ++i)
		{
			if (KMainWindow::classNameOfToplevel(i) == QString::fromLatin1("KAlarmMainWindow"))
			{
				KAlarmMainWindow* win = new KAlarmMainWindow;
				win->restore(i, false);
				if (win->hiddenTrayParent())
					trayParent = win;
				else
					win->show();
			}
			else if (KMainWindow::classNameOfToplevel(i) == QString::fromLatin1("MessageWin"))
			{
				MessageWin* win = new MessageWin;
				win->restore(i, false);
				if (win->errorMessage())
					delete win;
				else
					win->show();
			}
		}
		initCheck();           // register with the alarm daemon
		restored = true;       // make sure we restore only once

		// Display the system tray icon if it is configured to be autostarted,
		// or if we're in run-in-system-tray mode.
		if (settings()->autostartTrayIcon()
		||  KAlarmMainWindow::count()  &&  runInSystemTray())
			displayTrayIcon(true, trayParent);
	}
	else
	{
		setUpDcop();     // we're now ready to handle DCOP calls, so set up handlers
		KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

		// Use a 'do' loop which is executed only once to allow easy error exits.
		// Errors use 'break' to skip to the end of the function.
		do
		{
			if (args->isSet("stop"))
			{
				// Stop the alarm daemon
				args->clear();      // free up memory
				if (!stopDaemon())
				{
					exitCode = 1;
					break;
				}
			}
			else
			if (args->isSet("reset"))
			{
				// Reset the alarm daemon
				args->clear();      // free up memory
				resetDaemon();
			}
			else
			if (args->isSet("tray"))
			{
				// Display only the system tray icon
				kdDebug(5950)<<"KAlarmApp::newInstance(): tray\n";
				args->clear();      // free up memory
				if (!mKDEDesktop
				||  !initCheck())    // open the calendar, register with daemon
				{
					exitCode = 1;
					break;
				}
				if (runInSystemTray()  &&  !KAlarmMainWindow::count())
					new KAlarmMainWindow;
				if (!displayTrayIcon(true))
				{
					exitCode = 1;
					break;
				}
			}
			else
			if (args->isSet("handleEvent")  ||  args->isSet("triggerEvent")  ||  args->isSet("cancelEvent")  ||  args->isSet("calendarURL")
/* superseded */	||  args->isSet("displayEvent"))  // kept for compatibility
			{
				// Display or delete the event with the specified event ID
				kdDebug(5950)<<"KAlarmApp::newInstance(): handle event\n";
				EventFunc function = EVENT_HANDLE;
				int count = 0;
				const char* option = 0;
				if (args->isSet("handleEvent"))   { function = EVENT_HANDLE;  option = "handleEvent";  ++count; }
/* superseded */		if (args->isSet("displayEvent"))  { function = EVENT_TRIGGER;  option = "displayEvent";  ++count; }
				if (args->isSet("triggerEvent"))  { function = EVENT_TRIGGER;  option = "triggerEvent";  ++count; }
				if (args->isSet("cancelEvent"))   { function = EVENT_CANCEL;  option = "cancelEvent";  ++count; }
				if (!count)
				{
					usage = i18n("--calendarURL requires --handleEvent, --triggerEvent or --cancelEvent");
					break;
				}
				if (count > 1)
				{
					usage = i18n("--handleEvent, --triggerEvent, --cancelEvent mutually exclusive");
					break;
				}
				if (!initCheck())
				{
					exitCode = 1;
					break;
				}
				if (args->isSet("calendarURL"))
				{
					QString calendarUrl = args->getOption("calendarURL");
					if (KURL(calendarUrl).url() != mCalendar->urlString())
					{
						usage = i18n("--calendarURL: wrong calendar file");
						break;
					}
				}
				QString eventID = args->getOption(option);
				args->clear();          // free up memory
				if (!handleEvent(eventID, function))
				{
					exitCode = 1;
					break;
				}
			}
			else
			if (args->isSet("file")  ||  args->isSet("exec")  ||  args->count())
			{
				// Display a message or file, or execute a command
				KAlarmAlarm::Type type = KAlarmAlarm::MESSAGE;
				QCString alMessage;
				if (args->isSet("file"))
				{
					kdDebug(5950)<<"KAlarmApp::newInstance(): file\n";
					if (args->isSet("exec"))
					{
						usage = i18n("--exec incompatible with --file");
						break;
					}
					if (args->count())
					{
						usage = i18n("message incompatible with --file");
						break;
					}
					alMessage = args->getOption("file");
					type = KAlarmAlarm::FILE;
				}
				else if (args->isSet("exec"))
				{
					kdDebug(5950)<<"KAlarmApp::newInstance(): exec\n";
					alMessage = execArguments;
					type = KAlarmAlarm::COMMAND;
				}
				else
				{
					kdDebug(5950)<<"KAlarmApp::newInstance(): message\n";
					alMessage = args->arg(0);
				}

				QDateTime* alarmTime = 0L;
				QDateTime wakeup;
				QColor bgColour = settings()->defaultBgColour();
				int    repeatCount = 0;
				int    repeatInterval = 0;
				if (args->isSet("color"))
				{
					// Colour is specified
					QCString colourText = args->getOption("color");
					if (static_cast<const char*>(colourText)[0] == '0'
					&&  tolower(static_cast<const char*>(colourText)[1]) == 'x')
						colourText.replace(0, 2, "#");
					bgColour.setNamedColor(colourText);
					if (!bgColour.isValid())
					{
						usage = i18n("Invalid --color parameter");
						break;
					}
				}

				if (args->isSet("time"))
				{
					QCString dateTime = args->getOption("time");
					if (!convWakeTime(dateTime, wakeup))
					{
						usage = i18n("Invalid --time parameter");
						break;
					}
					alarmTime = &wakeup;
				}

				if (args->isSet("repeat"))
				{
					// Repeat count is specified
					if (!args->isSet("interval"))
					{
						usage = i18n("--repeat requires --interval");
						break;
					}
					bool ok;
					repeatCount = args->getOption("repeat").toInt(&ok);
					if (!ok || repeatCount < 0)
					{
						usage = i18n("Invalid --repeat parameter");
						break;
					}
					repeatInterval = args->getOption("interval").toInt(&ok);
					if (!ok || repeatInterval < 0)
					{
						usage = i18n("Invalid --interval parameter");
						break;
					}
				}
				else if (args->isSet("interval"))
				{
					usage = i18n("--interval requires --repeat");
					break;
				}

				int flags = 0;
				if (args->isSet("beep"))
					flags |= KAlarmEvent::BEEP;
				if (args->isSet("late-cancel"))
					flags |= KAlarmEvent::LATE_CANCEL;
				if (args->isSet("login"))
					flags |= KAlarmEvent::REPEAT_AT_LOGIN;
				args->clear();               // free up memory

				// Display or schedule the event
				if (!scheduleEvent(alMessage, alarmTime, bgColour, flags, type, repeatCount, repeatInterval))
				{
					exitCode = 1;
					break;
				}
			}
			else
			{
				if (args->isSet("beep"))
					usage += QString::fromLatin1("--beep ");
				if (args->isSet("color"))
					usage += QString::fromLatin1("--color ");
				if (args->isSet("late-cancel"))
					usage += QString::fromLatin1("--late-cancel ");
				if (args->isSet("login"))
					usage += QString::fromLatin1("--login ");
				if (args->isSet("time"))
					usage += QString::fromLatin1("--time ");
				if (!usage.isEmpty())
				{
					usage += i18n(": option(s) only valid with a message/--file/--exec");
					break;
				}

				args->clear();               // free up memory
				if (!initCheck())
				{
					exitCode = 1;
					break;
				}

				// No arguments - run interactively & display the dialogue
				(new KAlarmMainWindow)->show();
			}
		} while (0);    // only execute once
	}

	if (!usage.isEmpty())
	{
		// Note: we can't use args->usage() since that also quits any other
		// running 'instances' of the program.
		std::cerr << usage << i18n("\nUse --help to get a list of available command line options.\n");
		exitCode = 1;
	}
	--activeCount;

	// Quit the application if this was the last/only running "instance" of the program.
	// Executing 'return' doesn't work very well since the program continues to
	// run if no windows were created.
	quitIf(exitCode);
	return exitCode;
}

/******************************************************************************
* Quit the program if there are no more "instances" running.
*/
void KAlarmApp::quitIf(int exitCode)
{
	if (activeCount <= 0  &&  !KAlarmMainWindow::count()  &&  !MessageWin::instanceCount()  &&  !mTrayWindow)
	{
		// This was the last/only running "instance" of the program, so exit completely.
		exit(exitCode);
	}
}

/******************************************************************************
* Called when the session manager is about to close down the application.
*/
void KAlarmApp::commitData(QSessionManager& sm)
{
	mSessionClosingDown = true;
	KUniqueApplication::commitData(sm);
	mSessionClosingDown = false;         // reset in case shutdown is cancelled
}

/******************************************************************************
* Called when the system tray main window is closed.
*/
void KAlarmApp::removeWindow(TrayWindow*)
{
	mTrayWindow = 0L;
	quitIf();
}

/******************************************************************************
*  Display or close the system tray icon.
*/
bool KAlarmApp::displayTrayIcon(bool show, KAlarmMainWindow* parent)
{
	if (show)
	{
		if (!mTrayWindow)
		{
			if (!mKDEDesktop)
				return false;
			mTrayWindow = new TrayWindow(parent ? parent : KAlarmMainWindow::firstWindow());
			connect(mTrayWindow, SIGNAL(deleted()), this, SIGNAL(trayIconToggled()));
			mTrayWindow->show();
			emit trayIconToggled();
		}
	}
	else if (mTrayWindow)
		delete mTrayWindow;
	return true;
}

/******************************************************************************
*  Display a main window.
*/
void KAlarmApp::displayMainWindow()
{
	KAlarmMainWindow* win = KAlarmMainWindow::firstWindow();
	if (!win)
	{
		if (initCheck())
			(new KAlarmMainWindow)->show();
	}
	else
	{
		// There is already a main window, so make it the active window
		win->showNormal();
		win->raise();
		win->setActiveWindow();
	}
}

KAlarmMainWindow* KAlarmApp::trayMainWindow() const
{
	return mTrayWindow ? mTrayWindow->assocMainWindow() : 0L;
}

/******************************************************************************
* Called when the Alarms Enabled action is selected.
* The alarm daemon is told to stop or start monitoring the calendar file as
* appropriate.
*/
void KAlarmApp::toggleAlarmsEnabled()
{
	if (mDaemonGuiHandler)
		mDaemonGuiHandler->setAlarmsEnabled(!mActionAlarmEnable->alarmsEnabled());
}

/******************************************************************************
*  Called when a Preferences menu item is selected.
*/
void KAlarmApp::slotPreferences()
{
	(new KAlarmPrefDlg(settings()))->exec();
}

/******************************************************************************
* Called when a Configure Daemon menu item is selected.
* Displays the alarm daemon configuration dialog.
*/
void KAlarmApp::slotDaemonPreferences()
{
	KProcess proc;
	proc << locate("exe", QString::fromLatin1("kcmshell"));
	proc << QString::fromLatin1("alarmdaemonctrl");
	proc.start(KProcess::DontCare);
}

/******************************************************************************
*  Called when KAlarm settings have changed.
*/
void KAlarmApp::slotSettingsChanged()
{
	bool newRunInSysTray = settings()->runInSystemTray()  &&  mKDEDesktop;
	if (newRunInSysTray != mOldRunInSystemTray)
	{
		// The system tray run mode has changed
		++activeCount;          // prevent the application from quitting
		KAlarmMainWindow* win = mTrayWindow ? mTrayWindow->assocMainWindow() : 0L;
		delete mTrayWindow;     // remove the system tray icon if it is currently shown
		mTrayWindow = 0L;
		mOldRunInSystemTray = newRunInSysTray;
		if (newRunInSysTray)
		{
			if (!KAlarmMainWindow::count())
				new KAlarmMainWindow;
			displayTrayIcon(true);
		}
		else
		{
			if (win  &&  win->isHidden())
				delete win;
			displayTrayIcon(true);
		}
		--activeCount;
	}

	bool newDisableIfStopped = mKDEDesktop && mSettings->runInSystemTray() && mSettings->disableAlarmsIfStopped();
	if (newDisableIfStopped != mDisableAlarmsIfStopped)
	{
		registerWithDaemon();     // re-register with the alarm daemon
		mDisableAlarmsIfStopped = newDisableIfStopped;
	}
}

/******************************************************************************
*  Return whether the program is configured to be running in the system tray.
*/
bool KAlarmApp::runInSystemTray() const
{
	return mSettings->runInSystemTray()  &&  mKDEDesktop;
}

/******************************************************************************
* Called in response to a DCOP notification that a new alarm should be scheduled.
* Reply = true unless there was an error opening calendar file.
*/
bool KAlarmApp::scheduleEvent(const QString& message, const QDateTime* dateTime, const QColor& bg,
                                int flags, KAlarmAlarm::Type type, int repeatCount, int repeatInterval)
	{
	kdDebug(5950) << "KAlarmApp::scheduleEvent(): " << message << endl;
	bool display = true;
	QDateTime alarmTime;
	if (dateTime)
	{
		alarmTime = *dateTime;
		QDateTime now = QDateTime::currentDateTime();
		if ((flags & KAlarmEvent::LATE_CANCEL)  &&  *dateTime < now.addSecs(-maxLateness()))
			return true;               // alarm time was already expired too long ago
		display = (alarmTime <= now);
	}
	KAlarmEvent event(alarmTime, message, bg, type, flags, repeatCount, repeatInterval);
	if (display)
	{
		// Alarm is due for display already
		execAlarm(event, event.firstAlarm(), false);
		return true;
	}
	return addEvent(event, 0L);     // event instance will now belong to the calendar
}

/******************************************************************************
* Called in response to a DCOP notification by the alarm daemon that an event
* should be handled, i.e. displayed or cancelled.
* Optionally display the event. Delete the event from the calendar file and
* from every main window instance.
*/
void KAlarmApp::handleEvent(const QString& urlString, const QString& eventID, EventFunc function)
{
	kdDebug(5950) << "KAlarmApp::handleEvent(DCOP): " << eventID << endl;
	if (KURL(urlString).url() != mCalendar->urlString())
		kdError(5950) << "KAlarmApp::handleEvent(DCOP): wrong calendar file " << urlString << endl;
	else
		handleEvent(eventID, function);
}

/******************************************************************************
* Either:
* a) Display the event and then delete it if it has no outstanding repetitions.
* b) Delete the event.
* c) Reschedule the event for its next repetition. If none remain, delete it.
* If the event is deleted, it is removed from the calendar file and from every
* main window instance.
*/
bool KAlarmApp::handleEvent(const QString& eventID, EventFunc function)
{
	kdDebug(5950) << "KAlarmApp::handleEvent(): " << eventID << ", " << (function==EVENT_TRIGGER?"TRIGGER":function==EVENT_CANCEL?"CANCEL":function==EVENT_HANDLE?"HANDLE":"?") << endl;
	Event* kcalEvent = mCalendar->getEvent(eventID);
	if (!kcalEvent)
	{
		kdError(5950) << "KAlarmApp::handleEvent(): event ID not found: " << eventID << endl;
		return false;
	}
	KAlarmEvent event(*kcalEvent);
	AlarmFunc alfunction = ALARM_TRIGGER;
	switch (function)
	{
		case EVENT_TRIGGER:
		{
			// Only trigger one alarm from the event - we don't
			// want multiple identical messages, for example.
			KAlarmAlarm alarm = event.firstAlarm();
			if (alarm.valid())
				handleAlarm(event, alarm, ALARM_TRIGGER, true);
			break;
		}
		case EVENT_CANCEL:
			deleteEvent(event, 0L, false);
			break;

		case EVENT_HANDLE:
		{
			QDateTime now = QDateTime::currentDateTime();
			bool updateCalAndDisplay = false;
			KAlarmAlarm displayAlarm;
			// Check all the alarms in turn.
			// Note that the main alarm is fetched before any other alarms.
			for (KAlarmAlarm alarm = event.firstAlarm();  alarm.valid();  alarm = event.nextAlarm(alarm))
			{
				// Check whether this alarm is due yet
				int secs = alarm.dateTime().secsTo(now);
				if (secs < 0)
					continue;
				if (alarm.repeatAtLogin())
				{
					// Alarm is to be displayed at every login.
					// Check if the alarm has only just been set up.
					// (The alarm daemon will immediately notify that it is due
					//  since it is set up with a time in the past.)

					if (secs < maxLateness())
						continue;

					// Check if the main alarm is already being displayed.
					// (We don't want to display both at the same time.)
					if (displayAlarm.valid())
						continue;
				}
				if (alarm.lateCancel())
				{
					// Alarm is due, and it is to be cancelled if late.
					// Allow it to be just over a minute late before cancelling it.
					int maxlate = maxLateness();
					if (secs > maxlate)
					{
						// It's over the maximum interval late.
						// Find the latest repetition time before the current time
						if (alarm.lastDateTime().secsTo(now) > maxlate)
						{
							handleAlarm(event, alarm, ALARM_CANCEL, false);      // all repetitions have expired
							updateCalAndDisplay = true;
							continue;
						}
						if (alarm.repeatMinutes()  &&  secs % (alarm.repeatMinutes() * 60) > maxlate)
						{
							handleAlarm(event, alarm, ALARM_RESCHEDULE, false);  // the latest repetition was too long ago
							updateCalAndDisplay = true;
							continue;
						}
					}
				}
				if (alfunction == ALARM_TRIGGER)
				{
					displayAlarm = alarm;     // note the alarm to be displayed
					alfunction = ALARM_RESCHEDULE;    // only trigger one alarm for the event
				}
			}

			// If there is an alarm to display, do this last after rescheduling/cancelling
			// any others. This ensures that the updated event is only saved once to the calendar.
			if (displayAlarm.valid())
				handleAlarm(event, displayAlarm, ALARM_TRIGGER, true);
			else if (updateCalAndDisplay)
				updateEvent(event, 0L);     // update the window lists and calendar file
			break;
		}
	}
	return true;
}

/******************************************************************************
* Called when an alarm is displayed to reschedule it for its next repetition.
* If no repetitions remain, cancel it.
*/
void KAlarmApp::rescheduleAlarm(KAlarmEvent& event, int alarmID)
{
	kdDebug(5950) << "KAlarmApp::rescheduleAlarm(): " << event.id() << ":" << alarmID << endl;
	Event* kcalEvent = mCalendar->getEvent(event.id());
	if (!kcalEvent)
		kdError(5950) << "KAlarmApp::rescheduleAlarm(): event ID not found: " << event.id() << endl;
	else
	{
		KAlarmAlarm alarm = event.alarm(alarmID);
		if (!alarm.valid())
			kdError(5950) << "KAlarmApp::rescheduleAlarm(): alarm sequence not found: " << event.id() << ":" << alarmID << endl;
		handleAlarm(event, alarm, ALARM_RESCHEDULE, true);
	}
}

/******************************************************************************
* Either:
* a) Display the alarm and then delete it if it has no outstanding repetitions.
* b) Delete the alarm.
* c) Reschedule the alarm for its next repetition. If none remain, delete it.
* If the alarm is deleted and it is the last alarm for its event, the event is
* removed from the calendar file and from every main window instance.
*/
void KAlarmApp::handleAlarm(KAlarmEvent& event, KAlarmAlarm& alarm, AlarmFunc function, bool updateCalAndDisplay)
{
	switch (function)
	{
		case ALARM_TRIGGER:
			kdDebug(5950) << "KAlarmApp::handleAlarm(): TRIGGER" << endl;
			execAlarm(event, alarm, true);
			break;

		case ALARM_RESCHEDULE:
			// Leave an alarm which repeats at every login until its main alarm is deleted
			kdDebug(5950) << "KAlarmApp::handleAlarm(): RESCHEDULE" << endl;
			if (!alarm.repeatAtLogin())
			{
				int secs = alarm.dateTime().secsTo(QDateTime::currentDateTime());
				if (secs >= 0)
				{
					// The event is due by now
					int repeatSecs = alarm.repeatMinutes() * 60;
					if (repeatSecs)
					{
						int n = secs / repeatSecs + 1;
						int remainingCount = alarm.repeatCount() - n;
						if (remainingCount >= 0)
						{
							// Repetitions still remain, so rewrite the event
							event.updateRepetition(alarm.dateTime().addSecs(n * repeatSecs), remainingCount);

							if (updateCalAndDisplay)
								updateEvent(event, 0L);     // update the window lists and calendar file
							else
								event.setUpdated();    // note that the calendar file needs to be updated
							break;
						}
					}
					handleAlarm(event, alarm, ALARM_CANCEL, updateCalAndDisplay);
					break;
				}
			}
			else if (updateCalAndDisplay  &&  event.updated())
				updateEvent(event, 0L);     // update the window lists and calendar file
			break;

		case ALARM_CANCEL:
		{
			kdDebug(5950) << "KAlarmApp::handleAlarm(): CANCEL" << endl;
			event.removeAlarm(alarm.id());
			if (!event.alarmCount())
				deleteEvent(event, 0L, false);
			else if (updateCalAndDisplay)
				updateEvent(event, 0L);    // update the window lists and calendar file
			break;
		}
		default:
			kdError(5950) << "KAlarmApp::handleAlarm(): unknown function" << endl;
	}
}

/******************************************************************************
* Execute an alarm by displaying its message or file, or executing its command.
* Reply = false if an error message was output.
*/
bool KAlarmApp::execAlarm(KAlarmEvent& event, const KAlarmAlarm& alarm, bool reschedule, bool allowDefer)
{
	bool result = true;
	if (alarm.type() == KAlarmAlarm::COMMAND)
	{
		QString command = event.cleanText();
		kdDebug(5950) << "KAlarmApp::execAlarm(): COMMAND: " << command << endl;
		KShellProcess proc;
		proc << command;
		if (!proc.start(KProcess::DontCare))
		{
			kdDebug(5950) << "KAlarmApp::execAlarm(): failed\n";
			(new MessageWin(i18n("Failed to execute command:\n%1").arg(command), event, alarm, reschedule))->show();
			result = false;
		}
		if (reschedule)
			rescheduleAlarm(event, alarm.id());
	}
	else
		(new MessageWin(event, alarm, reschedule, allowDefer))->show();
	return result;
}

/******************************************************************************
* Add a new alarm.
* Save it in the calendar file and add it to every main window instance.
* Parameters:
*    win  = initiating main window instance (which has already been updated)
*/
bool KAlarmApp::addEvent(const KAlarmEvent& event, KAlarmMainWindow* win)
{
	kdDebug(5950) << "KAlarmApp::addEvent(): " << event.id() << endl;
	if (!initCheck())
		return false;

	// Save the event details in the calendar file, and get the new event ID
	mCalendar->addEvent(event);
	mCalendar->save();

	// Tell the daemon to reread the calendar file
	reloadDaemon();

	// Update the window lists
	KAlarmMainWindow::addEvent(event, win);
	return true;
}

/******************************************************************************
* Modify an alarm in every main window instance.
* The new event will have a different event ID from the old one.
* Parameters:
*    win  = initiating main window instance (which has already been updated)
*/
void KAlarmApp::modifyEvent(const QString& oldEventID, const KAlarmEvent& newEvent, KAlarmMainWindow* win)
{
	kdDebug(5950) << "KAlarmApp::modifyEvent(): '" << oldEventID << endl;

	// Update the event in the calendar file, and get the new event ID
	mCalendar->deleteEvent(oldEventID);
	mCalendar->addEvent(newEvent);
	mCalendar->save();

	// Tell the daemon to reread the calendar file
	reloadDaemon();

	// Update the window lists
	KAlarmMainWindow::modifyEvent(oldEventID, newEvent, win);
}

/******************************************************************************
* Update an alarm in every main window instance.
* The new event will have the same event ID as the old one.
* Parameters:
*    win  = initiating main window instance (which has already been updated)
*/
void KAlarmApp::updateEvent(const KAlarmEvent& event, KAlarmMainWindow* win)
{
	kdDebug(5950) << "KAlarmApp::updateEvent(): " << event.id() << endl;

	// Update the event in the calendar file
	const_cast<KAlarmEvent&>(event).incrementRevision();
	mCalendar->updateEvent(event);
	mCalendar->save();

	// Tell the daemon to reread the calendar file
	reloadDaemon();

	// Update the window lists
	KAlarmMainWindow::modifyEvent(event, win);
}

/******************************************************************************
* Delete an alarm from every main window instance.
* Parameters:
*    win  = initiating main window instance (which has already been updated)
*/
void KAlarmApp::deleteEvent(KAlarmEvent& event, KAlarmMainWindow* win, bool tellDaemon)
{
	kdDebug(5950) << "KAlarmApp::deleteEvent(): " << event.id() << endl;

	// Update the window lists
	KAlarmMainWindow::deleteEvent(event, win);

	// Delete the event from the calendar file
	mCalendar->deleteEvent(event.id());
	mCalendar->save();

	// Tell the daemon to reread the calendar file
	if (tellDaemon)
		reloadDaemon();
}

/******************************************************************************
* Set up the DCOP handlers.
*/
void KAlarmApp::setUpDcop()
{
	if (!mDcopHandler)
	{
		mDcopHandler      = new DcopHandler(QString::fromLatin1(DCOP_OBJECT_NAME));
		mDaemonGuiHandler = new DaemonGuiHandler(QString::fromLatin1(GUI_DCOP_OBJECT_NAME));
	}
}

/******************************************************************************
* If this is the first time through, open the calendar file, optionally start
* the alarm daemon and register with it, and set up the DCOP handler.
*/
bool KAlarmApp::initCheck(bool calendarOnly)
{
	if (!mCalendar->isOpen())
	{
		kdDebug(5950) << "KAlarmApp::initCheck(): opening calendar\n";

		// First time through. Open the calendar file.
		if (!mCalendar->open())
			return false;

		if (!calendarOnly)
			startDaemon();    // Make sure the alarm daemon is running
	}
	else if (!mDaemonRegistered)
		startDaemon();

	if (!calendarOnly)
		setUpDcop();     // we're now ready to handle DCOP calls, so set up handlers
	return true;
}

/******************************************************************************
* Start the alarm daemon if necessary, and register this application with it.
*/
void KAlarmApp::startDaemon()
{
	kdDebug(5950) << "KAlarmApp::startDaemon()\n";
	mCalendar->getURL();    // check that the calendar file name is OK - program exit if not
	if (!dcopClient()->isApplicationRegistered(DAEMON_APP_NAME))
	{
		// Start the alarm daemon. It is a KUniqueApplication, which means that
		// there is automatically only one instance of the alarm daemon running.
		QString execStr = locate("exe",QString::fromLatin1(DAEMON_APP_NAME));
		system(QFile::encodeName(execStr));
		kdDebug(5950) << "KAlarmApp::startDaemon(): Alarm daemon started" << endl;
	}

	// Register this application with the alarm daemon
	registerWithDaemon();

	// Tell alarm daemon to load the calendar
	{
		QByteArray data;
		QDataStream arg(data, IO_WriteOnly);
		arg << QCString(aboutData()->appName()) << mCalendar->urlString();
		if (!dcopClient()->send(DAEMON_APP_NAME, DAEMON_DCOP_OBJECT, "addMsgCal(QCString,QString)", data))
			kdDebug(5950) << "KAlarmApp::startDaemon(): addCal dcop send failed" << endl;
	}

	mDaemonRegistered = true;
	kdDebug(5950) << "KAlarmApp::startDaemon(): started daemon" << endl;
}

/******************************************************************************
* Start the alarm daemon if necessary, and register this application with it.
*/
void KAlarmApp::registerWithDaemon()
{
	kdDebug(5950) << "KAlarmApp::registerWithDaemon()\n";
	QByteArray data;
	QDataStream arg(data, IO_WriteOnly);
	arg << QCString(aboutData()->appName()) << aboutData()->programName()
	    << QCString(DCOP_OBJECT_NAME)
	    << (int)(mDisableAlarmsIfStopped ? ClientInfo::NO_START_NOTIFY : ClientInfo::COMMAND_LINE_NOTIFY)
	    << (Q_INT8)0;
	if (!dcopClient()->send(DAEMON_APP_NAME, DAEMON_DCOP_OBJECT, "registerApp(QCString,QString,QCString,int,bool)", data))
		kdDebug(5950) << "KAlarmApp::registerWithDaemon(): registerApp dcop send failed" << endl;
}

/******************************************************************************
* Stop the alarm daemon if it is running.
*/
bool KAlarmApp::stopDaemon()
{
	kdDebug(5950) << "KAlarmApp::stopDaemon()" << endl;
	if (dcopClient()->isApplicationRegistered(DAEMON_APP_NAME))
	{
		QByteArray data;
		if (!dcopClient()->send(DAEMON_APP_NAME, DAEMON_DCOP_OBJECT, "quit()", data))
		{
			kdError(5950) << "KAlarmApp::stopDaemon(): quit dcop send failed" << endl;
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
	kdDebug(5950) << "KAlarmApp::resetDaemon()" << endl;
	if (!dcopClient()->isApplicationRegistered(DAEMON_APP_NAME))
		startDaemon();
	else
	{
		QByteArray data;
		QDataStream arg(data, IO_WriteOnly);
		arg << QCString(aboutData()->appName()) << mCalendar->urlString();
		if (!dcopClient()->send(DAEMON_APP_NAME, DAEMON_DCOP_OBJECT, "resetMsgCal(QCString,QString)", data))
			kdDebug(5950) << "KAlarmApp::resetDaemon(): addCal dcop send failed" << endl;
	}
}

/******************************************************************************
* Tell the alarm daemon to reread the calendar file.
*/
void KAlarmApp::reloadDaemon()
{
	QByteArray data;
	QDataStream arg(data, IO_WriteOnly);
	arg << QCString(aboutData()->appName()) << mCalendar->urlString();
	if (!dcopClient()->send(DAEMON_APP_NAME, DAEMON_DCOP_OBJECT, "reloadMsgCal(QCString,QString)", data))
		kdDebug(5950) << "KAlarmApp::reloadDaemon(): dcop send failed" << endl;
}

/******************************************************************************
* Check whether the alarm daemon is currently running.
*/
bool KAlarmApp::isDaemonRunning()
{
	bool running = dcopClient()->isApplicationRegistered(DAEMON_APP_NAME);
	if (running != mDaemonRunning)
	{
		// Daemon's status has changed
		mDaemonRunning = running;
		if (mDaemonRunning)
			startDaemon();      // re-register with the daemon
	}
	return mDaemonRunning;
}

/******************************************************************************
* Read the alarm daemon's alarm check interval from its config file. If it has
* reduced, any late-cancel alarms which are already due could potentially be
* cancelled erroneously. To avoid this, note the effective last time that it
* will have checked alarms.
*/
void KAlarmApp::readDaemonCheckInterval()
{
	KConfig config(locate("config", DAEMON_APP_NAME_DEF"rc"));
	config.setGroup("General");
	int checkInterval = 60 * config.readNumEntry("CheckInterval", 1);
	if (checkInterval < mDaemonCheckInterval)
	{
		// The daemon check interval has reduced,
		// Note the effective last time that the daemon checked alarms.
		QDateTime now = QDateTime::currentDateTime();
		mLastDaemonCheck = now.addSecs(-mDaemonCheckInterval);
		mNextDaemonCheck = now.addSecs(checkInterval);
	}
	mDaemonCheckInterval = checkInterval;
}

/******************************************************************************
* Find the maximum number of seconds late which a late-cancel alarm is allowed
* to be. This is calculated as the alarm daemon's check interval, plus a few
* seconds leeway to cater for any timing irregularities.
*/
int KAlarmApp::maxLateness()
{
	static const int LATENESS_LEEWAY = 5;

	readDaemonCheckInterval();
	if (mLastDaemonCheck.isValid())
	{
		QDateTime now = QDateTime::currentDateTime();
		if (mNextDaemonCheck > now)
		{
			// Daemon's check interval has just reduced, so allow extra time
			return mLastDaemonCheck.secsTo(now) + LATENESS_LEEWAY;
		}
		mLastDaemonCheck = QDateTime();
	}
	return mDaemonCheckInterval + LATENESS_LEEWAY;
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
* Check whether a file appears to be a text file by looking at its mime type.
* Reply = 0 if not a text file
*       = 1 if a plain text file
*       = 2 if a formatted text file.
*/
int KAlarmApp::isTextFile(const KURL& url)
{
	static const char* applicationTypes[] = {
		"x-shellscript", "x-nawk", "x-awk", "x-perl", "x-python",
		"x-desktop", "x-troff", 0 };
	static const char* formattedTextTypes[] = {
		"html", "xml", 0 };

	QString mimetype = KFileItem(-1, -1, url).mimetype();
	int slash = mimetype.find('/');
	if (slash < 0)
		return 0;
	QString subtype = mimetype.mid(slash + 1);
	if (mimetype.startsWith(QString::fromLatin1("application")))
	{
		for (int i = 0;  applicationTypes[i];  ++i)
			if (!strcmp(subtype, applicationTypes[i]))
				return 1;
	}
	else if (mimetype.startsWith(QString::fromLatin1("text")))
	{
		for (int i = 0;  formattedTextTypes[i];  ++i)
			if (!strcmp(subtype, formattedTextTypes[i]))
				return 2;
		return 1;
	}
	return 0;
}

/******************************************************************************
* This class's function is simply to act as a receiver for DCOP requests.
*/
DcopHandler::DcopHandler(const char* dcopObject)
	: QWidget(),
	  DCOPObject(dcopObject)
{
	kdDebug(5950) << "DcopHandler::DcopHandler()\n";
}

/******************************************************************************
* Process a DCOP request.
*/
bool DcopHandler::process(const QCString& func, const QByteArray& data, QCString& replyType, QByteArray&)
{
	kdDebug(5950) << "DcopHandler::process(): " << func << endl;
	enum
	{
		ERR, HANDLE, CANCEL, TRIGGER,
		REP_FLAG           = 0x01,
		SCHEDULE           = 0x100,
		SCHEDULE_n         = SCHEDULE | REP_FLAG,
		SCHEDULE_FILE      = SCHEDULE | 0x10,
		SCHEDULE_FILE_n    = SCHEDULE_FILE | REP_FLAG,
		SCHEDULE_COMMAND   = SCHEDULE | 0x20,
		SCHEDULE_COMMAND_n = SCHEDULE_COMMAND | REP_FLAG
	};
	int function;
	if      (func == "handleEvent(const QString&,const QString&)"
	||       func == "handleEvent(QString,QString)")
		function = HANDLE;
	else if (func == "cancelEvent(const QString&,const QString&)"
	||       func == "cancelEvent(QString,QString)"
	||       func == "cancelMessage(const QString&,const QString&)"    // deprecated
	||       func == "cancelMessage(QString,QString)")                 // deprecated
		function = CANCEL;
	else if (func == "triggerEvent(const QString&,const QString&)"
	||       func == "triggerEvent(QString,QString)"
	||       func == "displayMessage(const QString&,const QString&)"   // deprecated
	||       func == "displayMessage(QString,QString)")                // deprecated
		function = TRIGGER;
	else if (func == "scheduleMessage(const QString&,const QDateTime&,QColor,Q_UINT32)"
	||       func == "scheduleMessage(QString,QDateTime,QColor,Q_UINT32)")
		function = SCHEDULE;
	else if (func == "scheduleMessage(const QString&,const QDateTime&,QColor,Q_UINT32,Q_INT32,Q_INT32)"
	||       func == "scheduleMessage(QString,QDateTime,QColor,Q_UINT32,Q_INT32,Q_INT32)")
		function = SCHEDULE_n;
	else if (func == "scheduleFile(const QString&,const QDateTime&,QColor,Q_UINT32)"
	||       func == "scheduleFile(QString,QDateTime,QColor,Q_UINT32)")
		function = SCHEDULE_FILE;
	else if (func == "scheduleFile(const QString&,const QDateTime&,QColor,Q_UINT32,Q_INT32,Q_INT32)"
	||       func == "scheduleFile(QString,QDateTime,QColor,Q_UINT32,Q_INT32,Q_INT32)")
		function = SCHEDULE_FILE_n;
	else if (func == "scheduleCommand(const QString&,const QDateTime&,Q_UINT32)"
	||       func == "scheduleCommand(QString,QDateTime,Q_UINT32)")
		function = SCHEDULE_COMMAND;
	else if (func == "scheduleCommand(const QString&,const QDateTime&,Q_UINT32,Q_INT32,Q_INT32)"
	||       func == "scheduleCommand(QString,QDateTime,Q_UINT32,Q_INT32,Q_INT32)")
		function = SCHEDULE_COMMAND_n;
	else
	{
		kdDebug(5950) << "DcopHandler::process(): unknown DCOP function" << endl;
		return false;
	}

	KAlarmAlarm::Type type;;
	switch (function)
	{
		case HANDLE:        // trigger or cancel event with specified ID from calendar file
		case CANCEL:        // cancel event with specified ID from calendar file
		case TRIGGER:       // trigger event with specified ID in calendar file
		{

			QDataStream arg(data, IO_ReadOnly);
			QString urlString, vuid;
			arg >> urlString >> vuid;
			replyType = "void";
			switch (function)
			{
				case HANDLE:
					theApp()->handleEvent(urlString, vuid);
					break;
				case CANCEL:
					theApp()->deleteEvent(urlString, vuid);
					break;
				case TRIGGER:
					theApp()->triggerEvent(urlString, vuid);
					break;
			}
			break;
		}
		case SCHEDULE:         // schedule a new event
		case SCHEDULE_n:       // schedule a new repeating event
			type = KAlarmAlarm::MESSAGE;
			break;
		case SCHEDULE_FILE:    // schedule the display of a file's contents
		case SCHEDULE_FILE_n:  // schedule the repeating display of a file's contents
			type = KAlarmAlarm::FILE;
			break;
		case SCHEDULE_COMMAND:    // schedule the execution of a command
		case SCHEDULE_COMMAND_n:  // schedule the repeating execution of a command
			type = KAlarmAlarm::COMMAND;
			break;
	}
	if (function & SCHEDULE)
	{
		QDataStream arg(data, IO_ReadOnly);
		QString text;
		QDateTime dateTime;
		QColor bgColour;
		Q_UINT32 flags;
		Q_INT32 repeatCount = 0;
		Q_INT32 repeatInterval = 0;
		arg >> text;
		arg.readRawBytes((char*)&dateTime, sizeof(dateTime));
		if (type != KAlarmAlarm::COMMAND)
			arg.readRawBytes((char*)&bgColour, sizeof(bgColour));
		arg >> flags;
		if (function & REP_FLAG)
			arg >> repeatCount >> repeatInterval;
		theApp()->scheduleEvent(text, &dateTime, bgColour, flags, type, repeatCount, repeatInterval);
		replyType = "void";
	}
	return true;
}
