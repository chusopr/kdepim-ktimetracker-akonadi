/*
� � � � � � � �=.            This file is part of the OPIE Project
� � � � � � �.=l.            Copyright (c)  2002,2003 Holger Freyther <zecke@handhelds.org>
� � � � � �.>+-=                            2002,2003 Maximilian Rei� <harlekin@handhelds.org>
�_;:, � � .> � �:=|.         This library is free software; you can
.> <`_, � > �. � <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- � :           the terms of the GNU Library General Public
.="- .-=="i, � � .._         License as published by the Free Software
�- . � .-<_> � � .<>         Foundation; either version 2 of the License,
� � �._= =} � � � :          or (at your option) any later version.
� � .%`+i> � � � _;_.
� � .i_,=:_. � � �-<s.       This library is distributed in the hope that
� � �+ �. �-:. � � � =       it will be useful,  but WITHOUT ANY WARRANTY;
� � : .. � �.:, � � . . .    without even the implied warranty of
� � =_ � � � �+ � � =;=|`    MERCHANTABILITY or FITNESS FOR A
� _.=:. � � � : � �:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= � � � = � � � ;      Library General Public License for more
++= � -. � � .` � � .:       details.
�: � � = �...= . :.=-
�-. � .:....=;==+<;          You should have received a copy of the GNU
� -_. . . � )=. �=           Library General Public License along with
� � -- � � � �:-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/


#include <qwidgetstack.h>

#include <klocale.h>
#include <kstatusbar.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <kparts/componentfactory.h>
#include <kpopupmenu.h>

#include <syncer.h>
#include <syncuikde.h>

#include <konnectormanager.h>
#include <konnectorplugin.h>
#include <error.h>
#include <progress.h>

#include "syncconfig.h"
#include "configuredialog.h"
#include "partbar.h"
#include "profiledialog.h"

#include "konnectorbar.h"
#include "konnectordialog.h"
#include "mainwindow.h"
#include "syncalgo.h"

using namespace KSync;

namespace {

struct MainProgress
{
    static Error noKonnector();
    static Error noPush();
};

Error MainProgress::noKonnector()
{
    return Error(i18n("There is no current Konnector") );
}

Error MainProgress::noPush()
{
    return Error(i18n("The current Konnector does not support pushing") );
}

kdbgstream operator<<( kdbgstream str, const Notify& no )
{
    str << no.code() << " " << no.text();
    return str;
}

}

KSyncMainWindow::KSyncMainWindow(QWidget *widget, const char *name, WFlags f)
  :
  KParts::MainWindow( widget, name, f )
{

  m_konnectorManager = 0;
  m_syncAlg = 0;
  m_syncUi = 0;

  m_partsIt = 0;
//  m_outSyncee.setAutoDelete( true ); // for startSync..
  m_isSyncing = false;

  initActions();
  setXMLFile("ksyncgui.rc");
  setInstance( KGlobal::instance() );

  createGUI( 0 );
  // now add a layout or QWidget?
  m_lay = new QHBox( this, "main widget" );
  setCentralWidget( m_lay );

  m_bar = new PartBar(m_lay , "partBar" );
  m_stack = new QWidgetStack( m_lay, "dummy" );

  QWidget *test = new QWidget(m_stack);
  test->setBackgroundColor(Qt::red);
  m_stack->addWidget( test, 0 );
  m_stack->raiseWidget( 0 );
  m_bar->setMaximumWidth( 100 );
  m_bar->setMinimumWidth( 100 );

  connect( m_bar, SIGNAL( activated( ManipulatorPart * ) ),
           SLOT( slotActivated( ManipulatorPart * ) ) );

  resize(600,400);

  m_parts.setAutoDelete( true );

//  kdDebug(5210) << "Init konnector" << endl;
  initKonnector();
  initPlugins();

  //statusBar()->insertItem(i18n("Not Connected"), 10, 0, true );
  m_konBar = new KonnectorBar( statusBar() );
  m_konBar->setName(i18n("No Konnector") );
  connect( m_konBar, SIGNAL( toggled( bool ) ),
           SLOT( slotKonnectorBar( bool ) ) );
  statusBar()->addWidget( m_konBar, 0, true );
  statusBar()->show();

  // show systemtraypart
  initSystray();
  m_tray->show();
  initProfiles();
  slotProfile();
  slotKonnectorProfile();
}

KSyncMainWindow::~KSyncMainWindow()
{
    m_konprof->save();
    m_prof->save();
    createGUI( 0 );
}

void KSyncMainWindow::initActions()
{
  (void)new KAction( i18n("Synchronize" ), "reload", 0,
                     this, SLOT( slotSync() ),
		     actionCollection(), "sync" );

  (void)new KAction( i18n("Backup") ,  "mail_get", 0,
                     this, SLOT( slotBackup() ),
		     actionCollection(), "backup" );

  (void)new KAction( i18n("Restore"),  "mail_send", 0,
                     this, SLOT (slotRestore() ),
		     actionCollection(), "restore" );

  (void)new KAction( i18n("Quit"),  "exit", 0,
                     this, SLOT(slotQuit()),
		     actionCollection(), "quit" );

  (void)new KAction( i18n("Configure Connections"), "configure" , 0,
                     this, SLOT (slotConfigure() ),
                     actionCollection(), "configure" );

  (void)new KAction( i18n("Configure Profiles"),  "configure", 0,
                     this, SLOT(slotConfigProf() ),
                     actionCollection(), "config_profile" );

  (void)new KAction( i18n("Configure current Profile"),  "configure", 0,
                     this, SLOT(slotConfigCur() ),
                     actionCollection(), "config_current" );
  m_konAct = new KSelectAction( i18n("Konnector"),
                                KShortcut(),this,
                                SLOT(slotKonnectorProfile() ),
                                actionCollection(),
                                "select_kon");

  m_profAct = new KSelectAction( i18n("Profile"),  KShortcut(), this,
                                 SLOT(slotProfile() ),
                                 actionCollection(), "select_prof");
}

/*
 * we search for all installed plugins here
 * and add them to the ManPartService List
 * overview is special for us
 */
