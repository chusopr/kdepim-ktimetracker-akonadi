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

#include "kcal_resourcetvanytime.h"
#include "kcal_resourcetvanytimeconfig.h"
#include <kglobal.h>
#include <klocale.h>

using namespace KCal;

typedef KRES::PluginFactory< ResourceTVAnytime, ResourceTVAnytimeConfig > TVAnytimeFactory;
// FIXME: Use K_EXPORT_COMPONENT_FACTORY( kcal_tvanytime, TVAnytimeFactory ); here
// Problem: How do I insert the catalog???
extern "C"
{
  KDE_EXPORT void *init_kcal_tvanytime()
  {
    KGlobal::locale()->insertCatalog( "libkcal" );
    KGlobal::locale()->insertCatalog( "kres_tvanytime" );
    return new TVAnytimeFactory;
  }
}
