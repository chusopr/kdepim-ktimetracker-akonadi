/*
    main.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�vdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "aboutdata.h"
#include "systemtrayicon.h"
#ifndef KLEO_ONLY_UISERVER
# include "certmanager.h"
#else
# include "mainwindow.h"
# include "trayiconlistener.h"
#endif

#include "libkleo/kleo/cryptobackendfactory.h"

#ifdef HAVE_USABLE_ASSUAN
# include <uiserver/uiserver.h>
# include <uiserver/assuancommand.h>
# include <uiserver/echocommand.h>
# include <uiserver/decryptcommand.h>
# include <uiserver/verifycommand.h>
# include <uiserver/decryptverifyfilescommand.h>
# include <uiserver/encryptcommand.h>
# include <uiserver/signcommand.h>
#endif

#include <models/keycache.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <QTextDocument> // for Qt::escape
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include <boost/shared_ptr.hpp>

namespace {
    template <typename T>
    boost::shared_ptr<T> make_shared_ptr( T * t ) {
        return t ? boost::shared_ptr<T>( t ) : boost::shared_ptr<T>() ;
    }
}

int main( int argc, char** argv )
{
  AboutData aboutData;

  KCmdLineArgs::init(argc, argv, &aboutData);

  KCmdLineOptions options;
#ifndef KLEO_ONLY_UISERVER
  options.add("external", ki18n("Search for external certificates initially"));
  options.add("query ", ki18n("Initial query string"));
#else
  options.add("daemon", ki18n("Run UI server only, hide main window"));
#endif
  options.add("import-certificate ", ki18n("Name of certificate file to import"));
#ifdef HAVE_USABLE_ASSUAN
  options.add("uiserver-socket <argument>", ki18n("Location of the socket the ui server is listening on" ) );
#endif
  KCmdLineArgs::addCmdLineOptions( options );

  // pin KeyCache to a shared_ptr to define it's minimum lifetime:
  const boost::shared_ptr<Kleo::KeyCache> keyCache = Kleo::KeyCache::mutableInstance();

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KGlobal::locale()->insertCatalog( "libkleopatra" );
  KIconLoader::global()->addAppDir( "libkleopatra" );

  SystemTrayIcon sysTray;
  sysTray.show();

#ifndef KLEO_ONLY_UISERVER
  if( !Kleo::CryptoBackendFactory::instance()->smime() ) {
    KMessageBox::error(0,
			i18n( "<qt>The crypto plugin could not be initialized.<br />"
			      "Certificate Manager will terminate now.</qt>") );
    return -2;
  }

  CertManager* manager = new CertManager( args->isSet("external"),
					  args->getOption("query"),
					  args->getOption("import-certificate") );
  manager->show();
#else
  if ( !args->isSet("daemon") ) {
    MainWindow* mainWindow = new MainWindow;
    mainWindow->show();
    TrayIconListener* trayListener = new TrayIconListener( mainWindow );
    QObject::connect( &sysTray, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
                      trayListener, SLOT( activated( QSystemTrayIcon::ActivationReason ) ) );
  }
#endif

  int rc;
#ifdef HAVE_USABLE_ASSUAN
  try {
      Kleo::UiServer server( args->getOption("uiserver-socket") );

      sysTray.setToolTip( i18n( "Kleopatra UI Server listening on %1", server.socketName() ) );

#define REGISTER( Command ) server.registerCommandFactory( boost::shared_ptr<Kleo::AssuanCommandFactory>( new Kleo::GenericAssuanCommandFactory<Kleo::Command> ) )
      REGISTER( DecryptCommand );
      REGISTER( DecryptVerifyFilesCommand );
      REGISTER( EchoCommand );
      REGISTER( EncryptCommand );
      REGISTER( SignCommand );
      REGISTER( VerifyCommand );
#undef REGISTER

      server.start();
#endif

      args->clear();
      QApplication::setQuitOnLastWindowClosed( false );
      rc = app.exec();

#ifdef HAVE_USABLE_ASSUAN
      server.stop();
      server.waitForStopped();
  } catch ( const std::exception & e ) {
      QMessageBox::information( 0, i18n("GPG UI Server Error"),
                                i18n("<qt>The Kleopatra GPG UI Server Module couldn't be initialized.<br/>"
                                     "The error given was: <b>%1</b><br/>"
                                     "You can use Kleopatra as a certificate manager, but cryptographic plugins that "
                                     "rely on a GPG UI Server being present might not work correctly, or at all.</qt>",
                                     Qt::escape( QString::fromUtf8( e.what() ) ) ));
      rc = app.exec();
  }
#endif

  return rc;
}
