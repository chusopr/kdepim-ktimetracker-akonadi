/***************************************************************************
 *   Copyright (C) 2006 by Andreas Gungl <a.gungl@gmx.de>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "datastore.h"
#include "persistentsearchmanager.h"

#include <QSqlQuery>
#include <QSqlField>
#include <QSqlDriver>
#include <QVariant>
#include <QThread>
#include <QUuid>
#include <QString>
#include <QStringList>


using namespace Akonadi;

/***************************************************************************
 *   DataStore                                                           *
 ***************************************************************************/
DataStore::DataStore()
{
  m_database = QSqlDatabase::addDatabase( "QSQLITE", QUuid::createUuid().toString() );
  m_database.setDatabaseName( "akonadi.db" );
  m_dbOpened = m_database.open();

  if ( !m_dbOpened )
    debugLastDbError( "Cannot open database." );
  else
    qDebug() << "Database akonadi.db opened.";
}

DataStore::~DataStore()
{
  m_database.close();
}

/* -- High level API -- */
const CollectionList Akonadi::DataStore::listCollections( const QByteArray & prefix,
                                                          const QByteArray & mailboxPattern ) const
{
  CollectionList result;

  if ( mailboxPattern.isEmpty() )
    return result;

  // normalize path and pattern
  QByteArray sanitizedPattern( mailboxPattern );
  QByteArray fullPrefix( prefix );
  const bool hasPercent = mailboxPattern.contains('%');
  const bool hasStar = mailboxPattern.contains('*');
  const int endOfPath = mailboxPattern.lastIndexOf('/') + 1;

  if ( mailboxPattern[0] != '/' && fullPrefix != "/" )
    fullPrefix += "/";
  fullPrefix += mailboxPattern.left( endOfPath );

  if ( hasPercent )
    sanitizedPattern = "%";
  else if ( hasStar )
    sanitizedPattern = "*";
  else
    sanitizedPattern = mailboxPattern.mid( endOfPath );

  qDebug() << "FullPrefix: " << fullPrefix << " pattern: " << sanitizedPattern;

  if ( fullPrefix == "/" ) {
    // list resources and queries
    const QList<Resource> resources = listResources();
    foreach ( Resource r, resources ) {
      Collection c( r.resource() );
      c.setNoSelect( true );
      result.append( c );
    }

    CollectionList persistenSearches = PersistentSearchManager::self()->collections();
    if ( !persistenSearches.isEmpty() )
      result.append( Collection( "Search" ) );

  } else if ( fullPrefix == "/Search/" ) {
    result += PersistentSearchManager::self()->collections();
  } else {
    const QByteArray resource = fullPrefix.mid( 1, fullPrefix.indexOf( '/', 1 ) - 1 );
    qDebug() << "Listing folders in resource: " << resource;
    Resource r = resourceByName( resource );
    const QList<Location> locations = listLocations( r );

    foreach( Location l, locations ) {
      const QString location = "/" + resource + l.location();
      //qDebug() << "Location: " << location << " " << resource << " prefix: " << fullPrefix;
      const bool atFirstLevel = location.lastIndexOf( '/' ) == fullPrefix.lastIndexOf( '/' );
      if ( location.startsWith( fullPrefix ) ) {
        if ( hasStar || ( hasPercent && atFirstLevel ) ||
             location == fullPrefix + sanitizedPattern ) {
          Collection c( location.right( location.size() -1 ) );
          c.setMimeTypes( MimeType::asCommaSeparatedString( l.mimeTypes() ) );
          result.append( c );
        }
      }
    }
  }

  return result;
}

/* --- Flag ---------------------------------------------------------- */
bool DataStore::appendFlag( const QString & name )
{
  if ( !m_dbOpened )
    return false;

  int foundRecs = 0;
  QSqlQuery query( m_database );

  query.prepare( "SELECT COUNT(*) FROM Flags WHERE name = :name" );
  query.bindValue( ":name", name );
  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during check before insertion of Flag." );
    return false;
  }

  if (query.next())
    foundRecs = query.value(0).toInt();

  if ( foundRecs > 0 ) {
    qDebug() << "Cannot insert flag " << name
             << " because it already exists.";
    return false;
  }

  query.prepare( "INSERT INTO Flags (name) VALUES (:name)" );
  query.bindValue( ":name", name );

  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during insertion of single Flag." );
    return false;
  }

  return true;
}

