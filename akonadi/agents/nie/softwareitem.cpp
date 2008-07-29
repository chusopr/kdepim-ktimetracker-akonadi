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
#include "softwareitem.h"


Nepomuk::SoftwareItem::SoftwareItem()
  : DataObject( QUrl(), QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SoftwareItem") )
{
}


Nepomuk::SoftwareItem::SoftwareItem( const SoftwareItem& res )
  : DataObject( res )
{
}


Nepomuk::SoftwareItem::SoftwareItem( const Nepomuk::Resource& res )
  : DataObject( res )
{
}


Nepomuk::SoftwareItem::SoftwareItem( const QString& uri )
  : DataObject( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SoftwareItem") )
{
}

Nepomuk::SoftwareItem::SoftwareItem( const QUrl& uri )
  : DataObject( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SoftwareItem") )
{
}

Nepomuk::SoftwareItem::SoftwareItem( const QString& uri, const QUrl& type )
  : DataObject( uri, type )
{
}

Nepomuk::SoftwareItem::SoftwareItem( const QUrl& uri, const QUrl& type )
  : DataObject( uri, type )
{
}

Nepomuk::SoftwareItem::~SoftwareItem()
{
}


Nepomuk::SoftwareItem& Nepomuk::SoftwareItem::operator=( const SoftwareItem& res )
{
    Resource::operator=( res );
    return *this;
}


QString Nepomuk::SoftwareItem::resourceTypeUri()
{
    return "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SoftwareItem";
}

QList<Nepomuk::SoftwareItem> Nepomuk::SoftwareItem::allSoftwareItems()
{
    return Nepomuk::convertResourceList<SoftwareItem>( ResourceManager::instance()->allResourcesOfType( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#SoftwareItem") ) );
}


