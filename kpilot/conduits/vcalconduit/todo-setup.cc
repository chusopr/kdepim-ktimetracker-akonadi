/* KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the todo-conduit plugin.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <q3buttongroup.h>
#include <kaboutdata.h>

#include "ui_setup_base.h"
#include "todo-conduit.h"
#include "todo-setup.h"
#include "todo-setup.moc"



ToDoWidgetSetup::ToDoWidgetSetup(QWidget *w, const QVariantList &) :
	VCalWidgetSetupBase(w)
{
	FUNCTIONSETUP;
	fConduitName = i18n("To-do");
	KAboutData *fAbout = new KAboutData("todoConduit", 0,
		ki18n("To-do Conduit for KPilot"),
		KPILOT_VERSION,
		ki18n("Configures the To-do Conduit for KPilot"),
		KAboutData::License_GPL,
		ki18n("(C) 2001, Adriaan de Groot\n(C) 2002-2003, Reinhold Kainhofer"));
	fAbout->addAuthor(ki18n("Dan Pilone"),
		ki18n("Original Author"));
	fAbout->addAuthor(ki18n("Preston Brown"),
		ki18n("Original Author"));
	fAbout->addAuthor(ki18n("Herwin-Jan Steehouwer"),
		ki18n("Original Author"));
	fAbout->addAuthor(ki18n("Adriaan de Groot"),
		ki18n("Maintainer"),
		"groot@kde.org",
		"http://www.cs.kun.nl/~adridg/kpilot");
	fAbout->addAuthor(ki18n("Reinhold Kainhofer"),
		ki18n("Maintainer"),
		"reinhold@kainhofer.com",
		"http://reinhold.kainhofer.com/Linux/");

	ConduitConfigBase::addAboutPage(fConfigWidget.tabWidget,fAbout);

	fConfigWidget.fSyncDestination->setTitle(i18n("To-do Destination"));
}

ToDoWidgetSetup::~ToDoWidgetSetup()
{
	FUNCTIONSETUP;
}

/* static */ ConduitConfigBase *ToDoWidgetSetup::create(QWidget *w, const char *n)
{
	ConduitConfigBase *t = new ToDoWidgetSetup(w);
	t->setObjectName(n);
	return t;
}

VCalConduitSettings*ToDoWidgetSetup::config()
{
	return TodoConduit::theConfig();
}

