/*
    This file is part of KDE.

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

#ifndef KWSDL_SERVICE_H
#define KWSDL_SERVICE_H

#include <qmap.h>
#include <qstringlist.h>

namespace KWSDL {

class Service
{
  public:
    class Port
    {
      public:
        typedef QValueList<Port> List;

        QString mName;
        QString mBinding;
        QString mLocation;
    };

    Service();
    Service( const QString &name );

    void setName( const QString &name ) { mName = name; }
    QString name() const { return mName; }

    void addPort( const Port &port );
    Port port( const QString &name ) const;
    Port::List ports() const;

  private:
    QString mName;
    QMap<QString, Port> mPorts;
};

}

#endif