bool DataStore::removeFlag( const Flag & flag )
{
  return removeCachePolicy( flag.id() );
}

bool DataStore::removeFlag( int id )
{
  return removeById( id, "Flags" );
}

Flag DataStore::flagById( int id )
{
  if ( !m_dbOpened )
    return Flag();

  QSqlQuery query( m_database );
//  query.prepare( "SELECT id, name FROM Flags WHERE id = :id" );
//  query.bindValue( ":id", id );
  const QString statement = QString( "SELECT id, name FROM Flags WHERE id = %1" ).arg( id );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during selection of single Flag." );
    return Flag();
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during selection of single Flag." );
    return Flag();
  }

  int flagId = query.value( 0 ).toInt();
  QString name = query.value( 1 ).toString();

  return Flag( flagId, name );
}

Flag DataStore::flagByName( const QString &name )
{
  if ( !m_dbOpened )
    return Flag();

  QSqlQuery query( m_database );
//  query.prepare( "SELECT id, name FROM Flags WHERE name = :id" );
//  query.bindValue( ":id", id );
  const QString statement = QString( "SELECT id, name FROM Flags WHERE name = \"%1\"" ).arg( name );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during selection of single Flag." );
    return Flag();
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during selection of single Flag." );
    return Flag();
  }

  int id = query.value( 0 ).toInt();
  QString flagName = query.value( 1 ).toString();

  return Flag( id, flagName );
}

QList<Flag> DataStore::listFlags()
{
  QList<Flag> list;

  if ( !m_dbOpened )
    return list;

  QSqlQuery query( m_database );
  if ( !query.exec( "SELECT id, name FROM Flags" ) ) {
    debugLastQueryError( query, "Error during selection of Flags." );
    return list;
  }

  while ( query.next() ) {
    int id = query.value( 0 ).toInt();
    QString name = query.value( 1 ).toString();
    list.append( Flag( id, name ) );
  }

  return list;
}

/* --- ItemFlags ----------------------------------------------------- */

bool DataStore::setItemFlags( const PimItem &item, const QList<Flag> &flags )
{
  if ( !m_dbOpened )
    return false;

  QSqlQuery query( m_database );

  // first delete all old flags of this pim item
  const QString statement = QString( "DELETE FROM ItemFlags WHERE pimitem_id = %1" ).arg( item.id() );
  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during deletion of ItemFlags." );
    return false;
  }

  // then add the new flags
  for ( int i = 0; i < flags.count(); ++i ) {
    const QString statement = QString( "INSERT INTO ItemFlags (flag_id, pimitem_id) VALUES ( '%1', '%2' )" )
                                     .arg( flags[ i ].name() ).arg( item.id() );
    if ( !query.exec( statement ) ) {
      debugLastQueryError( query, "Error during adding new ItemFlags." );
      return false;
    }
  }

  return true;
}

bool DataStore::appendItemFlags( const PimItem &item, const QList<Flag> &flags )
{
  if ( !m_dbOpened )
    return false;

  QSqlQuery query( m_database );

  for ( int i = 0; i < flags.count(); ++i ) {
    const QString statement = QString( "SELECT COUNT(*) FROM ItemFlags WHERE flag_id = %1 AND pimitem_id = %2" )
                                     .arg( flags[ i ].id() ).arg( item.id() );
    if ( !query.exec( statement ) ) {
      debugLastQueryError( query, "Error during select on ItemFlags." );
      return false;
    }

    query.next();
    int exists = query.value( 0 ).toInt();

    if ( !exists ) {
      const QString statement = QString( "INSERT INTO ItemFlags (flag_id, pimitem_id) VALUES ( '%1', '%2' )" )
                                       .arg( flags[ i ].id(), item.id() );
      if ( !query.exec( statement ) ) {
        debugLastQueryError( query, "Error during adding new ItemFlags." );
        return false;
      }
    }
  }

  return true;
}

bool DataStore::removeItemFlags( const PimItem &item, const QList<Flag> &flags )
{
  if ( !m_dbOpened )
    return false;

  QSqlQuery query( m_database );

  for ( int i = 0; i < flags.count(); ++i ) {
    const QString statement = QString( "DELETE FROM ItemFlags WHERE flag_id = %1 AND pimitem_id = %2" )
                                     .arg( flags[ i ].id() ).arg( item.id() );
    if ( !query.exec( statement ) ) {
      debugLastQueryError( query, "Error during deletion of ItemFlags." );
      return false;
    }
  }

  return true;
}