void KSyncMainWindow::initPlugins()
{
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KitchenSync/Manipulator"),
						     QString::null);

  for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it){
      ManPartService ser( (*it) );
      m_partsLst.append( ser );
  }

}

void KSyncMainWindow::addModPart(ManipulatorPart *part)
{
    /* the usual signal and slots */
    connect( part, SIGNAL(sig_progress(ManipulatorPart*, int ) ),
             SLOT(slotPartProg(ManipulatorPart*, int ) ) );
    connect( part, SIGNAL(sig_progress(ManipulatorPart*, const Progress& ) ),
             SLOT(slotPartProg(ManipulatorPart*, const Progress& ) ) );
    connect( part, SIGNAL(sig_error(ManipulatorPart*, const Error& ) ),
             SLOT(slotPartErr(ManipulatorPart*, const Error& ) ) );
    connect( part, SIGNAL(sig_syncStatus(ManipulatorPart*, int) ),
             SLOT(slotPartSyncStatus( ManipulatorPart*, int ) ) );

    static int id=1;
    if( part->partIsVisible() )  {
        kdDebug(5210) << "Part is Visible " << part->name() << endl;
        int pos = -1;
        m_stack->addWidget( part->widget(), id );

        /* overview is special for us ;) */
        if( part->type() == i18n("Overview") ){
            m_stack->raiseWidget(id );
            pos = 0;
        }
        m_bar->insertItem( part, pos );
    }
    m_parts.append( part );
    id++;
}

void KSyncMainWindow::initSystray( void )
{

    m_tray = new KSyncSystemTray( this, "KSyncSystemTray");
    KPopupMenu *popMenu = m_tray->getContextMenu();
    popMenu->insertSeparator();
}

