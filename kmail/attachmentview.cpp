/*
 * This file is part of KMail.
 * Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
 *
 * Parts based on KMail code by:
 * Copyright (c) 2003 Ingo Kloecker <kloecker@kde.org>
 * Copyright (c) 2007 Thomas McGuire <Thomas.McGuire@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "attachmentview.h"

#include "attachmentmodel.h"

#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QSortFilterProxyModel>

#include <KDebug>

#include <libkdepim/attachmentpart.h>
#include <boost/shared_ptr.hpp>
using KPIM::AttachmentPart;

using namespace KMail;

class KMail::AttachmentView::Private
{
  public:
    AttachmentModel *model;
};

AttachmentView::AttachmentView( AttachmentModel *model, QWidget *parent )
  : QTreeView( parent )
  , d( new Private )
{
  d->model = model;
  connect( model, SIGNAL(encryptEnabled(bool)), this, SLOT(setEncryptEnabled(bool)) );
  connect( model, SIGNAL(signEnabled(bool)), this, SLOT(setSignEnabled(bool)) );
  connect( model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(hideIfEmpty()) );
  connect( model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(hideIfEmpty()) );

  QSortFilterProxyModel *sortModel = new QSortFilterProxyModel( this );
  sortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  sortModel->setSourceModel( model );
  setModel( sortModel );

  setRootIsDecorated( false );
  setUniformRowHeights( true );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setSortingEnabled( true );
}

AttachmentView::~AttachmentView()
{
  delete d;
}

void AttachmentView::contextMenuEvent( QContextMenuEvent *event )
{
  Q_UNUSED( event );
  emit contextMenuRequested();
}

void AttachmentView::keyPressEvent( QKeyEvent *event )
{
  if( event->key() == Qt::Key_Delete ) {
    Q_ASSERT( dynamic_cast<QSortFilterProxyModel*>( model() ) );
    QSortFilterProxyModel *sortModel = static_cast<QSortFilterProxyModel*>( model() );

    // Indexes are based on row numbers, and row numbers change when items are deleted.
    // Therefore, first we need to make a list of AttachmentParts to delete.
    AttachmentPart::List toRemove;
    QModelIndexList selection = selectionModel()->selectedRows();
    foreach( const QModelIndex &index, selection ) {
      const QModelIndex sourceIndex = sortModel->mapToSource( index );
      toRemove.append( d->model->attachment( sourceIndex ) );
    }
    foreach( const AttachmentPart::Ptr &part, toRemove ) {
      d->model->removeAttachment( part );
    }
  }
}

void AttachmentView::setEncryptEnabled( bool enabled )
{
  setColumnHidden( AttachmentModel::EncryptColumn, !enabled );
}

void AttachmentView::setSignEnabled( bool enabled )
{
  setColumnHidden( AttachmentModel::SignColumn, !enabled );
}

void AttachmentView::hideIfEmpty()
{
  setVisible( d->model->rowCount() > 0 );
}

#include "attachmentview.moc"