QList<Flag> DataStore::itemFlags( const PimItem &item )
{
  QList<Flag> flags;

  if ( !m_dbOpened )
    return flags;

  QSqlQuery query( m_database );
  const QString statement = QString( "SELECT Flags.id, Flags.name FROM Flags, ItemFlags WHERE Flags.id = ItemFlags.flag_id AND ItemFlags.pimitem_id = %1" )
                                   .arg( item.id() );
  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during selection of ItemFlags." );
    return flags;
  }

  while (query.next()) {
    int id = query.value( 0 ).toInt();
    QString name = query.value( 1 ).toString();
    flags.append( Flag( id, name ) );
  }

  return flags;
}

/* --- CachePolicy --------------------------------------------------- */
bool DataStore::appendCachePolicy( const QString & policy )
{
  if ( !m_dbOpened )
    return false;

  int foundRecs = 0;
  QSqlQuery query( m_database );

  query.prepare( "SELECT COUNT(*) FROM CachePolicies WHERE name = :name" );
  query.bindValue( ":name", policy );

  if ( !query.exec() ) {
    return false;
    debugLastQueryError( query, "Error during check before insertion of CachePolicy." );
  }

  if ( !query.next() ) {
    return false;
    debugLastQueryError( query, "Error during check before insertion of CachePolicy." );
  }

  foundRecs = query.value( 0 ).toInt();

  if ( foundRecs > 0 ) {
    qDebug() << "Cannot insert policy " << policy
             << " because it already exists.";
    return false;
  }

  query.prepare( "INSERT INTO CachePolicies (name) VALUES (:name)" );
  query.bindValue( ":name", policy );

  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during insertion of single CachePolicy." );
    return false;
  }

  return true;
}

bool DataStore::removeCachePolicy( const CachePolicy & policy )
{
  return removeCachePolicy( policy.id() );
}

bool DataStore::removeCachePolicy( int id )
{
  return removeById( id, "CachePolicies" );
}

CachePolicy DataStore::cachePolicyById( int id )
{
  if ( !m_dbOpened )
    return CachePolicy();

  QSqlQuery query( m_database );
  query.prepare( "SELECT id, name FROM CachePolicies WHERE id = :id" );
  query.bindValue( ":id", id );

  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during selection of single CachePolicy." );
    return CachePolicy();
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during selection of single CachePolicy." );
    return CachePolicy();
  }

  int policyId = query.value( 0 ).toInt();
  QString policy = query.value( 1 ).toString();

  return CachePolicy( policyId, policy );
}

QList<CachePolicy> DataStore::listCachePolicies()
{
  if ( !m_dbOpened )
    return QList<CachePolicy>();

  QSqlQuery query( m_database );
  if ( !query.exec( "SELECT id, name FROM CachePolicies" ) ) {
    debugLastQueryError( query, "Error during selection of CachePolicies." );
    return QList<CachePolicy>();
  }

  QList<CachePolicy> list;
  while ( query.next() ) {
    int id = query.value( 0 ).toInt();
    QString policy = query.value( 1 ).toString();

    list.append( CachePolicy( id, policy ) );
  }

  return list;
}

/* --- ItemMetaData--------------------------------------------------- */
/*
bool appendItemMetaData( const QString & metadata, const MetaType & metatype );
bool removeItemMetaData( const ItemMetaData & metadata );
bool removeItemMetaData( int id );
ItemMetaData * itemMetaDataById( int id );
QList<ItemMetaData> * listItemMetaData();
QList<ItemMetaData> * listItemMetaData( const MetaType & metatype );
*/

/* --- Location ------------------------------------------------------ */
bool DataStore::appendLocation( const QString & location,
                                const Resource & resource,
                                int *insertId )
{
  if ( !m_dbOpened )
    return false;

  int foundRecs = 0;
  QSqlQuery query( m_database );

  if ( !query.exec( "SELECT COUNT(*) FROM Locations WHERE uri = \"" + location + "\"" ) ) {
    debugLastQueryError( query, "Error during check before insertion of Location." );
    return false;
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during check before insertion of Location." );
    return false;
  }

  foundRecs = query.value( 0 ).toInt();

  if ( foundRecs > 0 ) {
    qDebug() << "Cannot insert location " << location
             << " because it already exists.";
    return false;
  }

  QString statement = QString( "INSERT INTO Locations (uri, cachepolicy_id, "
                               "resource_id, exists_count, recent_count, "
                               "unseen_count, first_unseen, uid_validity) "
                               "VALUES (\"%1\", NULL, \"%2\", 0, 0, 0, 0, 0 )" )
                               .arg( location ).arg( resource.id() );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during insertion of single Location." );
    return false;
  }

  if ( insertId ) {
    QVariant v = query.lastInsertId();
    if ( v.isValid() )
      *insertId = v.toInt();
  }

  return true;
}

