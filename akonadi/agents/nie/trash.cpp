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
#include "trash.h"


Nepomuk::Trash::Trash()
  : DataContainer( QUrl(), QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Trash") )
{
}


Nepomuk::Trash::Trash( const Trash& res )
  : DataContainer( res )
{
}


Nepomuk::Trash::Trash( const Nepomuk::Resource& res )
  : DataContainer( res )
{
}


Nepomuk::Trash::Trash( const QString& uri )
  : DataContainer( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Trash") )
{
}

Nepomuk::Trash::Trash( const QUrl& uri )
  : DataContainer( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Trash") )
{
}

Nepomuk::Trash::Trash( const QString& uri, const QUrl& type )
  : DataContainer( uri, type )
{
}

Nepomuk::Trash::Trash( const QUrl& uri, const QUrl& type )
  : DataContainer( uri, type )
{
}

Nepomuk::Trash::~Trash()
{
}


Nepomuk::Trash& Nepomuk::Trash::operator=( const Trash& res )
{
    Resource::operator=( res );
    return *this;
}


QString Nepomuk::Trash::resourceTypeUri()
{
    return "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Trash";
}

QList<Nepomuk::Trash> Nepomuk::Trash::allTrashs()
{
    return Nepomuk::convertResourceList<Trash>( ResourceManager::instance()->allResourcesOfType( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Trash") ) );
}


