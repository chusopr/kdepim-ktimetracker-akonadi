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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
 */

#ifndef SCHEMA_TYPES_H
#define SCHEMA_TYPES_H

#include "complextype.h"
#include "element.h"
#include "simpletype.h"

namespace Schema {

class Types
{
  public:
    void setSimpleTypes( const SimpleType::List &simpleTypes );
    SimpleType::List simpleTypes() const;

    void setComplexTypes( const ComplexType::List &complexTypes );
    ComplexType::List complexTypes() const;

    void setNameMap( const XSDType::NameMap &nameMap );
    XSDType::NameMap nameMap() const;

    QString typeName( int type ) const;

    void setElements( const Element::List &elements );
    Element::List elements() const;

  private:
    SimpleType::List mSimpleTypes;
    ComplexType::List mComplexTypes;
    XSDType::NameMap mNameMap;
    Element::List mElements;
};

}

#endif
