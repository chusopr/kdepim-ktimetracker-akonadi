/* KPilot
**
** Copyright (C) 2005 by Adriaan de Groot
**
** This file defines the factory for the recordconduit plugin.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtabwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kconfig.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>

#include "pluginfactory.h"
#include "pilotDatabase.h"
#include "recordConduit.h"

#include "setup_base.h"
#include "factory.h"
#include "settings.h"


class ConduitConfig : public ConduitConfigBase
{
Q_OBJECT
public:
	ConduitConfig(QWidget *parent=0L, const QVariantList &);
	virtual void commit();
	virtual void load();
protected:
	RecordWidget *fConfigWidget;
	KAboutData *fAbout;
} ;

#include "factory.moc"

ConduitConfig::ConduitConfig(QWidget *p, const QVariantList &args) :
	ConduitConfigBase(p,args),
	fConfigWidget(new RecordWidget(p))
{
	FUNCTIONSETUP;
	fConduitName = i18n("Record Conduit");
	fAbout = new KAboutData("recordConduit", 0,
		ki18n("Record Conduit for KPilot"),
		KPILOT_VERSION,
		ki18n("Configures the Record Conduit for KPilot"),
		KAboutData::License_GPL,
		ki18n("(C) 2005, Adriaan de Groot"));
	fAbout->addAuthor(ki18n("Adriaan de Groot"),
		ki18n("Primary Author"),
		"groot@kde.org",
		"http://people.fruitsalad.org/adridg/");

	ConduitConfigBase::addAboutPage(fConfigWidget->tabWidget,fAbout);
	fWidget=fConfigWidget;
	QObject::connect(fConfigWidget->fLogMessage,SIGNAL(textChanged(const QString&)),
		this,SLOT(modified()));
	QObject::connect(fConfigWidget->fDatabases,SIGNAL(textChanged(const QString&)),
		this,SLOT(modified()));
	QObject::connect(fConfigWidget->fFailImmediately,SIGNAL(toggled(bool)),
		this,SLOT(modified()));
}

/* virtual */ void ConduitConfig::commit()
{
	FUNCTIONSETUP;

	ConduitSettings::setLogMessage( fConfigWidget->fLogMessage->text() );
	ConduitSettings::setDatabases( fConfigWidget->fDatabases->text() );
	ConduitSettings::setFailImmediately( fConfigWidget->fFailImmediately->isChecked());
	ConduitSettings::self()->writeConfig();
	unmodified();
}

/* virtual */ void ConduitConfig::load()
{
	FUNCTIONSETUP;
	ConduitSettings::self()->readConfig();

	fConfigWidget->fLogMessage->setText( ConduitSettings::logMessage() );
	fConfigWidget->fDatabases->setText( ConduitSettings::databases().join(",") );
	fConfigWidget->fFailImmediately->setChecked( ConduitSettings::failImmediately() );

	unmodified();
}

typedef PilotDatabase PilotDatabaseContainer;

typedef RecordConduit<PilotRecord, PilotDatabaseContainer, PilotRecord, PilotAppInfoBase, NullMapper<PilotRecord> > RecordAction;

extern "C"
{

void *init_conduit_record()
{
	return new ConduitFactory<ConduitConfig,RecordAction>(0,"recordconduit");
}

}

