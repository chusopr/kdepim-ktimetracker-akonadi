/*
    This file is part of libkpimexchange.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include <typeinfo>

#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <libkcal/vcaldrag.h>
#include <libkcal/vcalformat.h>
#include <libkcal/icalformat.h>

#include "resourceimap.h"
#include "resourceimapconfig.h"

using namespace KCal;

ResourceIMAPConfig::ResourceIMAPConfig( QWidget* parent,  const char* name )
    : KRES::ResourceConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  QLabel *label = new QLabel( i18n( "Server:" ), this );
  mainLayout->addWidget( label, 1, 0 );

  // get serverlist from kmail ...

}

void ResourceIMAPConfig::loadSettings( KRES::Resource *resource )
{
  ResourceIMAP* res = dynamic_cast<ResourceIMAP*>( resource );
  if ( res ) {
      // load servername
      kdDebug() << "ERROR: ResourceIMAPConfig::loadSettings(): Unknown format type" << endl;
  } else
    kdDebug(5700) << "ERROR: ResourceIMAPConfig::loadSettings(): no ResourceIMAP, cast failed" << endl;
}

void ResourceIMAPConfig::saveSettings( KRES::Resource *resource )
{
  ResourceIMAP* res = dynamic_cast<ResourceIMAP*>( resource );
  if (res) {
    // save setting
  } else
    kdDebug(5700) << "ERROR: ResourceIMAPConfig::saveSettings(): no ResourceIMAP, cast failed" << endl;
}

#include "resourceimapconfig.moc"
