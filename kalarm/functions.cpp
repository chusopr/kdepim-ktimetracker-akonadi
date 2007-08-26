/*
 *  functions.cpp  -  miscellaneous functions
 *  Program:  kalarm
 *  Copyright © 2001-2007 by David Jarvie <software@astrojar.org.uk>
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

#include "kalarm.h"   //krazy:exclude=includes (kalarm.h must be first)
#include "functions.h"

#include "alarmcalendar.h"
#include "alarmevent.h"
#include "eventlistmodel.h"
#include "alarmlistview.h"
#include "alarmresources.h"
#include "daemon.h"
#include "editdlg.h"
#include "kalarmapp.h"
#include "kamail.h"
#include "mainwindow.h"
#include "messagewin.h"
#include "preferences.h"
#include "shellprocess.h"
#include "templatelistview.h"
#include "templatemenuaction.h"

#include <QDir>
#include <QRegExp>
#include <QDesktopWidget>
#include <QtDBus>

#include <kconfiggroup.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ksystemtimezone.h>
#include <KStandardGuiItem>
#include <kstandardshortcut.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kicon.h>

#include <kcal/event.h>
#include <kcal/icalformat.h>
#include <kpimidentities/identitymanager.h>
#include <kpimidentities/identity.h>
#include <kcal/person.h>
#include <ktoolinvocation.h>


namespace
{
bool          resetDaemonQueued = false;
QString       korganizerName    = "korganizer";
QString       korgStartError;
QDBusInterface* korgInterface = 0;

const char*   KORG_DBUS_WINDOW  = "MainWindow_1";
const char*   KMAIL_DBUS_WINDOW = "kmail_mainwindow1";
const char*   KMAIL_DBUS_SERVICE = "org.kde.kmail";
const char*   KORG_DBUS_SERVICE = "org.kde.korganizer";
const char*   KORG_DBUS_IFACE   = "org.kde.korganizer.Korganizer";
const char*   KORG_DBUS_OBJECT  = "/";    // D-Bus object path of KOrganizer's notification interface
const QString KORGANIZER_UID    = QString::fromLatin1("-korg");

const char*   ALARM_OPTS_FILE        = "alarmopts";
const char*   DONT_SHOW_ERRORS_GROUP = "DontShowErrors";

bool sendToKOrganizer(const KAEvent&);
bool deleteFromKOrganizer(const QString& eventID);
bool runKOrganizer();
QString uidKOrganizer(const QString& eventID);
}


namespace KAlarm
{

void doEditNewAlarm(EditAlarmDlg*);


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
* Create a New From Template QAction.
*/
TemplateMenuAction* createNewFromTemplateAction(const QString& label, KActionCollection* actions, const QString& name)
{
	return new TemplateMenuAction(KIcon(QLatin1String("new_from_template")), label, actions, name);
}

/******************************************************************************
* Add a new active (non-archived) alarm.
* Save it in the calendar file and add it to every main window instance.
* Parameters: msgParent = parent widget for any resource selection prompt or
* error message.
* 'event' is updated with the actual event ID.
*/
UpdateStatus addEvent(KAEvent& event, AlarmResource* resource, QWidget* msgParent, int options, bool showKOrgErr)
{
	kDebug(5950) << "KAlarm::addEvent():" << event.id();
	UpdateStatus status = UPDATE_OK;
	if (!theApp()->checkCalendarDaemon())    // ensure calendar is open and daemon started
		status = UPDATE_FAILED;
	else
	{
		// Save the event details in the calendar file, and get the new event ID
		AlarmCalendar* cal = AlarmCalendar::resources();
		KCal::Event* kcalEvent = cal->addEvent(event, msgParent, (options & USE_EVENT_ID), resource, (options & NO_RESOURCE_PROMPT));
		if (!kcalEvent)
			status = UPDATE_FAILED;
		else if (!cal->save())
			status = SAVE_FAILED;
		else if (status == UPDATE_OK)
		{
			if ((options & ALLOW_KORG_UPDATE)  &&  event.copyToKOrganizer())
			{
				if (!sendToKOrganizer(event))    // tell KOrganizer to show the event
					status = UPDATE_KORG_ERR;
			}

			// Update the window lists
			EventListModel::alarms()->addEvent(kcalEvent);
		}
	}

	if (status != UPDATE_OK  &&  msgParent)
		displayUpdateError(msgParent, status, ERR_ADD, 1, 1, showKOrgErr);
	return status;
}

/******************************************************************************
* Add a list of new active (non-archived) alarms.
* Save them in the calendar file and add them to every main window instance.
* The events are updated with their actual event IDs.
*/
UpdateStatus addEvents(QList<KAEvent>& events, QWidget* msgParent, bool allowKOrgUpdate, bool showKOrgErr)
{
	kDebug(5950) << "KAlarm::addEvents(" << events.count() << ")";
	if (events.isEmpty())
		return UPDATE_OK;
	int warnErr = 0;
	int warnKOrg = 0;
	UpdateStatus status = UPDATE_OK;
	AlarmResource* resource;
	if (!theApp()->checkCalendarDaemon())    // ensure calendar is open and daemon started
		status = UPDATE_FAILED;
	else
	{
		resource = AlarmResources::instance()->destination(KCalEvent::ACTIVE, msgParent);
		if (!resource)
		{
			kDebug(5950) << "KAlarm::addEvents(): no resource";
			status = UPDATE_FAILED;
		}
	}
	if (status == UPDATE_OK)
	{
		QString selectID;
		AlarmCalendar* cal = AlarmCalendar::resources();
		for (int i = 0, end = events.count();  i < end;  ++i)
		{
			// Save the event details in the calendar file, and get the new event ID
			KAEvent& event = events[i];
			KCal::Event* kcalEvent = cal->addEvent(event, msgParent, false, resource);
			if (!kcalEvent)
			{
				status = UPDATE_ERROR;
				++warnErr;
				continue;
			}
			if (allowKOrgUpdate  &&  event.copyToKOrganizer())
			{
				if (!sendToKOrganizer(event))    // tell KOrganizer to show the event
				{
					++warnKOrg;
					if (status == UPDATE_OK)
						status = UPDATE_KORG_ERR;
				}
			}

			// Update the window lists, but not yet which item is selected
			EventListModel::alarms()->addEvent(kcalEvent);
//			selectID = event.id();
		}
		if (warnErr == events.count())
			status = UPDATE_FAILED;
		else if (!cal->save())
		{
			status = SAVE_FAILED;
			warnErr = 0;    // everything failed
		}
	}

	if (status != UPDATE_OK  &&  msgParent)
		displayUpdateError(msgParent, status, ERR_ADD, (warnErr ? warnErr : events.count()), warnKOrg, showKOrgErr);
	return status;
}

