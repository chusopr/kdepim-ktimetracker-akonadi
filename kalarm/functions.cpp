/*
 *  functions.cpp  -  miscellaneous functions
 *  Program:  kalarm
 *  Copyright � 2001-2006 by David Jarvie <software@astrojar.org.uk>
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kalarm.h"

#include <qdir.h>
#include <qregexp.h>
#include <QDesktopWidget>

#include <kconfig.h>
#include <kaction.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kstdaccel.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <dcopclient.h>
#include <kdebug.h>

#include <libkcal/event.h>
#include <libkcal/icalformat.h>
#include <libkpimidentities/identitymanager.h>
#include <libkpimidentities/identity.h>
#include <libkcal/person.h>
#include <ktoolinvocation.h>

#include "alarmcalendar.h"
#include "alarmevent.h"
#include "alarmlistview.h"
#include "daemon.h"
#include "kalarmapp.h"
#include "kamail.h"
#include "mainwindow.h"
#include "messagewin.h"
#include "preferences.h"
#include "shellprocess.h"
#include "templatelistview.h"
#include "templatemenuaction.h"
#include "functions.h"


namespace
{
bool        resetDaemonQueued = false;
DCOPCString korganizerName    = "korganizer";
QString     korgStartError;
const char* KORG_DCOP_OBJECT  = "KOrganizerIface";
const char* KORG_DCOP_WINDOW  = "KOrganizer MainWindow";
const char* KMAIL_DCOP_WINDOW = "kmail-mainwindow#1";

bool sendToKOrganizer(const KAEvent&);
bool deleteFromKOrganizer(const QString& eventID);
inline bool runKOrganizer()   { return KAlarm::runProgram("korganizer", KORG_DCOP_WINDOW, korganizerName, korgStartError); }
}


namespace KAlarm
{

/******************************************************************************
*  Display a main window with the specified event selected.
*/
MainWindow* displayMainWindowSelected(const QString& eventID)
{
	MainWindow* win = MainWindow::firstWindow();
	if (!win)
	{
		if (theApp()->checkCalendarDaemon())    // ensure calendar is open and daemon started
		{
			win = MainWindow::create();
			win->show();
		}
	}
	else
	{
		// There is already a main window, so make it the active window
		bool visible = win->isVisible();
		if (visible)
			win->hide();        // in case it's on a different desktop
		if (!visible  ||  win->isMinimized())
			win->showNormal();
		win->raise();
		win->activateWindow();
	}
	if (win  &&  !eventID.isEmpty())
		win->selectEvent(eventID);
	return win;
}

/******************************************************************************
* Create a New Alarm KAction.
*/
KAction* createNewAlarmAction(const QString& label, QObject* receiver, const char* slot, KActionCollection* actions, const char* name)
{
	return new KAction(label, "filenew", KStdAccel::openNew(), receiver, slot, actions, name);
}

/******************************************************************************
* Create a New From Template KAction.
*/
TemplateMenuAction* createNewFromTemplateAction(const QString& label, QObject* receiver, const char* slot, KActionCollection* actions, const char* name)
{
	return new TemplateMenuAction(label, "new_from_template", receiver, slot, actions, name);
}

/******************************************************************************
* Add a new active (non-expired) alarm.
* Save it in the calendar file and add it to every main window instance.
* If 'selectionView' is non-null, the selection highlight is moved to the new
* event in that listView instance.
* 'event' is updated with the actual event ID.
*/
UpdateStatus addEvent(KAEvent& event, AlarmListView* selectionView, bool useEventID, bool allowKOrgUpdate)
{
	kDebug(5950) << "KAlarm::addEvent(): " << event.id() << endl;
	if (!theApp()->checkCalendarDaemon())    // ensure calendar is open and daemon started
		return UPDATE_ERROR;

	// Save the event details in the calendar file, and get the new event ID
	AlarmCalendar* cal = AlarmCalendar::activeCalendar();
	cal->addEvent(event, useEventID);
	cal->save();

	UpdateStatus ret = UPDATE_OK;
	if (allowKOrgUpdate  &&  event.copyToKOrganizer())
	{
		if (!sendToKOrganizer(event))    // tell KOrganizer to show the event
			ret = UPDATE_KORG_ERR;
	}

	// Update the window lists
	AlarmListView::addEvent(event, selectionView);
	return ret;
}

