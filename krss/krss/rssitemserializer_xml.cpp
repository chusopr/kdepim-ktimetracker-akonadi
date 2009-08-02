/*
 * This file is part of the krss library
 *
 * Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "rssitemserializer.h"
#include "category.h"
#include "enclosure.h"
#include "rssitem.h"
#include "person.h"

#include <syndication/atom/constants.h>
#include <syndication/constants.h>

#include <KDateTime>

#include <QHash>
#include <QString>
#include <QVariant>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace KRss;

namespace {

enum TextMode {
    PlainText,
    Html
};

static QString krssNamespace()
{
    return "http://akregator.kde.org/krss#";
}

class Element
{
public:
    Element( const QString& ns_, const QString& name_, const QVariant& defaultValue_ = QVariant() ) : ns( ns_ ), name( name_ ), qualifiedName( ns + ':' + name ), defaultValue( defaultValue_ )
    {
    }

    const QString ns;
    const QString name;
    const QString qualifiedName;
    const QVariant defaultValue;

    bool isNextIn( const QXmlStreamReader& reader ) const
    {
        return reader.isStartElement() && reader.name() == name && reader.namespaceUri() == ns;
    }

    void writeStartElement( QXmlStreamWriter& writer ) const
    {
        if ( !ns.isNull() )
            writer.writeStartElement( ns, name );
        else
            writer.writeStartElement( name );
    }

    void write( const QVariant& value , QXmlStreamWriter& writer, TextMode mode = PlainText ) const
    {
        if ( value == defaultValue )
            return;

        if ( ns.isEmpty() )
            writer.writeStartElement( name );
        else
            writer.writeStartElement( ns, name );
        if ( mode == Html )
        {
            writer.writeAttribute( "type", "html" );
        }
        const QVariant qv( value );
        Q_ASSERT( qv.canConvert( QVariant::String ) );
        writer.writeCharacters( qv.toString() );
        writer.writeEndElement();
    }
};

struct Elements
{
    Elements() : atomNS( Syndication::Atom::atom1Namespace() ),
                 krssNS( krssNamespace() ),
                 commentNS( Syndication::commentApiNamespace() ),
                 title( atomNS, "title", QString() ),
                 summary( atomNS, "summary", QString() ),
                 content( atomNS, "content", QString() ),
                 link( atomNS, "link", QString() ),
                 language( atomNS, "language", QString() ),
                 guid( atomNS, "id", QString() ),
                 published( atomNS, "published", KDateTime().toString( KDateTime::ISODate ) ),
                 updated( atomNS, "updated", KDateTime().toString( KDateTime::ISODate ) ),
                 commentsCount( Syndication::slashNamespace(), "comments", -1 ),
                 commentsFeed( commentNS, "commentRss", QString() ),
                 commentPostUri( commentNS, "comment", QString() ),
                 commentsLink( krssNS, "commentsLink", QString() ),
                 hash( krssNS, "hash", 0 ),
                 guidIsHash( krssNS, "idIsHash", false ),
                 sourceFeedId( krssNS, "sourceFeedId", -1 ),
                 name( atomNS, "name", QString() ),
                 uri( atomNS, "uri", QString() ),
                 email( atomNS, "email", QString() ),
                 author( atomNS, "author", QString() ),
                 category( atomNS, "category", QString() ),
                 customProperty( krssNS, "customProperty", QString() ),
                 key( krssNS, "key", QString() ),
                 value( krssNS, "value", QString() ),
                 entry( atomNS, "entry", QString() )
{}
    const QString atomNS;
    const QString krssNS;
    const QString commentNS;
    const Element title;
    const Element summary;
    const Element content;
    const Element link;
    const Element language;
    const Element guid;
    const Element published;
    const Element updated;
    const Element commentsCount;
    const Element commentsFeed;
    const Element commentPostUri;
    const Element commentsLink;
    const Element hash;
    const Element guidIsHash;
    const Element sourceFeedId;
    const Element name;
    const Element uri;
    const Element email;
    const Element author;
    const Element category;
    const Element customProperty;
    const Element key;
    const Element value;
    const Element entry;
    static const Elements instance;
};

const Elements Elements::instance;

static void writeAttributeIfNotEmpty( const QString& ns, const QString& element, const QVariant& value, QXmlStreamWriter& writer )
{
    const QString text = value.toString();
    if ( text.isEmpty() )
        return;
    writer.writeAttribute( ns, element, text );
}

static void writeAttributeIfNotEmpty( const QString& element, const QVariant& value, QXmlStreamWriter& writer )
{
    const QString text = value.toString();
    if ( text.isEmpty() )
        return;
    writer.writeAttribute( element, text );
}

static void writeLink( const QString& url, QXmlStreamWriter& writer )
{
    if ( url.isEmpty() )
        return;
    Elements::instance.link.writeStartElement( writer );
    writer.writeAttribute( "rel", "alternate" );
    writeAttributeIfNotEmpty( "href", url, writer );
    writer.writeEndElement();
}

static void writeCategory( const KRss::Category& category, QXmlStreamWriter& writer )
{
    Elements::instance.category.writeStartElement( writer );
    writeAttributeIfNotEmpty( "term", category.term(), writer );
    writeAttributeIfNotEmpty( "scheme", category.scheme(), writer );
    writeAttributeIfNotEmpty( "label", category.label(), writer );
    writer.writeEndElement();
}

static void writeAuthor( const KRss::Person& person, QXmlStreamWriter& writer )
{
    const QString atomNS = Syndication::Atom::atom1Namespace();
    Elements::instance.author.writeStartElement( writer );
    Elements::instance.name.write( person.name(), writer );
    Elements::instance.uri.write( person.uri(), writer );
    Elements::instance.email.write( person.email(), writer );
    writer.writeEndElement();
}

static void writeEnclosure( const KRss::Enclosure& enclosure, QXmlStreamWriter& writer )
{
    Elements::instance.link.writeStartElement( writer );
    writeAttributeIfNotEmpty( "rel", "enclosure", writer );
    writeAttributeIfNotEmpty( "href", enclosure.url(), writer );
    writeAttributeIfNotEmpty( "title", enclosure.title(), writer );
    writeAttributeIfNotEmpty( "length", enclosure.length(), writer );
    writeAttributeIfNotEmpty( "type", enclosure.type(), writer );
    const uint duration = enclosure.duration();
    if ( duration != 0 )
        writer.writeAttribute( Syndication::itunesNamespace(), "duration", QString::number( duration ) );
    writer.writeEndElement();
}

static void writeItemHeaders( const KRss::RssItem& item, QXmlStreamWriter& writer )
{
    Elements::instance.title.write( item.title(), writer, Html );
    writeLink( item.link(), writer );
    Elements::instance.guid.write( item.guid(), writer );

    const KDateTime updated = item.dateUpdated();
    const KDateTime published = item.datePublished();
    Elements::instance.published.write( published.toString( KDateTime::ISODate ), writer );
    if ( updated.isValid() && updated != published )
        Elements::instance.updated.write( updated.toString( KDateTime::ISODate ), writer );

    Q_FOREACH( const KRss::Category& i, item.categories() )
        writeCategory( i, writer );

    Q_FOREACH( const KRss::Person& i, item.authors() )
        writeAuthor( i, writer );

    Q_FOREACH( const KRss::Enclosure& i, item.enclosures() )
        writeEnclosure( i, writer );

    Elements::instance.hash.write( item.hash(), writer );
    Elements::instance.guidIsHash.write( item.guidIsHash(), writer );
    Elements::instance.sourceFeedId.write( item.sourceFeedId(), writer );

}

static void writeItemContent( const KRss::RssItem& item, QXmlStreamWriter& writer )
{
    const QString description = item.description();
    Elements::instance.summary.write( description, writer, Html );
    const QString content = item.content();
    if ( content != description )
        Elements::instance.content.write( content, writer, Html );
    Elements::instance.language.write( item.language(), writer );
    Elements::instance.commentsFeed.write( item.commentsFeed(), writer );
    Elements::instance.commentPostUri.write( item.commentPostUri(), writer );
    Elements::instance.commentsCount.write( item.commentsCount(), writer );
    Elements::instance.commentsLink.write( item.commentsLink(), writer );
    const QHash<QString, QString> props = item.customProperties();
    QHash<QString, QString>::const_iterator it = props.constBegin();
    while ( it != props.constEnd() )
    {
        Elements::instance.customProperty.writeStartElement( writer );
        Elements::instance.key.write( it.key(), writer );
        Elements::instance.value.write( it.value(), writer );
        writer.writeEndElement();
        ++it;
    }
}

static void writeItem( const KRss::RssItem& item, QXmlStreamWriter& writer, KRss::RssItemSerializer::ItemPart part )
{
    const QString atomNS = Syndication::Atom::atom1Namespace();
    const QString commentNS = Syndication::commentApiNamespace();
    const QString krssNS = krssNamespace();
    writer.writeDefaultNamespace( atomNS );
    writer.writeNamespace( commentNS, "comment" );
    writer.writeNamespace( krssNS, "krss" );
    writer.writeNamespace( Syndication::itunesNamespace(), "itunes" );

    Elements::instance.entry.writeStartElement( writer );
    if ( ( part & RssItemSerializer::Headers ) != 0 )
        writeItemHeaders( item, writer );
    if ( ( part & RssItemSerializer::Content ) != 0 )
        writeItemContent( item, writer );

    writer.writeEndElement();   // Entry
}

static void readLink( KRss::RssItem& item, QXmlStreamReader& reader )
{
    const QXmlStreamAttributes attrs = reader.attributes();
    const QString rel = attrs.value( QString(), "rel" ).toString();
    if (  rel == "alternate" )
    {
        item.setLink( attrs.value( QString(), "href" ).toString() );
    }
    else if ( rel == "enclosure" )
    {
        KRss::Enclosure enc;
        enc.setUrl( attrs.value( QString(), "href" ).toString() );
        enc.setType( attrs.value( QString(), "type" ).toString() );
        enc.setTitle( attrs.value( QString(), "title" ).toString() );
        bool ok;
        const uint length = attrs.value( QString(), "length" ).toString().toUInt( &ok );
        if ( ok )
            enc.setLength( length );
        const uint duration = attrs.value( Syndication::itunesNamespace(), "duration" ).toString().toUInt( &ok );
        if ( ok )
            enc.setDuration( duration );
        QList<KRss::Enclosure> encs = item.enclosures();
        encs.append( enc );
        item.setEnclosures( encs );
    }
}

static void readAuthor( KRss::RssItem& item, QXmlStreamReader& reader )
{
    KRss::Person author;
    int depth = 1;
    while ( !reader.atEnd() && depth > 0 )
    {
        reader.readNext();
        if ( reader.isEndElement() )
            --depth;
        else if ( reader.isStartElement() )
        {
            if ( Elements::instance.name.isNextIn( reader ) )
                author.setName( reader.readElementText() );
            else if ( Elements::instance.uri.isNextIn( reader ) )
                author.setUri( reader.readElementText() );
            else if ( Elements::instance.email.isNextIn( reader ) )
                author.setEmail( reader.readElementText() );
        }

    }
    QList<KRss::Person> authors = item.authors();
    authors.append( author );
    item.setAuthors( authors );
}

static void readCategory( KRss::RssItem& item, QXmlStreamReader& reader )
{
    const QXmlStreamAttributes attrs = reader.attributes();
    KRss::Category cat;
    cat.setTerm( attrs.value( QString(), "term" ).toString() );
    cat.setScheme( attrs.value( QString(), "scheme" ).toString() );
    cat.setLabel( attrs.value( QString(), "label" ).toString() );
    QList<KRss::Category> cats = item.categories();
    cats.append( cat );
    item.setCategories( cats );
}

static void readCustomProperty( KRss::RssItem& item, QXmlStreamReader& reader )
{
    QString key;
    QString value;
    int depth = 1;
    while ( !reader.atEnd() && depth > 0 )
    {
        reader.readNext();
        if ( reader.isEndElement() )
            --depth;
        else if ( reader.isStartElement() )
        {
            if ( Elements::instance.key.isNextIn( reader ) )
                key = reader.readElementText();
            else if ( Elements::instance.value.isNextIn( reader ) )
                value = reader.readElementText();
        }
    }
    item.setCustomProperty( key, value );
}

} // namespace

void KRss::RssItemSerializer::serialize( const KRss::RssItem& item, QByteArray& array, ItemPart part )
{
    QXmlStreamWriter writer( &array );
    writer.writeStartDocument();
    ::writeItem( item, writer, part );
    writer.writeEndDocument();
}

bool KRss::RssItemSerializer::deserialize( KRss::RssItem& item, const QByteArray& array, ItemPart part )
{
    QXmlStreamReader reader( array );
    reader.setNamespaceProcessing( true );

    const bool readHeaders = ( part & Headers ) != 0;
    const bool readContent = ( part & Content ) != 0;

    if ( readHeaders )
        item.setHeadersLoaded( true );

    if ( readContent )
        item.setContentLoaded( true );

    while ( !reader.atEnd() )
    {
        reader.readNext();
        if ( reader.isStartElement() )
        {
            if ( readContent ) {
                if ( Elements::instance.summary.isNextIn( reader )  )
                    item.setDescription( reader.readElementText() );
                else if ( Elements::instance.content.isNextIn( reader ) )
                    item.setContent( reader.readElementText() );
                else if ( Elements::instance.language.isNextIn( reader ) )
                    item.setLanguage( reader.readElementText() );
                else if ( Elements::instance.commentsLink.isNextIn( reader ) )
                    item.setCommentsLink( reader.readElementText() );
                else if ( Elements::instance.commentPostUri.isNextIn( reader ) )
                    item.setCommentPostUri( reader.readElementText() );
                else if ( Elements::instance.commentsCount.isNextIn( reader ) )
                    item.setCommentsCount( reader.readElementText().toInt() );
                else if ( Elements::instance.commentsFeed.isNextIn( reader ) )
                    item.setCommentsFeed( reader.readElementText() );
                else if ( Elements::instance.customProperty.isNextIn( reader ) )
                    ::readCustomProperty( item, reader );
            }
            if ( readHeaders ) {
                if ( Elements::instance.title.isNextIn( reader ) )
                    item.setTitle( reader.readElementText() );
                else if ( Elements::instance.guid.isNextIn( reader ) )
                    item.setGuid( reader.readElementText() );
                else if ( Elements::instance.hash.isNextIn( reader ) )
                    item.setHash( reader.readElementText().toInt() );
                else if ( Elements::instance.guidIsHash.isNextIn( reader ) )
                    item.setGuidIsHash( QVariant( reader.readElementText() ).toBool() );
                else if ( Elements::instance.sourceFeedId.isNextIn( reader ) )
                    item.setSourceFeedId( reader.readElementText().toInt() );
                else if ( Elements::instance.link.isNextIn( reader ) )
                    ::readLink( item, reader );
                else if ( Elements::instance.author.isNextIn( reader ) )
                    ::readAuthor( item, reader );
                else if ( Elements::instance.category.isNextIn( reader ) )
                    ::readCategory( item, reader );
                else if ( Elements::instance.published.isNextIn( reader ) )
                    item.setDatePublished( KDateTime::fromString( reader.readElementText(), KDateTime::ISODate ) );
                else if ( Elements::instance.updated.isNextIn( reader ) )
                    item.setDateUpdated( KDateTime::fromString( reader.readElementText(), KDateTime::ISODate ) );
            }
        }
    }
    return !reader.hasError();
}