/******************************************************************************
* Save the event in the archived resource and adjust every main window instance.
* The event's ID is changed to an archived ID if necessary.
*/
bool addArchivedEvent(KAEvent& event, AlarmResource* resource)
{
	kDebug(5950) << "KAlarm::addArchivedEvent(" << event.id() << ")";
	KAEvent oldEvent = event;     // so that we can reinstate the event if there's an error
	bool archiving = (event.category() == KCalEvent::ACTIVE);
	if (archiving)
	{
		if (!Preferences::archivedKeepDays())
			return false;   // expired alarms aren't being kept
		event.setCategory(KCalEvent::ARCHIVED);    // this changes the event ID
		event.setSaveDateTime(KDateTime::currentUtcDateTime());   // time stamp to control purging
	}
	AlarmCalendar* cal = AlarmCalendar::resources();
	KCal::Event* kcalEvent = cal->addEvent(event, 0, false, resource);
	if (!kcalEvent)
	{
		event = oldEvent;     // failed to add to calendar - revert event to its original state
		return false;
	}
	if (!cal->save())
		return false;

	// Update window lists
	if (!archiving)
		EventListModel::alarms()->addEvent(kcalEvent);
	else
		EventListModel::alarms()->updateEvent(kcalEvent);
	return true;
}

/******************************************************************************
* Add a new template.
* Save it in the calendar file and add it to every template list view.
* 'event' is updated with the actual event ID.
* Parameters: promptParent = parent widget for any resource selection prompt.
*/
UpdateStatus addTemplate(KAEvent& event, AlarmResource* resource, QWidget* msgParent)
{
	kDebug(5950) << "KAlarm::addTemplate():" << event.id();
	UpdateStatus status = UPDATE_OK;

	// Add the template to the calendar file
	AlarmCalendar* cal = AlarmCalendar::resources();
	KCal::Event* kcalEvent = cal->addEvent(event, msgParent, false, resource);
	if (!kcalEvent)
		status = UPDATE_FAILED;
	else if (!cal->save())
		status = SAVE_FAILED;
	else
	{
		cal->emitEmptyStatus();

		// Update the window lists
		EventListModel::templates()->addEvent(kcalEvent);
		return UPDATE_OK;
	}

	if (msgParent)
		displayUpdateError(msgParent, status, ERR_TEMPLATE, 1);
	return status;
}

/******************************************************************************
* Modify an active (non-archived) alarm in the calendar file and in every main
* window instance.
* The new event must have a different event ID from the old one.
*/
UpdateStatus modifyEvent(KAEvent& oldEvent, const KAEvent& newEvent, QWidget* msgParent, bool showKOrgErr)
{
	kDebug(5950) << "KAlarm::modifyEvent(): '" << oldEvent.id();

	UpdateStatus status = UPDATE_OK;
	if (!newEvent.valid())
	{
		deleteEvent(oldEvent, true);
		status = UPDATE_FAILED;
	}
	else
	{
		QString oldId = oldEvent.id();
		if (oldEvent.copyToKOrganizer())
		{
			// Tell KOrganizer to delete its old event.
			// But ignore errors, because the user could have manually
			// deleted it since KAlarm asked KOrganizer to set it up.
			deleteFromKOrganizer(oldId);
		}

		// Update the event in the calendar file, and get the new event ID
		AlarmCalendar* cal = AlarmCalendar::resources();
		KCal::Event* oldKcalEvent = cal->event(oldId);
		KCal::Event* kcalEvent = cal->modifyEvent(oldId, const_cast<KAEvent&>(newEvent));
		if (!kcalEvent)
			status = UPDATE_FAILED;
		else if (!cal->save())
			status = SAVE_FAILED;
		else if (status == UPDATE_OK)
		{
			if (newEvent.copyToKOrganizer())
			{
				if (!sendToKOrganizer(newEvent))    // tell KOrganizer to show the new event
					status = UPDATE_KORG_ERR;
			}

			// Remove "Don't show error messages again" for the old alarm
			setDontShowErrors(oldId);

			// Update the window lists
			EventListModel::alarms()->updateEvent(oldKcalEvent, kcalEvent);
		}
	}

	if (status != UPDATE_OK  &&  msgParent)
		displayUpdateError(msgParent, status, ERR_MODIFY, 1, 1, showKOrgErr);
	return status;
}

/******************************************************************************
* Update an active (non-archived) alarm from the calendar file and from every
* main window instance.
* The new event will have the same event ID as the old one.
* The event is not updated in KOrganizer, since this function is called when an
* existing alarm is rescheduled (due to recurrence or deferral).
*/
UpdateStatus updateEvent(KAEvent& event, QWidget* msgParent, bool archiveOnDelete, bool incRevision)
{
	kDebug(5950) << "KAlarm::updateEvent():" << event.id();

	if (!event.valid())
		deleteEvent(event, archiveOnDelete);
	else
	{
		// Update the event in the calendar file.
		if (incRevision)
			event.incrementRevision();    // ensure alarm daemon sees the event has changed
		AlarmCalendar* cal = AlarmCalendar::resources();
		KCal::Event* kcalEvent = cal->updateEvent(event);
		if (!cal->save())
		{
			if (msgParent)
				displayUpdateError(msgParent, SAVE_FAILED, ERR_ADD, 1);
			return SAVE_FAILED;
		}

		// Update the window lists
		EventListModel::alarms()->updateEvent(kcalEvent);
	}
	return UPDATE_OK;
}