/******************************************************************************
* Save the event in the expired calendar file and adjust every main window instance.
* The event's ID is changed to an expired ID if necessary.
*/
bool addExpiredEvent(KAEvent& event)
{
	kDebug(5950) << "KAlarm::addExpiredEvent(" << event.id() << ")\n";
	AlarmCalendar* cal = AlarmCalendar::expiredCalendarOpen();
	if (!cal)
		return false;
	bool archiving = (KAEvent::uidStatus(event.id()) == KAEvent::ACTIVE);
	if (archiving)
		event.setSaveDateTime(QDateTime::currentDateTime());   // time stamp to control purging
	KCal::Event* kcalEvent = cal->addEvent(event);
	cal->save();

	// Update window lists
	if (!archiving)
		AlarmListView::addEvent(event, 0);
	else if (kcalEvent)
		AlarmListView::modifyEvent(KAEvent(*kcalEvent), 0);
	return true;
}

/******************************************************************************
* Add a new template.
* Save it in the calendar file and add it to every template list view.
* If 'selectionView' is non-null, the selection highlight is moved to the new
* event in that listView instance.
* 'event' is updated with the actual event ID.
*/
bool addTemplate(KAEvent& event, TemplateListView* selectionView)
{
	kDebug(5950) << "KAlarm::addTemplate(): " << event.id() << endl;

	// Add the template to the calendar file
	AlarmCalendar* cal = AlarmCalendar::templateCalendarOpen();
	if (!cal)
		return false;
	cal->addEvent(event);
	cal->save();
	cal->emitEmptyStatus();

	// Update the window lists
	TemplateListView::addEvent(event, selectionView);
	return true;
}

/******************************************************************************
* Modify an active (non-expired) alarm in the calendar file and in every main
* window instance.
* The new event will have a different event ID from the old one.
* If 'selectionView' is non-null, the selection highlight is moved to the
* modified event in that listView instance.
*/
UpdateStatus modifyEvent(KAEvent& oldEvent, const KAEvent& newEvent, AlarmListView* selectionView)
{
	kDebug(5950) << "KAlarm::modifyEvent(): '" << oldEvent.id() << endl;

	if (!newEvent.valid())
		return deleteEvent(oldEvent, true);
	else
	{
		UpdateStatus ret = UPDATE_OK;
		if (oldEvent.copyToKOrganizer())
		{
			// Tell KOrganizer to delete its old event.
			// But ignore errors, because the user could have manually
			// deleted it since KAlarm asked KOrganizer to set it up.
			deleteFromKOrganizer(oldEvent.id());
		}

		// Update the event in the calendar file, and get the new event ID
		AlarmCalendar* cal = AlarmCalendar::activeCalendar();
		cal->deleteEvent(oldEvent.id());
		cal->addEvent(const_cast<KAEvent&>(newEvent), true);
		cal->save();

		if (newEvent.copyToKOrganizer())
		{
			if (!sendToKOrganizer(newEvent))    // tell KOrganizer to show the new event
				ret = UPDATE_KORG_ERR;
		}

		// Update the window lists
		AlarmListView::modifyEvent(oldEvent.id(), newEvent, selectionView);
		return ret;
	}
}

/******************************************************************************
* Update an active (non-expired) alarm from the calendar file and from every
* main window instance.
* The new event will have the same event ID as the old one.
* If 'selectionView' is non-null, the selection highlight is moved to the
* updated event in that listView instance.
* The event is not updated in KOrganizer, since this function is called when an
* existing alarm is rescheduled (due to recurrence or deferral).
*/
void updateEvent(KAEvent& event, AlarmListView* selectionView, bool archiveOnDelete, bool incRevision)
{
	kDebug(5950) << "KAlarm::updateEvent(): " << event.id() << endl;

	if (!event.valid())
		deleteEvent(event, archiveOnDelete);
	else
	{
		// Update the event in the calendar file.
		if (incRevision)
			event.incrementRevision();    // ensure alarm daemon sees the event has changed
		AlarmCalendar* cal = AlarmCalendar::activeCalendar();
		cal->updateEvent(event);
		cal->save();

		// Update the window lists
		AlarmListView::modifyEvent(event, selectionView);
	}
}

