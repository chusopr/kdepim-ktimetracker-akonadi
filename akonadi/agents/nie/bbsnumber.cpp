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
#include "bbsnumber.h"


Nepomuk::BbsNumber::BbsNumber()
  : ModemNumber( QUrl(), QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#BbsNumber") )
{
}


Nepomuk::BbsNumber::BbsNumber( const BbsNumber& res )
  : ModemNumber( res )
{
}


Nepomuk::BbsNumber::BbsNumber( const Nepomuk::Resource& res )
  : ModemNumber( res )
{
}


Nepomuk::BbsNumber::BbsNumber( const QString& uri )
  : ModemNumber( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#BbsNumber") )
{
}

Nepomuk::BbsNumber::BbsNumber( const QUrl& uri )
  : ModemNumber( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#BbsNumber") )
{
}

Nepomuk::BbsNumber::BbsNumber( const QString& uri, const QUrl& type )
  : ModemNumber( uri, type )
{
}

Nepomuk::BbsNumber::BbsNumber( const QUrl& uri, const QUrl& type )
  : ModemNumber( uri, type )
{
}

Nepomuk::BbsNumber::~BbsNumber()
{
}


Nepomuk::BbsNumber& Nepomuk::BbsNumber::operator=( const BbsNumber& res )
{
    Resource::operator=( res );
    return *this;
}


QString Nepomuk::BbsNumber::resourceTypeUri()
{
    return "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#BbsNumber";
}

QList<Nepomuk::BbsNumber> Nepomuk::BbsNumber::allBbsNumbers()
{
    return Nepomuk::convertResourceList<BbsNumber>( ResourceManager::instance()->allResourcesOfType( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#BbsNumber") ) );
}


