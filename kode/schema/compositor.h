/*
    This file is part of KDE Schema Parser

    Copyright (c) 2006 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef SCHEMA_COMPOSITOR_H
#define SCHEMA_COMPOSITOR_H

#include <QString>
#include <common/qname.h>
#include <kdepim_export.h>

namespace XSD {

class KODE_SCHEMA_EXPORT Compositor
{
  public:
    typedef QList<Compositor> List;

    enum Type {
      Invalid,
      Choice,
      Sequence,
      All
    };

    Compositor();
    Compositor( Type );
    ~Compositor();

    bool isValid() const;

    void setType( Type );
    Type type() const;

    void addChild( const QName & );
    void setChildren( const QName::List & );
    QName::List children() const;

  private:
    Type mType;
    
    QName::List mChildren;
};

}

#endif