/******************************************************************************
* Update a template in the calendar file and in every template list view.
* If 'selectionView' is non-null, the selection highlight is moved to the
* updated event in that listView instance.
*/
void updateTemplate(const KAEvent& event, TemplateListView* selectionView)
{
	AlarmCalendar* cal = AlarmCalendar::templateCalendarOpen();
	if (cal)
	{
		cal->updateEvent(event);
		cal->save();

		TemplateListView::modifyEvent(event.id(), event, selectionView);
	}
}

/******************************************************************************
* Delete an alarm from the calendar file and from every main window instance.
* If the event is archived, the event's ID is changed to an expired ID if necessary.
*/
UpdateStatus deleteEvent(KAEvent& event, bool archive)
{
	QString id = event.id();
	kDebug(5950) << "KAlarm::deleteEvent(): " << id << endl;

	// Update the window lists
	AlarmListView::deleteEvent(id);

	UpdateStatus ret = UPDATE_OK;

	// Delete the event from the calendar file
	if (KAEvent::uidStatus(id) == KAEvent::EXPIRED)
	{
		AlarmCalendar* cal = AlarmCalendar::expiredCalendarOpen();
		if (cal)
			cal->deleteEvent(id, true);   // save calendar after deleting
	}
	else
	{
		if (event.copyToKOrganizer())
		{
			// The event was shown in KOrganizer, so tell KOrganizer to
			// delete it. Note that an error could occur if the user
			// manually deleted it from KOrganizer since it was set up.
			if (!deleteFromKOrganizer(event.id()))
				ret = UPDATE_KORG_ERR;
		}
		if (archive  &&  event.toBeArchived())
			addExpiredEvent(event);     // this changes the event ID to an expired ID
		AlarmCalendar* cal = AlarmCalendar::activeCalendar();
		cal->deleteEvent(id, true);    // save calendar after deleting
	}
	return ret;
}

/******************************************************************************
* Delete a template from the calendar file and from every template list view.
*/
void deleteTemplate(const KAEvent& event)
{
	QString id = event.id();

	// Delete the template from the calendar file
	AlarmCalendar* cal = AlarmCalendar::templateCalendarOpen();
	if (cal)
	{
		cal->deleteEvent(id, true);    // save calendar after deleting
		cal->emitEmptyStatus();
	}

	// Update the window lists
	TemplateListView::deleteEvent(id);
}

/******************************************************************************
* Delete an alarm from the display calendar.
*/
void deleteDisplayEvent(const QString& eventID)
{
	kDebug(5950) << "KAlarm::deleteDisplayEvent(" << eventID << ")\n";

	if (KAEvent::uidStatus(eventID) == KAEvent::DISPLAYING)
	{
		AlarmCalendar* cal = AlarmCalendar::displayCalendarOpen();
		if (cal)
			cal->deleteEvent(eventID, true);   // save calendar after deleting
	}
}

/******************************************************************************
* Undelete an expired alarm, and update every main window instance.
* The archive bit is set to ensure that it gets re-archived if it is deleted again.
* If 'selectionView' is non-null, the selection highlight is moved to the
* restored event in that listView instance.
*/
UpdateStatus reactivateEvent(KAEvent& event, AlarmListView* selectionView, bool useEventID)
{
	QString id = event.id();
	kDebug(5950) << "KAlarm::reactivateEvent(): " << id << endl;

	// Delete the event from the expired calendar file
	if (KAEvent::uidStatus(id) == KAEvent::EXPIRED)
	{
		QDateTime now = QDateTime::currentDateTime();
		if (event.occursAfter(now, true))
		{
			if (event.recurs())
				event.setNextOccurrence(now, true);   // skip any recurrences in the past
			event.setArchive();    // ensure that it gets re-archived if it is deleted

			// Save the event details in the calendar file, and get the new event ID
			AlarmCalendar* cal = AlarmCalendar::activeCalendar();
			cal->addEvent(event, useEventID);
			cal->save();

			UpdateStatus ret = UPDATE_OK;
			if (event.copyToKOrganizer())
			{
				if (!sendToKOrganizer(event))    // tell KOrganizer to show the event
					ret = UPDATE_KORG_ERR;
			}

			// Update the window lists
			AlarmListView::undeleteEvent(id, event, selectionView);

			cal = AlarmCalendar::expiredCalendarOpen();
			if (cal)
				cal->deleteEvent(id, true);   // save calendar after deleting
			return ret;
		}
	}
	return UPDATE_ERROR;
}

