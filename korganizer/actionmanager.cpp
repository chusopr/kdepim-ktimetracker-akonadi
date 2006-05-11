/*
  This file is part of KOrganizer.

  Copyright (c) 2002 Mike Pilone <mpilone@slac.com>
  Copyright (c) 2002 Don Sanders <sanders@kde.org>
  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "actionmanager.h"

#include "alarmclient.h"
#include "calendarview.h"
#include "kocore.h"
#include "kodialogmanager.h"
#include "koglobals.h"
#include "koprefs.h"
#include "koviewmanager.h"
#include "kowindowlist.h"
#include "kprocess.h"
#include "konewstuff.h"
#include "history.h"
#include "kogroupware.h"
#include "resourceview.h"
#include "importdialog.h"
#include "eventarchiver.h"
#include "stdcalendar.h"

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/htmlexport.h>
#include <libkcal/htmlexportsettings.h>

#include <dcopclient.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <kkeydialog.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <ktip.h>
#include <ktempfile.h>
#include <kxmlguiclient.h>
#include <kwin.h>
#include <knotification.h>
#include <kstdguiitem.h>
#include <kdeversion.h>
#include <kactionclasses.h>
#include <krecentfilesaction.h>
#include <kstdaction.h>
#include <k3widgetaction.h>
#include <QApplication>
#include <QTimer>
#include <QLabel>


// FIXME: Several places in the file don't use KConfigXT yet!
KOWindowList *ActionManager::mWindowList = 0;

ActionManager::ActionManager( KXMLGUIClient *client, CalendarView *widget,
                              QObject *parent, KOrg::MainWindow *mainWindow,
                              bool isPart )
  : QObject( parent ), KCalendarIface(), mRecent( 0 ),
    mResourceButtonsAction( 0 ), mResourceViewShowAction( 0 ), mCalendar( 0 ),
    mCalendarResources( 0 ), mResourceView( 0 ), mIsClosing( false )
{
  mGUIClient = client;
  mACollection = mGUIClient->actionCollection();
  mCalendarView = widget;
  mIsPart = isPart;
  mTempFile = 0;
  mNewStuff = 0;
  mHtmlExportSync = false;
  mMainWindow = mainWindow;
}

ActionManager::~ActionManager()
{
  delete mNewStuff;

  // Remove Part plugins
  KOCore::self()->unloadParts( mMainWindow, mParts );

  delete mTempFile;

  // Take this window out of the window list.
  mWindowList->removeWindow( mMainWindow );

  delete mCalendarView;

  delete mCalendar;

  kDebug(5850) << "~ActionManager() done" << endl;
}

// see the Note: below for why this method is necessary
void ActionManager::init()
{
  // Construct the groupware object
  KOGroupware::create( mCalendarView, mCalendarResources );

  // add this instance of the window to the static list.
  if ( !mWindowList ) {
    mWindowList = new KOWindowList;
    // Show tip of the day, when the first calendar is shown.
    if ( !mIsPart )
      QTimer::singleShot( 0, this, SLOT( showTipOnStart() ) );
  }
  // Note: We need this ActionManager to be fully constructed, and
  // parent() to have a valid reference to it before the following
  // addWindow is called.
  mWindowList->addWindow( mMainWindow );

  initActions();

  // set up autoSaving stuff
  mAutoSaveTimer = new QTimer( this );
  connect( mAutoSaveTimer,SIGNAL( timeout() ), SLOT( checkAutoSave() ) );
  if ( KOPrefs::instance()->mAutoSave &&
       KOPrefs::instance()->mAutoSaveInterval > 0 ) {
    mAutoSaveTimer->start( 1000 * 60 * KOPrefs::instance()->mAutoSaveInterval );
  }

  mAutoArchiveTimer = new QTimer( this );
  mAutoArchiveTimer->setSingleShot( true );
  connect( mAutoArchiveTimer, SIGNAL( timeout() ), SLOT( slotAutoArchive() ) );
  // First auto-archive should be in 5 minutes (like in kmail).
  if ( KOPrefs::instance()->mAutoArchive )
    mAutoArchiveTimer->start( 5 * 60 * 1000 ); // singleshot

  setTitle();

  connect( mCalendarView, SIGNAL( modifiedChanged( bool ) ), SLOT( setTitle() ) );
  connect( mCalendarView, SIGNAL( configChanged() ), SLOT( updateConfig() ) );

  connect( mCalendarView, SIGNAL( incidenceSelected( Incidence * ) ),
           this, SLOT( processIncidenceSelection( Incidence * ) ) );
  connect( mCalendarView, SIGNAL( exportHTML( HTMLExportSettings * ) ),
           this, SLOT( exportHTML( HTMLExportSettings * ) ) );

  processIncidenceSelection( 0 );

  // Update state of paste action
  mCalendarView->checkClipboard();
}

void ActionManager::createCalendarLocal()
{
  mCalendar = new CalendarLocal( KOPrefs::instance()->mTimeZoneId );
  mCalendarView->setCalendar( mCalendar );
  mCalendarView->readSettings();

  initCalendar( mCalendar );
}

void ActionManager::createCalendarResources()
{
  mCalendarResources = KOrg::StdCalendar::self();

  CalendarResourceManager *manager = mCalendarResources->resourceManager();

  kDebug(5850) << "CalendarResources used by KOrganizer:" << endl;
  CalendarResourceManager::Iterator it;
  for( it = manager->begin(); it != manager->end(); ++it ) {
    kDebug(5850) << "  " << (*it)->resourceName() << endl;
    (*it)->setResolveConflict( true );
//    (*it)->dump();
  }

  setDestinationPolicy();

  mCalendarView->setCalendar( mCalendarResources );
  mCalendarView->readSettings();

  ResourceViewFactory factory( mCalendarResources, mCalendarView );
  mCalendarView->addExtension( &factory );
  mResourceView = factory.resourceView();

  connect( mCalendarResources, SIGNAL( calendarChanged() ),
           mCalendarView, SLOT( slotCalendarChanged() ) );
  connect( mCalendarResources, SIGNAL( signalErrorMessage( const QString & ) ),
           mCalendarView, SLOT( showErrorMessage( const QString & ) ) );

  connect( mCalendarView, SIGNAL( configChanged() ),
           SLOT( updateConfig() ) );

  initCalendar( mCalendarResources );
}

void ActionManager::initCalendar( Calendar *cal )
{
  cal->setOwner( Person( KOPrefs::instance()->fullName(),
                         KOPrefs::instance()->email() ) );
  // setting fullName and email do not really count as modifying the calendar
  mCalendarView->setModified( false );
}

void ActionManager::initActions()
{
  KAction *action;


  //*************************** FILE MENU **********************************

  //~~~~~~~~~~~~~~~~~~~~~~~ LOADING / SAVING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if ( mIsPart ) {
    if ( mMainWindow->hasDocument() ) {
      KStdAction::openNew( this, SLOT(file_new()), mACollection, "korganizer_openNew" );
      KStdAction::open( this, SLOT( file_open() ), mACollection, "korganizer_open" );
      mRecent = KStdAction::openRecent( this, SLOT( file_open( const KUrl& ) ),
                                     mACollection, "korganizer_openRecent" );
      KStdAction::revert( this,SLOT( file_revert() ), mACollection, "korganizer_revert" );
      KStdAction::saveAs( this, SLOT( file_saveas() ), mACollection,
                   "korganizer_saveAs" );
      KStdAction::save( this, SLOT( file_save() ), mACollection, "korganizer_save" );
    }
    KStdAction::print( mCalendarView, SLOT( print() ), mACollection, "korganizer_print" );
  } else {
    KStdAction::openNew( this, SLOT( file_new() ), mACollection );
    KStdAction::open( this, SLOT( file_open() ), mACollection );
    mRecent = KStdAction::openRecent( this, SLOT( file_open( const KUrl& ) ),
                                     mACollection );
    if ( mMainWindow->hasDocument() ) {
      KStdAction::revert( this,SLOT( file_revert() ), mACollection );
      KStdAction::save( this, SLOT( file_save() ), mACollection );
      KStdAction::saveAs( this, SLOT( file_saveas() ), mACollection );
    }
    KStdAction::print( mCalendarView, SLOT( print() ), mACollection );
  }


  //~~~~~~~~~~~~~~~~~~~~~~~~ IMPORT / EXPORT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  action = new KAction( i18n("Import &Calendar..."), mACollection, "import_icalendar" );
  connect(action, SIGNAL(triggered(bool) ), SLOT( file_merge() ));
  action = new KAction( i18n("&Import From UNIX Ical tool"), mACollection, "import_ical" );
  connect(action, SIGNAL(triggered(bool) ), SLOT( file_icalimport() ));
  action = new KAction( i18n("Get &Hot New Stuff..."), mACollection, "downloadnewstuff" );
  connect(action, SIGNAL(triggered(bool) ), SLOT( downloadNewStuff() ));

  action = new KAction(KIcon("webexport"),  i18n("Export &Web Page..."), mACollection, "export_web" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( exportWeb() ));
  action = new KAction( i18n("&iCalendar..."), mACollection, "export_icalendar" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( exportICalendar() ));
  action = new KAction( i18n("&vCalendar..."), mACollection, "export_vcalendar" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( exportVCalendar() ));
  action = new KAction( i18n("Upload &Hot New Stuff..."), mACollection, "uploadnewstuff" );
  connect(action, SIGNAL(triggered(bool) ), SLOT( uploadNewStuff() ));



  action = new KAction( i18n("Archive O&ld Entries..."), mACollection, "file_archive" );
  connect(action, SIGNAL(triggered(bool) ), SLOT( file_archive() ));
  action = new KAction( i18nc("delete completed to-dos", "Pur&ge Completed To-dos"),
                        mACollection, "purge_completed" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( purgeCompleted() ));


  //************************** EDIT MENU *********************************
  KAction *pasteAction;
  KOrg::History *h = mCalendarView->history();
  if ( mIsPart ) {
    // edit menu
    mCutAction = KStdAction::cut( mCalendarView, SLOT( edit_cut() ),
                                  mACollection, "korganizer_cut" );
    mCopyAction = KStdAction::copy( mCalendarView, SLOT( edit_copy() ),
                                    mACollection, "korganizer_copy" );
    pasteAction = KStdAction::paste( mCalendarView, SLOT( edit_paste() ),
                                     mACollection, "korganizer_paste" );
    mUndoAction = KStdAction::undo( h, SLOT( undo() ),
                                    mACollection, "korganizer_undo" );
    mRedoAction = KStdAction::redo( h, SLOT( redo() ),
                                    mACollection, "korganizer_redo" );
  } else {
    mCutAction = KStdAction::cut( mCalendarView,SLOT( edit_cut() ),
                                  mACollection );
    mCopyAction = KStdAction::copy( mCalendarView,SLOT( edit_copy() ),
                                    mACollection );
    pasteAction = KStdAction::paste( mCalendarView,SLOT( edit_paste() ),
                                     mACollection );
    mUndoAction = KStdAction::undo( h, SLOT( undo() ), mACollection );
    mRedoAction = KStdAction::redo( h, SLOT( redo() ), mACollection );
  }
  mDeleteAction = new KAction(KIcon("editdelete"),  i18n("&Delete"), mACollection, "edit_delete" );
  connect(mDeleteAction, SIGNAL(triggered(bool) ), mCalendarView, SLOT( appointment_delete() ));
  if ( mIsPart ) {
    KStdAction::find( mCalendarView->dialogManager(), SLOT( showSearchDialog() ),
                     mACollection, "korganizer_find" );
  } else {
    KStdAction::find( mCalendarView->dialogManager(), SLOT( showSearchDialog() ),
                     mACollection );
  }
  pasteAction->setEnabled( false );
  mUndoAction->setEnabled( false );
  mRedoAction->setEnabled( false );
  connect( mCalendarView, SIGNAL( pasteEnabled( bool ) ),
           pasteAction, SLOT( setEnabled( bool ) ) );
  connect( h, SIGNAL( undoAvailable( const QString & ) ),
           SLOT( updateUndoAction( const QString & ) ) );
  connect( h, SIGNAL( redoAvailable( const QString & ) ),
           SLOT( updateRedoAction( const QString & ) ) );




  //************************** VIEW MENU *********************************

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ VIEWS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  action = new KAction(KIcon("whatsnext"),  i18n("What's &Next"), mACollection, "view_whatsnext" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( showWhatsNextView() ));
  action = new KAction(KIcon("1day"),  i18n("&Day"), mACollection, "view_day" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( showDayView() ));
  mNextXDays = new KAction(KIcon("xdays"),  QString(), mACollection, "view_nextx" );
  connect(mNextXDays, SIGNAL(triggered(bool)), mCalendarView->viewManager(), SLOT( showNextXView() ));
  mNextXDays->setText( i18np( "&Next Day", "Ne&xt %n Days",
                             KOPrefs::instance()->mNextXDays ) );
  action = new KAction(KIcon("5days"),  i18n("W&ork Week"), mACollection, "view_workweek" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( showWorkWeekView() ));
  action = new KAction(KIcon("7days"),  i18n("&Week"), mACollection, "view_week" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( showWeekView() ));
  action = new KAction(KIcon("month"),  i18n("&Month"), mACollection, "view_month" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( showMonthView() ));
  action = new KAction(KIcon("list"),  i18n("&List"), mACollection, "view_list" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( showListView() ));
  action = new KAction(KIcon("todo"),  i18n("&To-do List"), mACollection, "view_todo" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( showTodoView() ));
  action = new KAction(KIcon("journal"),  i18n("&Journal"), mACollection, "view_journal" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( showJournalView() ));


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~ FILTERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  action = new KAction( i18n("&Refresh"), mACollection, "update" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( updateView() ));
// TODO:
//   new KAction( i18n("Hide &Completed To-dos"), 0,
//                     mCalendarView, SLOT( toggleHideCompleted() ),
//                     mACollection, "hide_completed_todos" );

  mFilterAction = new KSelectAction( i18n("F&ilter"),
                  mACollection, "filter_select" );
  mFilterAction->setEditable( false );
  connect( mFilterAction, SIGNAL( activated(int) ),
           mCalendarView, SLOT( filterActivated( int ) ) );
  connect( mCalendarView, SIGNAL( newFilterListSignal( const QStringList & ) ),
           mFilterAction, SLOT( setItems( const QStringList & ) ) );
  connect( mCalendarView, SIGNAL( selectFilterSignal( int ) ),
           mFilterAction, SLOT( setCurrentItem( int ) ) );
  connect( mCalendarView, SIGNAL( filterChanged() ),
           this, SLOT( setTitle() ) );


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ZOOM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // TODO: try to find / create better icons for the following 4 actions
  action = new KAction(KIcon("viewmag+"),  i18n( "Zoom In Horizontally" ), mACollection, "zoom_in_horizontally" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( zoomInHorizontally() ));
  action = new KAction(KIcon("viewmag-"),  i18n( "Zoom Out Horizontally" ), mACollection, "zoom_out_horizontally" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( zoomOutHorizontally() ));
  action = new KAction(KIcon("viewmag+"),  i18n( "Zoom In Vertically" ), mACollection, "zoom_in_vertically" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( zoomInVertically() ));
  action = new KAction(KIcon("viewmag-"),  i18n( "Zoom Out Vertically" ), mACollection, "zoom_out_vertically" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->viewManager(), SLOT( zoomOutVertically() ));




  //************************** Actions MENU *********************************

  action = new KAction(KIcon("today"),  i18n("Go to &Today"), mACollection, "go_today" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( goToday() ));
  bool isRTL = QApplication::isRightToLeft();
  action = new KAction(KIcon(isRTL ? "forward" : "back"),  i18n("Go &Backward"), mACollection, "go_previous" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( goPrevious() ));

  // Changing the action text by setText makes the toolbar button disappear.
  // This has to be fixed first, before the connects below can be reenabled.
  /*
  connect( mCalendarView, SIGNAL( changeNavStringPrev( const QString & ) ),
           action, SLOT( setText( const QString & ) ) );
  connect( mCalendarView, SIGNAL( changeNavStringPrev( const QString & ) ),
           this, SLOT( dumpText( const QString & ) ) );*/

  action = new KAction(KIcon(isRTL ? "back" : "forward"),  i18n("Go &Forward"), mACollection, "go_next" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( goNext() ));
  /*
  connect( mCalendarView,SIGNAL( changeNavStringNext( const QString & ) ),
           action,SLOT( setText( const QString & ) ) );
  */


  //************************** Actions MENU *********************************
  action = new KAction(KIcon("appointment"),  i18n("New E&vent..."), mACollection, "new_event" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( newEvent() ));
  action = new KAction(KIcon("newtodo"),  i18n("New &To-do..."), mACollection, "new_todo" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( newTodo() ));
  action = new KAction( i18n("New Su&b-to-do..."), mACollection, "new_subtodo" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( newSubTodo() ));
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( todoSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );
  action = new KAction( i18n("New &Journal..."), mACollection, "new_journal" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( newJournal() ));

  mShowIncidenceAction = new KAction( i18n("&Show"), mACollection, "show_incidence" );
  connect(mShowIncidenceAction, SIGNAL(triggered(bool) ), mCalendarView, SLOT( showIncidence() ));
  mEditIncidenceAction = new KAction( i18n("&Edit..."), mACollection, "edit_incidence" );
  connect(mEditIncidenceAction, SIGNAL(triggered(bool) ), mCalendarView, SLOT( editIncidence() ));
  mDeleteIncidenceAction = new KAction( i18n("&Delete"), mACollection, "delete_incidence" );
  connect(mDeleteIncidenceAction, SIGNAL(triggered(bool) ), mCalendarView, SLOT( deleteIncidence()));
  mDeleteIncidenceAction->setShortcut(Qt::Key_Delete);

  action = new KAction( i18n("&Make Sub-to-do Independent"), mACollection, "unsub_todo" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( todo_unsub() ));
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( subtodoSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );
// TODO: Add item to move the incidence to different resource
//   mAssignResourceAction = new KAction( i18n("Assign &Resource..."), 0,
//                                        mCalendarView, SLOT( assignResource()),
//                                        mACollection, "assign_resource" );
// TODO: Add item to quickly toggle the reminder of a given incidence
//   mToggleAlarmAction = new KToggleAction( i18n("&Activate Reminder"), 0,
//                                         mCalendarView, SLOT( toggleAlarm()),
//                                         mACollection, "activate_alarm" );




  //************************** SCHEDULE MENU ********************************
  mPublishEvent = new KAction(KIcon("mail_send"),  i18n("&Publish Item Information..."), mACollection, "schedule_publish" );
  connect(mPublishEvent, SIGNAL(triggered(bool) ), mCalendarView, SLOT( schedule_publish() ));
  mPublishEvent->setEnabled( false );

  action = new KAction(KIcon("mail_generic"),  i18n("Send &Invitation to Attendees"), mACollection, "schedule_request" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( schedule_request() ));
  action->setEnabled( false );
  connect( mCalendarView, SIGNAL( organizerEventsSelected( bool ) ),
           action, SLOT( setEnabled( bool ) ) );

  action = new KAction( i18n("Re&quest Update"), mACollection, "schedule_refresh" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( schedule_refresh() ));
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( groupEventsSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );

  action = new KAction( i18n("Send &Cancellation to Attendees"), mACollection, "schedule_cancel" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( schedule_cancel() ));
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( organizerEventsSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );

  action = new KAction(KIcon("mail_reply"),  i18n("Send Status &Update"), mACollection, "schedule_reply" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( schedule_reply() ));
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( groupEventsSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );

  action = new KAction( i18nc("counter proposal","Request Chan&ge"),0,
                        mCalendarView,SLOT( schedule_counter() ), mACollection, "schedule_counter" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( groupEventsSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );

  action = new KAction( i18n("&Mail Free Busy Information..."), mACollection, "mail_freebusy" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( mailFreeBusy() ));
  action->setEnabled( true );

  action = new KAction( i18n("&Upload Free Busy Information"), mACollection, "upload_freebusy" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( uploadFreeBusy() ));
  action->setEnabled( true );

  if ( !mIsPart ) {
      action = new KAction(KIcon("contents"),  i18n("&Addressbook"), mACollection, "addressbook" );
      connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( openAddressbook() ));
  }




  //************************** SETTINGS MENU ********************************

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIDEBAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mDateNavigatorShowAction = new KToggleAction( i18n("Show Date Navigator"), 0,
                      this, SLOT( toggleDateNavigator() ),
                      mACollection, "show_datenavigator" );
  mTodoViewShowAction = new KToggleAction ( i18n("Show To-do View"), 0,
                      this, SLOT( toggleTodoView() ),
                      mACollection, "show_todoview" );
  mEventViewerShowAction = new KToggleAction ( i18n("Show Item Viewer"), 0,
                      this, SLOT( toggleEventViewer() ),
                      mACollection, "show_eventviewer" );
  KConfig *config = KOGlobals::self()->config();
  config->setGroup( "Settings" );
  mDateNavigatorShowAction->setChecked(
      config->readEntry( "DateNavigatorVisible", true ) );
  // if we are a kpart, then let's not show the todo in the left pane by
  // default since there's also a Todo part and we'll assume they'll be
  // using that as well, so let's not duplicate it (by default) here
  mTodoViewShowAction->setChecked(
      config->readEntry( "TodoViewVisible", mIsPart ? false : true ) );
  mEventViewerShowAction->setChecked(
      config->readEntry( "EventViewerVisible", true ) );
  toggleDateNavigator();
  toggleTodoView();
  toggleEventViewer();

  if ( !mMainWindow->hasDocument() ) {
    mResourceViewShowAction = new KToggleAction ( i18n("Show Resource View"), 0,
                        this, SLOT( toggleResourceView() ),
                        mACollection, "show_resourceview" );
    mResourceButtonsAction = new KToggleAction( i18n("Show &Resource Buttons"), 0,
                        this, SLOT( toggleResourceButtons() ),
                        mACollection, "show_resourcebuttons" );
    mResourceViewShowAction->setChecked(
        config->readEntry( "ResourceViewVisible", true ) );
    mResourceButtonsAction->setChecked(
        config->readEntry( "ResourceButtonsVisible", true ) );

    toggleResourceView();
    toggleResourceButtons();
  }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIDEBAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  action = new KAction( i18n("Configure &Date && Time..."), mACollection, "conf_datetime" );
  connect(action, SIGNAL(triggered(bool) ), SLOT( configureDateTime() ));