bool DataStore::appendLocation( const QString & location,
                                const Resource & resource,
                                const CachePolicy & policy )
{
  if ( !m_dbOpened )
    return false;

  int foundRecs = 0;
  QSqlQuery query( m_database );

  query.prepare( "SELECT COUNT(*) FROM Locations WHERE uri = :uri" );
  query.bindValue( ":uri", location );
  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during check before insertion of Location." );
    return false;
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during check before insertion of Location." );
    return false;
  }

  foundRecs = query.value( 0 ).toInt();

  if ( foundRecs > 0 ) {
    qDebug() << "Cannot insert location " << location
             << " because it already exists.";
    return false;
  }

  query.prepare( "INSERT INTO Locations (uri, cachepolicy_id, resource_id) "
                 "VALUES (:uri, :policy, :resource)" );
  query.bindValue( ":uri", location );
  query.bindValue( ":policy", policy.id() );
  query.bindValue( ":resource", resource.id() );
  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during insertion of single Location." );
    return false;
  }

  return true;
}

bool DataStore::removeLocation( const Location & location )
{
  return removeLocation( location.id() );
}

bool DataStore::removeLocation( int id )
{
  return removeById( id, "Locations" );
}

static void addToUpdateAssignments( QStringList & l, int change, const QString & name )
{
    if ( change > 0 )
        // return a = a + n
        l.append( name + " = " + name + " + " + QString::number( change ) );
    else if ( change < 0 )
        // return a = a - |n|
        l.append( name + " = " + name + " - " + QString::number( -change ) );
}

bool DataStore::updateLocationCounts( const Location & location, int existsChange,
                                      int recentChange, int unseenChange )
{
    if ( existsChange == 0 && recentChange == 0 && unseenChange == 0 )
        return true; // there's nothing to do

    QSqlQuery query( m_database );
    if ( m_dbOpened ) {
        QStringList assignments;
        addToUpdateAssignments( assignments, existsChange, "exists_count" );
        addToUpdateAssignments( assignments, recentChange, "recent_count" );
        addToUpdateAssignments( assignments, unseenChange, "unseen_count" );
        QString q = QString( "UPDATE Locations SET %1 WHERE id = \"%2\"" )
                    .arg( assignments.join(",") )
                    .arg( location.id() );
        qDebug() << "Executing SQL query " << q;
        if ( query.exec( q ) )
            return true;
        else
            debugLastQueryError( query, "Error while updating the counts of a single Location." );
    }
    return false;
}

bool DataStore::changeLocationPolicy( const Location & location,
                                      const CachePolicy & policy )
{
  if ( !m_dbOpened )
    return false;

  QSqlQuery query( m_database );

  query.prepare( "UPDATE Locations SET cachepolicy_id = :policy WHERE id = :id" );
  query.bindValue( ":policy", policy.id() );
  query.bindValue( ":id", location.id() );
  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during setting the cache policy of a single Location." );
    return false;
  }

  return true;
}

bool DataStore::resetLocationPolicy( const Location & location )
{
  if ( !m_dbOpened )
    return false;

  QSqlQuery query( m_database );

  query.prepare( "UPDATE Locations SET cachepolicy_id = NULL WHERE id = :id" );
  query.bindValue( ":id", location.id() );
  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during reset of the cache policy of a single Location." );
    return false;
  }

  return true;
}

Location DataStore::locationById( int id ) const
{
  if ( !m_dbOpened )
    return Location();

  QSqlQuery query( m_database );
  query.prepare( "SELECT id, uri, cachepolicy_id, resource_id FROM Locations WHERE id = :id" );
  query.bindValue( ":id", id );
  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during selection of single Location." );
    return Location();
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during selection of single Location." );
    return Location();
  }

  int locationId = query.value( 0 ).toInt();
  QString uri = query.value( 1 ).toString();
  int policy = query.value( 2 ).toInt();
  int resource = query.value( 3 ).toInt();

  return Location( locationId, uri, policy, resource );
}