/******************************************************************************
* Enable or disable an alarm in the calendar file and in every main window instance.
* The new event will have the same event ID as the old one.
* If 'selectionView' is non-null, the selection highlight is moved to the
* updated event in that listView instance.
*/
void enableEvent(KAEvent& event, AlarmListView* selectionView, bool enable)
{
	kDebug(5950) << "KAlarm::enableEvent(" << enable << "): " << event.id() << endl;

	if (enable != event.enabled())
	{
		event.setEnabled(enable);

		// Update the event in the calendar file
		AlarmCalendar* cal = AlarmCalendar::activeCalendar();
		cal->updateEvent(event);
		cal->save();

		// If we're disabling a display alarm, close any message window
		if (!enable  &&  event.displayAction())
		{
			MessageWin* win = MessageWin::findEvent(event.id());
			delete win;
		}

		// Update the window lists
		AlarmListView::modifyEvent(event, selectionView);
	}
}

/******************************************************************************
* Display an error message corresponding to a specified alarm update error code.
*/
void displayKOrgUpdateError(QWidget* parent, UpdateError code, int nAlarms)
{
	QString errmsg;
	switch (code)
	{
		case KORG_ERR_ADD:
			errmsg = (nAlarms > 1) ? i18n("Unable to show alarms in KOrganizer")
			                       : i18n("Unable to show alarm in KOrganizer");
			break;
		case KORG_ERR_MODIFY:
			errmsg = i18n("Unable to update alarm in KOrganizer");
			break;
		case KORG_ERR_DELETE:
			errmsg = (nAlarms > 1) ? i18n("Unable to delete alarms from KOrganizer")
			                       : i18n("Unable to delete alarm from KOrganizer");
			break;
	}
	KMessageBox::error(parent, errmsg);
}

/******************************************************************************
*  Returns a list of all alarm templates.
*  If shell commands are disabled, command alarm templates are omitted.
*/
QList<KAEvent> templateList()
{
	QList<KAEvent> templates;
	AlarmCalendar* cal = AlarmCalendar::templateCalendarOpen();
	if (cal)
	{
		bool includeCmdAlarms = ShellProcess::authorised();
		KCal::Event::List events = cal->events();
		for (KCal::Event::List::ConstIterator it = events.begin();  it != events.end();  ++it)
		{
			KCal::Event* kcalEvent = *it;
			KAEvent event(*kcalEvent);
			if (includeCmdAlarms  ||  event.action() != KAEvent::COMMAND)
				templates.append(event);
		}
	}
	return templates;
}

/******************************************************************************
* To be called after an alarm has been edited.
* Prompt the user to re-enable alarms if they are currently disabled, and if
* it's an email alarm, warn if no 'From' email address is configured.
*/
void outputAlarmWarnings(QWidget* parent, const KAEvent* event)
{
	if (event  &&  event->action() == KAEvent::EMAIL
	&&  Preferences::emailAddress().isEmpty())
		KMessageBox::information(parent, i18n("Please set the 'From' email address...",
		                                      "%1\nPlease set it in the Preferences dialog.").arg(KAMail::i18n_NeedFromEmailAddress()));

	if (!Daemon::monitoringAlarms())
	{
		if (KMessageBox::warningYesNo(parent, i18n("Alarms are currently disabled.\nDo you want to enable alarms now?"),
		                              QString(), i18n("Enable"), i18n("Keep Disabled"),
		                              QLatin1String("EditEnableAlarms"))
		                == KMessageBox::Yes)
			Daemon::setAlarmsEnabled();
	}
}

/******************************************************************************
* Reset the alarm daemon and reload the calendar.
* If the daemon is not already running, start it.
*/
void resetDaemon()
{
	kDebug(5950) << "KAlarm::resetDaemon()" << endl;
	if (!resetDaemonQueued)
	{
		resetDaemonQueued = true;
		theApp()->processQueue();
	}
}

