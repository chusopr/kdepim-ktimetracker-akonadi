/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include "mainview.h"

#include <QtDeclarative/QDeclarativeEngine>

#include <KDebug>
#include <KGlobal>
#include <KStandardDirs>

#include <akonadi/entitytreemodel.h>

#include "notelistproxy.h"
#include <QDeclarativeContext>
#include <KMime/KMimeMessage>

using namespace Akonadi;

MainView::MainView( QWidget *parent ) : KDeclarativeMainView( "notes", new NoteListProxy( Akonadi::EntityTreeModel::UserRole ), parent )
{
  addMimeType( "text/x-vnd.akonadi.note" );
}

QString MainView::noteTitle(Entity::Id id)
{
  if ( id < 0 )
    return QString();

  QObject *itemModelObject = engine()->rootContext()->contextProperty( "itemModel").value<QObject *>();
  QAbstractItemModel *itemModel = qobject_cast<QAbstractItemModel *>( itemModelObject );
  kDebug() << itemModel << itemModelObject;
  if ( !itemModel )
    return QString();

  QModelIndexList indexList = itemModel->match(itemModel->index(0, 0), EntityTreeModel::ItemRole, QVariant::fromValue( Item( id ) ), 1, Qt::MatchRecursive );
  kDebug() << indexList;
  if ( indexList.isEmpty() )
    return QString();

  Item item = indexList.first().data( EntityTreeModel::ItemRole ).value<Item>();

  kDebug() << item.id();
  if ( !item.isValid() )
    return QString();

  if ( !item.hasPayload<KMime::Message::Ptr>() )
   return QString();

  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();

  return note->subject()->asUnicodeString();
}

QString MainView::noteContent(Entity::Id id)
{
  if ( id < 0 )
    return QString();

  QObject *itemModelObject = engine()->rootContext()->contextProperty( "itemModel").value<QObject *>();
  QAbstractItemModel *itemModel = qobject_cast<QAbstractItemModel *>( itemModelObject );
  if ( !itemModel )
    return QString();

  QModelIndexList indexList = itemModel->match(itemModel->index(0, 0), EntityTreeModel::ItemRole, QVariant::fromValue( Item( id ) ), 1, Qt::MatchRecursive );
  kDebug() << indexList;
  if ( indexList.isEmpty() )
    return QString();

  Item item = indexList.first().data( EntityTreeModel::ItemRole ).value<Item>();

  kDebug() << item.id();
  if ( !item.isValid() )
    return QString();

  if ( !item.hasPayload<KMime::Message::Ptr>() )
   return QString();

  KMime::Message::Ptr note = item.payload<KMime::Message::Ptr>();

  // TODO: Rich mimetype.
  return note->mainBodyPart()->decodedText();
}

