/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "resourceremoteconfig.h"
#include "resourceremote.h"

#include <kglobal.h>
#include <klocale.h>

using namespace KCal;

extern "C"
{
  void *init_kcal_remote()
  {
    KGlobal::locale()->insertCatalogue( "kres_remote" );
    return new KRES::PluginFactory<ResourceRemote,ResourceRemoteConfig>();
  }
}
