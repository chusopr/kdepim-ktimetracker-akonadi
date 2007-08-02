/* MAL-setup.cc                      KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the setup dialog for the MAL-conduit plugin.
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
**
**
** Specific permission is granted for this code to be linked to libmal
** (this is necessary because the libmal license is not GPL-compatible).
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kaboutdata.h>
#include <kcombobox.h>
#include <knuminput.h>
#include <kpassworddialog.h>

#include <kconfig.h>


#include "mal-setup_dialog.h"

#include "mal-factory.h"
#include "mal-setup.moc"
#include "malconduitSettings.h"


MALWidgetSetup::MALWidgetSetup(QWidget *w) :
	ConduitConfigBase(w),
	fConfigWidget(new MALWidget(w))
{
	FUNCTIONSETUP;

	fConduitName=i18n("MAL");
	fAbout = new KAboutData("MALconduit", 0,
		ki18n("MAL Synchronization Conduit for KPilot"),
		KPILOT_VERSION,
		ki18n("Synchronizes the content from MAL Servers like AvantGo to the Handheld"),
		KAboutData::License_GPL,
		ki18n("(C) 2002, Reinhold Kainhofer"));
	fAbout->addAuthor(ki18n("Reinhold Kainhofer"),
		ki18n("Primary Author"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/");
	fAbout->addCredit(ki18n("Jason Day"),
		ki18n("Author of libmal and the JPilot AvantGo conduit"), "jasonday@worldnet.att.net");
	fAbout->addCredit(ki18n("Tom Whittaker"),
		ki18n("Author of syncmal"), "tom@tomw.org", "http://www.tomw.org/");
	fAbout->addCredit(ki18n("AvantGo, Inc."),
		ki18n("Authors of the malsync library (c) 1997-1999"), "", "http://www.avantgo.com/");

	ConduitConfigBase::addAboutPage(fConfigWidget->tabWidget,fAbout);
	fWidget = fConfigWidget;

	fConfigWidget->tabWidget->adjustSize();
	fConfigWidget->resize(fConfigWidget->tabWidget->size());
#define CM(a,b) connect(fConfigWidget->a,b,this,SLOT(modified()));
	CM( syncTime, SIGNAL(clicked(int)) );
	CM( proxyType, SIGNAL(clicked(int)) );

	CM( proxyServerName, SIGNAL(textChanged(const QString &)) );
	CM( proxyCustomPortCheck, SIGNAL(clicked()) );
	CM( proxyCustomPort, SIGNAL(valueChanged(int)) );
	CM( proxyUserName, SIGNAL(textChanged(const QString &)) );
	CM( proxyPassword, SIGNAL(textChanged(const QString &)) );

	CM( malServerName, SIGNAL(textChanged(const QString &)) );
	CM( malCustomPortCheck, SIGNAL(clicked()) );
	CM( malCustomPort, SIGNAL(valueChanged(int)) );
	CM( malUserName, SIGNAL(textChanged(const QString &)) );
	CM( malPassword, SIGNAL(textChanged(const QString &)) );
#undef CM
}

MALWidgetSetup::~MALWidgetSetup()
{
	FUNCTIONSETUP;
}

/* virtual */ void MALWidgetSetup::commit()
{
	FUNCTIONSETUP;

	MALConduitSettings::setSyncFrequency(
		fConfigWidget->syncTime->id(fConfigWidget->syncTime->selected()));

	// Proxy settings
	MALConduitSettings::setProxyType(
		fConfigWidget->proxyType->id(fConfigWidget->proxyType->selected()));
	MALConduitSettings::setProxyServer( fConfigWidget->proxyServerName->currentText() );

	if (fConfigWidget->proxyCustomPortCheck->isChecked() )
	{
		MALConduitSettings::setProxyPort( fConfigWidget->proxyCustomPort->text().toInt());
	}
	else
	{
		MALConduitSettings::setProxyPort(0);
	}
	MALConduitSettings::setProxyUser( fConfigWidget->proxyUserName->text() );
	MALConduitSettings::setProxyPassword( fConfigWidget->proxyPassword->text() );

	// MAL Server settings (not yet possible!!!)
	MALConduitSettings::setMALServer( fConfigWidget->malServerName->currentText() );

	if (fConfigWidget->malCustomPortCheck->isChecked() )
	{
		MALConduitSettings::setMALPort( fConfigWidget->malCustomPort->text().toInt());
	}
	else
	{
		MALConduitSettings::setMALPort(0);
	}
	MALConduitSettings::setMALUser( fConfigWidget->malUserName->text() );
	MALConduitSettings::setMALPassword( fConfigWidget->malPassword->text() );

	MALConduitSettings::self()->writeConfig();
	unmodified();
}



/* virtual */ void MALWidgetSetup::load()
{
	FUNCTIONSETUP;
	MALConduitSettings::self()->readConfig();

	fConfigWidget->syncTime->setButton( MALConduitSettings::syncFrequency() );

	// Proxy settings
	fConfigWidget->proxyType->setButton(MALConduitSettings::proxyType());
	fConfigWidget->proxyServerName->setEditText(MALConduitSettings::proxyServer());

	int proxyPortNr=MALConduitSettings::proxyPort();
	if (proxyPortNr>0 && proxyPortNr<65536)
	{
		fConfigWidget->proxyCustomPortCheck->setChecked(true);
		fConfigWidget->proxyCustomPort->setEnabled(true);
		fConfigWidget->proxyCustomPort->setText( QString::number(proxyPortNr) );
	}
	fConfigWidget->proxyUserName->setText(MALConduitSettings::proxyUser());
	fConfigWidget->proxyPassword->setText(QString::null);
	fConfigWidget->proxyPassword->insert(MALConduitSettings::proxyPassword());

#ifdef DEBUG
	DEBUGKPILOT << fname <<": Got proxy password <"
		<< MALConduitSettings::proxyPassword()
		<< "> set Text <"
		<< fConfigWidget->proxyPassword->text()
		<< "> and Pwd <"
//		<< fConfigWidget->proxyPassword->password()
		<< ">" << endl;
#endif

	// MAL Server settings (not yet possible!!!)
	fConfigWidget->malServerName->setEditText(MALConduitSettings::mALServer());

	int malPortNr=MALConduitSettings::mALPort();
	if (malPortNr>0 && malPortNr<65536)
	{
		fConfigWidget->malCustomPortCheck->setChecked(true);
		fConfigWidget->malCustomPort->setEnabled(true);
		fConfigWidget->malCustomPort->setText( QString::number(proxyPortNr) );
	}
	fConfigWidget->malUserName->setText(MALConduitSettings::mALUser());
	fConfigWidget->malPassword->setText(MALConduitSettings::mALPassword());
	unmodified();
}

/* static */ ConduitConfigBase *MALWidgetSetup::create(QWidget *w, const char *n)
{
	return new MALWidgetSetup(w);
}

