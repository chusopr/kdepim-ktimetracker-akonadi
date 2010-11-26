/*
* This file is part of Akonadi
*
* Copyright 2010 Stephen Kelly <steveire@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include "mainview.h"

#include <akonadi/attributefactory.h>
#include <akonadi/kmime/messagefolderattribute.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdeclarativeapplication.h>

#ifdef Q_OS_WINCE
# include <windows.h>
# include <winuser.h>
#endif

#include <QtCore/QDateTime>

#ifdef MAIL_SERIALIZER_PLUGIN_STATIC
#include <QtCore/QtPlugin>

Q_IMPORT_PLUGIN(akonadi_serializer_mail)
Q_IMPORT_PLUGIN(akonadi_serializer_addressee)
Q_IMPORT_PLUGIN(akonadi_serializer_contactgroup)
Q_IMPORT_PLUGIN(akonadi_serializer_kcalcore)
#endif

int main( int argc, char **argv )
{
  kWarning() << "Starting main function" << QDateTime::currentDateTime();
#ifdef Q_OS_WINCE
  SetCursor( LoadCursor( NULL, IDC_WAIT ) );
#endif
  const QByteArray& ba = QByteArray( "kmail-mobile" );
  const KLocalizedString name = ki18n( "KMail Mobile" );
  KAboutData aboutData( ba, ba, name, ba, name );
  aboutData.setProductName( "KMail Mobile" ); //has to match the bugzilla product name

  KCmdLineArgs::init( argc, argv, &aboutData );
  KDeclarativeApplication app;

  KGlobal::locale()->insertCatalog( "libakonadi-kmime" );
  KGlobal::locale()->insertCatalog( "libmessagecore" );
  KGlobal::locale()->insertCatalog( "libmessagecomposer" );
  KGlobal::locale()->insertCatalog( "libmessageviewer" );
  KGlobal::locale()->insertCatalog( "libtemplateparser" );
  KGlobal::locale()->insertCatalog( "libmailcommon" );
  KGlobal::locale()->insertCatalog( "kmail" ); // for identity dialog

  Akonadi::AttributeFactory::registerAttribute<Akonadi::MessageFolderAttribute>();

  MainView view;
#ifdef Q_OS_WINCE
  SetCursor( LoadCursor( NULL, NULL ) );
#else
  view.show();
#endif

  return app.exec();
}
