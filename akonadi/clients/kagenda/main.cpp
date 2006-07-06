/*
    This file is part of Akonadi.

    Copyright (c) 2006 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "agendaview.h"
#include "dataprovider.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

int main( int argc, char **argv )
{
  KAboutData aboutData( "kagenda", "KAgenda", "0.1" );
  KCmdLineArgs::init( argc, argv , &aboutData );
  KApplication app;

  AgendaView widget;

  QString arg;
  if ( argc == 2 ) arg = argv[ 1 ];

  DataProvider provider;

  if ( arg == "contact" ) provider.setupContactData();
  else if ( arg == "dummy" ) provider.setupDummyData();
  else provider.setupEventData();

  widget.setDataProvider( &provider );

  widget.show();
    
  return app.exec();
}
