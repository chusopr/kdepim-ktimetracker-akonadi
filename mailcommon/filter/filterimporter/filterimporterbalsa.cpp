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

#include "filterimporterbalsa_p.h"
#include "filtermanager.h"
#include "mailfilter.h"

#include <KConfig>
#include <KConfigGroup>

#include <KDebug>

#include <QFile>

using namespace MailCommon;

FilterImporterBalsa::FilterImporterBalsa( QFile *file )
  :FilterImporterAbstract()
{
  KConfig config( file->fileName() );
  const QStringList filterList = config.groupList().filter( QRegExp( "filter-\\d+" ) );
  Q_FOREACH(const QString &filter, filterList) {
    KConfigGroup grp = config.group(filter);
    addFilter(grp);
  }
}

FilterImporterBalsa::~FilterImporterBalsa()
{
}

QString FilterImporterBalsa::defaultPath()
{
  return QString::fromLatin1( "%1/.balsa/config" ).arg( QDir::homePath() );
}


void FilterImporterBalsa::addFilter(const KConfigGroup &grp)
{
  MailCommon::MailFilter *filter = new MailCommon::MailFilter();
  const QString name = grp.readEntry(QLatin1String("Name"));
  filter->pattern()->setName( name );
  filter->setToolbarName( name );

  const QString condition = grp.readEntry(QLatin1String("Condition"));
  const QString sound = grp.readEntry(QLatin1String("Sound"));
  const QString popupText = grp.readEntry(QLatin1String("Popup-text"));
  const int actionType = grp.readEntry(QLatin1String("Action-type"),-1);
  const QString actionStr = grp.readEntry(QLatin1String("Action-string"));

  //TODO

  appendFilter(filter);
}
