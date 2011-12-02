/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2011 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef FILTERACTIONMISSINGCOLLECTIONDIALOG_H
#define FILTERACTIONMISSINGCOLLECTIONDIALOG_H

#include <KDialog>
#include <akonadi/collection.h>

class QAbstractItemModel;
class QModelIndex;
class QListWidget;

namespace MailCommon {
  class FolderRequester;
}

class FilterActionMissingCollectionDialog : public KDialog
{
  Q_OBJECT
public:
  explicit FilterActionMissingCollectionDialog( const Akonadi::Collection::List& list, const QString & filtername, QWidget *parent = 0 );
  ~FilterActionMissingCollectionDialog();

  Akonadi::Collection selectedCollection() const;
  static Akonadi::Collection::List potentialCorrectFolders( const QString& path, bool & exactPath  );

private Q_SLOTS:
  void slotCurrentItemChanged();


private:
  static void getPotentialFolders(  const QAbstractItemModel *model, const QModelIndex& parentIndex, const QString& realPath, Akonadi::Collection::List& list );
  enum collectionEnum {
    IdentifyCollection = Qt::UserRole +1
  };

private:
  MailCommon::FolderRequester *mFolderRequester;
  QListWidget *mListwidget;

};

#endif /* FILTERACTIONMISSINGCOLLECTIONDIALOG_H */