/*
 * Get the current konnector
 * check if we can push
 * and then push
 */
void KSyncMainWindow::slotSync()
{
    emit partProgress( 0, Progress(i18n("Trying to push sync") ) );
    KonnectorProfile prof = konnectorProfile();
    if ( !prof.konnector() ) {
        emit partError( 0, MainProgress::noKonnector() );
        return;
    }

    if ( !prof.kapabilities().supportsPushSync() ) {
        emit partError( 0, MainProgress::noPush() );
        return;
    }

    prof.konnector()->startSync();
}

void KSyncMainWindow::slotBackup()
{
    emit partProgress(0, i18n("Starting to backup") );
    kdDebug(5210) << "Slot backup " << endl;

    QString path = KFileDialog::getSaveFileName(
                          QDir::homeDirPath(), i18n("*.xml|Backup files"), this,
                          i18n("Please enter a filename to backup the data"));
    if (!path.isEmpty() )
    {
      // Check if .xml added. If not, add it.
      QFileInfo fi(path);
      if (!(fi.extension().lower() == "xml"))
      {
        path.append(".xml");
      }
    }
    else
    {
      return;
    }

    KonnectorProfile prof = konnectorProfile();
    if ( !prof.konnector() ) {
        emit partError(0, MainProgress::noKonnector() );
        return;
    }

/*    if (!prof.kapabilities().supportsPushSync() )
        return;
*/
    prof.konnector()->startBackup( path );
}

void KSyncMainWindow::slotRestore()
{
    kdDebug(5210) << "Slot restore " << endl;

    QString path = KFileDialog::getOpenFileName(
                           QDir::homeDirPath(), "*.xml|Backup files", this,
                           i18n("Please choose a backup file to restore the data"));
    if (path.isEmpty() )
      return;

    KonnectorProfile prof = konnectorProfile();
    if ( !prof.konnector() ) {
        emit partError(0, MainProgress::noKonnector() );
        return;
    }

/*    if (!prof.kapabilities().supportsPushSync() )
        return;
*/
    prof.konnector()->startRestore( path );
}

void KSyncMainWindow::slotConfigure()
{
    KonnectorDialog dlg( m_konprof->list(), m_konnectorManager );
    /* clicked ok - now clean up*/
    if ( dlg.exec() == QDialog::Accepted) {
        m_konprof->clear();
        KonnectorProfile::ValueList all = dlg.devices();
        removeDeleted( dlg.removed() );
        loadUnloaded( dlg.toLoad(), all );
        unloadLoaded( dlg.toUnload(), all );
	updateEdited( dlg.edited() );

        /* the rest and unchanged items */
        for ( KonnectorProfile::ValueList::Iterator it = all.begin();
              it != all.end(); ++it ) {
            m_konprof->add( (*it) );
        }
        m_konprof->save();
        initKonnectorList();
        slotKonnectorProfile();
    }
}

void KSyncMainWindow::removeDeleted( const KonnectorProfile::ValueList& list )
{
    KonnectorProfile::ValueList::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
        emit partProgress(0, Progress(i18n("Going to unload %1").arg( (*it).name() ) ) );
        m_konnectorManager->unload( (*it).konnector() );
    }
}

/*
 * loadUnloaded items need to load items and remove them from
 * all list so we have no duplicates
 */
void KSyncMainWindow::loadUnloaded( const KonnectorProfile::ValueList& list,
                                    KonnectorProfile::ValueList& all )
{
    kdDebug() << "KSyncMainWindow::loadUnloaded" << endl;

    KonnectorProfile::ValueList::ConstIterator itList;
    KonnectorProfile::ValueList::Iterator itAll;
    for ( itList = list.begin(); itList != list.end(); ++itList ) {
        itAll = all.find( (*itList) );
        /* found it */
        if ( itAll != all.end() )  {
            emit partProgress(0, Progress(i18n("Going to load %1").arg( (*itList).name() ) ) );
            Konnector *k = m_konnectorManager->load( (*itList).device() );
            if ( k ) {
                KonnectorProfile prof = (*itList);
                k->setCapabilities( prof.kapabilities() );
                m_konprof->add( prof );
            } else {
                emit partError( 0, Error(i18n("Could not load the Konnector") ) );
            }
            all.remove( itAll );
        }
    }
}

