/*
 *  editdlg.h  -  dialogue to create or modify an alarm
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

#ifndef EDITDLG_H
#define EDITDLG_H

#include <qcheckbox.h>
#include <qdatetime.h>
#include <qradiobutton.h>

#include <kdialogbase.h>

#include "msgevent.h"
using namespace KCal;

class QButtonGroup;
class QGroupBox;
class QMultiLineEdit;
class QSpinBox;
class FontColourChooser;
class ColourCombo;
class TimeSpinBox;
class AlarmTimeWidget;
class RecurrenceEdit;

/**
 * EditAlarmDlg: A dialog for the input of an alarm's details.
 */
class EditAlarmDlg : public KDialogBase
{
		Q_OBJECT
	public:
		enum MessageType { MESSAGE, FILE };

		EditAlarmDlg(const QString& caption, QWidget* parent = 0L, const char* name = 0L,
                             const KAlarmEvent* = 0L);
		virtual ~EditAlarmDlg();

		void              getEvent(KAlarmEvent&);

	protected:
		virtual void showEvent(QShowEvent*);
		virtual void resizeEvent(QResizeEvent*);
	protected slots:
		virtual void slotOk();
		virtual void slotCancel();
		virtual void slotTry();
		void         slotRepeatTypeChange(int repeatType);
		void         slotMessageTypeClicked(int id);
		void         slotMessageTextChanged();
		void         slotRecurrenceResized(QSize old, QSize New);
		void         slotEditDeferral();
		void         slotSoundToggled(bool on);
		void         slotPickSound();

	private:
		KAlarmAlarm::Type getAlarmType() const;
		int               getAlarmFlags() const;
		bool              checkText(QString& result);
		QString           getMessageText();
		void              enableMessageControls(bool);
		void              setSoundPicker();

		QButtonGroup*    actionGroup;
		QRadioButton*    messageRadio;
		QRadioButton*    commandRadio;
		QRadioButton*    fileRadio;
#ifdef KALARM_EMAIL
		QRadioButton*    emailRadio;
#endif
		QPushButton*     browseButton;
		QMultiLineEdit*  messageEdit;     // alarm message edit box
		QGroupBox*       deferGroup;
		QLabel*          deferTimeLabel;
		AlarmTimeWidget* timeWidget;
		RecurrenceEdit*  recurrenceEdit;
		QCheckBox*       lateCancel;
		QCheckBox*       sound;
		QPushButton*     soundPicker;
		QCheckBox*       confirmAck;
#ifdef KALARM_EMAIL
		QCheckBox*       emailBcc;
#endif
#ifdef SELECT_FONT
		FontColourChooser* fontColour;
#else
		ColourCombo*     bgColourChoose;
#endif
		QString          alarmMessage;
		QDateTime        alarmDateTime;
		QDateTime        deferDateTime;
		QString          soundFile;        // sound file to play when alarm is triggered, or null for beep
		QString          multiLineText;    // message text before single-line mode was selected
		QSize            basicSize;        // size without recurrence edit or deferred time widgets
		int              deferGroupHeight; // height added by deferred time widget
		bool             alarmAnyTime;     // alarmDateTime is only a date, not a time
		bool             singleLineOnly;   // no multi-line text input allowed
		bool             timeDialog;       // the dialog shows date/time fields only
		bool             shown;            // the dialog has already been displayed
};

#endif // EDITDLG_H
