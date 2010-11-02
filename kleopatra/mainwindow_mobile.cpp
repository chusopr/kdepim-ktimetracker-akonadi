/* -*- mode: c++; c-basic-offset:4 -*-
    mainwindow_mobile.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

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

#include "mainwindow_mobile.h"

#include "aboutdata.h"

#include "models/keylistmodel.h"
#include "models/keylistsortfilterproxymodel.h"

#if 0
#include "view/searchbar.h"
#include "view/tabwidget.h"
#endif
#include "view/keytreeview.h"
#include "view/keylistcontroller.h"

#include "commands/selftestcommand.h"
#include "commands/importcrlcommand.h"
#include "commands/importcertificatefromfilecommand.h"
#include "commands/decryptverifyfilescommand.h"
#include "commands/signencryptfilescommand.h"

#include "utils/detail_p.h"
#include "utils/gnupg-helper.h"
#include "utils/stl_util.h"
#include "utils/action_data.h"
#include "utils/classify.h"
#include "utils/filedialog.h"

// from libkdepim
#include "statusbarprogresswidget.h"
#include "progressdialog.h"

// from mobileui
#include "declarativewidgetbase.h"

#include <KActionCollection>
#include <KLocale>
#include <KTabWidget>
#include <KStatusBar>
#include <KStandardAction>
#include <KAction>
#include <KAboutData>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KStandardDirs>
#if 0
#include <KShortcutsDialog>
#include <KEditToolBar>
#endif
#include <KAboutApplicationDialog>
#include <kdebug.h>

#include <QTreeView>
#include <QFile>
#include <QToolBar>
#include <QWidgetAction>
#include <QApplication>
#include <QCloseEvent>
#include <QMenu>
#include <QTimer>
#include <QProcess>
#include <QPointer>
#include <QDeclarativeItem>

#include <kleo/cryptobackendfactory.h>
#include <ui/cryptoconfigdialog.h>
#include <kleo/cryptoconfig.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

#ifdef Q_OS_WIN32
static const bool OS_WIN = true;
#else
static const bool OS_WIN = false;
#endif

using namespace Kleo;
using namespace Kleo::Commands;
using namespace boost;
using namespace GpgME;

namespace {

    static const KAboutData * aboutGpg4WinData() {
        static const AboutGpg4WinData data;
        return &data;
    }

}

class MainWindow::KeyTreeViewItem : public DeclarativeWidgetBase<KeyTreeView,MainWindow,&MainWindow::registerKeyTreeView> {
    Q_OBJECT
public:
    explicit KeyTreeViewItem( QGraphicsItem * parent=0 )
        : DeclarativeWidgetBase<KeyTreeView,MainWindow,&MainWindow::registerKeyTreeView>( parent ) {}
    ~KeyTreeViewItem() {}
};

static KGuiItem KStandardGuiItem_quit() {
    static const QString app = KGlobal::mainComponent().aboutData()->programName();
    KGuiItem item = KStandardGuiItem::quit();
    item.setText( i18nc( "Quit [ApplicationName]", "&Quit %1", app ) );
    return item;
}

static KGuiItem KStandardGuiItem_close() {
    KGuiItem item = KStandardGuiItem::close();
    item.setText( i18n("Only &Close Window" ) );
    return item;
}

class MainWindow::Private {
    friend class ::MainWindow;
    MainWindow * const q;

public:
    explicit Private( MainWindow * qq );
    ~Private();

    template <typename T>
    void createAndStart() {
        ( new T( this->currentView(), &this->controller ) )->start();
    }
    template <typename T>
    void createAndStart( QAbstractItemView * view ) {
        ( new T( view, &this->controller ) )->start();
    }
    template <typename T>
    void createAndStart( const QStringList & a ) {
        ( new T( a, this->currentView(), &this->controller ) )->start();
    }
    template <typename T>
    void createAndStart( const QStringList & a, QAbstractItemView * view ) {
        ( new T( a, view, &this->controller ) )->start();
    }

    void closeAndQuit() {
        const QString app = KGlobal::mainComponent().aboutData()->programName();
        const int rc = KMessageBox::questionYesNoCancel( q,
                                                         i18n("%1 may be used by other applications as a service.\n"
                                                              "You may instead want to close this window without exiting %1.", app ),
                                                         i18n("Really Quit?"), KStandardGuiItem_close(), KStandardGuiItem_quit(), KStandardGuiItem::cancel(),
                                                         "really-quit-" + app.toLower() );
        if ( rc == KMessageBox::Cancel )
            return;
        if ( !q->close() )
            return;
        // WARNING: 'this' might be deleted at this point!
        if ( rc == KMessageBox::No )
            qApp->quit();
    }

    void selfTest() {
        createAndStart<SelfTestCommand>();
    }
    void configureBackend();
    void configDialogRequested() {
        qDebug( "configDialogRequested: not implemented" );
    }

    void showHandbook();

    void gnupgLogViewer() {
        if( !QProcess::startDetached("kwatchgnupg" ) )
            KMessageBox::error( q, i18n( "Could not start the GnuPG Log Viewer (kwatchgnupg). "
                                         "Please check your installation." ),
                                i18n( "Error Starting KWatchGnuPG" ) );
    }

    void gnupgAdministrativeConsole() {
        if( !QProcess::startDetached("kgpgconf" ) )
            KMessageBox::error( q, i18n( "Could not start the GnuPG Administrative Console (kgpgconf). "
                                         "Please check your installation." ),
                                i18n( "Error Starting KGpgConf" ) );
    }

    void slotConfigCommitted();

    void aboutGpg4Win() {
        ( new KAboutApplicationDialog( aboutGpg4WinData(), KAboutApplicationDialog::HideKdeVersion|KAboutApplicationDialog::HideTranslators, q ) )->show();
    }

private:
    void setupActions();

    QAbstractItemView * currentView() const {
        return controller.currentView();
    }

private:
    Kleo::KeyListController controller;
    bool firstShow : 1;
};

MainWindow::Private::Private( MainWindow * qq )
    : q( qq ),
      controller( q ),
      firstShow( true )
{
    KDAB_SET_OBJECT_NAME( controller );
    
    AbstractKeyListModel * flatModel = AbstractKeyListModel::createFlatKeyListModel( q );
    AbstractKeyListModel * hierarchicalModel = AbstractKeyListModel::createHierarchicalKeyListModel( q );

    KDAB_SET_OBJECT_NAME( flatModel );
    KDAB_SET_OBJECT_NAME( hierarchicalModel );


    controller.setFlatModel( flatModel );
    controller.setHierarchicalModel( hierarchicalModel );

}

MainWindow::Private::~Private() {}

MainWindow::MainWindow( QWidget * parent )
    : KDeclarativeFullScreenView( QLatin1String("kleopatra-mobile"), parent ), d( new Private( this ) )
{
}

MainWindow::~MainWindow() {}


void MainWindow::Private::setupActions() {

    KActionCollection * const coll = q->actionCollection();

    const action_data action_data[] = {
        // Settings menu
        { "settings_self_test", i18n("Perform Self-Test"), QString(),
          0, q, SLOT(selfTest()), QString(), false, true },
    };

    make_actions_from_data( action_data, coll );

    KStandardAction::close( q, SLOT(close()), coll );
    KStandardAction::quit( q, SLOT(closeAndQuit()), coll );
    KStandardAction::preferences( q, SIGNAL(configDialogRequested()), coll );

    controller.createActions( coll );
}

void MainWindow::delayedInit() {
    qmlRegisterType<KeyTreeViewItem>( "org.kde.kleopatra", 2, 1, "KeyTreeView" );
    KDeclarativeFullScreenView::delayedInit();
    d->setupActions();
}

void MainWindow::registerKeyTreeView( KeyTreeView * view ) {
    if ( !view )
        return;
    view->setFlatModel( d->controller.flatModel() );
    view->setHierarchicalModel( d->controller.hierarchicalModel() );
    d->controller.addView( view->view() );
}

void MainWindow::Private::slotConfigCommitted() {
    controller.updateConfig();
}

void MainWindow::closeEvent( QCloseEvent * e ) {
    // KMainWindow::closeEvent() insists on quitting the application,
    // so do not let it touch the event...
    kDebug();
    if ( d->controller.hasRunningCommands() ) {
        if ( d->controller.shutdownWarningRequired() ) {
            const int ret = KMessageBox::warningContinueCancel( this, i18n("There are still some background operations ongoing. "
                                                                           "These will be terminated when closing the window. "
                                                                           "Proceed?"),
                                                                i18n("Ongoing Background Tasks") );
            if ( ret != KMessageBox::Continue ) {
                e->ignore();
                return;
            }
        }
        d->controller.cancelCommands();
        if ( d->controller.hasRunningCommands() ) {
            // wait for them to be finished:
            setEnabled( false );
            QEventLoop ev;
            QTimer::singleShot( 100, &ev, SLOT(quit()) );
            connect( &d->controller, SIGNAL(commandsExecuting(bool)), &ev, SLOT(quit()) );
            ev.exec();
            kWarning( d->controller.hasRunningCommands() )
                << "controller still has commands running, this may crash now...";
            setEnabled( true );
        }
    }
    e->accept();
}

void MainWindow::importCertificatesFromFile( const QStringList & files ) {
    if ( !files.empty() )
        d->createAndStart<ImportCertificateFromFileCommand>( files );
}

#include "moc_mainwindow_mobile.cpp"
#include "mainwindow_mobile.moc"