/******************************************************************************
* This method must only be called from the main KAlarm queue processing loop,
* to prevent asynchronous calendar operations interfering with one another.
*
* If resetDaemon() has been called, reset the alarm daemon and reload the calendars.
* If the daemon is not already running, start it.
*/
void resetDaemonIfQueued()
{
	if (resetDaemonQueued)
	{
		kDebug(5950) << "KAlarm::resetDaemonIfNeeded()" << endl;
		AlarmCalendar::activeCalendar()->reload();
		AlarmCalendar::expiredCalendar()->reload();

		// Close any message windows for alarms which are now disabled
		KAEvent event;
		KCal::Event::List events = AlarmCalendar::activeCalendar()->events();
		for (KCal::Event::List::ConstIterator it = events.begin();  it != events.end();  ++it)
		{
			KCal::Event* kcalEvent = *it;
			event.set(*kcalEvent);
			if (!event.enabled()  &&  event.displayAction())
			{
				MessageWin* win = MessageWin::findEvent(event.id());
				delete win;
			}
		}

		MainWindow::refresh();
		if (!Daemon::reset())
			Daemon::start();
		resetDaemonQueued = false;
	}
}

/******************************************************************************
*  Start KMail if it isn't already running, and optionally iconise it.
*  Reply = reason for failure to run KMail (which may be the empty string)
*        = null string if success.
*/
QString runKMail(bool minimise)
{
	DCOPCString dcopName;
	QString errmsg;
	if (!runProgram("kmail", (minimise ? KMAIL_DCOP_WINDOW : ""), dcopName, errmsg))
		return i18n("Unable to start KMail\n(%1)").arg(errmsg);
	return QString();
}

/******************************************************************************
*  Start another program for DCOP access if it isn't already running.
*  If 'windowName' is not empty, the program's window of that name is iconised.
*  On exit, 'dcopName' contains the DCOP name to access the application, and
*  'errorMessage' contains an error message if failure.
*  Reply = true if the program is now running.
*/
bool runProgram(const DCOPCString& program, const DCOPCString& windowName, DCOPCString& dcopName, QString& errorMessage)
{
	if (!kapp->dcopClient()->isApplicationRegistered(program))
	{
		// KOrganizer is not already running, so start it
		if (KToolInvocation::startServiceByDesktopName(QLatin1String(program), QString(), &errorMessage, &dcopName))
		{
			kError(5950) << "runProgram(): couldn't start " << program << " (" << errorMessage << ")\n";
			return false;
		}
		// Minimise its window - don't use hide() since this would remove all
		// trace of it from the panel if it is not configured to be docked in
		// the system tray.
		kapp->dcopClient()->send(dcopName, windowName, "minimize()", QString());
	}
	else if (dcopName.isEmpty())
		dcopName = program;
	errorMessage.clear();
	return true;
}

/******************************************************************************
*  Read the size for the specified window from the config file, for the
*  current screen resolution.
*  Reply = true if size set in the config file, in which case 'result' is set
*        = false if no size is set, in which case 'result' is unchanged.
*/
bool readConfigWindowSize(const char* window, QSize& result)
{
	KConfig* config = KGlobal::config();
	config->setGroup(QLatin1String(window));
	QWidget* desktop = KApplication::desktop();
	QSize s = QSize(config->readEntry(QString::fromLatin1("Width %1").arg(desktop->width()), (int)0),
	                config->readEntry(QString::fromLatin1("Height %1").arg(desktop->height()), (int)0));
	if (s.isEmpty())
		return false;
	result = s;
	return true;
}

/******************************************************************************
*  Write the size for the specified window to the config file, for the
*  current screen resolution.
*/
void writeConfigWindowSize(const char* window, const QSize& size)
{
	KConfig* config = KGlobal::config();
	config->setGroup(QLatin1String(window));
	QWidget* desktop = KApplication::desktop();
	config->writeEntry(QString::fromLatin1("Width %1").arg(desktop->width()), size.width());
	config->writeEntry(QString::fromLatin1("Height %1").arg(desktop->height()), size.height());
	config->sync();
}

