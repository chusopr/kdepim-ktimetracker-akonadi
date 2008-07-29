/*
 *
 * $Id: $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

/*
 * This file has been generated by the Nepomuk Resource class generator.
 * DO NOT EDIT THIS FILE.
 * ANY CHANGES WILL BE LOST.
 */

#include <nepomuk/tools.h>
#include <nepomuk/variant.h>
#include <nepomuk/resourcemanager.h>
#include "sourcecode.h"


Nepomuk::SourceCode::SourceCode()
  : PlainTextDocument( QUrl(), QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SourceCode") )
{
}


Nepomuk::SourceCode::SourceCode( const SourceCode& res )
  : PlainTextDocument( res )
{
}


Nepomuk::SourceCode::SourceCode( const Nepomuk::Resource& res )
  : PlainTextDocument( res )
{
}


Nepomuk::SourceCode::SourceCode( const QString& uri )
  : PlainTextDocument( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SourceCode") )
{
}

Nepomuk::SourceCode::SourceCode( const QUrl& uri )
  : PlainTextDocument( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SourceCode") )
{
}

Nepomuk::SourceCode::SourceCode( const QString& uri, const QUrl& type )
  : PlainTextDocument( uri, type )
{
}

Nepomuk::SourceCode::SourceCode( const QUrl& uri, const QUrl& type )
  : PlainTextDocument( uri, type )
{
}

Nepomuk::SourceCode::~SourceCode()
{
}


Nepomuk::SourceCode& Nepomuk::SourceCode::operator=( const SourceCode& res )
{
    Resource::operator=( res );
    return *this;
}


QString Nepomuk::SourceCode::resourceTypeUri()
{
    return "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SourceCode";
}

QList<qint64> Nepomuk::SourceCode::commentCharacterCounts() const
{
    return ( property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#commentCharacterCount") ).toInt64List());
}

void Nepomuk::SourceCode::setCommentCharacterCounts( const QList<qint64>& value )
{
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#commentCharacterCount"), Variant( value ) );
}

void Nepomuk::SourceCode::addCommentCharacterCount( const qint64& value )
{
    Variant v = property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#commentCharacterCount") );
    v.append( value );
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#commentCharacterCount"), v );
}

QUrl Nepomuk::SourceCode::commentCharacterCountUri()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#commentCharacterCount");
}

QStringList Nepomuk::SourceCode::programmingLanguages() const
{
    return ( property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#programmingLanguage") ).toStringList());
}

void Nepomuk::SourceCode::setProgrammingLanguages( const QStringList& value )
{
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#programmingLanguage"), Variant( value ) );
}

void Nepomuk::SourceCode::addProgrammingLanguage( const QString& value )
{
    Variant v = property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#programmingLanguage") );
    v.append( value );
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#programmingLanguage"), v );
}

QUrl Nepomuk::SourceCode::programmingLanguageUri()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#programmingLanguage");
}

QStringList Nepomuk::SourceCode::definesClasses() const
{
    return ( property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesClass") ).toStringList());
}

void Nepomuk::SourceCode::setDefinesClasses( const QStringList& value )
{
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesClass"), Variant( value ) );
}

void Nepomuk::SourceCode::addDefinesClass( const QString& value )
{
    Variant v = property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesClass") );
    v.append( value );
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesClass"), v );
}

QUrl Nepomuk::SourceCode::definesClassUri()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesClass");
}

QStringList Nepomuk::SourceCode::definesFunctions() const
{
    return ( property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesFunction") ).toStringList());
}

void Nepomuk::SourceCode::setDefinesFunctions( const QStringList& value )
{
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesFunction"), Variant( value ) );
}

void Nepomuk::SourceCode::addDefinesFunction( const QString& value )
{
    Variant v = property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesFunction") );
    v.append( value );
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesFunction"), v );
}

QUrl Nepomuk::SourceCode::definesFunctionUri()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesFunction");
}

QStringList Nepomuk::SourceCode::definesGlobalVariables() const
{
    return ( property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesGlobalVariable") ).toStringList());
}

void Nepomuk::SourceCode::setDefinesGlobalVariables( const QStringList& value )
{
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesGlobalVariable"), Variant( value ) );
}

void Nepomuk::SourceCode::addDefinesGlobalVariable( const QString& value )
{
    Variant v = property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesGlobalVariable") );
    v.append( value );
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesGlobalVariable"), v );
}

QUrl Nepomuk::SourceCode::definesGlobalVariableUri()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#definesGlobalVariable");
}

QList<Nepomuk::SourceCode> Nepomuk::SourceCode::allSourceCodes()
{
    return Nepomuk::convertResourceList<SourceCode>( ResourceManager::instance()->allResourcesOfType( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SourceCode") ) );
}