/******************************************************************************
* Update a template in the calendar file and in every template list view.
* If 'selectionView' is non-null, the selection highlight is moved to the
* updated event in that listView instance.
*/
UpdateStatus updateTemplate(const KAEvent& event, QWidget* msgParent)
{
	AlarmCalendar* cal = AlarmCalendar::resources();
	KCal::Event* kcalEvent = cal->updateEvent(event);
	if (!cal->save())
	{
		if (msgParent)
			displayUpdateError(msgParent, SAVE_FAILED, ERR_TEMPLATE, 1);
		return SAVE_FAILED;
	}

	EventListModel::templates()->updateEvent(kcalEvent);
	return UPDATE_OK;
}

/******************************************************************************
* Delete alarms from the calendar file and from every main window instance.
* If the events are archived, the events' IDs are changed to archived IDs if necessary.
*/
UpdateStatus deleteEvent(KAEvent& event, bool archive, QWidget* msgParent, bool showKOrgErr)
{
	QList<KAEvent> events;
	events += event;
	return deleteEvents(events, archive, msgParent, showKOrgErr);
}

UpdateStatus deleteEvents(QList<KAEvent>& events, bool archive, QWidget* msgParent, bool showKOrgErr)
{
	kDebug(5950) << "KAlarm::deleteEvents(" << events.count() << ")";
	if (events.isEmpty())
		return UPDATE_OK;
	int warnErr = 0;
	int warnKOrg = 0;
	UpdateStatus status = UPDATE_OK;
	AlarmCalendar* cal = AlarmCalendar::resources();
	for (int i = 0, end = events.count();  i < end;  ++i)
	{
		// Save the event details in the calendar file, and get the new event ID
		KAEvent& event = events[i];
		QString id = event.id();

		// Update the window lists
		EventListModel::alarms()->removeEvent(id);

		// Delete the event from the calendar file
		if (event.category() != KCalEvent::ARCHIVED)
		{
			if (event.copyToKOrganizer())
			{
				// The event was shown in KOrganizer, so tell KOrganizer to
				// delete it. But ignore errors, because the user could have
				// manually deleted it from KOrganizer since it was set up.
				if (!deleteFromKOrganizer(id))
				{
					++warnKOrg;
					if (status == UPDATE_OK)
						status = UPDATE_KORG_ERR;
				}
			}
			if (archive  &&  event.toBeArchived())
				addArchivedEvent(event);     // this changes the event ID to an archived ID
		}
		if (!cal->deleteEvent(id, false))   // don't save calendar after deleting
		{
			status = UPDATE_ERROR;
			++warnErr;
		}

		// Remove "Don't show error messages again" for this alarm
		setDontShowErrors(id);
	}

	if (warnErr == events.count())
		status = UPDATE_FAILED;
	else if (!cal->save())      // save the calendars now
	{
		status = SAVE_FAILED;
		warnErr = events.count();
	}
	if (status != UPDATE_OK  &&  msgParent)
		displayUpdateError(msgParent, status, ERR_DELETE, warnErr, warnKOrg, showKOrgErr);
	return status;
}

/******************************************************************************
* Delete templates from the calendar file and from every template list view.
*/
UpdateStatus deleteTemplates(const QStringList& eventIDs, QWidget* msgParent)
{
	kDebug(5950) << "KAlarm::deleteTemplates(" << eventIDs.count() << ")";
	if (eventIDs.isEmpty())
		return UPDATE_OK;
	int warnErr = 0;
	UpdateStatus status = UPDATE_OK;
	AlarmCalendar* cal = AlarmCalendar::resources();
	for (int i = 0, end = eventIDs.count();  i < end;  ++i)
	{
		// Delete the template from the calendar file
		QString id = eventIDs[i];
		AlarmCalendar* cal = AlarmCalendar::resources();
		if (!cal->deleteEvent(id, false))    // don't save calendar after deleting
		{
			status = UPDATE_ERROR;
			++warnErr;
		}

		// Update the window lists
		EventListModel::templates()->removeEvent(id);
	}

	if (warnErr == eventIDs.count())
		status = UPDATE_FAILED;
	else if (!cal->save())      // save the calendars now
	{
		status = SAVE_FAILED;
		warnErr = eventIDs.count();
	}
	cal->emitEmptyStatus();
	if (status != UPDATE_OK  &&  msgParent)
		displayUpdateError(msgParent, status, ERR_TEMPLATE, warnErr);
	return status;
}

/******************************************************************************
* Delete an alarm from the display calendar.
*/
void deleteDisplayEvent(const QString& eventID)
{
	kDebug(5950) << "KAlarm::deleteDisplayEvent(" << eventID << ")";
	AlarmCalendar* cal = AlarmCalendar::displayCalendarOpen();
	if (cal)
		cal->deleteEvent(eventID, true);   // save calendar after deleting
}

/******************************************************************************
* Undelete archived alarms, and update every main window instance.
* The archive bit is set to ensure that they get re-archived if deleted again.
* 'ineligibleIDs' is filled in with the IDs of any ineligible events.
*/
UpdateStatus reactivateEvent(KAEvent& event, AlarmResource* resource, QWidget* msgParent, bool showKOrgErr)
{
	QStringList ids;
	QList<KAEvent> events;
	events += event;
	return reactivateEvents(events, ids, resource, msgParent, showKOrgErr);
}