/******************************************************************************
* Convert the supplied KAlarm version string to a version number.
* Reply = version number (double digit for each of major, minor & issue number,
*         e.g. 010203 for 1.2.3
*       = 0 if invalid version string.
*/
int getVersionNumber(const QString& version, QString* subVersion)
{
	// N.B. Remember to change  Version(int major, int minor, int rev)
	//      if the representation returned by this method changes.
	if (subVersion)
		*subVersion = QString();
	QStringList nums = version.split(QLatin1Char('.'), QString::KeepEmptyParts);
	int count = nums.count();
	if (count < 2  ||  count > 3)
		return 0;
	bool ok;
	int vernum = nums[0].toInt(&ok) * 10000;    // major version
	if (!ok)
		return 0;
	int v = nums[1].toInt(&ok);                 // minor version
	if (!ok)
		return 0;
	vernum += (v < 99 ? v : 99) * 100;
	if (count == 3)
	{
		// Issue number: allow other characters to follow the last digit
		QString issue = nums[2];
		if (!issue.at(0).isDigit())
			return 0;
		v = issue.toInt();   // issue number
		vernum += (v < 99 ? v : 99);
		if (subVersion)
		{
			int i;
			for (i = 1;  const_cast<const QString&>(issue).at(i).isDigit();  ++i) ;
			*subVersion = issue.mid(i);
		}
	}
	return vernum;
}

/******************************************************************************
* Check from its mime type whether a file appears to be a text or image file.
* If a text file, its type is distinguished.
* Reply = file type.
*/
FileType fileType(const QString& mimetype)
{
	static const char* applicationTypes[] = {
		"x-shellscript", "x-nawk", "x-awk", "x-perl", "x-python",
		"x-desktop", "x-troff", 0 };
	static const char* formattedTextTypes[] = {
		"html", "xml", 0 };

	if (mimetype.startsWith(QLatin1String("image/")))
		return Image;
	int slash = mimetype.indexOf(QLatin1Char('/'));
	if (slash < 0)
		return Unknown;
	QString type = mimetype.mid(slash + 1);
	const char* typel = type.toLatin1();
	if (mimetype.startsWith(QLatin1String("application")))
	{
		for (int i = 0;  applicationTypes[i];  ++i)
			if (!strcmp(typel, applicationTypes[i]))
				return TextApplication;
	}
	else if (mimetype.startsWith(QLatin1String("text")))
	{
		for (int i = 0;  formattedTextTypes[i];  ++i)
			if (!strcmp(typel, formattedTextTypes[i]))
				return TextFormatted;
		return TextPlain;
	}
	return Unknown;
}

/******************************************************************************
* Display a modal dialogue to choose an existing file, initially highlighting
* any specified file.
* @param initialFile The file to initially highlight - must be a full path name or URL.
* @param defaultDir The directory to start in if @p initialFile is empty. If empty,
*                   the user's home directory will be used. Updated to the
*                   directory containing the selected file, if a file is chosen.
* @param mode OR of KFile::Mode values, e.g. ExistingOnly, LocalOnly.
* Reply = URL selected. If none is selected, URL.isEmpty() is true.
*/
QString browseFile(const QString& caption, QString& defaultDir, const QString& initialFile,
                   const QString& filter, int mode, QWidget* parent)
{
	QString initialDir = !initialFile.isEmpty() ? QString(initialFile).remove(QRegExp("/[^/]*$"))
	                   : !defaultDir.isEmpty()  ? defaultDir
	                   :                          QDir::homePath();
	KFileDialog fileDlg(initialDir, filter, parent);
	fileDlg.setOperationMode(mode & KFile::ExistingOnly ? KFileDialog::Opening : KFileDialog::Saving);
	fileDlg.setMode(KFile::File | mode);
	fileDlg.setCaption(caption);
	if (!initialFile.isEmpty())
		fileDlg.setSelection(initialFile);
	if (fileDlg.exec() != QDialog::Accepted)
		return QString();
	KUrl url = fileDlg.selectedURL();
	defaultDir = url.path();
	return url.prettyURL();
}

/******************************************************************************
*  Return the first day of the week for the user's locale.
*  Reply = 1 (Mon) .. 7 (Sun).
*/
int localeFirstDayOfWeek()
{
	static int firstDay = 0;
	if (!firstDay)
		firstDay = KGlobal::locale()->weekStartDay();
	return firstDay;
}

