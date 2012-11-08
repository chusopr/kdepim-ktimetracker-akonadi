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

#ifndef IMPORTLIBREOFFICEAUTOCORRECTION_H
#define IMPORTLIBREOFFICEAUTOCORRECTION_H

#include "importabstractautocorrection.h"

class KTempDir;
class KZip;
class QDomDocument;
class QFile;
class KArchiveDirectory;

namespace MessageComposer {

class ImportLibreOfficeAutocorrection : public ImportAbstractAutocorrection
{
public:
  explicit ImportLibreOfficeAutocorrection(QWidget *parent = 0);
  ~ImportLibreOfficeAutocorrection();

  bool import(const QString& fileName, ImportAbstractAutocorrection::LoadAttribute loadAttribute = All);

private:
  enum Type {DOCUMENT, SENTENCE, WORD };

  void importAutoCorrectionFile();
  void closeArchive();
  bool loadDomElement( QDomDocument &doc, QFile *file );
  bool importFile(Type type, const KArchiveDirectory* archiveDirectory);
  KZip *mArchive;
  KTempDir *mTempDir;
};

}

#endif // IMPORTLIBREOFFICEAUTOCORRECTION_H