UpdateStatus reactivateEvents(QList<KAEvent>& events, QStringList& ineligibleIDs, AlarmResource* resource, QWidget* msgParent, bool showKOrgErr)
{
	kDebug(5950) << "KAlarm::reactivateEvents(" << events.count() << ")";
	ineligibleIDs.clear();
	if (events.isEmpty())
		return UPDATE_OK;
	int warnErr = 0;
	int warnKOrg = 0;
	UpdateStatus status = UPDATE_OK;
	if (!resource)
		resource = AlarmResources::instance()->destination(KCalEvent::ACTIVE, msgParent);
	if (!resource)
	{
		kDebug(5950) << "KAlarm::reactivateEvents(): no resource";
		status = UPDATE_FAILED;
		warnErr = events.count();
	}
	else
	{
		QString selectID;
		int count = 0;
		AlarmCalendar* cal = AlarmCalendar::resources();
		KDateTime now = KDateTime::currentUtcDateTime();
		for (int i = 0, end = events.count();  i < end;  ++i)
		{
			// Delete the event from the archived resource
			KAEvent& event = events[i];
			if (event.category() != KCalEvent::ARCHIVED
			||  !event.occursAfter(now, true))
			{
				ineligibleIDs += event.id();
				continue;
			}
			++count;

			KAEvent oldEvent = event;    // so that we can reinstate the event if there's an error
			QString oldid = event.id();
			event.setCategory(KCalEvent::ACTIVE);    // this changes the event ID
			if (event.recurs()  ||  event.repeatCount())
				event.setNextOccurrence(now);   // skip any recurrences in the past
			event.setArchive();    // ensure that it gets re-archived if it is deleted

			// Save the event details in the calendar file.
			// This converts the event ID.
			KCal::Event* kcalEvent = cal->addEvent(event, msgParent, true, resource);
			if (!kcalEvent)
			{
				event = oldEvent;     // failed to add to calendar - revert event to its original state
				status = UPDATE_ERROR;
				++warnErr;
				continue;
			}
			if (event.copyToKOrganizer())
			{
				if (!sendToKOrganizer(event))    // tell KOrganizer to show the event
				{
					++warnKOrg;
					if (status == UPDATE_OK)
						status = UPDATE_KORG_ERR;
				}
			}

			// Update the window lists
			EventListModel::alarms()->updateEvent(cal->event(oldid), kcalEvent);
//			selectID = event.id();

			if (cal->event(oldid)    // no error if event doesn't exist in archived resource
			&&  !cal->deleteEvent(oldid, false))   // don't save calendar after deleting
			{
				status = UPDATE_ERROR;
				++warnErr;
			}
		}

		if (warnErr == count)
			status = UPDATE_FAILED;
		// Save the calendars, even if all events failed, since more than one calendar was updated
		if (!cal->save()  &&  status != UPDATE_FAILED)
		{
			status = SAVE_FAILED;
			warnErr = count;
		}
	}
	if (status != UPDATE_OK  &&  msgParent)
		displayUpdateError(msgParent, status, ERR_REACTIVATE, warnErr, warnKOrg, showKOrgErr);
	return status;
}

/******************************************************************************
* Enable or disable alarms in the calendar file and in every main window instance.
* The new events will have the same event IDs as the old ones.
*/
UpdateStatus enableEvents(QList<KAEvent>& events, bool enable, QWidget* msgParent)
{
	kDebug(5950) << "KAlarm::enableEvents(" << events.count() << ")";
	if (events.isEmpty())
		return UPDATE_OK;
	UpdateStatus status = UPDATE_OK;
	AlarmCalendar* cal = AlarmCalendar::resources();
	for (int i = 0, end = events.count();  i < end;  ++i)
	{
		KAEvent& event = events[i];
		if (enable != event.enabled())
		{
			event.setEnabled(enable);

			// Update the event in the calendar file
			KCal::Event* kcalEvent = cal->updateEvent(event);

			// If we're disabling a display alarm, close any message window
			if (!enable  &&  event.displayAction())
			{
				MessageWin* win = MessageWin::findEvent(event.id());
				delete win;
			}

			// Update the window lists
			EventListModel::alarms()->updateEvent(kcalEvent);
		}
	}

	if (!cal->save())
		status = SAVE_FAILED;
	if (status != UPDATE_OK  &&  msgParent)
		displayUpdateError(msgParent, status, ERR_ADD, events.count(), 0);
	return status;
}

/******************************************************************************
* This method must only be called from the main KAlarm queue processing loop,
* to prevent asynchronous calendar operations interfering with one another.
*
* Purge all archived events from the default archived alarm resource whose end
* time is longer ago than 'purgeDays'. All events are deleted if 'purgeDays' is
* zero.
*/
void purgeArchive(int purgeDays)
{
	if (purgeDays < 0)
		return;
	kDebug(5950) << "KAlarm::purgeArchive(" << purgeDays << ")";
	QDate cutoff = QDate::currentDate().addDays(-purgeDays);
	AlarmResource* resource = AlarmResources::instance()->getStandardResource(AlarmResource::ARCHIVED);
	if (!resource)
		return;
	KCal::Event::List evnts = resource->rawEvents();
	for (int i = 0;  i < evnts.count();  )
	{
		if (purgeDays  &&  evnts[i]->created().date() >= cutoff)
			evnts.removeAt(i);
		else
			EventListModel::alarms()->removeEvent(evnts[i++]);   // update the window lists
	}
	if (!evnts.isEmpty())
		AlarmCalendar::resources()->purgeEvents(evnts);   // delete the events and save the calendar
}

/******************************************************************************
* Display an error message about an error when saving an event.
*/
void displayUpdateError(QWidget* parent, UpdateStatus status, UpdateError code, int nAlarms, int nKOrgAlarms, bool showKOrgError)
{
	QString errmsg;
	if (status > UPDATE_KORG_ERR)
	{
		switch (code)
		{
			case ERR_ADD:
			case ERR_MODIFY:
				errmsg = (nAlarms > 1) ? i18nc("@info", "Error saving alarms")
				                       : i18nc("@info", "Error saving alarm");
				break;
			case ERR_DELETE:
				errmsg = (nAlarms > 1) ? i18nc("@info", "Error deleting alarms")
				                       : i18nc("@info", "Error deleting alarm");
				break;
			case ERR_REACTIVATE:
				errmsg = (nAlarms > 1) ? i18nc("@info", "Error saving reactivated alarms")
				                       : i18nc("@info", "Error saving reactivated alarm");
				break;
			case ERR_TEMPLATE:
				errmsg = (nAlarms > 1) ? i18nc("@info", "Error saving alarm templates")
				                       : i18nc("@info", "Error saving alarm template");
				break;
		}
		KMessageBox::error(parent, errmsg);
	}
	else if (showKOrgError)
		displayKOrgUpdateError(parent, code, nKOrgAlarms);
}

