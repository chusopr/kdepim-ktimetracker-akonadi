/*
    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

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

#include <QtCore/QFile>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include <libakonadi/collectionlistjob.h>
#include <libakonadi/collectionmodifyjob.h>
#include <libakonadi/itemappendjob.h>
#include <libakonadi/itemfetchjob.h>
#include <libakonadi/itemstorejob.h>
#include <libakonadi/session.h>

#include <klocale.h>

#include "knutresource.h"

using namespace Akonadi;

KnutResource::KnutResource( const QString &id )
  : ResourceBase( id )
{
  mDataFile = settings()->value( "General/DataFile" ).toString();
}

KnutResource::~KnutResource()
{
}

void KnutResource::configure()
{
}

void KnutResource::aboutToQuit()
{
}

bool KnutResource::setConfiguration( const QString &config )
{
  mDataFile = config;
  settings()->setValue( "General/DataFile", mDataFile );

  return true;
}

QString KnutResource::configuration() const
{
  return mDataFile;
}

bool KnutResource::requestItemDelivery( const DataReference &ref, int type, const QDBusMessage &msg )
{
  Q_UNUSED( type );

  const QString remoteId = ref.externalUrl().toString();

  QMutableMapIterator<QString, CollectionEntry> it( mCollections );
  while ( it.hasNext() ) {
    const CollectionEntry &entry = it.value();

    QByteArray data;

    if ( entry.addressees.contains( remoteId ) )
      data = mVCardConverter.createVCard( entry.addressees.value( remoteId ) );

    if ( entry.incidences.contains( remoteId ) )
      data = mICalConverter.toString( entry.incidences.value( remoteId ) ).toUtf8();

    if ( !data.isEmpty() ) {
      ItemStoreJob *job = new ItemStoreJob( ref, session() );
      job->setData( data );

      return deliverItem( job, msg );
    }
  }

  return false;
}

void KnutResource::itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
{
  if ( !mCollections.contains( collection.remoteId() ) )
    return;

  CollectionEntry &entry = mCollections[ collection.remoteId() ];

  if ( item.mimeType() == QLatin1String( "text/vcard" ) ) {
    const KABC::Addressee addressee = mVCardConverter.parseVCard( item.data() );
    if ( !addressee.isEmpty() )
      entry.addressees.insert( addressee.uid(), addressee );
  } else if ( item.mimeType() == QLatin1String( "text/calendar" ) ) {
    KCal::Incidence *incidence = mICalConverter.fromString( QString::fromUtf8( item.data() ) );
    if ( incidence ) {
      entry.incidences.insert( incidence->uid(), incidence );
    }
  }
}

void KnutResource::itemChanged( const Akonadi::Item &item )
{
  Q_UNUSED( item );
}

void KnutResource::itemRemoved( const Akonadi::DataReference &ref )
{
  Q_UNUSED( ref );
}

void KnutResource::retrieveCollections()
{
  Akonadi::Collection::List collections;
  QMapIterator<QString, CollectionEntry> it( mCollections );
  while ( it.hasNext() ) {
    it.next();
    collections.append( it.value().collection );
  }

  collectionsRetrieved( collections );
}

void KnutResource::synchronizeCollection( const Akonadi::Collection &collection )
{
  ItemFetchJob *fetch = new ItemFetchJob( collection, session() );
  if ( !fetch->exec() ) {
    changeStatus( Error, i18n( "Unable to fetch listing of collection '%1': %2", collection.name(), fetch->errorString() ) );
    return;
  }

  changeProgress( 0 );

  Item::List items = fetch->items();

  KABC::Addressee::List addressees;
  QMapIterator<QString, CollectionEntry> it( mCollections );
  while ( it.hasNext() ) {
    if ( it.value().collection.id() == collection.id() ) {
      addressees = it.value().addressees.values();
      break;
    }
  }

  int counter = 0;
  foreach ( KABC::Addressee addressee, addressees ) {
    QString uid = addressee.uid();

    bool found = false;
    foreach ( Item item, items ) {
      if ( item.reference().externalUrl().toString() == uid ) {
        found = true;
        break;
      }
    }

    if ( found )
      continue;

    ItemAppendJob *append = new ItemAppendJob( collection, "text/vcard", session() );
    append->setRemoteId( uid );
    if ( !append->exec() ) {
      changeProgress( 0 );
      changeStatus( Error, i18n( "Appending new contact failed: %1", append->errorString() ) );
      return;
    }

    counter++;
    int percentage = (counter * 100) / addressees.count();
    changeProgress( percentage );
  }

  collectionSynchronized();
}

bool KnutResource::loadData()
{
  mCollections.clear();

  QFile file( mDataFile );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    changeStatus( Error, "Unable to open data file" );
    return false;
  }

  QDomDocument document;
  if ( !document.setContent( &file, true ) ) {
    changeStatus( Error, "Unable to parse data file" );
    return false;
  }

  QDomElement element = document.documentElement();
  if ( element.tagName() != QLatin1String( "knut" ) ) {
    changeStatus( Error, "Data file has invalid format" );
    return false;
  }

  Collection parentCollection;
  parentCollection.setParent( Collection::root() );
  parentCollection.setRemoteId( mDataFile );
  parentCollection.setName( name() );

  CollectionEntry entry;
  entry.collection = parentCollection;

  element = element.firstChildElement();
  while ( !element.isNull() ) {
    if ( element.tagName() == QLatin1String( "collection" ) ) {
      addCollection( element, parentCollection );
    } else if ( element.tagName() == QLatin1String( "item" ) ) {
      if ( element.attribute( "mimetype" ) == QLatin1String( "text/vcard" ) )
        addAddressee( element, entry );
      else if ( element.attribute( "mimetype" ) == QLatin1String( "text/calendar" ) )
        addIncidence( element, entry );
    }

    element = element.nextSiblingElement();
  }

  mCollections.insert( entry.collection.remoteId(), entry );

  return true;
}

void KnutResource::addCollection( const QDomElement &element, const Akonadi::Collection &parentCollection )
{
  Collection collection;
  collection.setParent( parentCollection );
  collection.setName( element.attribute( "name" ) );
  collection.setRemoteId( mDataFile + "#" + element.attribute( "name" ) );
  collection.setContentTypes( element.attribute( "mimetypes" ).split( ";", QString::SkipEmptyParts ) );

  CollectionEntry entry;
  entry.collection = collection;

  QDomElement childElement = element.firstChildElement();
  while ( !childElement.isNull() ) {
    if ( childElement.tagName() == QLatin1String( "collection" ) ) {
      addCollection( childElement, collection );
    } else if ( childElement.tagName() == QLatin1String( "item" ) ) {
      if ( childElement.attribute( "mimetype" ) == QLatin1String( "text/x-vcard" ) )
        addAddressee( childElement, entry );
      else if ( childElement.attribute( "mimetype" ) == QLatin1String( "text/x-calendar" ) )
        addIncidence( childElement, entry );
    }

    childElement = childElement.nextSiblingElement();
  }

  mCollections.insert( entry.collection.remoteId(), entry );
}

void KnutResource::addAddressee( const QDomElement &element, CollectionEntry &entry )
{
  const QString data = element.text();

  KABC::Addressee addressee = mVCardConverter.parseVCard( data.toUtf8() );
  if ( !addressee.isEmpty() )
    entry.addressees.insert( addressee.uid(), addressee );
}

void KnutResource::addIncidence( const QDomElement &element, CollectionEntry &entry )
{
  const QString data = element.text();

  KCal::Incidence *incidence = mICalConverter.fromString( data );
  if ( incidence )
    entry.incidences.insert( incidence->uid(), incidence );
}

#include "knutresource.moc"
