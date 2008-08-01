/*
    This file is part of KAddressbook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kcmkabcustomfields.h"

#include "kabprefs.h"

#include <QtCore/QRegExp>

#include <kdemacros.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>

K_PLUGIN_FACTORY(KCMKabCustomFieldsFactory, registerPlugin<KCMKabCustomFields>();)
K_EXPORT_PLUGIN(KCMKabCustomFieldsFactory( "kcmkabcustomfields" ))

KCMKabCustomFields::KCMKabCustomFields( QWidget *parent, const QVariantList & )
  : KCMDesignerFields( KCMKabCustomFieldsFactory::componentData(), parent )
{
}

QString KCMKabCustomFields::localUiDir()
{
  return kabLocalDir() + "contacteditorpages/";
}

QString KCMKabCustomFields::uiPath()
{
  return "kaddressbook/contacteditorpages/";
}

void KCMKabCustomFields::writeActivePages( const QStringList &activePages )
{
  KABPrefs::instance()->setAdvancedCustomFields( activePages );
  KABPrefs::instance()->writeConfig();
}

QStringList KCMKabCustomFields::readActivePages()
{
  return KABPrefs::instance()->advancedCustomFields();
}

QString KCMKabCustomFields::applicationName()
{
  return "KADDRESSBOOK";
}

QString KCMKabCustomFields::kabLocalDir()
{
  QStringList kabdirs;
  kabdirs << KStandardDirs::locateLocal("data", "kaddressbook/");

  const QStringList results = kabdirs.filter( QRegExp( '^'+KGlobal::dirs()->localkdedir() ) );
  if ( !results.isEmpty() )
    return results.first();
  else
    return QString();
}

