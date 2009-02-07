/*
    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "kabcmodel.h"

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kicon.h>
#include <klocale.h>
#include <akonadi/item.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>

using namespace Akonadi;

class KABCModel::Private
{
  public:
};

KABCModel::KABCModel( QObject *parent )
  : Akonadi::ItemModel( parent ),
    d( new Private() )
{
  fetchScope().fetchFullPayload();
}

KABCModel::~KABCModel()
{
  delete d;
}

int KABCModel::columnCount( const QModelIndex& ) const
{
  return 4;
}

QVariant KABCModel::data( const QModelIndex &index, int role ) const
{
  // pass through the queries for ItemModel
  if ( role == ItemModel::IdRole || role == ItemModel::MimeTypeRole )
    return ItemModel::data( index, role );

  if ( !index.isValid() )
    return QVariant();

  if ( index.row() >= rowCount() )
    return QVariant();

  const Item item = itemForIndex( index );

  if ( item.mimeType() == QLatin1String( "text/directory" ) ) {
    if ( !item.hasPayload<KABC::Addressee>() )
      return QVariant();

    const KABC::Addressee addr = item.payload<KABC::Addressee>();

    if ( role == Qt::DecorationRole ) {
      if ( index.column() == 0 ) {
        const KABC::Picture picture = addr.photo();
        if ( picture.isIntern() ) {
          return picture.data().scaled( QSize( 16, 16 ) );
        } else {
          return KIcon( QLatin1String( "x-office-contact" ) );
        }
      }
      return QVariant();
    } else if ( role == Qt::DisplayRole ) {
      switch ( index.column() ) {
        case 0:
          if ( !addr.formattedName().isEmpty() )
            return addr.formattedName();
          else
            return addr.assembledName();
        case 1:
          return addr.givenName();
          break;
        case 2:
          return addr.familyName();
          break;
        case 3:
          return addr.preferredEmail();
          break;
        default:
          break;
      }
    }
  } else if ( item.mimeType() == KABC::ContactGroup::mimeType() ) {
    if ( !item.hasPayload<KABC::ContactGroup>() )
      return QVariant();

    if ( role == Qt::DecorationRole ) {
      if ( index.column() == 0 )
        return KIcon( QLatin1String( "x-mail-distribution-list" ) );
      else
        return QVariant();
    } else if ( role == Qt::DisplayRole ) {
      switch ( index.column() ) {
        case 0:
          {
            const KABC::ContactGroup group = item.payload<KABC::ContactGroup>();
            return group.name();
          }
          break;
        default:
          break;
      }
    }
  }

  return QVariant();
}

QVariant KABCModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role != Qt::DisplayRole )
    return QVariant();

  if ( orientation != Qt::Horizontal )
    return QVariant();

  switch ( section ) {
    case 0:
      return i18nc( "@title:column, name of a person", "Name" );
      break;
    case 1:
      return KABC::Addressee::givenNameLabel();
      break;
    case 2:
      return KABC::Addressee::familyNameLabel();
      break;
    case 3:
      return KABC::Addressee::emailLabel();
      break;
    default:
      break;
  }

  return QVariant();
}
