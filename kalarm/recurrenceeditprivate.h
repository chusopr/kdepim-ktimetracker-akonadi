/*
 *  recurrenceeditprivate.h  -  private classes for recurrenceedit.cpp
 *  Program:  kalarm
 *  (C) 2003 by David Jarvie <software@astrojar.org.uk>
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
 */

#ifndef RECURRENCEEDITPRIVATE_H
#define RECURRENCEEDITPRIVATE_H

#include <qhbox.h>
class QWidget;
class SpinBox;
class TimeSpinBox;
class QString;


class RecurFrequency : public QHBox
{
		Q_OBJECT
	public:
		RecurFrequency(bool time, const QString& text, const QString& whatsThis, 
		               bool readOnly, QWidget* parent, const char* name = 0);
		int          value() const;
		void         setValue(int);
	signals:
		void         valueChanged();
	private:
		QWidget*     mSpinBox;
		SpinBox*     mIntSpinBox;
		TimeSpinBox* mTimeSpinBox;
};

#endif // RECURRENCEEDITPRIVATE_H
