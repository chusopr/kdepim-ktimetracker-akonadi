/*
 *  editdlg.h  -  dialogue to create or modify an alarm or alarm template
 *  Program:  kalarm
 *  (C) 2001 - 2004 by David Jarvie <software@astrojar.org.uk>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *  In addition, as a special exception, the copyright holders give permission
 *  to link the code of this program with any edition of the Qt library by
 *  Trolltech AS, Norway (or with modified versions of Qt that use the same
 *  license as Qt), and distribute linked combinations including the two.
 *  You must obey the GNU General Public License in all respects for all of
 *  the code used other than Qt.  If you modify this file, you may extend
 *  this exception to your version of the file, but you are not obligated to
 *  do so. If you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef EDITDLG_H
#define EDITDLG_H

#include <qdatetime.h>
#include <qlineedit.h>

#include <kdialogbase.h>

#include "alarmevent.h"
#include "datetime.h"

class QButton;
class QButtonGroup;
class QGroupBox;
class QComboBox;
class QHBox;
class ColourCombo;
class FontColourButton;
class ComboBox;
class ButtonGroup;
class TimeSpinBox;
class RadioButton;
class PushButton;
class CheckBox;
class TimePeriod;
class AlarmTimeWidget;
class RecurrenceEdit;
class SoundPicker;
class Reminder;
class LineEdit;
class TextEdit;

/**
 * EditAlarmDlg: A dialog for the input of an alarm's details.
 */
class EditAlarmDlg : public KDialogBase
{
		Q_OBJECT
	public:
		enum MessageType { MESSAGE, FILE };

		EditAlarmDlg(bool Template, const QString& caption, QWidget* parent = 0, const char* name = 0,
                             const KAEvent* = 0, bool readOnly = false);
		virtual ~EditAlarmDlg();
		void         getEvent(KAEvent&);
		void         setAction(KAEvent::Action, const QString& text);

		static ColourCombo* createBgColourChooser(QHBox** box, QWidget* parent, const char* name = 0);
		static CheckBox*    createConfirmAckCheckbox(QWidget* parent, const char* name = 0);
		static CheckBox*    createLateCancelCheckbox(QWidget* parent, const char* name = 0);
		static QString i18n_ConfirmAck();         // plain text of 'Confirm acknowledgement' checkbox
		static QString i18n_k_ConfirmAck();       // text of 'Confirm acknowledgement' checkbox, with 'k' shortcut
		static QString i18n_CancelIfLate();       // plain text of 'Cancel if late' checkbox
		static QString i18n_l_CancelIfLate();     // text of 'Cancel if late' checkbox, with 'L' shortcut
		static QString i18n_n_CancelIfLate();     // text of 'Cancel if late' checkbox, with 'N' shortcut
		static QString i18n_CopyEmailToSelf();    // plain text of 'Copy email to self' checkbox
		static QString i18n_e_CopyEmailToSelf();  // text of 'Copy email to self' checkbox, with 'E' shortcut
		static QString i18n_s_CopyEmailToSelf();  // text of 'Copy email to self' checkbox, with 'S' shortcut

	protected:
		virtual void resizeEvent(QResizeEvent*);
	protected slots:
		virtual void slotOk();
		virtual void slotCancel();
		virtual void slotTry();
		virtual void slotDefault();   // Load Template
	private slots:
		void         slotRecurTypeChange(int repeatType);
		void         slotRecurFrequencyChange();
		void         slotAlarmTypeClicked(int id);
		void         slotEditDeferral();
		void         slotBrowseFile();
		void         slotFontColourSelected();
		void         slotBgColourSelected(const QColor&);
		void         openAddressBook();
		void         slotAddAttachment();
		void         slotRemoveAttachment();
		void         slotShowMainPage();
		void         slotShowRecurrenceEdit();
		void         slotAnyTimeToggled(bool anyTime);
		void         slotTemplateTimeType(int id);

	private:
		void              initialise(const KAEvent*);
		void              setReadOnly();
		KAEvent::Action   getAlarmType() const;
		int               getAlarmFlags() const;
		bool              checkText(QString& result, bool showErrorMessage = true) const;
		void              setSoundPicker();
		bool              checkEmailData();

		void              initDisplayAlarms(QWidget* parent);
		void              initCommand(QWidget* parent);
		void              initEmail(QWidget* parent);
		void              saveState(const KAEvent*);
		bool              stateChanged() const;

		int               mMainPageIndex;
		int               mRecurPageIndex;
		bool              mMainPageShown;            // true once the main tab has been displayed
		bool              mRecurPageShown;           // true once the recurrence tab has been displayed
		bool              mRecurSetDefaultEndDate;   // adjust default end date/time when recurrence tab is displayed

