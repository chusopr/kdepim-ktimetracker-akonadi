/*
    This file is part of KDE Kontact.

    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstartupinfo.h>
#include <kuniqueapplication.h>
#include <kwin.h>

#include <qlabel.h>
#if (QT_VERSION-0 >= 0x030200)
#include <qsplashscreen.h>
#else
#include "splash.h"
#endif

#include "mainwindow.h"

static const char description[] =
    I18N_NOOP( "A KDE Personal Information Manager" );

static const char version[] = "0.7.3 (Beta 1)";

class KontactApp : public KUniqueApplication {
  public:
    KontactApp() : mMainWindow( 0 ) {}
    ~KontactApp() {}

    int newInstance();

  private:
    Kontact::MainWindow *mMainWindow;
};

int KontactApp::newInstance()
{
  QWidget* splash = 0;
  if ( !mMainWindow ) // only the first time
  {
    // show splash
#if (QT_VERSION-0 >= 0x030200)
    QPixmap splashPixmap( UserIcon( "splash" ) );

    splash = new QSplashScreen( splashPixmap );
    splash->show();
#else
    splash = new Kontact::Splash( 0, "splash" );
    splash->show();
#endif
  }

  if ( isRestored() ) {
    // There can only be one main window
    if ( KMainWindow::canBeRestored( 1 ) ) {
      mMainWindow = new Kontact::MainWindow;
      setMainWidget( mMainWindow );
      mMainWindow->show();
      mMainWindow->restore( 1 );
    }
  } else {
    if ( !mMainWindow ) {
      mMainWindow = new Kontact::MainWindow;
      mMainWindow->show();
      setMainWidget( mMainWindow );
    }
  }

  delete splash;

  // Handle startup notification and window activation
  // (The first time it will do nothing except note that it was called)
  return KUniqueApplication::newInstance();
}

int main(int argc, char **argv)
{
  KAboutData about( "kontact", I18N_NOOP( "Kontact" ), version, description,
                    KAboutData::License_GPL, I18N_NOOP("(C) 2001-2003 The Kontact developers"), 0, "http://kontact.kde.org", "kde-pim@kde.org" );
  about.addAuthor( "Daniel Molkentin", 0, "molkentin@kde.org" );
  about.addAuthor( "Don Sanders", 0, "sanders@kde.org" );
  about.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  about.addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  about.addAuthor( "Sven Lüppken", 0, "sven@kde.org" );
  about.addAuthor( "Zack Rusin", 0, "zack@kde.org" );
  about.addAuthor( "Matthias Hoelzer-Kluepfel", I18N_NOOP("Original Author"), "mhk@kde.org" );

  KCmdLineArgs::init( argc, argv, &about );

  if ( !KontactApp::start() ) {
    kdError() << "Kontact is already running!" << endl;
    return 0;
  }

  KontactApp app;

  bool ret = app.exec();
  while ( KMainWindow::memberList->first() )
    delete KMainWindow::memberList->first();

  return ret;
}