Location DataStore::locationByRawMailbox( const QByteArray& mailbox ) const
{
  int secondSlash = mailbox.indexOf( '/', 2 );
  // qDebug() << "Select: " << mailbox.mid( secondSlash, mailbox.size() - secondSlash )
  //         << " in resource: " << mailbox.left( secondSlash );

  Resource resource = resourceByName( mailbox.left( secondSlash ) );

  return locationByName( resource, mailbox.mid( secondSlash, mailbox.size() - secondSlash ) );
}

QList<MimeType> DataStore::mimeTypesForLocation( int id ) const
{
  if ( !m_dbOpened )
    return QList<MimeType>();

  QSqlQuery query( m_database );
  const QString statement = QString( "SELECT mimetype_id FROM LocationMimeTypes WHERE location_id = %1" ).arg( id );
  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during selection of Locations." );
    return QList<MimeType>();
  }

  QList<MimeType> mimeTypes;
  while ( query.next() ) {
    int id = query.value( 0 ).toInt();
    MimeType mimeType = mimeTypeById( id );

    if ( mimeType.isValid() )
      mimeTypes.append( mimeType );
  }

  return mimeTypes;
}

bool DataStore::appendMimeTypeForLocation( int locationId, const QString & mimeType )
{
  //qDebug() << "DataStore::appendMimeTypeForLocation( " << locationId << ", '" << mimeType << "' )";
  int mimeTypeId;
  MimeType m = mimeTypeByName( mimeType );
  if ( !m.isValid() ) {
    // the MIME type doesn't exist, so we have to add it to the db
    if ( !appendMimeType( mimeType, &mimeTypeId ) )
      return false;
  } else {
    mimeTypeId = m.id();
  }

  return appendMimeTypeForLocation( locationId, mimeTypeId );
}

bool DataStore::appendMimeTypeForLocation( int locationId, int mimeTypeId )
{
  if ( !m_dbOpened )
    return false;

  //qDebug() << "DataStore::appendMimeTypeForLocation( " << locationId << ", '" << mimeTypeId << "' )";
  QSqlQuery query( m_database );

  QString statement = QString( "SELECT COUNT(*) FROM LocationMimeTypes WHERE location_id = \"%1\" AND mimetype_id = \"%2\"" )
                             .arg( locationId ).arg( mimeTypeId );

  if ( !query.exec( statement ) ) {
    debugLastDbError( "Error during check before insertion of LocationMimeType." );
    return false;
  }

  if ( !query.next() ) {
    debugLastDbError( "Error during check before insertion of LocationMimeType." );
    return false;
  }

  int foundRecs = query.value( 0 ).toInt();

  if ( foundRecs > 0 ) {
    qDebug() << "Cannot insert location-mime type ( " << locationId
             << ", " << mimeTypeId << " ) because it already exists.";
    return false;
  }

  statement = QString( "INSERT INTO LocationMimeTypes (location_id, mimetype_id) VALUES (\"%1\", \"%2\")" )
                     .arg( locationId ).arg( mimeTypeId );

  if ( !query.exec( statement ) ) {
    debugLastDbError( "Error during insertion of single LocationMimeType." );
    return false;
  }

  return true;
}


QList<Location> DataStore::listLocations() const
{
  if ( !m_dbOpened )
    return QList<Location>();

  QSqlQuery query( m_database );
  const QString statement = QString( "SELECT id, uri, cachepolicy_id, resource_id, exists_count, recent_count, unseen_count, first_unseen, uid_validity FROM Locations" );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during selection of Locations." );
    return QList<Location>();
  }

  QList<Location> locations;

  while ( query.next() ) {
    Location location;
    location.fillFromQuery( query );
    location.setMimeTypes( mimeTypesForLocation( location.id() ) );
    locations.append( location );
  }

  return locations;
}

QList<Location> DataStore::listLocations( const Resource & resource ) const
{
  if ( !m_dbOpened || !resource.isValid() )
    return QList<Location>();

  QSqlQuery query( m_database );
  // query.prepare( "SELECT id, uri, cachepolicy_id, resource_id FROM Locations WHERE resource_id = :id" );
  // query.bindValue( ":id", resource.id() );

  const QString statement = QString( "SELECT id, uri, cachepolicy_id, resource_id FROM Locations WHERE resource_id = %1" )
                                   .arg( resource.id() );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during selection of Locations from a Resource." );
    return QList<Location>();
  }

  QList<Location> locations;
  while ( query.next() ) {
    Location location;
    location.fillFromQuery( query );
    location.setMimeTypes( mimeTypesForLocation( location.id() ) );
    locations.append( location );
  }

  return locations;
}