/*
 * unloadLoaded unloads loaded KonnectorPlugins
 * through the KonnectorManager
 */
void KSyncMainWindow::unloadLoaded( const KonnectorProfile::ValueList& list,
                                    KonnectorProfile::ValueList& all )
{
    kdDebug() << "KSyncMainWindow::unloadLoaded" << endl;

    KonnectorProfile::ValueList::ConstIterator itList;
    KonnectorProfile::ValueList::Iterator itAll;
    for ( itList = list.begin(); itList != list.end(); ++itList ) {
        itAll = all.find( (*itList) );
        /* found it */
        if ( itAll != all.end() )  {
            m_konnectorManager->unload( (*itList).konnector() );
            KonnectorProfile prof = (*itList);
            prof.setKonnector( 0 );
            m_konprof->add( prof );
            all.remove( itAll );
        }
    }
}

/*
 * update edited updates the kapabilities of the currently loaded
 * konnectors..
 */
void KSyncMainWindow::updateEdited( const KonnectorProfile::ValueList& list )
{
    KonnectorProfile::ValueList::ConstIterator it;
    for( it = list.begin(); it != list.end(); ++it ){
	(*it).konnector()->setCapabilities( (*it).kapabilities() );
    }
}

void KSyncMainWindow::slotActivated( ManipulatorPart *part )
{
    emit partChanged( part );
    m_stack->raiseWidget( part->widget() );
    createGUI( part );
}

void KSyncMainWindow::slotQuit()
{
    close();
}

KSyncSystemTray* KSyncMainWindow::tray()
{
    return m_tray;
}

KonnectorManager* KSyncMainWindow::konnectorManager()
{
    return m_konnectorManager;
}

QString KSyncMainWindow::currentId() const
{
    return m_currentId;
}

void KSyncMainWindow::initKonnector()
{
    kdDebug(5210) << "initKonnector" << endl;

    m_konnectorManager = KonnectorManager::self();

    connect( m_konnectorManager, SIGNAL( sync( Konnector *, Syncee::PtrList ) ),
             SLOT( slotSync( Konnector *,  Syncee::PtrList) ) );
    connect( m_konnectorManager, SIGNAL( progress( Konnector *, const Progress& ) ),
             SLOT( slotKonnectorProg( Konnector *, const Progress&) ) );
    connect( m_konnectorManager, SIGNAL( error( Konnector *, const Error& ) ),
             SLOT( slotKonnectorErr( Konnector *, const Error&) ) );
    connect( m_konnectorManager, SIGNAL( downloaded( Konnector *, Syncee::PtrList ) ),
             SIGNAL( konnectorDownloaded( Konnector *, Syncee::PtrList) ) );
}

/*
 * we're now initializing the ordinary profiles
 */
void KSyncMainWindow::initProfiles()
{
    kdDebug() << "KSyncMainWindow::initProfiles()" << endl;

    emit partProgress(0, Progress(i18n("Loading previously loaded Konnectors") ) );
    m_konprof = new KonnectorProfileManager();
    m_konprof->load();

    /* now load the wasLoaded() items */
    KonnectorProfile::ValueList list = m_konprof->list();
    KonnectorProfile::ValueList newL;
    KonnectorProfile::ValueList::Iterator konIt;
    for (konIt= list.begin(); konIt != list.end(); ++konIt ) {
        KonnectorProfile prof = (*konIt);
        if ( (*konIt).wasLoaded() ) {
            emit partProgress(0, Progress(i18n("Loading %1").arg( (*konIt).name() ) ) );
            Konnector *k = m_konnectorManager->load( prof.device() );
            prof.setKonnector( k );
            k->setCapabilities( prof.kapabilities() );
        }
        newL.append( prof );
    }

    m_konprof->setList( newL );
    initKonnectorList();
    slotKonnectorProfile();
    /* end the hack */

    m_prof = new ProfileManager();
    m_prof->load();
    initProfileList();
    slotProfile();

    kdDebug() << "KSyncMainWindow::initProfiles() done" << endl;
}