/******************************************************************************
* Display an error message corresponding to a specified alarm update error code.
*/
void displayKOrgUpdateError(QWidget* parent, UpdateError code, int nAlarms)
{
	QString errmsg;
	switch (code)
	{
		case ERR_ADD:
		case ERR_REACTIVATE:
			errmsg = (nAlarms > 1) ? i18nc("@info", "Unable to show alarms in KOrganizer")
			                       : i18nc("@info", "Unable to show alarm in KOrganizer");
			break;
		case ERR_MODIFY:
			errmsg = i18nc("@info", "Unable to update alarm in KOrganizer");
			break;
		case ERR_DELETE:
			errmsg = (nAlarms > 1) ? i18nc("@info", "Unable to delete alarms from KOrganizer")
			                       : i18nc("@info", "Unable to delete alarm from KOrganizer");
			break;
		case ERR_TEMPLATE:
			return;
	}
	KMessageBox::error(parent, errmsg);
}

/******************************************************************************
* Execute a New Alarm dialog for the specified alarm type.
*/
void editNewAlarm(EditAlarmDlg::Type type, QWidget* parent)
{
	EditAlarmDlg* editDlg = EditAlarmDlg::create(false, type, true, parent);
	doEditNewAlarm(editDlg);
	delete editDlg;
}

/******************************************************************************
* Execute a New Alarm dialog for the specified alarm type.
*/
void editNewAlarm(KAEvent::Action action, QWidget* parent, const AlarmText* text)
{
	bool setAction = false;
	EditAlarmDlg::Type type;
	switch (action)
	{
		case KAEvent::MESSAGE:
		case KAEvent::FILE:
			type = EditAlarmDlg::DISPLAY;
			setAction = true;
			break;
		case KAEvent::COMMAND:
			type = EditAlarmDlg::COMMAND;
			break;
		case KAEvent::EMAIL:
			type = EditAlarmDlg::EMAIL;
			break;
	}
	EditAlarmDlg* editDlg = EditAlarmDlg::create(false, type, true, parent);
	if (setAction  ||  text)
		editDlg->setAction(action, *text);
	doEditNewAlarm(editDlg);
	delete editDlg;
}

/******************************************************************************
* Execute a New Alarm dialog, optionally either presetting it to the supplied
* event, or setting the action and text.
*/
void editNewAlarm(const KAEvent& preset, QWidget* parent)
{
	EditAlarmDlg* editDlg = EditAlarmDlg::create(false, preset, true, parent);
	doEditNewAlarm(editDlg);
	delete editDlg;
}

/******************************************************************************
* Common code for editNewAlarm() variants.
*/
void doEditNewAlarm(EditAlarmDlg* editDlg)
{
	if (editDlg->exec() == QDialog::Accepted)
	{
		KAEvent event;
		AlarmResource* resource;
		editDlg->getEvent(event, resource);

		// Add the alarm to the displayed lists and to the calendar file
		if (addEvent(event, resource, editDlg) == UPDATE_KORG_ERR)
			displayKOrgUpdateError(editDlg, ERR_ADD, 1);
		Undo::saveAdd(event, resource);

		outputAlarmWarnings(editDlg, &event);
	}
}

/******************************************************************************
* Display the alarm edit dialog to edit a new alarm, preset with a template.
*/
bool editNewAlarm(const QString& templateName, QWidget* parent)
{
	if (!templateName.isEmpty())
	{
		KAEvent templateEvent = AlarmCalendar::resources()->templateEvent(templateName);
		if (templateEvent.valid())
		{
			editNewAlarm(templateEvent, parent);
			return true;
		}
		kWarning(5950) << "KAlarm::editNewAlarm(" << templateName << "): template not found";
	}
	return false;
}

/******************************************************************************
* Create a new template.
*/
void editNewTemplate(EditAlarmDlg::Type type, QWidget* parent)
{
	EditAlarmDlg* editDlg = EditAlarmDlg::create(true, type, true, parent);
	if (editDlg->exec() == QDialog::Accepted)
	{
		KAEvent event;
		AlarmResource* resource;
		editDlg->getEvent(event, resource);

		// Add the template to the displayed lists and to the calendar file
		addTemplate(event, resource, editDlg);
		Undo::saveAdd(event, resource);
	}
	delete editDlg;
}

/******************************************************************************
* Create a new template, based on an existing event or template.
*/
void editNewTemplate(const KAEvent& preset, QWidget* parent)
{
	EditAlarmDlg* editDlg = EditAlarmDlg::create(true, preset, true, parent);
	if (editDlg->exec() == QDialog::Accepted)
	{
		KAEvent event;
		AlarmResource* resource;
		editDlg->getEvent(event, resource);

		// Add the template to the displayed lists and to the calendar file
		addTemplate(event, resource, editDlg);
		Undo::saveAdd(event, resource);
	}
	delete editDlg;
}

/******************************************************************************
* Open the Edit Alarm dialog to edit the specified alarm.
*/
void editAlarm(KAEvent& event, QWidget* parent)
{
	EditAlarmDlg* editDlg = EditAlarmDlg::create(false, event, false, parent, EditAlarmDlg::RES_USE_EVENT_ID);
	if (editDlg->exec() == QDialog::Accepted)
	{
		KAEvent newEvent;
		AlarmResource* resource;
		bool changeDeferral = !editDlg->getEvent(newEvent, resource);

		// Update the event in the displays and in the calendar file
		Undo::Event undo(event, resource);
		if (changeDeferral)
		{
			// The only change has been to an existing deferral
			if (updateEvent(newEvent, editDlg, true, false) != UPDATE_OK)   // keep the same event ID
				return;   // failed to save event
		}
		else
		{
			if (modifyEvent(event, newEvent, editDlg) == UPDATE_KORG_ERR)
				displayKOrgUpdateError(editDlg, ERR_MODIFY, 1);
		}
		Undo::saveEdit(undo, newEvent);

		outputAlarmWarnings(editDlg, &newEvent);
	}
	delete editDlg;
}