/* --- MimeType ------------------------------------------------------ */
bool DataStore::appendMimeType( const QString & mimetype, int *insertId )
{
  if ( !m_dbOpened )
    return false;

  QSqlQuery query( m_database );

  const QString statement = QString( "SELECT COUNT(*) FROM MimeTypes WHERE mime_type = \"%1\"" )
                                   .arg( mimetype );
  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during check before insertion of MimeType." );
    return false;
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during check before insertion of MimeType." );
    return false;
  }

  int foundRecs = query.value( 0 ).toInt();

  if ( foundRecs > 0 ) {
    qDebug() << "Cannot insert mimetype " << mimetype
             << " because it already exists.";
    return false;
  }

  query.prepare( "INSERT INTO MimeTypes (mime_type) VALUES (:type)" );
  query.bindValue( ":type", mimetype );

  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during insertion of single MimeType." );
    return false;
  }

  if ( insertId ) {
    QVariant v = query.lastInsertId();
    if ( v.isValid() )
      *insertId = v.toInt();
  }

  return true;
}

bool DataStore::removeMimeType( const MimeType & mimetype )
{
  return removeMimeType( mimetype.id() );
}

bool DataStore::removeMimeType( int id )
{
  return removeById( id, "MimeTypes" );
}

MimeType DataStore::mimeTypeById( int id ) const
{
  if ( !m_dbOpened )
    return MimeType();

  QSqlQuery query( m_database );
  const QString statement = QString( "SELECT id, mime_type FROM MimeTypes WHERE id = %1" ).arg( id );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during selection of single MimeType." );
    return MimeType();
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during selection of single MimeType." );
    return MimeType();
  }

  int mimeTypeId = query.value( 0 ).toInt();
  QString type = query.value( 1 ).toString();

  return MimeType( mimeTypeId, type );
}

MimeType DataStore::mimeTypeByName( const QString & name ) const
{
  if ( !m_dbOpened )
    return MimeType();

  QSqlQuery query( m_database );
  const QString statement = QString( "SELECT id, mime_type FROM MimeTypes WHERE mime_type = \"" + name + "\"" );

  if ( !query.exec( statement ) ) {
    debugLastDbError( "Error during selection of single MimeType." );
    return MimeType();
  }

  if ( !query.next() ) {
    debugLastDbError( "Error during selection of single MimeType." );
    return MimeType();
  }

  int id = query.value( 0 ).toInt();
  QString type = query.value( 1 ).toString();

  return MimeType( id, type );
}

QList<MimeType> DataStore::listMimeTypes()
{
  if ( !m_dbOpened )
    return QList<MimeType>();

  QSqlQuery query( m_database );
  if ( !query.exec( "SELECT id, mime_type FROM MimeTypes" ) ) {
    debugLastQueryError( query, "Error during selection of MimeTypes." );
    return QList<MimeType>();
  }

  QList<MimeType> mimeTypes;
  while ( query.next() ) {
    int id = query.value( 0 ).toInt();
    QString type = query.value( 1 ).toString();

    mimeTypes.append( MimeType( id, type ) );
  }

  return mimeTypes;
}


/* --- MetaType ------------------------------------------------------ */
bool DataStore::appendMetaType( const QString & metatype, const MimeType & mimetype )
{
  // TODO implement
  return false;
}

bool DataStore::removeMetaType( const MetaType & metatype )
{
  // TODO implement
  return false;
}

bool DataStore::removeMetaType( int id )
{
  // TODO implement
  return false;
}

MetaType DataStore::metaTypeById( int id )
{
  // TODO implement
  MetaType m;
  m.setId( id );
  m.setMetaType( "dummyMetaType" ).setMimeTypeId( 1 );
  return m;
}

QList<MetaType> DataStore::listMetaTypes()
{
  // TODO implement
  QList<MetaType> list;
  list.append( metaTypeById( 1 ) );
  return list;
}