// TODO: Add an item to show the resource management dlg
//   new KAction( i18n("Manage &Resources..."), 0,
//                     this, SLOT( manageResources() ),
//                     mACollection, "conf_resources" );
  action = new KAction(KIcon("configure"),  i18n("Manage View &Filters..."), mACollection, "edit_filters" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( editFilters() ));
  action = new KAction( i18n("Manage C&ategories..."), mACollection, "edit_categories" );
  connect(action, SIGNAL(triggered(bool) ), mCalendarView->dialogManager(), SLOT( showCategoryEditDialog() ));
  if ( mIsPart ) {
    action = new KAction(KIcon("configure"),  i18n("&Configure Calendar..."), mACollection, "korganizer_configure" );
    connect(action, SIGNAL(triggered(bool) ), mCalendarView, SLOT( edit_options() ));
    KStdAction::keyBindings( this, SLOT( keyBindings() ),
                             mACollection, "korganizer_configure_shortcuts" );
  } else {
    KStdAction::preferences( mCalendarView, SLOT( edit_options() ),
                            mACollection );
    KStdAction::keyBindings( this, SLOT( keyBindings() ), mACollection );
  }




  //**************************** HELP MENU **********************************
  KStdAction::tipOfDay( this, SLOT( showTip() ), mACollection,
                        "help_tipofday" );
