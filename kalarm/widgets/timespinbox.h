/*
 *  timespinbox.h  -  time spinbox widget
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
 *
 *  As a special exception, permission is given to link this program
 *  with any edition of Qt, and distribute the resulting executable,
 *  without including the source code for Qt in the source distribution.
 */

#ifndef TIMESPINBOX_H
#define TIMESPINBOX_H

#include <qdatetime.h>
#include "spinbox2.h"


class TimeSpinBox : public SpinBox2
{
		Q_OBJECT
	public:
		TimeSpinBox(QWidget* parent = 0, const char* name = 0);
		TimeSpinBox(int minMinute, int maxMinute, QWidget* parent = 0, const char* name = 0);
		bool            valid() const;
		QTime           time() const;
		void            setValid(bool);
		static QString  shiftWhatsThis();
	public slots:
		virtual void    setValue(int value);
		virtual void    stepUp();
		virtual void    stepDown();
	protected:
		virtual QString mapValueToText(int v);
		virtual int     mapTextToValue(bool* ok);
	private:
		class TimeValidator;
		TimeValidator*  validator;
		int             minimumValue;
		bool            invalid;            // value is currently invalid (asterisks)
		bool            enteredSetValue;    // to prevent infinite recursion in setValue()
};

#endif // TIMESPINBOX_H
