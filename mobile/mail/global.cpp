/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "global.h"

#include <kpimidentities/identitymanager.h>
#include <kglobal.h>

#include <QScopedPointer>

class GlobalPrivate
{
  public:
    GlobalPrivate() :
      m_identityManager( new KPIMIdentities::IdentityManager )
    {
    }

    QScopedPointer<KPIMIdentities::IdentityManager> m_identityManager;
};

K_GLOBAL_STATIC( GlobalPrivate, s_global )

KPIMIdentities::IdentityManager* Global::identityManager()
{
  return s_global->m_identityManager.data();
}