//   new KAction( i18n("Show Intro Page"), 0,
//                     mCalendarView,SLOT( showIntro() ),
//                     mACollection,"show_intro" );




  //************************* TOOLBAR ACTIONS *******************************
  QLabel *filterLabel = new QLabel( i18n("Filter: "), mCalendarView );
  filterLabel->hide();
  new K3WidgetAction( filterLabel, i18n("Filter: "), 0, 0, 0,
                     mACollection, "filter_label" );

}

void ActionManager::readSettings()
{
  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config = KOGlobals::self()->config();
  if ( mRecent ) mRecent->loadEntries( config );
  mCalendarView->readSettings();
}

void ActionManager::writeSettings()
{
  kDebug(5850) << "ActionManager::writeSettings" << endl;

  KConfig *config = KOGlobals::self()->config();
  mCalendarView->writeSettings();

  config->setGroup( "Settings" );
  if ( mResourceButtonsAction ) {
    config->writeEntry( "ResourceButtonsVisible",
                        mResourceButtonsAction->isChecked() );
  }
  if ( mDateNavigatorShowAction ) {
    config->writeEntry( "DateNavigatorVisible",
                        mDateNavigatorShowAction->isChecked() );
  }
  if ( mTodoViewShowAction ) {
    config->writeEntry( "TodoViewVisible",
                        mTodoViewShowAction->isChecked() );
  }
  if ( mResourceViewShowAction ) {
    config->writeEntry( "ResourceViewVisible",
                        mResourceViewShowAction->isChecked() );
  }
  if ( mEventViewerShowAction ) {
    config->writeEntry( "EventViewerVisible",
                        mEventViewerShowAction->isChecked() );
  }

  if ( mRecent ) mRecent->saveEntries( config );

  config->sync();

  if ( mCalendarResources ) {
    mCalendarResources->resourceManager()->writeConfig();
  }
}

