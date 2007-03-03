/*
    This file is part of libqopensync.

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

#ifndef QSYNC_PLUGIN_H
#define QSYNC_PLUGIN_H

#include <qstring.h>

class OSyncPlugin;

namespace QSync {

class Plugin
{
  friend class Environment;
  friend class Member;

  public:
    Plugin();
    ~Plugin();

    /**
      Returns whether the object is a valid plugin.
     */
    bool isValid() const;

    /**
      Returns the name of the plugin.
     */
    QString name() const;

    /**
      Returns the long name of the plugin.
     */
    QString longName() const;

    /**
      Returns the description of the plugin.
     */
    QString description() const;

  private:
    OSyncPlugin *mPlugin;
};

}

#endif

