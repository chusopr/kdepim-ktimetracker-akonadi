/*
 *  recurrenceedit.h  -  widget to edit the event's recurrence definition
 *  Program:  kalarm
 *  (C) 2002 by David Jarvie  software@astrojar.org.uk
 *
 *  Based on KOrganizer module koeditorrecurrence.h,
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef RECURRENCEEDIT_H
#define RECURRENCEEDIT_H

#include <qcheckbox.h>
#include <libkcal/event.h>

class QWidgetStack;
class QSpinBox;
class QComboBox;
class QRadioButton;
class DateSpinBox;
class TimeSpinBox;
class ButtonGroup;

using namespace KCal;


class RecurrenceEdit : public QWidget
{
		Q_OBJECT
	public:
		RecurrenceEdit (QWidget* parent = 0L, const char* name = 0L);
		virtual ~RecurrenceEdit()  { }

		/** Set widgets to default values */
		void         setDefaults(const QDateTime& from, bool allday);
		/** Initialise according to a specified event */
		void         set(const KAlarmEvent&, bool repeatAtLogin);
		/** Write event settings to event object */
		void         writeEvent(KAlarmEvent&);
		bool         repeatAtLogin() const  { return repeatAtLoginCheckBox->isChecked(); }
		void         setRepeatAtLogin(bool set)   { repeatAtLoginCheckBox->setChecked(set); }

		/** Check if the input is valid. */
		bool         validateInput();

	public slots:
		void         setDateTime(const QDateTime& start)   { currStartDateTime = start; }
		void         setDateTimeStr(const QString&);

	signals:
		void         dateTimesChanged(QDateTime start, QDateTime end);

	protected slots:
		void         recurToggled(bool);
		void         periodClicked(int);
		void         monthlyClicked(int);
		void         yearlyClicked(int);
		void         disableRange(bool);
		void         enableDurationRange(bool);
		void         enableDateRange(bool);

	protected:
		void         unsetAllCheckboxes();
		void         setDefaults(const QDateTime& from);
		void         checkDay(int day);
		void         getCheckedDays(QBitArray& rDays);
		void         setCheckedDays(QBitArray& rDays);

		void         initNone();
		void         initSubdaily();
		void         initDaily();
		void         initWeekly();
		void         initMonthly();
		void         initYearly();

	private:
		/* main rule box and choices. */
		QGroupBox*    ruleGroupBox;
		QGroupBox*    recurGroup;
		QFrame*       ruleFrame;
		QWidgetStack* ruleStack;
		ButtonGroup*  ruleButtonGroup;
		QCheckBox*    recurCheckBox;
		QCheckBox*    repeatAtLoginCheckBox;
		QRadioButton* subdailyButton;
		QRadioButton* dailyButton;
		QRadioButton* weeklyButton;
		QRadioButton* monthlyButton;
		QRadioButton* yearlyButton;
		int           subdailyButtonId;
		int           dailyButtonId;
		int           weeklyButtonId;
		int           monthlyButtonId;
		int           yearlyButtonId;

		QWidgetStack* recurFrequencyStack;
		QSpinBox*     recurFrequency;
		TimeSpinBox*  recurHourMinFrequency;

		QFrame*       noneFrame;

		/* weekly rule choices */
		QFrame*       weeklyFrame;
		QCheckBox*    dayBox[7];

		/* monthly rule choices */
		QFrame*       monthlyFrame;
		ButtonGroup*  monthlyButtonGroup;
		QRadioButton* onNthDayButton;
		QComboBox*    nthDayEntry;
		QRadioButton* onNthTypeOfDayButton;
		QComboBox*    nthNumberEntry;
		QComboBox*    nthTypeOfDayEntry;
		int           onNthDayButtonId;
		int           onNthTypeOfDayButtonId;

		/* yearly rule choices */
		QFrame*       yearlyFrame;
		ButtonGroup*  yearlyButtonGroup;
		QRadioButton* yearMonthButton;
		QRadioButton* yearDayButton;
		QComboBox*    yearMonthDayEntry;
		QComboBox*    yearMonthComboBox;
		QSpinBox*     yearDayEntry;
		int           yearMonthButtonId;
		int           yearDayButtonId;

		/* range stuff */
		QButtonGroup* rangeButtonGroup;
		QRadioButton* noEndDateButton;
		QRadioButton* repeatCountButton;
		QSpinBox*     repeatCountEntry;
		QLabel*       repeatCountLabel;
		QRadioButton* endDateButton;
		DateSpinBox*  endDateEdit;
		TimeSpinBox*  endTimeEdit;

		// current start date and time
		QDateTime     currStartDateTime;
};



class ButtonGroup : public QButtonGroup
{
		Q_OBJECT
	public:
		ButtonGroup(QWidget* parent, const char* name = 0)  : QButtonGroup(parent, name) { }
		ButtonGroup(int strips, Qt::Orientation o, QWidget* parent, const char* name = 0)  : QButtonGroup(strips, o, parent, name) { }
		virtual void setButton(int id)  { QButtonGroup::setButton(id);  emit clicked(id); }
};

#endif // RECURRENCEEDIT_H
