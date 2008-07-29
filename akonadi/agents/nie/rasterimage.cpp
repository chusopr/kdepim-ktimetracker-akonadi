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
#include "rasterimage.h"


Nepomuk::RasterImage::RasterImage()
  : Image( QUrl(), QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#RasterImage") )
{
}


Nepomuk::RasterImage::RasterImage( const RasterImage& res )
  : Image( res )
{
}


Nepomuk::RasterImage::RasterImage( const Nepomuk::Resource& res )
  : Image( res )
{
}


Nepomuk::RasterImage::RasterImage( const QString& uri )
  : Image( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#RasterImage") )
{
}

Nepomuk::RasterImage::RasterImage( const QUrl& uri )
  : Image( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#RasterImage") )
{
}

Nepomuk::RasterImage::RasterImage( const QString& uri, const QUrl& type )
  : Image( uri, type )
{
}

Nepomuk::RasterImage::RasterImage( const QUrl& uri, const QUrl& type )
  : Image( uri, type )
{
}

Nepomuk::RasterImage::~RasterImage()
{
}


Nepomuk::RasterImage& Nepomuk::RasterImage::operator=( const RasterImage& res )
{
    Resource::operator=( res );
    return *this;
}


QString Nepomuk::RasterImage::resourceTypeUri()
{
    return "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#RasterImage";
}

QList<Nepomuk::RasterImage> Nepomuk::RasterImage::allRasterImages()
{
    return Nepomuk::convertResourceList<RasterImage>( ResourceManager::instance()->allResourcesOfType( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#RasterImage") ) );
}


