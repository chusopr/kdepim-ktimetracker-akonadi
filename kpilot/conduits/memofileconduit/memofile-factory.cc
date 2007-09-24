/* memofile-factory.cc                      KPilot
**
** Copyright (C) 2004-2007 by Jason 'vanRijn' Kasper
**
** This file defines the factory for the memofile-conduit plugin.
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
#include <kurlrequester.h>

#include "ui_setup_base.h"
#include "memofile-conduit.h"
#include "memofileSettings.h"

#include "pluginfactory.h"

class MemofileConduitConfig : public ConduitConfigBase
{
Q_OBJECT
public:
	MemofileConduitConfig(QWidget *parent=0L, const QVariantList &  = QVariantList());
	virtual void commit();
	virtual void load();
protected:
	Ui::MemofileWidget fConfigWidget;
} ;

#include "memofile-factory.moc"

MemofileConduitConfig::MemofileConduitConfig(QWidget *p, const QVariantList &) :
	ConduitConfigBase(p)
{
	FUNCTIONSETUP;

	fWidget = new QWidget();
	fConfigWidget.setupUi( fWidget );
	
	fConduitName = i18n("Memofile");
	
	KAboutData *about = new KAboutData("MemofileConduit", 0,
		ki18n("Memofile Conduit for KPilot"),
		KPILOT_VERSION,
		ki18n("Configures the Memofile Conduit for KPilot"),
		KAboutData::License_GPL,
		ki18n("(C) 2004, Jason 'vanRijn' Kasper"));
	about->addAuthor(ki18n("Jason 'vanRijn' Kasper"),
		ki18n("Primary Author"),
		"vR@movingparts.net",
		"http://www.cs.kun.nl/~adridg/kpilot");

	ConduitConfigBase::addAboutPage(fConfigWidget.tabWidget,about);

	QObject::connect(fConfigWidget.fDirectory,SIGNAL(textChanged(const QString&)),
		this,SLOT(modified()));
	QObject::connect(fConfigWidget.fSyncPrivate,SIGNAL(toggled(bool)),
					 this,SLOT(modified()));
}

/* virtual */ void MemofileConduitConfig::commit()
{
	FUNCTIONSETUP;

	DEBUGKPILOT
		<< ": Directory="
		<< fConfigWidget.fDirectory->url().path();

	MemofileConduitSettings::setDirectory( fConfigWidget.fDirectory->url().path() );
	MemofileConduitSettings::setSyncPrivate( fConfigWidget.fSyncPrivate->isChecked() );
	MemofileConduitSettings::self()->writeConfig();
	unmodified();
}

/* virtual */ void MemofileConduitConfig::load()
{
	FUNCTIONSETUP;
	MemofileConduitSettings::self()->readConfig();

	fConfigWidget.fDirectory->setUrl( MemofileConduitSettings::directory() );
	fConfigWidget.fSyncPrivate->setChecked( MemofileConduitSettings::syncPrivate() );

	DEBUGKPILOT
		<< ": Read Directory: ["
		<< fConfigWidget.fDirectory->url().path()
		<< "], sync private records: ["
		<< fConfigWidget.fSyncPrivate
		<< ']';

	unmodified();
}

DECLARE_KPILOT_PLUGIN(kpilot_conduit_memofile,MemofileConduitConfig,MemofileConduit)

