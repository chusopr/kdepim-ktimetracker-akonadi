/*
    This file is part of kdepim.
    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

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

#include "kcal_resourcexmlrpc.h"
#include "kcal_resourcexmlrpcconfig.h"

#include <kglobal.h>
#include <klocale.h>

using namespace KCal;

typedef KRES::PluginFactory<ResourceXMLRPC, ResourceXMLRPCConfig> XMLRPCFactory;
// FIXME: Use K_EXPORT_COMPONENT_FACTORY( kcal_xmlrpc, XMLRPCFactory ); here
// Problem: How can I insert the catalogue then?
extern "C"
{
  void *init_kcal_xmlrpc()
  {
    KGlobal::locale()->insertCatalogue( "kres_xmlrpc" );
    return new XMLRPCFactory;
  }
}