Profile KSyncMainWindow::currentProfile() const
{
    return m_prof->currentProfile();
}

ProfileManager* KSyncMainWindow::profileManager() const
{
    return m_prof;
}

KonnectorProfile KSyncMainWindow::konnectorProfile() const
{
    return m_konprof->current();
}

KonnectorProfileManager* KSyncMainWindow::konnectorProfileManager() const
{
    return m_konprof;
}

// do we need to change the Konnector first?
// raise overview and then pipe informations
// when switching to KSyncee/KSyncEntry we will make
// it asynchronus
void KSyncMainWindow::slotSync( Konnector *konnector, Syncee::PtrList lis)
{
    if ( konnector != konnectorProfile().konnector() ) {
        emit partError( 0, Error(i18n("A Konnector wanted to sync but it's not the current one") ) );
        m_konnectorManager->write( konnector, lis );
        return;
    }
    if ( m_isSyncing ) {
        emit partError( 0, Error(i18n("A sync is currently taking place. We will just ignore this request.") ) );
        return;
    }
    m_isSyncing = true;
    emit startSync();
    m_outSyncee.clear();
    m_inSyncee = lis;
    kdDebug(5210) << "KSyncMainWindow::Start sync" << endl;
    m_partsIt = new QPtrListIterator<ManipulatorPart>( m_parts );

    ManipulatorPart *part = m_partsIt->current();
    if ( part ) {
        kdDebug(5210) << "Syncing first " << endl;
        emit startSync( part );
        emit syncProgress( part, 0, 0 );
        part->sync( m_inSyncee, m_outSyncee );
    } else {
        emit partProgress( 0, Progress(i18n("Error could not start syncing with the parts.") ) );
        delete m_partsIt;
        m_partsIt = 0;
        m_konnectorManager->write( konnector, lis );
        m_isSyncing = false;
    }
}

/*
 * configure profiles
 */
void KSyncMainWindow::slotConfigProf()
{
    ProfileDialog dlg( m_prof->profiles(), m_partsLst ) ;
    if ( dlg.exec() ) {
        m_prof->setProfiles( dlg.profiles() );
        m_prof->save();
        // switch profile
        initProfileList();
        slotProfile();
    }
}

void KSyncMainWindow::switchProfile( const Profile &prof )
{
    m_bar->clear();

    m_parts.setAutoDelete( true );
    m_parts.clear();
    delete m_partsIt;
    m_partsIt = 0;

    ManPartService::ValueList lst = prof.manParts();
    ManPartService::ValueList::Iterator it;
    for (it = lst.begin(); it != lst.end(); ++it ) {
        addPart( (*it) );
    }
    Profile oldprof = m_prof->currentProfile();
    m_prof->setCurrentProfile( oldprof );
    emit profileChanged( prof );
}

void KSyncMainWindow::addPart( const ManPartService &service )
{
    ManipulatorPart *part = KParts::ComponentFactory
                            ::createInstanceFromLibrary<ManipulatorPart>( service.libname().local8Bit(),
                                                         this );
    if ( part ) addModPart( part );
}

