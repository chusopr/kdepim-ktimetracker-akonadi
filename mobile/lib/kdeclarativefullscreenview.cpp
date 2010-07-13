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

#include "kdeclarativefullscreenview.h"
#include "stylesheetloader.h"

#include <KDebug>
#include <KGlobalSettings>
#include <KStandardDirs>
#include <KMessageBox>
#include <klocalizedstring.h>
#include <KAction>
#include <KActionCollection>
#include <KCmdLineArgs>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeError>

#include <boost/bind.hpp>
#include <algorithm>


KDeclarativeFullScreenView::KDeclarativeFullScreenView(const QString& qmlFileName, QWidget* parent) :
  QDeclarativeView( parent )
{
  static const bool debugTiming = KCmdLineArgs::parsedArgs()->isSet("timeit");

  QTime t;
  if ( debugTiming ) {
    t.start();
    kWarning() << "Start KDeclarativeFullScreenView ctor" << &t << " - " << QDateTime::currentDateTime();
  }

  setResizeMode( QDeclarativeView::SizeRootObjectToView );
#ifdef Q_WS_MAEMO_5
  setWindowState( Qt::WindowFullScreen );
  // use the oxygen black on whilte palette instead of the native white on black maemo5 one
  setPalette( KGlobalSettings::createApplicationPalette( KGlobal::config() ) );
#endif

  if ( debugTiming ) {
    kWarning() << "Applying style" << t.elapsed() << &t;
  }
  StyleSheetLoader::applyStyle( this );

  if ( debugTiming ) {
    kWarning() << "Applying style done" << t.elapsed() << &t;
  }

  connect( this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(slotStatusChanged(QDeclarativeView::Status)) );

  engine()->rootContext()->setContextProperty( "window", QVariant::fromValue( static_cast<QObject*>( this ) ) );

  if ( debugTiming ) {
    kWarning() << "Adding QML import paths" << t.elapsed() << &t;
  }
  foreach ( const QString &importPath, KGlobal::dirs()->findDirs( "module", "imports" ) )
    engine()->addImportPath( importPath );
  QString qmlPath = KStandardDirs::locate( "appdata", qmlFileName + ".qml" );

  if ( debugTiming ) {
    kWarning() << "Applying style done" << t.elapsed() << &t;
  }

  if ( qmlPath.isEmpty() ) // Try harder
    qmlPath = KStandardDirs::locate( "data", QLatin1String( "mobileui" ) + QDir::separator() + qmlFileName + ".qml" );

  // call setSource() only once our derived classes have set up everything
  QMetaObject::invokeMethod( this, "setQmlFile", Qt::QueuedConnection, Q_ARG( QString, qmlPath ) );

  // TODO: Get this from a KXMLGUIClient?
  mActionCollection = new KActionCollection( this );

  if ( debugTiming ) {
    kWarning() << "KDeclarativeFullScreenView ctor done" << t.elapsed() << &t << QDateTime::currentDateTime();
  }
}
void KDeclarativeFullScreenView::setQmlFile(const QString& source)
{
  static const bool debugTiming = KCmdLineArgs::parsedArgs()->isSet("timeit");
  QTime t;
  if ( debugTiming ) {
    t.start();
    kWarning() << "start setSource" << &t << " - " << QDateTime::currentDateTime();
  }
  setSource( source );
  if ( debugTiming ) {
    kWarning() << "setSourceDone" << t.elapsed() << &t;
  }
}

void KDeclarativeFullScreenView::triggerTaskSwitcher()
{
#ifdef Q_WS_MAEMO_5
  QDBusConnection::sessionBus().call( QDBusMessage::createSignal( QLatin1String( "/" ), QLatin1String( "com.nokia.hildon_desktop" ), QLatin1String( "exit_app_view" ) ), QDBus::NoBlock );
#else
  kDebug() << "not implemented for this platform";
#endif
}

void KDeclarativeFullScreenView::slotStatusChanged ( QDeclarativeView::Status status )
{
  kWarning() << status << QDateTime::currentDateTime();

  if ( status == QDeclarativeView::Error ) {
    QStringList errorMessages;
    std::transform( errors().constBegin(), errors().constEnd(), std::back_inserter( errorMessages ), boost::bind( &QDeclarativeError::toString, _1 ) );
    KMessageBox::error( this, i18n( "Application loading failed: %1", errorMessages.join( QLatin1String( "\n" ) ) ) );
    QCoreApplication::instance()->exit( 1 );
  }
}

KActionCollection* KDeclarativeFullScreenView::actionCollection() const
{
  return mActionCollection;
}

QObject* KDeclarativeFullScreenView::getAction( const QString &name ) const
{
  return mActionCollection->action( name );
}


#include "kdeclarativefullscreenview.moc"