QList<MetaType> DataStore::listMetaTypes( const MimeType & mimetype )
{
  // TODO implement
  QList<MetaType> list;
  // let's fake it for now
  if ( mimetype.id() == 1 )
    list.append( this->metaTypeById( 1 ) );
  return list;
}


/* --- PimItem ------------------------------------------------------- */
bool DataStore::appendPimItem( const QByteArray & data,
                               const MimeType & mimetype,
                               const Location & location )
{
  if ( !m_dbOpened )
    return false;

  QSqlQuery query( m_database );

  QSqlField field( "data", QVariant::String );
  field.setValue( data );
  QString escaped = m_database.driver()->formatValue( field );
  QString statement = QString( "INSERT INTO PimItems (data, location_id, mimetype_id) "
                               "VALUES (%1, %2, %3)")
                             .arg( escaped )
                             .arg( location.id() )
                             .arg( mimetype.id() );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during insertion of single PimItem." );
    return false;
  }

  return true;
}

bool DataStore::removePimItem( const PimItem & pimItem )
{
  return removePimItem( pimItem.id() );
}

bool DataStore::removePimItem( int id )
{
  return removeById( id, "PimItems" );
}

PimItem DataStore::pimItemById( int id )
{
  if ( !m_dbOpened )
    return PimItem();

  QSqlQuery query( m_database );
  query.prepare( "SELECT id, data, location_id, mimetype_id FROM PimItems WHERE id = :id" );
  query.bindValue( ":id", id );

  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during selection of single Location." );
    return PimItem();
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during selection of single Location." );
    return PimItem();
  }

  int pimItemId = query.value( 0 ).toInt();
  QByteArray data = query.value( 1 ).toByteArray();
  int location = query.value( 2 ).toInt();
  int mimetype = query.value( 3 ).toInt();

  return PimItem( pimItemId, data, location, mimetype );
}

QList<PimItem> DataStore::listPimItems( const MimeType & mimetype,
                                        const Location & location )
{
  // TODO implement
  QList<PimItem> list;
  list.append( pimItemById( 1 ) );
  return list;
}

int DataStore::highestPimItemId()
{
  if ( !m_dbOpened )
    return -1;

  QSqlQuery query( m_database );
  const QString statement = QString( "SELECT MAX(id) FROM PimItems" );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "DataStore::highestPimItemId" );
    return -1;
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "DataStore::highestPimItemId" );
    return -1;
  }

  return query.value( 0 ).toInt();
}

int DataStore::highestPimItemIdByLocation( const Location &location )
{
  if ( !m_dbOpened || !location.isValid() )
    return -1;

  QSqlQuery query( m_database );
  const QString statement = QString( "SELECT COUNT(*) AS count FROM PimItems WHERE location_id = %1" ).arg( location.id() );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "DataStore::highestPimItemIdByLocation" );
    return -1;
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "DataStore::highestPimItemIdByLocation" );
    return -1;
  }

  return query.value( 0 ).toInt();
}



/* --- Resource ------------------------------------------------------ */
bool DataStore::appendResource( const QString & resource,
                                const CachePolicy & policy )
{
  if ( !m_dbOpened )
    return false;

  QSqlQuery query( m_database );
  query.prepare( "SELECT COUNT(*) FROM Resources WHERE name = :name" );
  query.bindValue( ":name", resource );

  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during check before insertion of Resource." );
    return false;
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during check before insertion of Resource." );
    return false;
  }

  int foundRecs = query.value( 0 ).toInt();

  if ( foundRecs > 0) {
    qDebug() << "Cannot insert resource " << resource
             << " because it already exists.";
    return false;
  }

  query.prepare( "INSERT INTO Resources (name, cachepolicy_id) "
                 "VALUES (:name, :policy)" );
  query.bindValue( ":name", resource );
  query.bindValue( ":policy", policy.id() );

  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during insertion of single Resource." );
    return false;
  }

  return true;
}

bool DataStore::removeResource( const Resource & resource )
{
  return removeResource( resource.id() );
}

bool DataStore::removeResource( int id )
{
  return removeById( id, "Resources" );
}

Resource DataStore::resourceById( int id )
{
  if ( !m_dbOpened )
    return Resource();

  QSqlQuery query( m_database );
  query.prepare( "SELECT id, name, cachepolicy_id FROM Resources WHERE id = :id" );
  query.bindValue( ":id", id );

  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during selection of single Resource." );
    return Resource();
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during selection of single Resource." );
    return Resource();
  }

  int resourceId = query.value( 0 ).toInt();
  QString name = query.value( 1 ).toString();
  int id_res = query.value( 0 ).toInt();

  return Resource( resourceId, name, id_res );
}

