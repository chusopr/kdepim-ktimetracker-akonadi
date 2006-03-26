/* 
    This file is part of KDE Schema Parser

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#include "types.h"

#include <QDebug>

using namespace XSD;

void Types::setSimpleTypes( const SimpleType::List &simpleTypes )
{
  mSimpleTypes = simpleTypes;
}

SimpleType::List Types::simpleTypes() const
{
  return mSimpleTypes;
}

void Types::setComplexTypes( const ComplexType::List &complexTypes )
{
  mComplexTypes = complexTypes;
}

ComplexType::List Types::complexTypes() const
{
  return mComplexTypes;
}

void Types::setElements( const Element::List &elements )
{
  mElements = elements;
}

Element::List Types::elements() const
{
  return mElements;
}

void Types::setAttributes( const Attribute::List &attributes )
{
  mAttributes = attributes;
}

Attribute::List Types::attributes() const
{
  return mAttributes;
}

void Types::setAttributeGroups( const AttributeGroup::List &attributeGroups )
{
  mAttributeGroups = attributeGroups;
}

AttributeGroup::List Types::attributeGroups() const
{
  return mAttributeGroups;
}

void Types::setNamespaces( const QStringList &namespaces )
{
  mNamespaces = namespaces;
}

QStringList Types::namespaces() const
{
  return mNamespaces;
}


ComplexType Types::complexType( const Element &element ) const
{
  foreach( ComplexType type, mComplexTypes ) {
    if( element.type() == type.name() ) return type;
  }
  return ComplexType();
}

SimpleType Types::simpleType( const QName &typeName ) const
{
  qDebug() << "Types::simpleType()" << typeName.qname();
  foreach( SimpleType type, mSimpleTypes ) {
    qDebug() << "  BASETYPENAME:" << type.baseTypeName().qname();
    if ( type.qualifiedName() == typeName ) return type;
  }
  qDebug() << "not found";
  return SimpleType();
}
