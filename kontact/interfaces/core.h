/*
   This file is part of KDE Kontact.

   Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
   Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

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
#ifndef KONTACT_CORE_H
#define KONTACT_CORE_H

#include <kparts/mainwindow.h>
#include <kparts/part.h>

class KAction;

namespace Kontact
{

class Plugin;

/**
  This class provides the interface to the Kontact core for the plugins.
*/
class Core : public KParts::MainWindow
{
  public:
    virtual ~Core();

    /**
      Selects the given plugin @param plugin and raises the associated
      part.
     */
    virtual void selectPlugin( Kontact::Plugin *plugin ) = 0;

    /**
      This is an overloaded member function. It behaves essentially like the 
      above function.
     */
    virtual void selectPlugin( const QString &plugin ) = 0;

    /**
      Returns the pointer list of available plugins.
     */
    virtual QValueList<Kontact::Plugin*> pluginList() const = 0;

    KParts::ReadOnlyPart *createPart( const char *libname );

  protected:
    Core( QWidget *parentWidget = 0, const char *name = 0 );

  private:
    QMap<QCString,KParts::ReadOnlyPart *> mParts;

    class Private;
    Private *d;
};

}

#endif
