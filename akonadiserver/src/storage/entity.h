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

#ifndef ENTITY_H
#define ENTITY_H

#include <Qt>
#include <QDebug>
#include <QString>
#include <QDateTime>

class QSqlQuery;

namespace Akonadi {

class DataStore;

class CachePolicy;
class ItemMetaData;
class Location;
class MimeType;
class MetaType;
class PimItem;
class Resource;

class Entity
{
  public:
    int id() const { return m_id; }
    void setId( int id ) { m_id = id; }

    bool isValid() const { return m_id != -1; }

  protected:
    Entity() : m_id( -1 ) {}
    Entity( int id ) : m_id( id ) {}
    int m_id;
};

/***************************************************************************
 *   CachePolicy                                                           *
 ***************************************************************************/
class CachePolicy : public Entity
{
  public:
    CachePolicy();
    CachePolicy( int id, const QString & policy );
    ~CachePolicy();


    const QString & policy() const { return m_policy; };

  protected:
    CachePolicy & setCachePolicy( const QString & policy )
        { m_policy = policy; return *this; };

  private:
    QString m_policy;
    friend class DataStore;
};

/***************************************************************************
 *   Flag                                                                  *
 ***************************************************************************/
class Flag : public Entity
{
  public:
    Flag();
    Flag( int id, const QString & name );
    ~Flag();


    const QString & name() const { return m_name; };

  protected:
    Flag & setName( const QString & name )
        { m_name = name; return *this; };

  private:
    QString m_name;
    friend class DataStore;
};

/***************************************************************************
 *   ItemMetaData                                                          *
 ***************************************************************************/
class ItemMetaData : public Entity
{
  public:
    ItemMetaData();
    ItemMetaData( int id, const QString & metadata, const MetaType & metatype );
    ~ItemMetaData();

    int metaTypeId() const { return m_metatype_id; };
    const QString & itemMetaData() const { return m_metadata; };

  protected:
    ItemMetaData & setMetaTypeId( int metatype_id )
        { m_metatype_id = metatype_id; return *this; };

    ItemMetaData & setItemMetaData( const QString & metadata)
        { m_metadata = metadata; return *this; };

  private:
    int m_metatype_id;
    QString m_metadata;

    friend class DataStore;
};


/***************************************************************************
 *   MimeType                                                              *
 ***************************************************************************/
class MimeType : public Entity
{
  public:
    MimeType();
    MimeType( int id, const QString & mimetype );
    ~MimeType();
    QString mimeType() const { return m_mimetype; }
    static QByteArray asCommaSeparatedString( const QList<MimeType>& );

  protected:
    MimeType & setMimeType( const QString & mimetype )
        { m_mimetype = mimetype; return *this; };

  private:
    QString m_mimetype;

    friend class DataStore;
};

/***************************************************************************
 *   Location                                                              *
 ***************************************************************************/
class Location : public Entity
{
  public:
    Location();
    Location( int id, const QString & location,
              int policy_id, int resource_id );
    Location( int id, const QString & location,
              const CachePolicy & policy, const Resource & resource );
    void fillFromQuery( const QSqlQuery & query );
    ~Location();


    int policyId() const { return m_policy_id; };
    int resourceId() const { return m_resource_id; };
    const QString & location() const { return m_location; };
    void setMimeTypes( const QList<MimeType> & types )
    { m_mimeTypes = types; }

    QString flags() const { return m_flags; }
    int exists() const { return m_exists; }
    int recent() const { return m_recent; }
    int unseen() const { return m_unseen; }
    int firstUnseen() const { return m_unseen; }
    long uidValidity() const { return m_uidValidity; }
    QList<MimeType> mimeTypes() const { return m_mimeTypes; }

  protected:
    Location & setPolicyId( int policy_id )
        { m_policy_id = policy_id; return *this; };

    Location & setResourceId( int resource_id )
        { m_resource_id = resource_id; return *this; };

    Location & setLocation( const QString & location )
        { m_location = location; return *this; };
    void init();

  private:
    int m_policy_id;
    int m_resource_id;
    QString m_location;
    int m_exists;
    int m_recent;
    int m_unseen;
    int m_firstUnseen;
    long m_uidValidity;
    QString m_flags;
    QList<MimeType> m_mimeTypes;

    friend class DataStore;
};

QDebug & operator<< ( QDebug& d, const  Akonadi::Location& location);

/***************************************************************************
 *   MetaType                                                              *
 ***************************************************************************/
class MetaType : public Entity
{
  public:
    MetaType();
    MetaType( int id, const QString & metatype ,
              const MimeType & mimetype );
    ~MetaType();

    int mimeTypeId() const { return m_mimetype_id; };
    const QString & metaType() const { return m_metatype; };

  protected:
    MetaType & setMimeTypeId( int mimetype_id )
        { m_mimetype_id = mimetype_id; return *this; };

    MetaType & setMetaType( const QString & metatype )
        { m_metatype = metatype; return *this; };

  private:
    int m_mimetype_id;
    QString m_metatype;

    friend class DataStore;
};

/***************************************************************************
 *   Part                                                                  *
 ***************************************************************************/
// TODO

/***************************************************************************
 *   PartFlag                                                              *
 ***************************************************************************/
// TODO

/***************************************************************************
 *   PartMetaData                                                          *
 ***************************************************************************/
// TODO

/***************************************************************************
 *   PimItem                                                               *
 ***************************************************************************/
class PimItem : public Entity
{
  public:
    PimItem();
    PimItem( int id, const QByteArray & data, int location_id, int mimetype_id,
             const QDateTime & dateTime );
    ~PimItem();

    int mimeTypeId() const { return m_mimetype_id; };
    int locationId() const { return m_location_id; };
    const QByteArray & data() const { return m_data; };
    QDateTime dateTime() const { return m_datetime; };

  protected:
    PimItem & setMimeTypeId( int mimetype_id )
        { m_mimetype_id = mimetype_id; return *this; };

    PimItem & setLocationId( int location_id )
        { m_location_id = location_id; return *this; };

    PimItem & setData( const QByteArray & data )
        { m_data = data; return *this; };

    PimItem & setDateTime( const QDateTime & dateTime )
        { m_datetime = dateTime; return *this; };

  private:
    QByteArray m_data;
    int m_location_id;
    int m_mimetype_id;
    QDateTime m_datetime;

    friend class DataStore;
};

/***************************************************************************
 *   Resource                                                              *
 ***************************************************************************/
class Resource : public Entity
{
  public:
    Resource();
    Resource( int id, const QString & resource, int policy_id );
    Resource( int id, const QString & resource, const CachePolicy & policy );
    ~Resource();

    int policyId() const { return m_policy_id; };
    const QString & resource() const { return m_resource; };

  protected:
    Resource & setPolicyId( int policy_id )
        { m_policy_id = policy_id; return *this; };

    Resource & setResource( const QString & resource )
        { m_resource = resource; return *this; };

  private:
    int m_policy_id;
    QString m_resource;

    friend class DataStore;
};

}


#endif
