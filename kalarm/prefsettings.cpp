/*
 *  prefsettings.cpp  -  program preference settings
 *  Program:  kalarm
 *
 *  (C) 2001 by David Jarvie  software@astrojar.org.uk
 *
 *  This program is free software; you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option) any later version.                                   *
 */

#include <kglobal.h>
#include <kconfig.h>

#include "prefsettings.h"
#include "prefsettings.moc"


SettingsBase::SettingsBase(QWidget* parent)
	: QObject(parent)
{
}

SettingsBase::~SettingsBase()
{
}

void SettingsBase::loadSettings()
{
	emitSettingsChanged();
}

void SettingsBase::saveSettings()
{
}

void SettingsBase::emitSettingsChanged()
{
	emit settingsChanged();
}



const QColor GeneralSettings::default_defaultBgColour(red);
const QFont GeneralSettings::default_messageFont(QString::fromLatin1("Helvetica"), 16, QFont::Bold);

GeneralSettings::GeneralSettings(QWidget* parent)
	: SettingsBase(parent)
{
	loadSettings();
}

GeneralSettings::~GeneralSettings()
{
}

void GeneralSettings::loadSettings()
{
	KConfig* config = KGlobal::config();
	config->setGroup(QString::fromLatin1("General"));
	m_defaultBgColour = config->readColorEntry(QString::fromLatin1("Message background colour"), &default_defaultBgColour);
	m_messageFont = config->readFontEntry(QString::fromLatin1("Message font"), &default_messageFont);
	SettingsBase::loadSettings();
}

void GeneralSettings::saveSettings()
{
	KConfig* config = KGlobal::config();
	config->setGroup(QString::fromLatin1("General"));
	config->writeEntry(QString::fromLatin1("Message background colour"), m_defaultBgColour);
	config->writeEntry(QString::fromLatin1("Message font"), m_messageFont);
	SettingsBase::saveSettings();
}