/******************************************************************************
*  Return the supplied string with any accelerator code stripped out.
*/
QString stripAccel(const QString& text)
{
	int len = text.length();
	QString out = text;
	QChar *corig = (QChar*)out.unicode();
	QChar *cout  = corig;
	QChar *cin   = cout;
	while (len)
	{
		if (*cin == QLatin1Char('&'))
		{
			++cin;
			--len;
			if (!len)
				break;
		}
		*cout = *cin;
		++cout;
		++cin;
		--len;
	}
	int newlen = cout - corig;
	if (newlen != out.length())
		out.truncate(newlen);
	return out;
}

} // namespace KAlarm


namespace {

/******************************************************************************
*  Tell KOrganizer to put an alarm in its calendar.
*  It will be held by KOrganizer as a simple event, without alarms - KAlarm
*  is still responsible for alarming.
*/
bool sendToKOrganizer(const KAEvent& event)
{
	KCal::Event* kcalEvent = event.event();
	QString uid = KAEvent::uid(event.id(), KAEvent::KORGANIZER);
	kcalEvent->setUid(uid);
	kcalEvent->clearAlarms();
	QString userEmail;
	switch (event.action())
	{
		case KAEvent::MESSAGE:
		case KAEvent::FILE:
		case KAEvent::COMMAND:
			kcalEvent->setSummary(event.cleanText());
			userEmail = Preferences::emailAddress();
			break;
		case KAEvent::EMAIL:
		{
			QString from = event.emailFromKMail().isEmpty()
			             ? Preferences::emailAddress()
			             : KAMail::identityManager()->identityForName(event.emailFromKMail()).fullEmailAddr();
			AlarmText atext;
			atext.setEmail(event.emailAddresses(", "), from, QString(), QString(), event.emailSubject(), QString());
			kcalEvent->setSummary(atext.displayText());
			userEmail = from;
			break;
		}
	}
	kcalEvent->setOrganizer(KCal::Person(QString(), userEmail));

	// Translate the event into string format
	KCal::ICalFormat format;
	format.setTimeZone(QString(), false);
	QString iCal = format.toICalString(kcalEvent);
kDebug(5950)<<"Korg->"<<iCal<<endl;
	delete kcalEvent;

	// Send the event to KOrganizer
	if (!runKOrganizer())     // start KOrganizer if it isn't already running
		return false;
	QByteArray  data, replyData;
	DCOPCString replyType;
	QDataStream arg(&data, QIODevice::WriteOnly);
	arg << iCal;
	if (kapp->dcopClient()->call(korganizerName, KORG_DCOP_OBJECT, "addIncidence(QString)", data, replyType, replyData)
	&&  replyType == "bool")
	{
		bool result;
		QDataStream reply(&replyData, QIODevice::ReadOnly);
		reply >> result;
		if (result)
		{
			kDebug(5950) << "sendToKOrganizer(" << uid << "): success\n";
			return true;
		}
	}
	kError(5950) << "sendToKOrganizer(): KOrganizer addEvent(" << uid << ") dcop call failed\n";
	return false;
}

/******************************************************************************
*  Tell KOrganizer to delete an event from its calendar.
*/
bool deleteFromKOrganizer(const QString& eventID)
{
	if (!runKOrganizer())     // start KOrganizer if it isn't already running
		return false;
	QString newID = KAEvent::uid(eventID, KAEvent::KORGANIZER);
	QByteArray  data, replyData;
	DCOPCString replyType;
	QDataStream arg(&data, QIODevice::WriteOnly);
	arg << newID << true;
	if (kapp->dcopClient()->call(korganizerName, KORG_DCOP_OBJECT, "deleteIncidence(QString,bool)", data, replyType, replyData)
	&&  replyType == "bool")
	{
		bool result;
		QDataStream reply(&replyData, QIODevice::ReadOnly);
		reply >> result;
		if (result)
		{
			kDebug(5950) << "deleteFromKOrganizer(" << newID << "): success\n";
			return true;
		}
	}
	kError(5950) << "sendToKOrganizer(): KOrganizer deleteEvent(" << newID << ") dcop call failed\n";
	return false;
}

} // namespace
