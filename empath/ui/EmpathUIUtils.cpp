/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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
*/

// KDE includes
#include <kiconloader.h>
#include <kapp.h>
#include <kconfig.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathConfig.h"
#include "Empath.h"

	QPixmap
empathMimeIcon(const QString & name)
{
	QPixmap p(kapp->kde_datadir() + "/empath/pics/mime/" + name);
	return p;
}

	QPixmap
empathIcon(const QString & name)
{
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	QString iconSet = c->readEntry(EmpathConfig::KEY_ICON_SET, "8bit");
	QPixmap p(kapp->kde_datadir() + "/empath/pics/" + iconSet + "/" + name);
	return p;
}

	QFont
empathFixedFont()
{
	return kapp->fixedFont();
}

	QFont
empathGeneralFont()
{
	return kapp->generalFont();
}
	
	QColor
empathTextColour()
{
	return qApp->palette()->color(QPalette::Normal, QColorGroup::Text);
}

	QColor
empathWindowColour()
{
	return qApp->palette()->color(QPalette::Normal, QColorGroup::Base);
}
