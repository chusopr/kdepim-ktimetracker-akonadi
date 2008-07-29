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
#include "videoimaccount.h"


Nepomuk::VideoIMAccount::VideoIMAccount()
  : AudioIMAccount( QUrl(), QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#VideoIMAccount") )
{
}


Nepomuk::VideoIMAccount::VideoIMAccount( const VideoIMAccount& res )
  : AudioIMAccount( res )
{
}


Nepomuk::VideoIMAccount::VideoIMAccount( const Nepomuk::Resource& res )
  : AudioIMAccount( res )
{
}


Nepomuk::VideoIMAccount::VideoIMAccount( const QString& uri )
  : AudioIMAccount( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#VideoIMAccount") )
{
}

Nepomuk::VideoIMAccount::VideoIMAccount( const QUrl& uri )
  : AudioIMAccount( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#VideoIMAccount") )
{
}

Nepomuk::VideoIMAccount::VideoIMAccount( const QString& uri, const QUrl& type )
  : AudioIMAccount( uri, type )
{
}

Nepomuk::VideoIMAccount::VideoIMAccount( const QUrl& uri, const QUrl& type )
  : AudioIMAccount( uri, type )
{
}

Nepomuk::VideoIMAccount::~VideoIMAccount()
{
}


Nepomuk::VideoIMAccount& Nepomuk::VideoIMAccount::operator=( const VideoIMAccount& res )
{
    Resource::operator=( res );
    return *this;
}


QString Nepomuk::VideoIMAccount::resourceTypeUri()
{
    return "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#VideoIMAccount";
}

QList<Nepomuk::VideoIMAccount> Nepomuk::VideoIMAccount::allVideoIMAccounts()
{
    return Nepomuk::convertResourceList<VideoIMAccount>( ResourceManager::instance()->allResourcesOfType( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#VideoIMAccount") ) );
}


