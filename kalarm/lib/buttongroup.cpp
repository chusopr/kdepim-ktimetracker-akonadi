/*
 *  buttongroup.cpp  -  QButtonGroup with an extra signal and KDE 2 compatibility
 *  Program:  kalarm
 *  Copyright (C) 2002, 2004 by David Jarvie <software@astrojar.org.uk>
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

#include <qlayout.h>
#include <q3button.h>
//Added by qt3to4:
#include <QEvent>
#include <QChildEvent>
#include <QBoxLayout>
#include <kdialog.h>

#include "buttongroup.moc"


ButtonGroup::ButtonGroup(QWidget* parent, const char* name)
	: Q3ButtonGroup(parent, name)
#if QT_VERSION < 300
	, defaultAlignment(-1)
#endif
{
	connect(this, SIGNAL(clicked(int)), SIGNAL(buttonSet(int)));
}

ButtonGroup::ButtonGroup(const QString& title, QWidget* parent, const char* name)
	: Q3ButtonGroup(title, parent, name)
#if QT_VERSION < 300
	, defaultAlignment(-1)
#endif
{
	connect(this, SIGNAL(clicked(int)), SIGNAL(buttonSet(int)));
}

ButtonGroup::ButtonGroup(int strips, Qt::Orientation orient, QWidget* parent, const char* name)
	: Q3ButtonGroup(strips, orient, parent, name)
#if QT_VERSION < 300
	, defaultAlignment(orient == Qt::Horizontal ? Qt::AlignLeft : 0)
#endif
{
	connect(this, SIGNAL(clicked(int)), SIGNAL(buttonSet(int)));
}

ButtonGroup::ButtonGroup(int strips, Qt::Orientation orient, const QString& title, QWidget* parent, const char* name)
	: Q3ButtonGroup(strips, orient, title, parent, name)
#if QT_VERSION < 300
	, defaultAlignment(orient == Qt::Horizontal ? Qt::AlignLeft : 0)
#endif
{
	connect(this, SIGNAL(clicked(int)), SIGNAL(buttonSet(int)));
}

/******************************************************************************
 * Inserts a button in the group.
 * This should really be a virtual method...
 */
int ButtonGroup::insert(Q3Button* button, int id)
{
	id = Q3ButtonGroup::insert(button, id);
	connect(button, SIGNAL(toggled(bool)), SLOT(slotButtonToggled(bool)));
	return id;
}

/******************************************************************************
 * Called when one of the member buttons is toggled.
 */
void ButtonGroup::slotButtonToggled(bool)
{
	emit buttonSet(selectedId());
}

#if QT_VERSION < 300
/******************************************************************************
 * In a vertical layout, this adds widgets left aligned instead of QT 2's
 * centre aligned default.
 */
void ButtonGroup::childEvent(QChildEvent *ce)
{
	if (defaultAlignment != -1  &&  ce->inserted()  &&  ce->child()->isWidgetType())
	{
		QWidget* child = (QWidget*)ce->child();
		if (!child->isTopLevel())    //ignore dialogs etc.
		{
			((QBoxLayout*)layout())->addWidget(child, 0, defaultAlignment);
			QApplication::postEvent(this, new QEvent(QEvent::LayoutHint));
		}
	}
}
#endif