void ActionManager::file_new()
{
  emit actionNew();
}

void ActionManager::file_open()
{
  KUrl url;
  QString defaultPath = locateLocal( "data","korganizer/" );
  url = KFileDialog::getOpenURL( defaultPath,i18n("*.vcs *.ics|Calendar Files"),
                                dialogParent() );

  file_open( url );
}

void ActionManager::file_open( const KUrl &url )
{
  if ( url.isEmpty() ) return;

  // is that URL already opened somewhere else? Activate that window
  KOrg::MainWindow *korg=ActionManager::findInstance( url );
  if ( ( 0 != korg )&&( korg != mMainWindow ) ) {
    KWin::activateWindow( korg->topLevelWidget()->winId() );
    return;
  }

  kDebug(5850) << "ActionManager::file_open(): " << url.prettyURL() << endl;

  // Open the calendar file in the same window only if we have an empty calendar window, and not the resource calendar
  if ( !mCalendarView->isModified() && mFile.isEmpty() && !mCalendarResources ) {
    openURL( url );
  } else {
    emit actionNew( url );
  }
}

void ActionManager::file_icalimport()
{
  // FIXME: eventually, we will need a dialog box to select import type, etc.
  // for now, hard-coded to ical file, $HOME/.calendar.
  int retVal = -1;
  QString progPath;
  KTempFile tmpfn;

  QString homeDir = QDir::homePath() + QString::fromLatin1( "/.calendar" );

  if ( !QFile::exists( homeDir ) ) {
    KMessageBox::error( dialogParent(),
                       i18n( "You have no ical file in your home directory.\n"
                            "Import cannot proceed.\n" ) );
    return;
  }

  KProcess proc;
  proc << "ical2vcal" << tmpfn.name();
  bool success = proc.start( KProcess::Block );

  if ( !success ) {
    kDebug(5850) << "Error starting ical2vcal." << endl;
    return;
  } else {
    retVal = proc.exitStatus();
  }

  kDebug(5850) << "ical2vcal return value: " << retVal << endl;

  if ( retVal >= 0 && retVal <= 2 ) {
    // now we need to MERGE what is in the iCal to the current calendar.
    mCalendarView->openCalendar( tmpfn.name(),1 );
    if ( !retVal )
      KMessageBox::information( dialogParent(),
                               i18n( "KOrganizer successfully imported and "
                                    "merged your .calendar file from ical "
                                    "into the currently opened calendar." ),
                               "dotCalendarImportSuccess" );
    else
      KMessageBox::information( dialogParent(),
                           i18n( "KOrganizer encountered some unknown fields while "
                                "parsing your .calendar ical file, and had to "
                                "discard them; please check to see that all "
                                "your relevant data was correctly imported." ),
                                 i18n("ICal Import Successful with Warning") );
  } else if ( retVal == -1 ) {
    KMessageBox::error( dialogParent(),
                         i18n( "KOrganizer encountered an error parsing your "
                              ".calendar file from ical; import has failed." ) );
  } else if ( retVal == -2 ) {
    KMessageBox::error( dialogParent(),
                         i18n( "KOrganizer does not think that your .calendar "
                              "file is a valid ical calendar; import has failed." ) );
  }
  tmpfn.unlink();
}

void ActionManager::file_merge()
{
  KUrl url = KFileDialog::getOpenURL( locateLocal( "data","korganizer/" ),
                                     i18n("*.vcs *.ics|Calendar Files"),
                                     dialogParent() );
  if ( ! url.isEmpty() )  // isEmpty if user canceled the dialog
    importCalendar( url );
}

void ActionManager::file_archive()
{
  mCalendarView->archiveCalendar();
}

void ActionManager::file_revert()
{
  openURL( mURL );
}

