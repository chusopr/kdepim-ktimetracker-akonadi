/*
    This file is part of KContactManager.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "xxportmanager.h"

#include "collectionselectiondialog.h"

#include <akonadi/collection.h>
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>

#include <QtCore/QSignalMapper>
#include <QtGui/QAction>
#include <QtGui/QWidget>

XXPortManager::XXPortManager( QAbstractItemModel *collectionModel, QWidget *parent )
  : QObject( parent ), mCollectionModel( collectionModel ), mParentWidget( parent )
{
  mImportMapper = new QSignalMapper( this );
  mExportMapper = new QSignalMapper( this );

  connect( mImportMapper, SIGNAL( mapped( const QString& ) ),
           this, SLOT( slotImport( const QString& ) ) );
  connect( mExportMapper, SIGNAL( mapped( const QString& ) ),
           this, SLOT( slotExport( const QString& ) ) );
}

XXPortManager::~XXPortManager()
{
}

void XXPortManager::addImportAction( QAction *action, const QString &identifier )
{
  mImportMapper->setMapping( action, identifier );
  connect( action, SIGNAL( triggered( bool ) ), mImportMapper, SLOT( map() ) );
}

void XXPortManager::addExportAction( QAction *action, const QString &identifier )
{
  mExportMapper->setMapping( action, identifier );
  connect( action, SIGNAL( triggered( bool ) ), mExportMapper, SLOT( map() ) );
}

void XXPortManager::slotImport( const QString &identifier )
{
  const XXPort* xxport = mFactory.createXXPort( identifier, mParentWidget );

  const KABC::Addressee::List contacts = xxport->importContacts();

  delete xxport;

  if ( contacts.isEmpty() ) // nothing to import
    return;

  CollectionSelectionDialog dlg( mCollectionModel, mParentWidget );
  if ( !dlg.exec() )
    return;

  const Akonadi::Collection collection = dlg.selectedCollection();

  for ( int i = 0; i < contacts.count(); ++i ) {
    Akonadi::Item item;
    item.setPayload<KABC::Addressee>( contacts.at( i ) );
    item.setMimeType( KABC::Addressee::mimeType() );

    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection );
    job->exec();
  }
}

void XXPortManager::slotExport( const QString &identifier )
{
  const XXPort* xxport = mFactory.createXXPort( identifier, mParentWidget );

  delete xxport;
}

#include "xxportmanager.moc"