void KSyncMainWindow::switchProfile( KonnectorProfile &prof )
{
    m_konBar->setName( prof.name() );

    KonnectorProfile ole = m_konprof->current();

    if ( !prof.konnector() ) {
        Konnector *k = m_konnectorManager->load( prof.device() );
        if ( !k ) {
          kdDebug() << "KSyncMainWindow::switchProfile: Can't load Konnector."
                    << endl;
          return;
        }
        prof.setKonnector( k );
        k->setCapabilities( prof.kapabilities() );
    }
    m_konprof->setCurrent( prof );
    m_konBar->setState( prof.konnector()->isConnected() );
    m_tray->setState( prof.konnector()->isConnected() );

    emit konnectorChanged( ole.konnector() );
    emit konnectorChanged( ole );
}

/*
 * configure current loaded
 * we will hack our SyncAlgo configurator into  that
 * that widget
 */
void KSyncMainWindow::slotConfigCur()
{
    ConfigureDialog *dlg = new ConfigureDialog(this);
    ManipulatorPart *part = 0;
    SyncConfig* conf = new SyncConfig( currentProfile().confirmDelete(), currentProfile().confirmSync() );
    dlg->addWidget( conf, i18n("General"), new QPixmap( KGlobal::iconLoader()->loadIcon("package_settings", KIcon::Desktop, 48 ) ) );

    for (part = m_parts.first(); part != 0; part = m_parts.next() ) {
        if( part->configIsVisible() )
            dlg->addWidget(part->configWidget(),
                           part->name(),
                           part->pixmap() );
    }
    if (dlg->exec()) {
        Profile prof =  currentProfile();
        prof.setConfirmSync( conf->confirmSync() );
        prof.setConfirmDelete( conf->confirmDelete() );
        profileManager()->replaceProfile( prof );
        profileManager()->setCurrentProfile( prof );

        for (part = m_parts.first(); part != 0; part = m_parts.next() ) {
            part->slotConfigOk();
        }
    }
    delete dlg;
    m_prof->save();
}

void KSyncMainWindow::slotKonnectorProfile()
{
    int item = m_konAct->currentItem();
    if ( item == -1 ) item = 0;
    if ( m_konprof->count() == 0 ) return;

#if 0 // I don't understand this code - cornelius
    // make sure it honors the deactivated plugins - not really nice like this, maybe do it better later -Max
    // IT is hitting an assert -zecke
    for ( int j = 0; j <= item; j++ ) {
        if ( !m_konprof->profile( j ).konnector() ) {
            item++;
        }
    }

    if ( item > (int)m_konprof->count() ) return;
#endif

    KonnectorProfile cur = m_konprof->profile( item );
    switchProfile( cur );
    m_konprof->setCurrent( cur );
}

/*
 * the Profile was changed in the Profile KSelectAction
 */
void KSyncMainWindow::slotProfile()
{
    int item = m_profAct->currentItem();
    if ( item < 0 ) item = 0; // for initialisation
    if ( m_prof->count() == 0 ) return;

    Profile cur = m_prof->profile( item );

    switchProfile( cur );
    m_prof->setCurrentProfile( cur );
}

void KSyncMainWindow::initProfileList()
{
    Profile::ValueList list = m_prof->profiles();
    Profile::ValueList::Iterator it;
    QStringList lst;
    for (it = list.begin(); it != list.end(); ++it ) {
        lst << (*it).name();
    }
    m_profAct->setItems( lst);
}

void KSyncMainWindow::initKonnectorList()
{
    KonnectorProfile::ValueList list = m_konprof->list();
    KonnectorProfile::ValueList::Iterator it;
    QStringList lst;

    for ( it = list.begin() ; it != list.end(); ++it ) {
        // only show activated connection profiles
        if ( !(*it).konnector() )  {
            lst << (*it).name();
        }
    }
    m_konAct->setItems( lst );
}

SyncUi* KSyncMainWindow::syncUi()
{
    m_syncUi = new SyncUiKde( this, currentProfile().confirmDelete(), true );
    return m_syncUi;
}