void ActionManager::file_saveas()
{
  KUrl url = getSaveURL();

  if ( url.isEmpty() ) return;

  saveAsURL( url );
}

void ActionManager::file_save()
{
  if ( mMainWindow->hasDocument() ) {
    if ( mURL.isEmpty() ) {
      file_saveas();
      return;
    } else {
      saveURL();
    }
  } else {
    mCalendarView->calendar()->save();
  }

  // export to HTML
  if ( KOPrefs::instance()->mHtmlWithSave ) {
    exportHTML();
  }
}

void ActionManager::file_close()
{
  if ( !saveModifiedURL() ) return;

  mCalendarView->closeCalendar();
  KIO::NetAccess::removeTempFile( mFile );
  mURL="";
  mFile="";

  setTitle();
}

bool ActionManager::openURL( const KUrl &url,bool merge )
{
  kDebug(5850) << "ActionManager::openURL()" << endl;

  if ( url.isEmpty() ) {
    kDebug(5850) << "ActionManager::openURL(): Error! Empty URL." << endl;
    return false;
  }
  if ( !url.isValid() ) {
    kDebug(5850) << "ActionManager::openURL(): Error! URL is malformed." << endl;
    return false;
  }

  if ( url.isLocalFile() ) {
    mURL = url;
    mFile = url.path();
    if ( !KStandardDirs::exists( mFile ) ) {
      mMainWindow->showStatusMessage( i18n("New calendar '%1'.",
                                        url.prettyURL() ) );
      mCalendarView->setModified();
    } else {
      bool success = mCalendarView->openCalendar( mFile, merge );
      if ( success ) {
        showStatusMessageOpen( url, merge );
      }
    }
    setTitle();
  } else {
    QString tmpFile;
    if( KIO::NetAccess::download( url, tmpFile, view() ) ) {
      kDebug(5850) << "--- Downloaded to " << tmpFile << endl;
      bool success = mCalendarView->openCalendar( tmpFile, merge );
      if ( merge ) {
        KIO::NetAccess::removeTempFile( tmpFile );
        if ( success )
          showStatusMessageOpen( url, merge );
      } else {
        if ( success ) {
          KIO::NetAccess::removeTempFile( mFile );
          mURL = url;
          mFile = tmpFile;
          KConfig *config = KOGlobals::self()->config();
          config->setGroup( "General" );
          setTitle();
          kDebug(5850) << "-- Add recent URL: " << url.prettyURL() << endl;
          if ( mRecent ) mRecent->addUrl( url );
          showStatusMessageOpen( url, merge );
        }
      }
      return success;
    } else {
      QString msg;
      msg = i18n("Cannot download calendar from '%1'.", url.prettyURL() );
      KMessageBox::error( dialogParent(), msg );
      return false;
    }
  }
  return true;
}

bool ActionManager::addResource( const KUrl &mUrl )
{
  CalendarResources *cr = KOrg::StdCalendar::self();

  CalendarResourceManager *manager = cr->resourceManager();

  ResourceCalendar *resource = 0;

  QString name;

  kDebug(5850) << "URL: " << mUrl << endl;
  if ( mUrl.isLocalFile() ) {
    kDebug(5850) << "Local Resource" << endl;
    resource = manager->createResource( "file" );
    if ( resource )
      resource->setValue( "File", mUrl.path() );
    name = mUrl.path();
  } else {
    kDebug(5850) << "Remote Resource" << endl;
    resource = manager->createResource( "remote" );
    if ( resource ) {
      resource->setValue( "DownloadURL", mUrl.url() );
      resource->setReadOnly( true );
    }
    name = mUrl.prettyURL();
  }

  if ( resource ) {
    resource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
    resource->setResourceName( name );
    manager->add( resource );
    mMainWindow->showStatusMessage( i18n( "Added calendar resource for URL '%1'." ,
                 name ) );
    // we have to call resourceAdded manually, because for in-process changes
    // the dcop signals are not connected, so the resource's signals would not
    // be connected otherwise
    if ( mCalendarResources )
      mCalendarResources->resourceAdded( resource );
  } else {
    QString msg = i18n("Unable to create calendar resource '%1'.",
                        name );
    KMessageBox::error( dialogParent(), msg );
  }
  return true;
}


void ActionManager::showStatusMessageOpen( const KUrl &url, bool merge )
{
  if ( merge ) {
    mMainWindow->showStatusMessage( i18n("Merged calendar '%1'.",
                                      url.prettyURL() ) );
  } else {
    mMainWindow->showStatusMessage( i18n("Opened calendar '%1'.",
                                      url.prettyURL() ) );
  }
}

void ActionManager::closeURL()
{
  kDebug(5850) << "ActionManager::closeURL()" << endl;

  file_close();
}

bool ActionManager::saveURL()
{
  QString ext;

  if ( mURL.isLocalFile() ) {
    ext = mFile.right( 4 );
  } else {
    ext = mURL.fileName().right( 4 );
  }

  if ( ext == QLatin1String(".vcs") ) {
    int result = KMessageBox::warningContinueCancel(
      dialogParent(),
      i18n( "Your calendar will be saved in iCalendar format. Use "
            "'Export vCalendar' to save in vCalendar format." ),
      i18n("Format Conversion"), i18n("Proceed"),
      QString( "dontaskFormatConversion" ), KMessageBox::Notify );
    if ( result != KMessageBox::Continue ) return false;

    QString filename = mURL.fileName();
    filename.replace( filename.length() - 4, 4, ".ics" );
    mURL.setFileName( filename );
    if ( mURL.isLocalFile() ) {
      mFile = mURL.path();
    }
    setTitle();
    if ( mRecent ) mRecent->addUrl( mURL );
  }

  if ( !mCalendarView->saveCalendar( mFile ) ) {
    kDebug(5850) << "ActionManager::saveURL(): calendar view save failed."
                  << endl;
    return false;
  } else {
    mCalendarView->setModified( false );
  }

  if ( !mURL.isLocalFile() ) {
    if ( !KIO::NetAccess::upload( mFile, mURL, view() ) ) {
      QString msg = i18n("Cannot upload calendar to '%1'",
                      mURL.prettyURL() );
      KMessageBox::error( dialogParent() ,msg );
      return false;
    }
  }

  // keep saves on a regular interval
  if ( KOPrefs::instance()->mAutoSave ) {
    mAutoSaveTimer->stop();
    mAutoSaveTimer->start( 1000*60*KOPrefs::instance()->mAutoSaveInterval );
  }

  mMainWindow->showStatusMessage( i18n("Saved calendar '%1'.", mURL.prettyURL() ) );

  return true;
}

void ActionManager::exportHTML()
{
  HTMLExportSettings settings( "KOrganizer" );
  // Manually read in the config, because parametrized kconfigxt objects don't
  // seem to load the config theirselves
  settings.readConfig();

  QDate qd1;
  qd1 = QDate::currentDate();
  QDate qd2;
  qd2 = QDate::currentDate();
  if ( settings.monthView() )
    qd2.addMonths( 1 );
  else
    qd2.addDays( 7 );
  settings.setDateStart( QDateTime( qd1 ) );
  settings.setDateEnd( QDateTime( qd2 ) );
  exportHTML( &settings );
}

