/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sylpheed/sylpheedimportdata.h"
#include "mailimporter/filter_sylpheed.h"
#include "mailimporter/filterinfo.h"
#include "importfilterinfogui.h"

#include <KLocale>

#include <QDir>
#include <QWidget>


SylpheedImportData::SylpheedImportData(ImportMailPage*parent)
    :PimImportAbstract(parent)
{
    mPath = QDir::homePath() + QLatin1String( "/.sylpheed/" );
}

SylpheedImportData::~SylpheedImportData()
{
}

bool SylpheedImportData::foundMailer() const
{
  QDir directory( mPath );
  if ( directory.exists() )
    return true;
  return false;
}

QString SylpheedImportData::name() const
{
  return QLatin1String("Sylpheed");
}

bool SylpheedImportData::importSettings()
{
  return false;
}

bool SylpheedImportData::importMails()
{
    //* This should be usually ~/.thunderbird/xxxx.default/Mail/Local Folders/
    MailImporter::FilterInfo *info = initializeInfo();


    MailImporter::FilterSylpheed sylpheed;
    sylpheed.setFilterInfo( info );
    //info->setRootCollection( selectedCollection );    //TODO
    info->setStatusMessage(i18n("Import in progress"));
    const QString mailsPath = mPath  + QLatin1String("/Mail/Local Folders/"); //TODO
    QDir directory(mailsPath);
    if(directory.exists())
        sylpheed.importMails(mailsPath);
    else
        sylpheed.import();
    sylpheed.importMails(mailsPath);
    info->setStatusMessage(i18n("Import finished"));
    info->clear(); // Clear info from last time

    delete info;
    return true;
}

bool SylpheedImportData::importFilters()
{
  return false;
}

bool SylpheedImportData::importAddressBook()
{
  return false;
}

PimImportAbstract::TypeSupportedOptions SylpheedImportData::supportedOption()
{
  TypeSupportedOptions options;
  options |=PimImportAbstract::Mails;
  options |=PimImportAbstract::Filters;
  return options;
}