		QButtonGroup*     mActionGroup;
		RadioButton*      mMessageRadio;
		RadioButton*      mCommandRadio;
		RadioButton*      mFileRadio;
		RadioButton*      mEmailRadio;
		QWidgetStack*     mAlarmTypeStack;

		// Templates
		QLineEdit*        mTemplateName;
		ButtonGroup*      mTemplateTimeGroup;
		RadioButton*      mTemplateDefaultTime; // no alarm time is specified
		RadioButton*      mTemplateAnyTime;     // alarms have date only, no time
		RadioButton*      mTemplateUseTime;     // an alarm time is specified
		TimeSpinBox*      mTemplateTime;        // the alarm time which is specified

		// Display alarm options widgets
		QFrame*           mDisplayAlarmsFrame;
		QHBox*            mFileBox;
		QHBox*            mFilePadding;
		SoundPicker*      mSoundPicker;
		CheckBox*         mConfirmAck;
		FontColourButton* mFontColourButton;
		ColourCombo*      mBgColourChoose;
		Reminder*         mReminder;
		bool              mReminderDeferral;
		bool              mReminderArchived;
		// Text message alarm widgets
		TextEdit*         mTextMessageEdit;    // text message edit box
		// Text file alarm widgets
		LineEdit*         mFileMessageEdit;    // text file URL edit box
		QPushButton*      mFileBrowseButton;   // text file browse button
		QString           mFileDefaultDir;     // default directory for browse button
		// Command alarm widgets
		QFrame*           mCommandFrame;
		LineEdit*         mCommandMessageEdit; // command edit box
		// Email alarm widgets
		QFrame*           mEmailFrame;
		LineEdit*         mEmailToEdit;
		QPushButton*      mEmailAddressButton; // email open address book button
		QLineEdit*        mEmailSubjectEdit;
		TextEdit*         mEmailMessageEdit;   // email body edit box
		QComboBox*        mEmailAttachList;
		QPushButton*      mEmailAddAttachButton;
		QPushButton*      mEmailRemoveButton;
		CheckBox*         mEmailBcc;
		QString           mAttachDefaultDir;

		QGroupBox*        mDeferGroup;
		QLabel*           mDeferTimeLabel;
		QPushButton*      mDeferChangeButton;

		AlarmTimeWidget*  mTimeWidget;
		CheckBox*         mLateCancel;

		QLabel*           mRecurrenceText;
		RecurrenceEdit*   mRecurrenceEdit;

		QString           mAlarmMessage;       // message text/file name/command/email message
		DateTime          mAlarmDateTime;
		DateTime          mDeferDateTime;
		EmailAddressList  mEmailAddresses;     // list of addresses to send email to
		QStringList       mEmailAttachments;   // list of email attachment file names
		QSize             mBasicSize;          // size without deferred time widget
		int               mDeferGroupHeight;   // height added by deferred time widget
		bool              mTemplate;           // editing an alarm template
		bool              mDesiredReadOnly;    // the specified read-only status of the dialogue
		bool              mReadOnly;           // the actual read-only status of the dialogue

		// Initial state of all controls
		KAEvent*          mSavedEvent;
		QString           mSavedTemplateName;   // mTemplateName value
		QButton*          mSavedTemplateTimeType; // selected ID in mTemplateTimeGroup
		QTime             mSavedTemplateTime;   // mTemplateTime value
		QButton*          mSavedTypeRadio;      // mMessageRadio, etc
		bool              mSavedBeep;           // mSoundPicker beep status
		bool              mSavedRepeatSound;    // mSoundPicker repeat status
		QString           mSavedSoundFile;      // mSoundPicker sound file
		bool              mSavedConfirmAck;     // mConfirmAck status
		QFont             mSavedFont;           // mFontColourButton font
		QColor            mSavedBgColour;       // mBgColourChoose selection
		QColor            mSavedFgColour;       // mFontColourButton foreground colour
		int               mSavedReminder;       // mReminder value
		bool              mSavedOnceOnly;       // mReminder once-only status
		QString           mSavedTextFileCommandMessage;  // mTextMessageEdit/mFileMessageEdit/mCommandMessageEdit/mEmailMessageEdit value
		QString           mSavedEmailTo;        // mEmailToEdit value
		QString           mSavedEmailSubject;   // mEmailSubjectEdit value
		QStringList       mSavedEmailAttach;    // mEmailAttachList values
		bool              mSavedEmailBcc;       // mEmailBcc status
		DateTime          mSavedDateTime;       // mTimeWidget value
		bool              mSavedLateCancel;     // mLateCancel status
		int               mSavedRecurrenceType; // RecurrenceEdit::RepeatType value
};

#endif // EDITDLG_H
