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

#include "composerview.h"

#include "declarativeeditor.h"
#include "declarativeidentitycombobox.h"

#include <klocalizedstring.h>

QML_DECLARE_TYPE( DeclarativeEditor )
QML_DECLARE_TYPE( DeclarativeIdentityComboBox )

ComposerView::ComposerView(QWidget* parent) :
  KDeclarativeFullScreenView( QLatin1String( "kmail-composer" ), parent )
{
  setWindowTitle( i18n( "New mail" ) );

  qmlRegisterType<DeclarativeEditor>( "org.kde.messagecomposer", 4, 5, "Editor" );
  qmlRegisterType<DeclarativeIdentityComboBox>( "org.kde.kpimidentities", 4, 5, "IdentityComboBox" );
}

#include "composerview.moc"