/******************************************************************************
* Display the alarm edit dialog to edit a specified alarm.
*/
bool editAlarm(const QString& eventID, QWidget* parent)
{
	AlarmResources* resources = AlarmResources::instance();
	const KCal::Event* kcalEvent = resources->event(eventID);
	if (!kcalEvent)
	{
		kError(5950) << "KAlarm::editAlarm(): event ID not found:" << eventID;
		return false;
	}
	AlarmResource* resource = resources->resource(kcalEvent);
	if (!resource  ||  !resource->writable(kcalEvent))
	{
		kError(5950) << "KAlarm::editAlarm(" << eventID << "): read-only";
		return false;
	}
	switch (KCalEvent::status(kcalEvent))
	{
		case KCalEvent::ACTIVE:
		case KCalEvent::TEMPLATE:
			break;
		default:
			kError(5950) << "KAlarm::editAlarm(" << eventID << "): event not active or template";
			return false;
	}
	KAEvent event(kcalEvent);
	editAlarm(event, parent);
	return true;
}

/******************************************************************************
* Open the Edit Alarm dialog to edit the specified template.
*/
void editTemplate(KAEvent& event, QWidget* parent)
{
	EditAlarmDlg* editDlg = EditAlarmDlg::create(true, event, false, parent, EditAlarmDlg::RES_USE_EVENT_ID);
	if (editDlg->exec() == QDialog::Accepted)
	{
		KAEvent newEvent;
		AlarmResource* resource;
		editDlg->getEvent(newEvent, resource);
		QString id = event.id();
		newEvent.setEventID(id);

		// Update the event in the displays and in the calendar file
		Undo::Event undo(event, resource);
		updateTemplate(newEvent, editDlg);
		Undo::saveEdit(undo, newEvent);
	}
	delete editDlg;
}

/******************************************************************************
* Open the Edit Alarm dialog to view the specified alarm (read-only).
*/
void viewAlarm(const KAEvent& event, QWidget* parent)
{
	EditAlarmDlg* editDlg = EditAlarmDlg::create(false, event, false, parent, EditAlarmDlg::RES_PROMPT, true);
	editDlg->exec();
	delete editDlg;
}