void ActionManager::exportHTML( HTMLExportSettings *settings )
{
  if ( !settings || settings->outputFile().isEmpty() )
    return;
  settings->setEMail( KOPrefs::instance()->email() );
  settings->setName( KOPrefs::instance()->fullName() );

  settings->setCreditName( "KOrganizer" );
  settings->setCreditURL( "http://korganizer.kde.org" );

  KCal::HtmlExport mExport( mCalendarView->calendar(), settings );

  QDate cdate = settings->dateStart().date();
  QDate qd2 = settings->dateEnd().date();
  while ( cdate <= qd2 ) {
    if ( !KOGlobals::self()->holiday( cdate ).isEmpty() )
      mExport.addHoliday( cdate, KOGlobals::self()->holiday( cdate ) );
    cdate = cdate.addDays( 1 );
  }

  KUrl dest( settings->outputFile() );
  if ( dest.isLocalFile() ) {
    mExport.save( dest.path() );
  } else {
    KTempFile tf;
    QString tfile = tf.name();
    tf.close();
    mExport.save( tfile );
    if ( !KIO::NetAccess::upload( tfile, dest, view() ) ) {
      KNotification::event ( KNotification::Error,
                             i18n("Could not upload file.") );
    }
    tf.unlink();
  }
}

bool ActionManager::saveAsURL( const KUrl &url )
{
  kDebug(5850) << "ActionManager::saveAsURL() " << url.prettyURL() << endl;

  if ( url.isEmpty() ) {
    kDebug(5850) << "ActionManager::saveAsURL(): Empty URL." << endl;
    return false;
  }
  if ( !url.isValid() ) {
    kDebug(5850) << "ActionManager::saveAsURL(): Malformed URL." << endl;
    return false;
  }

  QString fileOrig = mFile;
  KUrl URLOrig = mURL;

  KTempFile *tempFile = 0;
  if ( url.isLocalFile() ) {
    mFile = url.path();
  } else {
    tempFile = new KTempFile;
    mFile = tempFile->name();
  }
  mURL = url;

  bool success = saveURL(); // Save local file and upload local file
  if ( success ) {
    delete mTempFile;
    mTempFile = tempFile;
    KIO::NetAccess::removeTempFile( fileOrig );
    KConfig *config = KOGlobals::self()->config();
    config->setGroup( "General" );
    setTitle();
    if ( mRecent ) mRecent->addUrl( mURL );
  } else {
    KMessageBox::sorry( dialogParent(), i18n("Unable to save calendar to the file %1.", mFile ), i18n("Error") );
    kDebug(5850) << "ActionManager::saveAsURL() failed" << endl;
    mURL = URLOrig;
    mFile = fileOrig;
    delete tempFile;
  }

  return success;
}


bool ActionManager::saveModifiedURL()
{
  kDebug(5850) << "ActionManager::saveModifiedURL()" << endl;

  // If calendar isn't modified do nothing.
  if ( !mCalendarView->isModified() ) return true;

  mHtmlExportSync = true;
  if ( KOPrefs::instance()->mAutoSave && !mURL.isEmpty() ) {
    // Save automatically, when auto save is enabled.
    return saveURL();
  } else {
    int result = KMessageBox::warningYesNoCancel(
        dialogParent(),
        i18n("The calendar has been modified.\nDo you want to save it?"),
        QString(),
        KStdGuiItem::save(), KStdGuiItem::discard() );
    switch( result ) {
      case KMessageBox::Yes:
        if ( mURL.isEmpty() ) {
          KUrl url = getSaveURL();
          return saveAsURL( url );
        } else {
          return saveURL();
        }
      case KMessageBox::No:
        return true;
      case KMessageBox::Cancel:
      default:
        {
          mHtmlExportSync = false;
          return false;
        }
    }
  }
}


KUrl ActionManager::getSaveURL()
{
  KUrl url = KFileDialog::getSaveURL( locateLocal( "data","korganizer/" ),
                                     i18n("*.vcs *.ics|Calendar Files"),
                                     dialogParent() );

  if ( url.isEmpty() ) return url;

  QString filename = url.fileName( false );

  QString e = filename.right( 4 );
  if ( e != QLatin1String(".vcs") && e != QLatin1String(".ics") ) {
    // Default save format is iCalendar
    filename += ".ics";
  }

  url.setFileName( filename );

  kDebug(5850) << "ActionManager::getSaveURL(): url: " << url.url() << endl;

  return url;
}

void ActionManager::saveProperties( KConfig *config )
{
  kDebug(5850) << "ActionManager::saveProperties" << endl;

  config->writeEntry( "UseResourceCalendar", !mMainWindow->hasDocument() );
  if ( mMainWindow->hasDocument() ) {
    config->writePathEntry( "Calendar",mURL.url() );
  }
}

void ActionManager::readProperties( KConfig *config )
{
  kDebug(5850) << "ActionManager::readProperties" << endl;

  bool isResourceCalendar(
    config->readEntry( "UseResourceCalendar", true ) );
  QString calendarUrl = config->readPathEntry( "Calendar" );

  if ( !isResourceCalendar && !calendarUrl.isEmpty() ) {
    mMainWindow->init( true );
    KUrl u( calendarUrl );
    openURL( u );
  } else {
    mMainWindow->init( false );
  }
}

void ActionManager::checkAutoSave()
{
  kDebug(5850) << "ActionManager::checkAutoSave()" << endl;

  // Don't save if auto save interval is zero
  if ( KOPrefs::instance()->mAutoSaveInterval == 0 ) return;

  // has this calendar been saved before? If yes automatically save it.
  if ( KOPrefs::instance()->mAutoSave ) {
    if ( mCalendarResources || ( mCalendar && !url().isEmpty() ) ) {
      saveCalendar();
    }
  }
}


// Configuration changed as a result of the options dialog.
void ActionManager::updateConfig()
{
  kDebug(5850) << "ActionManager::updateConfig()" << endl;

  if ( KOPrefs::instance()->mAutoSave && !mAutoSaveTimer->isActive() ) {
    checkAutoSave();
    if ( KOPrefs::instance()->mAutoSaveInterval > 0 ) {
      mAutoSaveTimer->start( 1000 * 60 *
                             KOPrefs::instance()->mAutoSaveInterval );
    }
  }
  if ( !KOPrefs::instance()->mAutoSave ) mAutoSaveTimer->stop();
  mNextXDays->setText( i18np( "&Next Day", "&Next %n Days",
                             KOPrefs::instance()->mNextXDays ) );

  KOCore::self()->reloadPlugins();
  mParts = KOCore::self()->reloadParts( mMainWindow, mParts );

  setDestinationPolicy();

  if ( mResourceView )
    mResourceView->updateView();
}

void ActionManager::setDestinationPolicy()
{
  if ( mCalendarResources ) {
    if ( KOPrefs::instance()->mDestination == KOPrefs::askDestination )
      mCalendarResources->setAskDestinationPolicy();
    else
      mCalendarResources->setStandardDestinationPolicy();
  }
}

void ActionManager::configureDateTime()
{
  KProcess *proc = new KProcess;
  *proc << "kcmshell" << "language";

  connect( proc,SIGNAL( processExited( KProcess * ) ),
          SLOT( configureDateTimeFinished( KProcess * ) ) );

  if ( !proc->start() ) {
      KMessageBox::sorry( dialogParent(),
        i18n("Could not start control module for date and time format.") );
      delete proc;
  }
}

void ActionManager::showTip()
{
  KTipDialog::showTip( dialogParent(),QString(),true );
}

void ActionManager::showTipOnStart()
{
  KTipDialog::showTip( dialogParent() );
}