SyncAlgorithm* KSyncMainWindow::syncAlgorithm()
{
    m_syncAlg = new PIMSyncAlg( syncUi() );

    return m_syncAlg;
}

const QPtrList<ManipulatorPart> KSyncMainWindow::parts() const
{
    return m_parts;
}

void KSyncMainWindow::slotKonnectorProg( Konnector *konnector,
                                         const Progress & prog )
{
    /*
     * see if it's the current Konnector and then look for errors
     * and success
     */
    if ( konnector != konnectorProfile().konnector() ) {
        switch( prog.code() ) {
        case Progress::Connected:
            m_konBar->setState( true );
            m_tray->setState( true );
            break;
        case Progress::Done:
            emit doneSync();
            m_isSyncing = false;
            break;
        default:
            break;
        }
    }
    emit konnectorProgress( konnector, prog );
}

void KSyncMainWindow::slotKonnectorErr( Konnector *konnector,
                                        const Error & prog )
{
    if ( konnector == konnectorProfile().konnector() ) {
        switch( prog.code() ) {
          case Error::ConnectionLost: // fall through
          case Error::CouldNotConnect:
            m_konBar->setState( false );
            m_tray->setState( false );
            break;
          case Error::CouldNotDisconnect:
            if ( konnector->isConnected() ) m_konBar->setState( true );
            m_tray->setState( true );
          default:
            break;
        }
    }
    emit konnectorError( konnector, prog );
}

/*
 * emitted when one part is done with syncing
 * go to the next part and continue
 */
void KSyncMainWindow::slotPartProg( ManipulatorPart* par, int prog )
{
    kdDebug(5210) << "PartProg: " << par << " " << prog << endl;
    if (prog != 2 ) return;

}

void KSyncMainWindow::slotPartProg( ManipulatorPart* part, const Progress& prog)
{
    emit partProgress( part, prog );
    emit syncProgress( part, 1, 0 );
}

void KSyncMainWindow::slotPartErr( ManipulatorPart* part, const Error& err )
{
    emit partError( part, err );
    emit syncProgress( part, 3, 0 );
}

void KSyncMainWindow::slotPartSyncStatus( ManipulatorPart* par, int err )
{
    kdDebug(5210) << "SyncStatus: " << par << " " << err << endl;
    emit doneSync( par );
    emit syncProgress( par, 2, 0 );
    // done() from ManipulatorPart now go on to the next ManipulatorPart...
    ++(*m_partsIt);
    ManipulatorPart* part = m_partsIt->current();
    if ( part ) {
        kdDebug(5210) << "Syncing " << part->name() << endl;
        emit startSync( part );
        emit syncProgress( part, 0, 0 );
        part->sync( m_inSyncee, m_outSyncee );
    } else { // we're done go write it back
        emit partProgress( 0, Progress(i18n("Going to write the information back now.") ) );
        m_inSyncee.setAutoDelete( true );
        m_inSyncee.clear();
        delete m_partsIt;
        m_partsIt = 0;
        kdDebug(5210) << "Going to write back " << m_outSyncee.count() << endl;
        m_konnectorManager->write( konnectorProfile().konnector(), m_outSyncee );
        m_outSyncee.setAutoDelete( false );
        m_outSyncee.clear();
        // now we only wait for the done
    }
}

QWidget* KSyncMainWindow::widgetStack()
{
    return m_stack;
}

void KSyncMainWindow::slotKonnectorBar( bool b )
{
    kdDebug(5210) << "slotKonnectorBar " << b << endl;

    Konnector *k = konnectorProfile().konnector();
    if ( b ) {
        if ( k->isConnected() ) {
            kdDebug(5210) << "Going to connect " << endl;
            k->connectDevice();
        }
    } else {
        kdDebug(5210) << "disconnecting " << endl;
        k->disconnectDevice();
        m_konBar->setState( b );
        m_tray->setState( b );
    }
}

#include "mainwindow.moc"
