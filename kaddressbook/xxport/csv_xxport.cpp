/*
    This file is part of KAddressbook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qfile.h>

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kurl.h>

#include "importdialog.h"

#include "csv_xxport.h"

class CSVXXPortFactory : public XXPortFactory
{
  public:
    XXPortObject *xxportObject( KABCore *core, QObject *parent, const char *name )
    {
      return new CSVXXPort( core, parent, name );
    }
};

extern "C"
{
  void *init_libkaddrbk_csv_xxport()
  {
    return ( new CSVXXPortFactory() );
  }
}


CSVXXPort::CSVXXPort( KABCore *core, QObject *parent, const char *name )
  : XXPortObject( core, parent, name )
{
  createImportAction( i18n( "Import CSV List..." ) );
  createExportAction( i18n( "Export CSV List..." ) );
}

bool CSVXXPort::exportContacts( const KABC::AddresseeList &list, const QString& )
{
  KURL url = KFileDialog::getSaveURL( "addressbook.csv" );
  if ( url.isEmpty() )
      return true;

  if ( !url.isLocalFile() ) {
    KTempFile tmpFile;
    if ( tmpFile.status() != 0 ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b>.%2.</qt>" );
      KMessageBox::error( core(), txt.arg( url.url() )
                          .arg( strerror( tmpFile.status() ) ) );
      return false;
    }

    doExport( tmpFile.file(), list );
    tmpFile.close();

    return KIO::NetAccess::upload( tmpFile.name(), url, core() );
  } else {
    QFile file( url.fileName() );
    if ( !file.open( IO_WriteOnly ) ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b>.</qt>" );
      KMessageBox::error( core(), txt.arg( url.fileName() ) );
      return false;
    }

    doExport( &file, list );
    file.close();

    return true;
  }
}

KABC::AddresseeList CSVXXPort::importContacts( const QString& ) const
{
  ContactImportDialog dlg( core()->addressBook(), core() );

  if ( dlg.exec() )
    return dlg.contacts();
  else
    return KABC::AddresseeList();
}

void CSVXXPort::doExport( QFile *fp, const KABC::AddresseeList &list )
{
  QTextStream t( fp );

  KABC::AddresseeList::ConstIterator iter;
  KABC::Field::List fields = core()->addressBook()->fields();
  KABC::Field::List::Iterator fieldIter;
  bool first = true;

  // First output the column headings
  for ( fieldIter = fields.begin(); fieldIter != fields.end(); ++fieldIter ) {
    if ( !first )
      t << ",";

    t << "\"" << (*fieldIter)->label() << "\"";
    first = false;
  }
  t << "\n";

  // Then all the addressee objects
  KABC::Addressee addr;
  for ( iter = list.begin(); iter != list.end(); ++iter ) {
    addr = *iter;
    first = true;

    for ( fieldIter = fields.begin(); fieldIter != fields.end(); ++fieldIter ) {
      if ( !first )
        t << ",";

      t << "\"" << (*fieldIter)->value( addr ).replace( "\n", "\\n" ) << "\"";
      first = false;
    }

    t << "\n";
  }
}

#include "csv_xxport.moc"
