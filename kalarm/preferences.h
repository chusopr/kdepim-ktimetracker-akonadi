/*
 *  preferences.h  -  program preference settings
 *  Program:  kalarm
 *
 *  (C) 2001, 2002, 2003 by David Jarvie <software@astrojar.org.uk>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  As a special exception, permission is given to link this program
 *  with any edition of Qt, and distribute the resulting executable,
 *  without including the source code for Qt in the source distribution.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "kalarm.h"

#include <qobject.h>
#include <qcolor.h>
#include <qfont.h>
#include <qdatetime.h>
#include <qvaluelist.h>
class QWidget;

#include "colourlist.h"
#include "recurrenceedit.h"
#include "reminder.h"


// Settings configured in the Preferences dialog
class Preferences : public QObject
{
		Q_OBJECT
	public:
		enum MailClient { SENDMAIL, KMAIL };

		Preferences(QWidget* parent);

		const ColourList& messageColours() const        { return mMessageColours; }
		QColor         defaultBgColour() const          { return mDefaultBgColour; }
		const QFont&   messageFont() const              { return mMessageFont; }
		const QTime&   startOfDay() const               { return mStartOfDay; }
		bool           startOfDayChanged() const        { return mStartOfDayChanged; }
		bool           runInSystemTray() const          { return mRunInSystemTray; }
		bool           disableAlarmsIfStopped() const   { return mDisableAlarmsIfStopped; }
		bool           autostartTrayIcon() const        { return mAutostartTrayIcon; }
		bool           confirmAlarmDeletion() const     { return mConfirmAlarmDeletion; }
		bool           modalMessages() const            { return mModalMessages; }
		bool           showExpiredAlarms() const        { return mShowExpiredAlarms; }
		bool           showAlarmTime() const            { return mShowAlarmTime; }
		bool           showTimeToAlarm() const          { return mShowTimeToAlarm; }
		int            tooltipAlarmCount() const        { return mTooltipAlarmCount; }
		bool           showTooltipAlarmTime() const     { return mShowTooltipAlarmTime; }
		bool           showTooltipTimeToAlarm() const   { return mShowTooltipTimeToAlarm; }
		const QString& tooltipTimeToPrefix() const      { return mTooltipTimeToPrefix; }
		int            daemonTrayCheckInterval() const  { return mDaemonTrayCheckInterval; }
		MailClient     emailClient() const              { return mEmailClient; }
		bool           emailQueuedNotify() const        { return mEmailQueuedNotify; }
		bool           emailUseControlCentre() const    { return mEmailUseControlCentre; }
		const QString& emailAddress() const             { return mEmailAddress; }
		QColor         expiredColour() const            { return mExpiredColour; }
		int            expiredKeepDays() const          { return mExpiredKeepDays; }
		const QString& defaultSoundFile() const         { return mDefaultBeep ? QString::null : mDefaultSoundFile; }
		bool           defaultBeep() const              { return mDefaultBeep; }
		bool           defaultLateCancel() const        { return mDefaultLateCancel; }
		bool           defaultConfirmAck() const        { return mDefaultConfirmAck; }
		bool           defaultEmailBcc() const          { return mDefaultEmailBcc; }
		RecurrenceEdit::RepeatType
		               defaultRecurPeriod() const       { return mDefaultRecurPeriod; }
		Reminder::Units defaultReminderUnits() const    { return mDefaultReminderUnits; }

		void           loadPreferences();
		void           savePreferences(bool syncToDisc = true);
		void           updateStartOfDayCheck();
		void           emitPreferencesChanged();

		static void    setNotify(const QString& messageID, bool notify);
		static bool    notifying(const QString& messageID);

		static const ColourList  default_messageColours;
		static const QColor      default_defaultBgColour;
		static QFont             default_messageFont;
		static const QTime       default_startOfDay;
		static const bool        default_runInSystemTray;
		static const bool        default_disableAlarmsIfStopped;
		static const bool        default_autostartTrayIcon;
		static const bool        default_confirmAlarmDeletion;
		static const bool        default_modalMessages;
		static const bool        default_showExpiredAlarms;
		static const bool        default_showAlarmTime;
		static const bool        default_showTimeToAlarm;
		static const int         default_tooltipAlarmCount;
		static const bool        default_showTooltipAlarmTime;
		static const bool        default_showTooltipTimeToAlarm;
		static const QString     default_tooltipTimeToPrefix;
		static const int         default_daemonTrayCheckInterval;
		static const MailClient  default_emailClient;
		static const bool        default_emailQueuedNotify;
		static const bool        default_emailUseControlCentre;
		static const QString     default_emailAddress;
		static const QColor      default_expiredColour;
		static const int         default_expiredKeepDays;
		static const QString     default_defaultSoundFile;
		static const bool        default_defaultBeep;
		static const bool        default_defaultLateCancel;
		static const bool        default_defaultConfirmAck;
		static const bool        default_defaultEmailBcc;
		static const RecurrenceEdit::RepeatType
		                         default_defaultRecurPeriod;
		static const Reminder::Units
		                         default_defaultReminderUnits;

	signals:
		void preferencesChanged();
	private:
		int                 startOfDayCheck() const;
		QString             mEmailAddress;

		// All the following members are accessed by the Preferences dialog classes
		friend class MiscPrefTab;
		friend class DefaultPrefTab;
		friend class ViewPrefTab;
		friend class MessagePrefTab;
		void                setEmailAddress(bool useControlCentre, const QString& address);
		ColourList          mMessageColours;
		QColor              mDefaultBgColour;
		QFont               mMessageFont;
		QTime               mStartOfDay;
		bool                mRunInSystemTray;
		bool                mDisableAlarmsIfStopped;
		bool                mAutostartTrayIcon;
		bool                mConfirmAlarmDeletion;
		bool                mModalMessages;
		bool                mShowExpiredAlarms;
		bool                mShowAlarmTime;
		bool                mShowTimeToAlarm;
		int                 mTooltipAlarmCount;
		bool                mShowTooltipAlarmTime;
		bool                mShowTooltipTimeToAlarm;
		QString             mTooltipTimeToPrefix;
		int                 mDaemonTrayCheckInterval;
		MailClient          mEmailClient;
		bool                mEmailQueuedNotify;
		bool                mEmailUseControlCentre;
		QColor              mExpiredColour;
		int                 mExpiredKeepDays;     // 0 = don't keep, -1 = keep indefinitely
		// Default settings for Edit Alarm dialog
		QString             mDefaultSoundFile;
		bool                mDefaultBeep;
		bool                mDefaultLateCancel;
		bool                mDefaultConfirmAck;
		bool                mDefaultEmailBcc;
		RecurrenceEdit::RepeatType  mDefaultRecurPeriod;
		Reminder::Units     mDefaultReminderUnits;
		bool                mStartOfDayChanged;   // start-of-day check value doesn't tally with mStartOfDay
};

#endif // PREFERENCES_H
