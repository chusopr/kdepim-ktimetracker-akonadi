/* 
    This file is part of KDE Schema Parser

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
                       based on wsdlpull parser by Vivek Krishna

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

#include "xsdtype.h"

using namespace XSD;

XSDType::XSDType()
  : mContentModel( SIMPLE )
{
}

XSDType::XSDType( const QString &nameSpace )
  : XmlElement( nameSpace ), mContentModel( SIMPLE )
{
}

XSDType::~XSDType()
{
}

void XSDType::setContentModel( ContentModel contentModel )
{
  mContentModel = contentModel;
}

XSDType::ContentModel XSDType::contentModel() const
{
  return mContentModel;
}
