/***************************************************************************
                          knusersettings.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <kconfig.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include "knusersettings.h"
#include "utilities.h"

KNUserSettings::KNUserSettings(QWidget *p) : KNSettingsWidget(p)
{
  user=new KNUserEntry();
  QHBoxLayout *layout = new QHBoxLayout(this);
  uw=new KNUserWidget(i18n("Identity"), this, 0);
  layout->addWidget(uw);
  init();

	layout->setResizeMode(QLayout::Minimum);
}

KNUserSettings::~KNUserSettings()
{
	delete user;
}

void KNUserSettings::init()
{
	KConfig *conf=CONF();
	conf->setGroup("IDENTITY");
	user->load(conf);
	uw->setData(user);
}



void KNUserSettings::apply()
{
	KConfig *conf=CONF();
	conf->setGroup("IDENTITY");
	uw->applyData();
	user->save(conf);
}
