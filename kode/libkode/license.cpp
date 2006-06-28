/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include <QtCore/QString>

#include "license.h"

using namespace KODE;

class License::Private
{
  public:
    Private()
      : mType( GPL ), mQtException( true )
    {
    }

    Type mType;
    bool mQtException;
};

License::License()
  : d( new Private )
{
}

License::License( const License &other )
  : d( new Private )
{
  *d = *other.d;
}

License::License( Type type )
  : d( new Private )
{
  d->mType = type;
}

License::~License()
{
  delete d;
}

License& License::operator=( const License &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

void License::setQtException( bool v )
{
  d->mQtException = v;
}

QString License::text() const
{
  QString txt;

  switch ( d->mType ) {
    case GPL:
      txt +=
            "This program is free software; you can redistribute it and/or modify\n"
            "it under the terms of the GNU General Public License as published by\n"
            "the Free Software Foundation; either version 2 of the License, or\n"
            "(at your option) any later version.\n"
            "\n"
            "This program is distributed in the hope that it will be useful,\n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
            "GNU General Public License for more details.\n"
            "\n"
            "You should have received a copy of the GNU General Public License\n"
            "along with this program; if not, write to the Free Software\n"
            "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,\n" "USA.\n";

      if ( d->mQtException ) {
        txt += '\n';
        txt +=
            "As a special exception, permission is given to link this program\n"
            "with any edition of Qt, and distribute the resulting executable,\n"
            "without including the source code for Qt in the source distribution.\n";
      }
      break;
    case LGPL:
      txt +=
            "This library is free software; you can redistribute it and/or\n"
            "modify it under the terms of the GNU Library General Public\n"
            "License as published by the Free Software Foundation; either\n"
            "version 2 of the License, or (at your option) any later version.\n"
            "\n"
            "This library is distributed in the hope that it will be useful,\n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
            "Library General Public License for more details.\n"
            "\n"
            "You should have received a copy of the GNU Library General Public License\n"
            "along with this library; see the file COPYING.LIB.  If not, write to\n"
            "the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
            "Boston, MA 02110-1301, USA.\n";
      break;
    default:
      break;
  }

  return txt;
}