KOrg::MainWindow *ActionManager::findInstance( const KUrl &url )
{
  if ( mWindowList ) {
    if ( url.isEmpty() ) return mWindowList->defaultInstance();
    else return mWindowList->findInstance( url );
  } else {
    return 0;
  }
}

void ActionManager::dumpText( const QString &str )
{
  kDebug(5850) << "ActionManager::dumpText(): " << str << endl;
}

void ActionManager::toggleDateNavigator()
{
  bool visible = mDateNavigatorShowAction->isChecked();
  if ( mCalendarView ) mCalendarView->showDateNavigator( visible );
}

void ActionManager::toggleTodoView()
{
  bool visible = mTodoViewShowAction->isChecked();
  if ( mCalendarView ) mCalendarView->showTodoView( visible );
}

void ActionManager::toggleEventViewer()
{
  bool visible = mEventViewerShowAction->isChecked();
  if ( mCalendarView ) mCalendarView->showEventViewer( visible );
}

void ActionManager::toggleResourceView()
{
  bool visible = mResourceViewShowAction->isChecked();
  kDebug(5850) << "toggleResourceView: " << endl;
  if ( mResourceView ) {
    if ( visible ) mResourceView->show();
    else mResourceView->hide();
  }
}

void ActionManager::toggleResourceButtons()
{
  bool visible = mResourceButtonsAction->isChecked();

  kDebug(5850) << "RESOURCE VIEW " << long( mResourceView ) << endl;

  if ( mResourceView ) mResourceView->showButtons( visible );
}

bool ActionManager::openURL( const QString &url )
{
  return openURL( KUrl( url ) );
}

bool ActionManager::mergeURL( const QString &url )
{
  return openURL( KUrl( url ),true );
}

bool ActionManager::saveAsURL( const QString &url )
{
  return saveAsURL( KUrl( url ) );
}

QString ActionManager::getCurrentURLasString() const
{
  return mURL.url();
}

bool ActionManager::editIncidence( const QString& uid )
{
  return mCalendarView->editIncidence( uid );
}

bool ActionManager::showIncidence( const QString& uid )
{
  return mCalendarView->showIncidence( uid );
}

bool ActionManager::showIncidenceContext( const QString& uid )
{
  return mCalendarView->showIncidenceContext( uid );
}

bool ActionManager::deleteIncidence( const QString& uid, bool force )
{
  return mCalendarView->deleteIncidence( uid, force );
}

bool ActionManager::addIncidence( const QString& ical )
{
  return mCalendarView->addIncidence( ical );
}

void ActionManager::configureDateTimeFinished( KProcess *proc )
{
  delete proc;
}

void ActionManager::downloadNewStuff()
{
  kDebug(5850) << "ActionManager::downloadNewStuff()" << endl;

  if ( !mNewStuff ) mNewStuff = new KONewStuff( mCalendarView );
  mNewStuff->download();
}

void ActionManager::uploadNewStuff()
{
  if ( !mNewStuff ) mNewStuff = new KONewStuff( mCalendarView );
  mNewStuff->upload();
}

QString ActionManager::localFileName()
{
  return mFile;
}

class ActionManager::ActionStringsVisitor : public IncidenceBase::Visitor
{
  public:
    ActionStringsVisitor() : mShow( 0 ), mEdit( 0 ), mDelete( 0 ) {}

    bool act( IncidenceBase *incidence, KAction *show, KAction *edit, KAction *del )
    {
      mShow = show;
      mEdit = edit;
      mDelete = del;
      return incidence->accept( *this );
    }

  protected:
    bool visit( Event * ) {
      if ( mShow ) mShow->setText( i18n("&Show Event") );
      if ( mEdit ) mEdit->setText( i18n("&Edit Event...") );
      if ( mDelete ) mDelete->setText( i18n("&Delete Event") );
      return true;
    }
    bool visit( Todo * ) {
      if ( mShow ) mShow->setText( i18n("&Show To-do") );
      if ( mEdit ) mEdit->setText( i18n("&Edit To-do...") );
      if ( mDelete ) mDelete->setText( i18n("&Delete To-do") );
      return true;
    }
    bool visit( Journal * ) { return assignDefaultStrings(); }
  protected:
    bool assignDefaultStrings() {
      if ( mShow ) mShow->setText( i18n("&Show") );
      if ( mEdit ) mEdit->setText( i18n("&Edit...") );
      if ( mDelete ) mDelete->setText( i18n("&Delete") );
      return true;
    }
    KAction *mShow;
    KAction *mEdit;
    KAction *mDelete;
};

void ActionManager::processIncidenceSelection( Incidence *incidence )
{
//  kDebug(5850) << "ActionManager::processIncidenceSelection()" << endl;

  if ( !incidence ) {
    enableIncidenceActions( false );
    return;
  }

  enableIncidenceActions( true );

  if ( incidence->isReadOnly() ) {
    mCutAction->setEnabled( false );
    mDeleteAction->setEnabled( false );
  }

  ActionStringsVisitor v;
  if ( !v.act( incidence, mShowIncidenceAction, mEditIncidenceAction, mDeleteIncidenceAction ) ) {
    mShowIncidenceAction->setText( i18n("&Show") );
    mEditIncidenceAction->setText( i18n("&Edit...") );
    mDeleteIncidenceAction->setText( i18n("&Delete") );
  }
}

void ActionManager::enableIncidenceActions( bool enabled )
{
  mShowIncidenceAction->setEnabled( enabled );
  mEditIncidenceAction->setEnabled( enabled );
  mDeleteIncidenceAction->setEnabled( enabled );
//   mAssignResourceAction->setEnabled( enabled );

  mCutAction->setEnabled( enabled );
  mCopyAction->setEnabled( enabled );
  mDeleteAction->setEnabled( enabled );
  mPublishEvent->setEnabled( enabled );
}

void ActionManager::keyBindings()
{
  KKeyDialog dlg( KKeyChooser::AllActions,
    KKeyChooser::LetterShortcutsDisallowed, view() );
  if ( mMainWindow )
    dlg.insert( mMainWindow->getActionCollection() );

  foreach ( KOrg::Part *part, mParts ) {
    if ( part ) dlg.insert( part->actionCollection(), part->shortInfo() );
  }
  dlg.configure();
}

void ActionManager::loadParts()
{
  mParts = KOCore::self()->loadParts( mMainWindow );
}

void ActionManager::setTitle()
{
  mMainWindow->setTitle();
}

KCalendarIface::ResourceRequestReply ActionManager::resourceRequest( const QList<QPair<QDateTime, QDateTime> >&,
 const QByteArray& resource,
 const QString& vCalIn )
{
    kDebug(5850) << k_funcinfo << "resource=" << resource << " vCalIn=" << vCalIn << endl;
    KCalendarIface::ResourceRequestReply reply;
    reply.vCalOut = "VCalOut";
    return reply;
}

void ActionManager::openEventEditor( const QString& text )
{
  mCalendarView->newEvent( text );
}

void ActionManager::openEventEditor( const QString& summary,
                                     const QString& description,
                                     const QString& attachment )
{
  mCalendarView->newEvent( summary, description, attachment );
}

void ActionManager::openEventEditor( const QString& summary,
                                     const QString& description,
                                     const QString& attachment,
                                     const QStringList& attendees )
{
  mCalendarView->newEvent( summary, description, attachment, attendees );
}

