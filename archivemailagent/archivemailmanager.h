/*
  Copyright (c) 2012-2013 Montel Laurent <montel.org>

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

#ifndef ARCHIVEMAILMANAGER_H
#define ARCHIVEMAILMANAGER_H

#include <QObject>

class ArchiveMailKernel;
class ArchiveMailInfo;
namespace Akonadi {
  class Collection;
}

class ArchiveMailManager : public QObject
{
  Q_OBJECT
public:
  explicit ArchiveMailManager(QObject *parent = 0);
  ~ArchiveMailManager();
  void removeCollection(const Akonadi::Collection& collection);
  void backupDone(ArchiveMailInfo *info);
  void pause();
  void resume();
public Q_SLOTS:
  void load();
private:
  QList<ArchiveMailInfo *> mListArchiveInfo;
  ArchiveMailKernel *mArchiveMailKernel;
};


#endif /* ARCHIVEMAILMANAGER_H */