const Resource DataStore::resourceByName( const QByteArray& name ) const
{
  if ( !m_dbOpened )
    return Resource();

  QSqlQuery query( m_database );
  // query.prepare( "SELECT id, name, cachepolicy_id FROM Resources WHERE name = :name" );
  // query.bindValue( ":name", name );
  const QString statement = QString( "SELECT id, name, cachepolicy_id FROM Resources WHERE name = \"" + name + "\"" );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during selection of single Resource." );
    return Resource();
  }

  if ( !query.next() ) {
    debugLastQueryError( query, "Error during selection of single Resource." );
    return Resource();
  }

  int id = query.value( 0 ).toInt();
  QString resourceName = query.value( 1 ).toString();
  int id_res = query.value( 0 ).toInt();

  return Resource( id, resourceName, id_res );
}

QList<Resource> DataStore::listResources() const
{
  if ( !m_dbOpened )
    return QList<Resource>();

  QSqlQuery query( m_database );
  if ( !query.exec( "SELECT id, name, cachepolicy_id FROM Resources" ) ) {
    debugLastQueryError( query, "Error during selection of Resources." );
    return QList<Resource>();
  }

  QList<Resource> resources;
  while ( query.next() ) {
    int id = query.value( 0 ).toInt();
    QString name = query.value( 1 ).toString();
    int id_res = query.value( 0 ).toInt();

    resources.append( Resource( id, name, id_res ) );
  }

  return resources;
}

QList<Resource> DataStore::listResources( const CachePolicy & policy )
{
  if ( !m_dbOpened )
    return QList<Resource>();

  QSqlQuery query( m_database );
  query.prepare( "SELECT id, name, cachepolicy_id FROM Resources WHERE cachepolicy_id = :id" );
  query.bindValue( ":id", policy.id() );

  if ( !query.exec() ) {
    debugLastQueryError( query, "Error during selection of Resources by a Policy." );
    return QList<Resource>();
  }

  QList<Resource> resources;
  while ( query.next() ) {
    int id = query.value( 0 ).toInt();
    QString name = query.value( 1 ).toString();
    int id_res = query.value( 0 ).toInt();

    resources.append( Resource( id, name, id_res ) );
  }

  return resources;
}


void DataStore::debugLastDbError( const QString & actionDescription ) const
{
  qDebug() << actionDescription
           << "\nDriver said: "
           << m_database.lastError().driverText()
           << "\nDatabase said: "
           << m_database.lastError().databaseText();
}

void DataStore::debugLastQueryError( const QSqlQuery &query, const QString & actionDescription ) const
{
  qDebug() << actionDescription
           << ": " << query.lastError().text();
}

bool DataStore::removeById( int id, const QString & tableName )
{
  if ( !m_dbOpened )
    return false;

  QSqlQuery query( m_database );
  const QString statement = QString( "DELETE FROM %1 WHERE id = :id" ).arg( tableName );
  query.prepare( statement );
  query.bindValue( ":id", id );

  if ( !query.exec() ) {
    QString msg = QString( "Error during deletion of a single row by ID from table %1: " ).arg( tableName );
    debugLastQueryError( query, msg );
    return false;
  }

  return true;
}

Location DataStore::locationByName( const Resource &resource,
                                    const QByteArray & name ) const
{
  qDebug() << "locationByName( " << resource.resource() << ", "
           << name << " )";

  if ( !m_dbOpened || !resource.isValid() )
    return Location();

  QSqlQuery query( m_database );
  const QString statement = QString( "SELECT id, uri, cachepolicy_id, resource_id, exists_count, recent_count,"
                                     " unseen_count, first_unseen, uid_validity FROM Locations "
                                     "WHERE resource_id = %1 AND uri = \"%2\"" ).arg( resource.id() ).arg( QString::fromLatin1( name ) );

  if ( !query.exec( statement ) ) {
    debugLastQueryError( query, "Error during selection of a Location by name from a Resource." );
    return Location();
  }

  Location location;
  while ( query.next() ) {
    location.fillFromQuery( query );
    location.setMimeTypes( mimeTypesForLocation( location.id() ) );
  }

  qDebug() << "Resource: " << resource.isValid() << " location: " << location.isValid();

  return location;
}
