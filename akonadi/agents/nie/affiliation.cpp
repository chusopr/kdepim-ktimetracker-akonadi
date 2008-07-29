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
#include "affiliation.h"

#include "personcontact.h"
#include "organizationcontact.h"
Nepomuk::Affiliation::Affiliation()
  : Role( QUrl(), QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Affiliation") )
{
}


Nepomuk::Affiliation::Affiliation( const Affiliation& res )
  : Role( res )
{
}


Nepomuk::Affiliation::Affiliation( const Nepomuk::Resource& res )
  : Role( res )
{
}


Nepomuk::Affiliation::Affiliation( const QString& uri )
  : Role( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Affiliation") )
{
}

Nepomuk::Affiliation::Affiliation( const QUrl& uri )
  : Role( uri, QUrl::fromEncoded("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Affiliation") )
{
}

Nepomuk::Affiliation::Affiliation( const QString& uri, const QUrl& type )
  : Role( uri, type )
{
}

Nepomuk::Affiliation::Affiliation( const QUrl& uri, const QUrl& type )
  : Role( uri, type )
{
}

Nepomuk::Affiliation::~Affiliation()
{
}


Nepomuk::Affiliation& Nepomuk::Affiliation::operator=( const Affiliation& res )
{
    Resource::operator=( res );
    return *this;
}


QString Nepomuk::Affiliation::resourceTypeUri()
{
    return "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Affiliation";
}

QList<Nepomuk::OrganizationContact> Nepomuk::Affiliation::orgs() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<OrganizationContact>( property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#org") ).toResourceList() );
}

void Nepomuk::Affiliation::setOrgs( const QList<Nepomuk::OrganizationContact>& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    QList<Resource> l;
    for( QList<OrganizationContact>::const_iterator it = value.constBegin();
         it != value.constEnd(); ++it ) {
        l.append( Resource( (*it) ) );
    }
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#org"), Variant( l ) );
}

void Nepomuk::Affiliation::addOrg( const Nepomuk::OrganizationContact& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#org") );
    v.append( Resource( value ) );
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#org"), v );
}

QUrl Nepomuk::Affiliation::orgUri()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#org");
}

QStringList Nepomuk::Affiliation::departments() const
{
    return ( property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#department") ).toStringList());
}

void Nepomuk::Affiliation::setDepartments( const QStringList& value )
{
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#department"), Variant( value ) );
}

void Nepomuk::Affiliation::addDepartment( const QString& value )
{
    Variant v = property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#department") );
    v.append( value );
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#department"), v );
}

QUrl Nepomuk::Affiliation::departmentUri()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#department");
}

QStringList Nepomuk::Affiliation::titles() const
{
    return ( property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#title") ).toStringList());
}

void Nepomuk::Affiliation::setTitles( const QStringList& value )
{
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#title"), Variant( value ) );
}

void Nepomuk::Affiliation::addTitle( const QString& value )
{
    Variant v = property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#title") );
    v.append( value );
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#title"), v );
}

QUrl Nepomuk::Affiliation::titleUri()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#title");
}

QStringList Nepomuk::Affiliation::roles() const
{
    return ( property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#role") ).toStringList());
}

void Nepomuk::Affiliation::setRoles( const QStringList& value )
{
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#role"), Variant( value ) );
}

void Nepomuk::Affiliation::addRole( const QString& value )
{
    Variant v = property( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#role") );
    v.append( value );
    setProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#role"), v );
}

QUrl Nepomuk::Affiliation::roleUri()
{
    return QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#role");
}

QList<Nepomuk::PersonContact> Nepomuk::Affiliation::affiliationOf() const
{
    return convertResourceList<PersonContact>( ResourceManager::instance()->allResourcesWithProperty( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#hasAffiliation"), *this ) );
}

QList<Nepomuk::Affiliation> Nepomuk::Affiliation::allAffiliations()
{
    return Nepomuk::convertResourceList<Affiliation>( ResourceManager::instance()->allResourcesOfType( QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Affiliation") ) );
}


