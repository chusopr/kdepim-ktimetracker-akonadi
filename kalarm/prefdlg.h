/*
 *  prefdlg.h  -  program preferences dialog
 *  Program:  kalarm
 *  (C) 2001 - 2003 by David Jarvie  software@astrojar.org.uk
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
 *
 *  As a special exception, permission is given to link this program
 *  with any edition of Qt, and distribute the resulting executable,
 *  without including the source code for Qt in the source distribution.
 */

#ifndef PREFDLG_H
#define PREFDLG_H

#include <qsize.h>
#include <qdatetime.h>
#include <ktabctl.h>
#include <kdialogbase.h>

#include "recurrenceedit.h"

class QButtonGroup;
class QCheckBox;
class QRadioButton;
class QPushButton;
class QSpinBox;
class QComboBox;
class QLineEdit;
class KColorCombo;
class FontColourChooser;
class Preferences;
class TimeSpinBox;

class AppearancePrefTab;
class DefaultPrefTab;
class MiscPrefTab;


// The Preferences dialog
class KAlarmPrefDlg : public KDialogBase
{
		Q_OBJECT
	public:
		KAlarmPrefDlg(Preferences*);
		~KAlarmPrefDlg();

		AppearancePrefTab* mAppearancePage;
		DefaultPrefTab*    mDefaultPage;
		MiscPrefTab*       mMiscPage;

	protected slots:
		virtual void slotOk();
		virtual void slotApply();
		virtual void slotHelp();
		virtual void slotDefault();
		virtual void slotCancel();
};

// Base class for each tab in the Preferences dialog
class PrefsTabBase : public QWidget
{
		Q_OBJECT
	public:
		PrefsTabBase(QVBox*);

		void         setPreferences(Preferences*);
		virtual void restore() = 0;
		virtual void apply(bool syncToDisc) = 0;
		virtual void setDefaults() = 0;

	protected:
		QVBox*       mPage;
		Preferences* mPreferences;
};


// Appearance tab of the Preferences dialog
class AppearancePrefTab : public PrefsTabBase
{
		Q_OBJECT
	public:
		AppearancePrefTab(QVBox*);

		virtual void restore();
		virtual void apply(bool syncToDisc);
		virtual void setDefaults();

	private:
		FontColourChooser*  mFontChooser;
		KColorCombo*        mExpiredColour;
};


// Miscellaneous tab of the Preferences dialog
class MiscPrefTab : public PrefsTabBase
{
		Q_OBJECT
	public:
		MiscPrefTab(QVBox*);

		virtual void restore();
		virtual void apply(bool syncToDisc);
		virtual void setDefaults();

	private slots:
		void         slotRunModeToggled(bool);
		void         slotDisableIfStoppedToggled(bool);
		void         slotExpiredToggled(bool);
		void         slotEmailUseCCToggled(bool);
		void         slotClearExpired();

	private:
		void         setExpiredControls(int purgeDays);
		void         setEmailAddress(bool useControlCentre, const QString& address);

		QRadioButton*  mRunInSystemTray;
		QRadioButton*  mRunOnDemand;
		QCheckBox*     mDisableAlarmsIfStopped;
		QCheckBox*     mQuitWarn;
		QCheckBox*     mAutostartTrayIcon1;
		QCheckBox*     mAutostartTrayIcon2;
		QCheckBox*     mConfirmAlarmDeletion;
		QCheckBox*     mKeepExpired;
		QCheckBox*     mPurgeExpired;
		QSpinBox*      mPurgeAfter;
		QLabel*        mPurgeAfterLabel;
		QPushButton*   mClearExpired;
		QSpinBox*      mDaemonTrayCheckInterval;
		TimeSpinBox*   mStartOfDay;
		QButtonGroup*  mEmailClient;
		QCheckBox*     mEmailUseControlCentre;
		QLineEdit*     mEmailAddress;
};


// Edit defaults tab of the Preferences dialog
class DefaultPrefTab : public PrefsTabBase
{
		Q_OBJECT
	public:
		DefaultPrefTab(QVBox*);

		virtual void restore();
		virtual void apply(bool syncToDisc);
		virtual void setDefaults();

	private slots:
		void         slotBeepToggled(bool);
		void         slotBrowseSoundFile();

	private:
		QCheckBox*     mDefaultLateCancel;
		QCheckBox*     mDefaultConfirmAck;
		QCheckBox*     mDefaultEmailBcc;
		QCheckBox*     mDefaultBeep;
		QLabel*        mDefaultSoundFileLabel;
		QLineEdit*     mDefaultSoundFile;
		QPushButton*   mDefaultSoundFileBrowse;
		QComboBox*     mDefaultRecurPeriod;
		QComboBox*     mDefaultReminderUnits;

		static int recurIndex(RecurrenceEdit::RepeatType);
};

#endif // PREFDLG_H