void ActionManager::openTodoEditor( const QString& text )
{
  mCalendarView->newTodo( text );
}

void ActionManager::openTodoEditor( const QString& summary,
                                    const QString& description,
                                    const QString& attachment )
{
  mCalendarView->newTodo( summary, description, attachment );
}

void ActionManager::openTodoEditor( const QString& summary,
                                    const QString& description,
                                    const QString& attachment,
                                    const QStringList& attendees )
{
  mCalendarView->newTodo( summary, description, attachment, attendees );
}

void ActionManager::openJournalEditor( const QDate& date )
{
  mCalendarView->newJournal( date );
}

void ActionManager::openJournalEditor( const QString& text, const QDate& date )
{
  mCalendarView->newJournal( text, date );
}

void ActionManager::openJournalEditor( const QString& text )
{
  mCalendarView->newJournal( text );
}

//TODO:
// void ActionManager::openJournalEditor( const QString& summary,
//                                        const QString& description,
//                                        const QString& attachment )
// {
//   mCalendarView->newJournal( summary, description, attachment );
// }


void ActionManager::showJournalView()
{
  mCalendarView->viewManager()->showJournalView();
}

void ActionManager::showTodoView()
{
  mCalendarView->viewManager()->showTodoView();
}

void ActionManager::showEventView()
{
  mCalendarView->viewManager()->showEventView();
}

void ActionManager::goDate( const QDate& date )
{
  mCalendarView->goDate( date );
}

void ActionManager::goDate( const QString& date )
{
  goDate( KGlobal::locale()->readDate( date ) );
}

void ActionManager::updateUndoAction( const QString &text )
{
  if ( text.isNull() ) {
    mUndoAction->setEnabled( false );
    mUndoAction->setText( i18n("Undo") );
  } else {
    mUndoAction->setEnabled( true );
    if ( text.isEmpty() ) mUndoAction->setText( i18n("Undo") );
    else mUndoAction->setText( i18n("Undo (%1)", text ) );
  }
}

void ActionManager::updateRedoAction( const QString &text )
{
  if ( text.isNull() ) {
    mRedoAction->setEnabled( false );
    mRedoAction->setText( i18n( "Redo" ) );
  } else {
    mRedoAction->setEnabled( true );
    if ( text.isEmpty() ) mRedoAction->setText( i18n("Redo") );
    else mRedoAction->setText( i18n( "Redo (%1)", text ) );
  }
}

bool ActionManager::queryClose()
{
  kDebug(5850) << "ActionManager::queryClose()" << endl;

  bool close = true;

  if ( mCalendar && mCalendar->isModified() ) {
    int res = KMessageBox::questionYesNoCancel( dialogParent(),
      i18n("The calendar contains unsaved changes. Do you want to save them before exiting?"), QString(), KStdGuiItem::save(), KStdGuiItem::discard() );
    // Exit on yes and no, don't exit on cancel. If saving fails, ask for exiting.
    if ( res == KMessageBox::Yes ) {
      close = saveModifiedURL();
      if ( !close ) {
        int res1 = KMessageBox::questionYesNo( dialogParent(), i18n("Unable to save the calendar. Do you still want to close this window?"), QString(), KStdGuiItem::close(), KStdGuiItem::cancel() );
        close = ( res1 == KMessageBox::Yes );
      }
    } else {
      close = ( res == KMessageBox::No );
    }
  } else if ( mCalendarResources ) {
    if ( !mIsClosing ) {
      kDebug(5850) << "!mIsClosing" << endl;
      if ( !saveResourceCalendar() ) return false;

      // FIXME: Put main window into a state indicating final saving.
      mIsClosing = true;
// FIXME: Close main window when save is finished
//      connect( mCalendarResources, SIGNAL( calendarSaved() ),
//               mMainWindow, SLOT( close() ) );
    }
    if ( mCalendarResources->isSaving() ) {
      kDebug(5850) << "ActionManager::queryClose(): isSaving" << endl;
      close = false;
      KMessageBox::information( dialogParent(),
          i18n("Unable to exit. Saving still in progress.") );
    } else {
      kDebug(5850) << "ActionManager::queryClose(): close = true" << endl;
      close = true;
    }
  } else {
    close = true;
  }

  return close;
}

void ActionManager::saveCalendar()
{
  if ( mCalendar ) {
    if ( view()->isModified() ) {
      if ( !url().isEmpty() ) {
        saveURL();
      } else {
        QString location = locateLocal( "data", "korganizer/kontact.ics" );
        saveAsURL( location );
      }
    }
  } else if ( mCalendarResources ) {
    mCalendarResources->save();
    // FIXME: Make sure that asynchronous saves don't fail.
  }
}

bool ActionManager::saveResourceCalendar()
{
  if ( !mCalendarResources ) return false;
  CalendarResourceManager *m = mCalendarResources->resourceManager();

  CalendarResourceManager::ActiveIterator it;
  for ( it = m->activeBegin(); it != m->activeEnd(); ++it ) {
    if ( (*it)->readOnly() ) continue;
    if ( !(*it)->save() ) {
      int result = KMessageBox::warningContinueCancel( view(),
        i18n( "Saving of '%1' failed. Check that the resource is "
             "properly configured.\nIgnore problem and continue without "
             "saving or cancel save?", (*it)->resourceName() ),
        i18n("Save Error"), KStdGuiItem::dontSave() );
      if ( result == KMessageBox::Cancel ) return false;
    }
  }
  return true;
}

void ActionManager::importCalendar( const KUrl &url )
{
  if ( !url.isValid() ) {
    KMessageBox::error( dialogParent(),
                        i18n("URL '%1' is invalid.", url.prettyURL() ) );
    return;
  }

  ImportDialog *dialog;
  dialog = new ImportDialog( url, mMainWindow->topLevelWidget() );
  connect( dialog, SIGNAL( dialogFinished( ImportDialog * ) ),
           SLOT( slotImportDialogFinished( ImportDialog * ) ) );
  connect( dialog, SIGNAL( openURL( const KUrl &, bool ) ),
           SLOT( openURL( const KUrl &, bool ) ) );
  connect( dialog, SIGNAL( newWindow( const KUrl & ) ),
           SIGNAL( actionNew( const KUrl & ) ) );
  connect( dialog, SIGNAL( addResource( const KUrl & ) ),
           SLOT( addResource( const KUrl & ) ) );

  dialog->show();
}

void ActionManager::slotImportDialogFinished( ImportDialog *dlg )
{
  dlg->deleteLater();
  mCalendarView->updateView();
}

void ActionManager::slotAutoArchivingSettingsModified()
{
  if ( KOPrefs::instance()->mAutoArchive )
    mAutoArchiveTimer->start( 4 * 60 * 60 * 1000 ); // check again in 4 hours
  else
    mAutoArchiveTimer->stop();
}

void ActionManager::slotAutoArchive()
{
  if ( !mCalendarView->calendar() ) // can this happen?
    return;
  mAutoArchiveTimer->stop();
  EventArchiver archiver;
  connect( &archiver, SIGNAL( eventsDeleted() ), mCalendarView, SLOT( updateView() ) );
  archiver.runAuto( mCalendarView->calendar(), mCalendarView, false /*no gui*/ );
  // restart timer with the correct delay ( especially useful for the first time )
  slotAutoArchivingSettingsModified();
}

QWidget *ActionManager::dialogParent()
{
  return mCalendarView->topLevelWidget();
}

#include "actionmanager.moc"