/******************************************************************************
* Returns a list of all alarm templates.
* If shell commands are disabled, command alarm templates are omitted.
*/
QList<KAEvent> templateList()
{
	QList<KAEvent> templates;
	bool includeCmdAlarms = ShellProcess::authorised();
	KCal::Event::List events = AlarmCalendar::resources()->events(KCalEvent::TEMPLATE);
	for (int i = 0, end = events.count();  i < end;  ++i)
	{
		KCal::Event* kcalEvent = events[i];
		KAEvent event(kcalEvent);
		if (includeCmdAlarms  ||  event.action() != KAEvent::COMMAND)
			templates.append(event);
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
		KMessageBox::information(parent, i18nc("@info Please set the 'From' email address...",
		                                       "<para>%1</para><para>Please set it in the Preferences dialog.</para>", KAMail::i18n_NeedFromEmailAddress()));

	if (!Daemon::monitoringAlarms())
	{
		if (KMessageBox::warningYesNo(parent, i18nc("@info", "<para>Alarms are currently disabled.</para><para>Do you want to enable alarms now?</para>"),
		                              QString(), KGuiItem(i18nc("@action:button", "Enable")), KGuiItem(i18nc("@action:button", "Keep Disabled")),
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
	kDebug(5950) << "KAlarm::resetDaemon()";
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
		kDebug(5950) << "KAlarm::resetDaemonIfNeeded()";
		AlarmCalendar::resources()->reload();

		// Close any message windows for alarms which are now disabled
		KAEvent event;
		KCal::Event::List events = AlarmCalendar::resources()->events(KCalEvent::ACTIVE);
		for (int i = 0, end = events.count();  i < end;  ++i)
		{
			KCal::Event* kcalEvent = events[i];
			event.set(kcalEvent);
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
	QString dbusService = KMAIL_DBUS_SERVICE;
	QString errmsg;
	if (!runProgram(QLatin1String("kmail"), (minimise ? QLatin1String(KMAIL_DBUS_WINDOW) : QString()), dbusService, errmsg))
		return i18nc("@info", "Unable to start <application>KMail</application>\n(<message>%1</message>)", errmsg);
	return QString();
}

/******************************************************************************
*  Start another program for D-Bus access if it isn't already running.
*  If 'windowName' is not empty, the program's window of that name is iconised.
*  On exit, 'errorMessage' contains an error message if failure.
*  Reply = true if the program is now running.
*/
bool runProgram(const QString& program, const QString& windowName, QString& dbusService, QString& errorMessage)
{
	QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(dbusService);
	if (!reply.isValid()  ||  !reply.value())
	{
		// Program is not already running, so start it
		if (KToolInvocation::startServiceByDesktopName(program, QString(), &errorMessage, &dbusService))
		{
			kError(5950) << "KAlarm::runProgram(): couldn't start" << program << " (" << errorMessage << ")";
			return false;
		}
		if (!windowName.isEmpty())
		{
			// Minimise its window - don't use hide() since this would remove all
			// trace of it from the panel if it is not configured to be docked in
			// the system tray.
//kapp->dcopClient()->send(dcopName, windowName, "minimize()", QString());
#ifdef __GNUC__
#warning Minimise window
#endif
//			QDBusInterface iface(dbusService, DBUS_OBJECT, DBUS_IFACE);
			QDBusInterface iface(dbusService, QString(), QString());
			QDBusError err = iface.callWithArgumentList(QDBus::NoBlock, QLatin1String("minimize"), QList<QVariant>());
			if (err.isValid())
				kError(5950) << "KAlarm::runProgram(" << program << "): minimize D-Bus call failed:" << err.message();
		}
	}
	errorMessage.clear();
	return true;
}

/******************************************************************************
* The "Don't show again" option for error messages is personal to the user on a
* particular computer. For example, he may want to inhibit error messages only
* on his laptop. So the status is not stored in the alarm calendar, but in the
* user's local KAlarm data directory.
******************************************************************************/

/******************************************************************************
* Return the Don't-show-again error message tags set for a specified alarm ID.
*/
QStringList dontShowErrors(const QString& eventId)
{
	if (eventId.isEmpty())
		return QStringList();
	KConfig config(KStandardDirs::locateLocal("appdata", ALARM_OPTS_FILE));
	KConfigGroup group(&config, DONT_SHOW_ERRORS_GROUP);
	return group.readEntry(eventId, QStringList());
}

/******************************************************************************
* Check whether the specified Don't-show-again error message tag is set for an
* alarm ID.
*/
bool dontShowErrors(const QString& eventId, const QString& tag)
{
	if (tag.isEmpty()  ||  eventId.isEmpty())
		return false;
	QStringList tags = dontShowErrors(eventId);
	return tags.indexOf(tag) >= 0;
}

/******************************************************************************
* Reset the Don't-show-again error message tags for an alarm ID.
* If 'tags' is empty, the config entry is deleted.
*/
void setDontShowErrors(const QString& eventId, const QStringList& tags)
{
	if (eventId.isEmpty())
		return;
	KConfig config(KStandardDirs::locateLocal("appdata", ALARM_OPTS_FILE));
	KConfigGroup group(&config, DONT_SHOW_ERRORS_GROUP);
	if (tags.isEmpty())
		group.deleteEntry(eventId);
	else
		group.writeEntry(eventId, tags);
	group.sync();
}

/******************************************************************************
* Set the specified Don't-show-again error message tag for an alarm ID.
* Existing tags are unaffected.
*/
void setDontShowErrors(const QString& eventId, const QString& tag)
{
	if (tag.isEmpty()  ||  eventId.isEmpty())
		return;
	KConfig config(KStandardDirs::locateLocal("appdata", ALARM_OPTS_FILE));
	KConfigGroup group(&config, DONT_SHOW_ERRORS_GROUP);
	QStringList tags = group.readEntry(eventId, QStringList());
	if (tags.indexOf(tag) < 0)
	{
		tags += tag;
		group.writeEntry(eventId, tags);
		group.sync();
	}
}

/******************************************************************************
*  Read the size for the specified window from the config file, for the
*  current screen resolution.
*  Reply = true if size set in the config file, in which case 'result' is set
*        = false if no size is set, in which case 'result' is unchanged.
*/
bool readConfigWindowSize(const char* window, QSize& result, int* splitterWidth)
{
	KConfigGroup config(KGlobal::config(), window);
	QWidget* desktop = KApplication::desktop();
	QSize s = QSize(config.readEntry(QString::fromLatin1("Width %1").arg(desktop->width()), (int)0),
	                config.readEntry(QString::fromLatin1("Height %1").arg(desktop->height()), (int)0));
	if (s.isEmpty())
		return false;
	result = s;
	if (splitterWidth)
		*splitterWidth = config.readEntry(QString::fromLatin1("Splitter %1").arg(desktop->width()), -1);
	return true;
}

/******************************************************************************
*  Write the size for the specified window to the config file, for the
*  current screen resolution.
*/
void writeConfigWindowSize(const char* window, const QSize& size, int splitterWidth)
{
	KConfigGroup config(KGlobal::config(), window);
	QWidget* desktop = KApplication::desktop();
	config.writeEntry(QString::fromLatin1("Width %1").arg(desktop->width()), size.width());
	config.writeEntry(QString::fromLatin1("Height %1").arg(desktop->height()), size.height());
	if (splitterWidth >= 0)
		config.writeEntry(QString::fromLatin1("Splitter %1").arg(desktop->width()), splitterWidth);
	config.sync();
}

/******************************************************************************
* Return the current KAlarm version number.
*/
int Version()
{
	static int version = 0;
	if (!version)
		version = getVersionNumber(KALARM_VERSION);
	return version;
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
		subVersion->clear();
	int count = version.count(QChar('.')) + 1;
	if (count < 2)
		return 0;
	bool ok;
	unsigned vernum = version.section('.', 0, 0).toUInt(&ok) * 10000;  // major version
	if (!ok)
		return 0;
	unsigned v = version.section('.', 1, 1).toUInt(&ok);               // minor version
	if (!ok)
		return 0;
	vernum += (v < 99 ? v : 99) * 100;
	if (count >= 3)
	{
		// Issue number: allow other characters to follow the last digit
		QString issue = version.section('.', 2);
		int n = issue.length();
		if (!n  ||  !issue[0].isDigit())
			return 0;
		int i;
		for (i = 0;  i < n && issue[i].isDigit();  ++i) ;
		if (subVersion)
			*subVersion = issue.mid(i);
		v = issue.left(i).toUInt();   // issue number
		vernum += (v < 99 ? v : 99);
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
* Display a modal dialog to choose an existing file, initially highlighting
* any specified file.
* @param initialFile The file to initially highlight - must be a full path name or URL.
* @param defaultDir The directory to start in if @p initialFile is empty. If empty,
*                   the user's home directory will be used. Updated to the
*                   directory containing the selected file, if a file is chosen.
* @param mode OR of KFile::Mode values, e.g. ExistingOnly, LocalOnly.
* Reply = URL selected. If none is selected, URL.isEmpty() is true.
*/
QString browseFile(const QString& caption, QString& defaultDir, const QString& initialFile,
                   const QString& filter, KFile::Modes mode, QWidget* parent)
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
	KUrl url = fileDlg.selectedUrl();
	defaultDir = url.path();
	return url.prettyUrl();
}

/******************************************************************************
* Check whether a date/time is during working hours.
*/
bool isWorkingTime(const KDateTime& dt)
{
	if (!Preferences::workDays().testBit(dt.date().dayOfWeek() - 1))
		return false;
	return dt.isDateOnly()
	   ||  dt.time() >= Preferences::workDayStart()  &&  dt.time() < Preferences::workDayEnd();
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
* Return the week day name (Monday = 1).
*/
QString weekDayName(int day, const KLocale* locale)
{
	switch (day)
	{
		case 1: return ki18nc("Name of the weekday", "Monday").toString(locale);
		case 2: return ki18nc("Name of the weekday", "Tuesday").toString(locale);
		case 3: return ki18nc("Name of the weekday", "Wednesday").toString(locale);
		case 4: return ki18nc("Name of the weekday", "Thursday").toString(locale);
		case 5: return ki18nc("Name of the weekday", "Friday").toString(locale);
		case 6: return ki18nc("Name of the weekday", "Saturday").toString(locale);
		case 7: return ki18nc("Name of the weekday", "Sunday").toString(locale);
	}
	return QString();
}

/******************************************************************************
* Convert a time zone specifier string and apply it to a given date and/or time.
* The time zone specifier is a system time zone name, e.g. "Europe/London",
* "UTC" or "Clock". If no time zone is specified, it defaults to the local time
* zone.
* If 'defaultDt' is valid, it supplies the time spec and default date.
*/
KDateTime applyTimeZone(const QString& tzstring, const QDate& date, const QTime& time,
                        bool haveTime, const KDateTime& defaultDt)
{
	bool error = false;
	KDateTime::Spec spec = KDateTime::LocalZone;
	QString zone = tzstring.trimmed();
	if (defaultDt.isValid())
	{
		// Time spec is supplied - time zone specifier is not allowed
		if (!zone.isEmpty())
			error = true;
		else
			spec = defaultDt.timeSpec();
	}
	else if (!zone.isEmpty())
	{
		if (zone == QLatin1String("Clock"))
			spec = KDateTime::ClockTime;
		else if (zone == QLatin1String("UTC"))
			spec = KDateTime::UTC;
		else
		{
			KTimeZone tz = KSystemTimeZones::zone(zone);
			error = !tz.isValid();
			if (!error)
				spec = tz;
		}
	}

	KDateTime result;
	if (!error)
	{
		if (!date.isValid())
		{
			// It's a time without a date
			if (defaultDt.isValid())
			       result = KDateTime(defaultDt.date(), time, spec);
			else if (spec == KDateTime::LocalZone  ||  spec == KDateTime::ClockTime)
				result = KDateTime(QDate::currentDate(), time, spec);
		}
		else if (haveTime)
		{
			// It's a date and time
			result = KDateTime(date, time, spec);
		}
		else
		{
			// It's a date without a time
			result = KDateTime(date, spec);
		}
	}
	return result;
}

/******************************************************************************
*  Return the supplied string with any accelerator code stripped out.
*/
QString stripAccel(const QString& text)
{
	int len = text.length();
	QString out(text.unicode(), len);
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
	KCal::Event* kcalEvent = AlarmCalendar::resources()->createKCalEvent(event);
	// Change the event ID to avoid duplicating the same unique ID as the original event
	QString uid = uidKOrganizer(event.id());
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
	format.setTimeSpec(Preferences::timeZone(true));
	QString iCal = format.toICalString(kcalEvent);
	delete kcalEvent;

	// Send the event to KOrganizer
	if (!runKOrganizer())     // start KOrganizer if it isn't already running, and create its D-Bus interface
		return false;
	QList<QVariant> args;
	args << iCal;
	QDBusReply<bool> reply = korgInterface->callWithArgumentList(QDBus::Block, QLatin1String("addIncidence"), args);
	if (!reply.isValid())
	{
		kError(5950) << "sendToKOrganizer(): addIncidence(" << uid << ") D-Bus call failed:" << reply.error().message();
		return false;
	}
	if (!reply.value())
	{
		kError(5950) << "sendToKOrganizer(): addIncidence(" << uid << ") D-Bus call returned false";
		return false;
	}
	kDebug(5950) << "sendToKOrganizer(" << uid << "): success";
	return true;
}

/******************************************************************************
*  Tell KOrganizer to delete an event from its calendar.
*/
bool deleteFromKOrganizer(const QString& eventID)
{
	if (!runKOrganizer())     // start KOrganizer if it isn't already running, and create its D-Bus interface
		return false;
	QString newID = uidKOrganizer(eventID);
	QList<QVariant> args;
	args << newID << true;
	QDBusReply<bool> reply = korgInterface->callWithArgumentList(QDBus::Block, QLatin1String("deleteIncidence"), args);
	if (!reply.isValid())
	{
		kError(5950) << "deleteFromKOrganizer(): deleteIncidence(" << newID << ") D-Bus call failed:" << reply.error().message();
		return false;
	}
	if (!reply.value())
	{
		kError(5950) << "deleteFromKOrganizer(): deleteIncidence(" << newID << ") D-Bus call returned false";
		return false;
	}
	kDebug(5950) << "deleteFromKOrganizer(" << newID << "): success";
	return true;
}

/******************************************************************************
*  Start KOrganizer if not already running, and create its D-Bus interface.
*/
bool runKOrganizer()
{
	QString dbusService = KORG_DBUS_SERVICE;
	if (!KAlarm::runProgram(QLatin1String("korganizer"), KORG_DBUS_WINDOW, dbusService, korgStartError))
		return false;
	if (korgInterface  &&  !korgInterface->isValid())
	{
		delete korgInterface;
		korgInterface = 0;
	}
	if (!korgInterface)
		korgInterface = new QDBusInterface(KORG_DBUS_SERVICE, KORG_DBUS_OBJECT, KORG_DBUS_IFACE);
	return true;
}

/******************************************************************************
* Insert a KOrganizer string after the hyphen in the supplied event ID.
*/
QString uidKOrganizer(const QString& id)
{
	QString result = id;
	int i = result.lastIndexOf('-');
	if (i < 0)
		i = result.length();
	return result.insert(i, KORGANIZER_UID);
}

} // namespace
