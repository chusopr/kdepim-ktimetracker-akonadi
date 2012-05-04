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

#include "archivemailmanager.h"
#include "archivemailinfo.h"
#include <KConfigGroup>
#include <KSharedConfig>
#include <KGlobal>
ArchiveMailManager::ArchiveMailManager(QObject *parent)
  : QObject( parent )
{
}

ArchiveMailManager::~ArchiveMailManager()
{
}

void ArchiveMailManager::load()
{
  KSharedConfig::Ptr config = KGlobal::config();
  const QStringList collectionList = config->groupList().filter( QRegExp( "ArchiveMailCollection \\d+" ) );
  qDebug()<<"collectionList "<<collectionList;
  const int numberOfCollection = collectionList.count();
  for(int i = 0 ; i < numberOfCollection; ++i) {
    KConfigGroup group = config->group(collectionList.at(i));
    ArchiveMailInfo *info = new ArchiveMailInfo(group);
    //TODO
  }

//test if necessary to archive.
}

#include "archivemailmanager.moc"
