/*
  $Id$
  
  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.troll.no and http://www.kde.org respectively
  
  Copyright (c) 1997, 1998, 1999, 2000
  Preston Brown (preston.brown@yale.edu)
  Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
  Ian Dawes (iadawes@globalserve.net)
  Laszlo Boloni (boloni@cs.purdue.edu)
  Cornelius Schumacher (schumacher@kde.org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <sys/types.h>
#include <signal.h>

#include <qfiledlg.h>
#include <qcursor.h>
#include <qmlined.h>
#include <qmsgbox.h>
#include <qtimer.h>
#include <qvbox.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kstdaccel.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kedittoolbar.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <dcopclient.h>

#include "version.h"
#include "koarchivedlg.h"
#include "komailclient.h"
#include "calprinter.h"
#include "aboutdlg.h"
#include "exportwebdialog.h"
#include "calendarview.h"

#include "korganizer.h"
#include "korganizer.moc"

#define AGENDABUTTON 0x10
#define NOACCEL 0

QList<KOrganizer> KOrganizer::windowList;

KOrganizer::KOrganizer(QString filename, bool fnOverride, const char *name ) 
  : KTMainWindow( name )
{
  qDebug("KOrganizer::KOrganizer()");

  mTempFile = 0;

  mAutoSave = false;

  // add this instance of the window to the static list.
  windowList.append(this);

//  setMinimumSize(600,400);	// make sure we don't get resized too small...

  if (!fnOverride) {
    KConfig *config(kapp->config());
    config->setGroup("General");
    QString str = config->readEntry("Active Calendar");
    if (!str.isEmpty() && QFile::exists(str))
      mFile = str;
  } else {
    mFile = filename;
  }

  mURL = KURL();
  mURL.setPath( mFile) ;

  mCalendarView = new CalendarView(mFile,this,"KOrganizer::CalendarView");
  setView(mCalendarView);

  initActions();

// We don't use a status bar up to now.
#if 0
  sb = new KStatusBar(this, "sb");
  setStatusBar(sb);
#endif

  readSettings();
  mCalendarView->readSettings();

  // set up autoSaving stuff
  mAutoSaveTimer = new QTimer(this);
  connect(mAutoSaveTimer, SIGNAL(timeout()),
	  this, SLOT(checkAutoSave()));
  if (autoSave())
    mAutoSaveTimer->start(1000*60);

  setTitle();

  qDebug("KOrganizer::KOrganizer() done");
}

KOrganizer::~KOrganizer()
{
  if (mTempFile) delete mTempFile;

  qDebug("~KOrganizer()");
  hide();

  // Free memory allocated for widgets (not children)
  // Take this window out of the window list.
  windowList.removeRef( this );
  qDebug("~KOrganizer() done");
}


void KOrganizer::readSettings()
{
  QString str;

  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config(kapp->config());

  int windowWidth = 600;
  int windowHeight = 400;
	
  config->setGroup("General");

  str = config->readEntry("Width");
  if (!str.isEmpty())
    windowWidth = str.toInt();
  str = config->readEntry("Height");
  if (!str.isEmpty())
    windowHeight = str.toInt();
  this->resize(windowWidth, windowHeight);

// We currently don't use a status bar
#if 0
  statusBarEnable = config->readBoolEntry("Status Bar", TRUE);
#endif

//  toolBarEnable = config->readBoolEntry("Tool Bar", TRUE);

  mAutoSave = config->readBoolEntry("Auto Save", FALSE);

  mRecent->loadEntries(config);

  mCalendarView->readSettings();
    
  config->sync();
}


void KOrganizer::writeSettings()
{
  KConfig *config(kapp->config());

  QString tmpStr;
  config->setGroup("General");

  tmpStr.sprintf("%d", this->width() );
  config->writeEntry("Width",	tmpStr);
	
  tmpStr.sprintf("%d", this->height() );
  config->writeEntry("Height",	tmpStr);

/*
  tmpStr.sprintf("%s", optionsMenu->isItemChecked(toolBarMenuId) ? 
		 "true" : "false");
  config->writeEntry("Tool Bar", tmpStr);
*/

// We currently don't use a status bar
#if 0
  tmpStr.sprintf("%s", optionsMenu->isItemChecked(statusBarMenuId) ?
		 "true" : "false");
  config->writeEntry("Status Bar", tmpStr);
#endif

  // Write only local Files to config file. This prevents loading of a remote
  // file automatically on startup, which could block KOrganizer even before
  // it has opened.
  QString file;
  if (mURL.isLocalFile()) file = mFile;

  config->writeEntry("Active Calendar", file);

  mRecent->saveEntries(config);

  mCalendarView->writeSettings();

  config->sync();
}


void KOrganizer::initActions()
{
  KStdAction::openNew(this, SLOT(file_new()), actionCollection());
  KStdAction::open(this, SLOT(file_open()), actionCollection());
  mRecent = KStdAction::openRecent(this, SLOT(file_openRecent(const KURL&)),
                                   actionCollection());
  KStdAction::save(this, SLOT(file_save()), actionCollection());
  KStdAction::saveAs(this, SLOT(file_saveas()), actionCollection());
  KStdAction::close(this, SLOT(file_close()), actionCollection());
  (void)new KAction(i18n("&Import From Ical"), 0, this, SLOT(file_import()),
                    actionCollection(), "import_ical");
  (void)new KAction(i18n("&Merge Calendar"), 0, this, SLOT(file_merge()),
                    actionCollection(), "merge_calendar");
  (void)new KAction(i18n("Archive Old Entries"), 0, this, SLOT(file_archive()),
                    actionCollection(), "file_archive");
  (void)new KAction(i18n("Export as web page"), 0,
                    mCalendarView, SLOT(exportWeb()),
                    actionCollection(), "export_web");
  (void)new KAction(i18n("Print Setup"), 0,
                    mCalendarView, SLOT(printSetup()),
                    actionCollection(), "print_setup");
  KStdAction::print(mCalendarView, SLOT(print()), actionCollection());
  KStdAction::printPreview(mCalendarView, SLOT(printPreview()),
                           actionCollection());
  KStdAction::quit(this, SLOT(close()), actionCollection());

  // setup edit menu
  KStdAction::cut(mCalendarView, SLOT(edit_cut()), actionCollection());
  KStdAction::copy(mCalendarView, SLOT(edit_copy()), actionCollection());
  KStdAction::paste(mCalendarView, SLOT(edit_paste()), actionCollection());

  // view menu
  (void)new KAction(i18n("&List"), BarIcon("listicon"), 0,
                    mCalendarView, SLOT(view_list()),
                    actionCollection(), "view_list");
  (void)new KAction(i18n("&Day"), BarIcon("dayicon"), 0,
                    mCalendarView, SLOT(view_day()),
                    actionCollection(), "view_day");
  (void)new KAction(i18n("W&ork Week"), BarIcon("5dayicon"), 0,
                    mCalendarView, SLOT(view_workweek()),
                    actionCollection(), "view_workweek");
  (void)new KAction(i18n("&Week"), BarIcon("weekicon"), 0,
                    mCalendarView, SLOT(view_week()),
                    actionCollection(), "view_week");
  (void)new KAction(i18n("&Month"), BarIcon("monthicon"), 0,
                    mCalendarView, SLOT(view_month()),
                    actionCollection(), "view_month");
  (void)new KAction(i18n("&To-do list"), BarIcon("todolist"), 0,
                    mCalendarView, SLOT(view_todolist()),
                    actionCollection(), "view_todo");
  (void)new KAction(i18n("&Update"), 0,
                    mCalendarView, SLOT(update()),
                    actionCollection(), "update");

  // event handling menu
  (void)new KAction(i18n("New &Appointment"), BarIcon("newevent"), 0,
                    mCalendarView,SLOT(appointment_new()),
                    actionCollection(), "new_appointment");
  (void)new KAction(i18n("New E&vent"), 0,
                    mCalendarView,SLOT(allday_new()),
                    actionCollection(), "new_event");
  (void)new KAction(i18n("New To-Do"), 0,
                    mCalendarView,SLOT(newTodo()),
                    actionCollection(), "new_todo");
  (void)new KAction(i18n("&Edit Appointment"), 0,
                    mCalendarView,SLOT(appointment_edit()),
                    actionCollection(), "new_appointment");
  (void)new KAction(i18n("&Delete Appointment"), 0,
                    mCalendarView,SLOT(appointment_delete()),
                    actionCollection(), "delete_appointment");

  KStdAction::find(mCalendarView, SLOT(action_search()), actionCollection());

  (void)new KAction(i18n("&Mail Appointment"), BarIcon("send"), 0,
                    mCalendarView,SLOT(action_mail()),
                    actionCollection(), "mail_appointment");
  
  (void)new KAction(i18n("Go to &Today"), BarIcon("todayicon"), 0,
                    mCalendarView,SLOT(goToday()),
                    actionCollection(), "go_today");
  (void)new KAction(i18n("&Previous Day"), BarIcon("1leftarrow"), 0,
                    mCalendarView,SLOT(goPrevious()),
                    actionCollection(), "go_previous");
  (void)new KAction(i18n("&Next Day"), BarIcon("1rightarrow"), 0,
                    mCalendarView,SLOT(goNext()),
                    actionCollection(), "go_next");
      
  // setup Settings menu
  KStdAction::showToolbar(this, SLOT(toggleToolBar()), actionCollection());
//  KStdAction::showStatusbar(this, SLOT(toggleStatusBar()), actionCollection());

  KStdAction::configureToolbars(this, SLOT(configureToolbars()),
                                actionCollection());
  KStdAction::preferences(mCalendarView, SLOT(edit_options()),
                          actionCollection());
  
  createGUI("korganizer.rc");
}


void KOrganizer::file_new()
{
  // Make new KOrganizer window containing empty calendar
  (new KOrganizer("",true))->show();
}


void KOrganizer::file_open()
{
  KURL url;
  QString defaultPath = locateLocal("appdata", "");
  url = KFileDialog::getOpenURL(defaultPath,"*.vcs",this);
  if (openURL(url)) {
    setTitle();
    mRecent->addURL(url);
  }
}


void KOrganizer::file_openRecent(const KURL& url)
{
  if (!url.isEmpty()) {
    if (openURL(url)) {
      setTitle();
    }
  }
}


void KOrganizer::file_import()
{
  // eventually, we will need a dialog box to select import type, etc.
  // for now, hard-coded to ical file, $HOME/.calendar.
  int retVal;
  QString progPath;
  char *tmpFn;

  QString homeDir;
  homeDir.sprintf("%s/.calendar",getenv("HOME"));
		  
  if (!QFile::exists(homeDir)) {
    QMessageBox::critical(this, i18n("KOrganizer Error"),
			  i18n("You have no ical file in your home directory.\n"
			       "Import cannot proceed.\n"));
    return;
  }
  tmpFn = tmpnam(0);
  progPath = locate("exe", "ical2vcal") + tmpFn;

  retVal = system(progPath.data());
  
  if (retVal >= 0 && retVal <= 2) {
    // now we need to MERGE what is in the iCal to the current calendar.
    mCalendarView->mergeCalendar(tmpFn);
    if (!retVal)
      QMessageBox::information(this, i18n("KOrganizer Info"),
			       i18n("KOrganizer succesfully imported and "
				    "merged your\n.calendar file from ical "
				    "into the currently\nopened calendar.\n"),
			       QMessageBox::Ok);
    else
      QMessageBox::warning(this, i18n("ICal Import Successful With Warning"),
			   i18n("KOrganizer encountered some unknown fields while\n"
				"parsing your .calendar ical file, and had to\n"
				"discard them.  Please check to see that all\n"
				"your relevant data was correctly imported.\n"));
  } else if (retVal == -1) {
    QMessageBox::warning(this, i18n("KOrganizer Error"),
			 i18n("KOrganizer encountered some error parsing your\n"
			      ".calendar file from ical.  Import has failed.\n"));
  } else if (retVal == -2) {
    QMessageBox::warning(this, i18n("KOrganizer Error"),
			 i18n("KOrganizer doesn't think that your .calendar\n"
			      "file is a valid ical calendar. Import has failed.\n"));
  }
}


void KOrganizer::file_merge()
{
  KURL url = KFileDialog::getOpenURL(locateLocal("appdata", ""),"*.vcs",this);
  mergeURL(url);
}


void KOrganizer::file_archive()
{
  ArchiveDialog ad;

  if (ad.exec()) {
    // ok was pressed.
  }
}


void KOrganizer::file_saveas()
{
  KURL url = getSaveURL();

  if (saveAsURL(url)) {
    setTitle();
    mRecent->addURL(url);
  }
}


KURL KOrganizer::getSaveURL()
{
  KURL url = KFileDialog::getSaveURL(locateLocal("appdata", ""),"*.vcs",this);

  QString filename = url.filename(false); 

  if(filename.length() >= 3) {
    QString e = filename.right(4);
    // Extension ending in '.vcs' or anything else '.???' is cool.
    if(e != ".vcs" && e.right(1) != ".")
    // Otherwise, force the default extension.
    filename += ".vcs";
  }

  url.setFileName(filename);

  qDebug("KOrganizer::getSaveURL(): url: %s",
         url.url().latin1());

  return url;
}


void KOrganizer::file_save()
{
  if (mURL.isEmpty()) file_saveas();
  else saveURL();
}


void KOrganizer::file_close()
{
  closeURL();

  setTitle();
}


void KOrganizer::file_quit()
{
  close();
}


bool KOrganizer::queryClose()
{
  // Write configuration. I don't know if it really makes sense doing it this
  // way, when having opened multiple calendars in different CalendarViews.
  writeSettings();

  return closeURL();
}


bool KOrganizer::queryExit()
{
  // Don't call writeSettings here, because filename isn't valid anymore. It is
  // now called in queryClose.
//  writeSettings();
  return true;
}


void KOrganizer::setTitle()
{
  qDebug("KOrganizer::setTitle");

  QString tmpStr;

  if (!mFile.isEmpty())
    tmpStr = mFile.mid(mFile.findRev('/')+1, mFile.length());
  else
    tmpStr = i18n("New Calendar");

  // display the modified thing in the title
  // if auto-save is on, we only display it on new calender (no file name)
  if (mCalendarView->isModified() && (mFile.isEmpty() || !autoSave())) {
    tmpStr += " (";
    tmpStr += i18n("modified");
    tmpStr += ")";
  }

  setCaption(tmpStr);
}

void KOrganizer::checkAutoSave()
{
// to be reimplemented.
/*
  // has this calendar been saved before? 
  if (autoSave() && !mFile.isEmpty()) {
    add_recent_file(mFile);
    mCalendarView->saveCalendar(mFile);
  }
*/
}


// Configuration changed as a result of the options dialog.
// I wanted to break this up, in order to avoid inefficiency 
// introduced as we were ALWAYS updating configuration
// in multiple widgets regardless of changed in information.
void KOrganizer::updateConfig()
{
  emit configChanged();
  readSettings(); // this is the best way to assure that we have them back
  if (autoSave() && !mAutoSaveTimer->isActive()) {
    checkAutoSave();
    mAutoSaveTimer->start(1000*60);
  }
  if (!autoSave())
    mAutoSaveTimer->stop();

  // static slot calls here
  KOEvent::updateConfig();
}


void KOrganizer::configureToolbars()
{
  KEditToolbar dlg(actionCollection());

  if (dlg.exec())
  {
    createGUI("korganizer.rc");
  }
}


bool KOrganizer::openURL( const KURL &url )
{
  qDebug("KOrganizer::openURL()");
  if (url.isMalformed()) return false;
  if (!closeURL()) return false;
  mURL = url;
  mFile = "";
  if( KIO::NetAccess::download( mURL, mFile ) ) {
    return mCalendarView->openCalendar(mFile);
  } else {
    return false;
  }
}


bool KOrganizer::mergeURL( const KURL &url )
{
  qDebug("KOrganizer::mergeURL()");
  if ( url.isMalformed() )
    return false;

  QString tmpFile;
  if( KIO::NetAccess::download( mURL, tmpFile ) ) {
    bool success = mCalendarView->mergeCalendar(tmpFile);
    KIO::NetAccess::removeTempFile( tmpFile );
    return success;
  } else {
    return false;
  }
}


bool KOrganizer::closeURL()
{
  qDebug("KOrganizer::closeURL()");
  if (mCalendarView->isModified())
  {
    int result = KMessageBox::warningYesNoCancel(0L,
            i18n("The calendar has been modified.\nDo you want to save it?"));

    switch(result) {
    case KMessageBox::Yes :
      if (mURL.isEmpty()) {
        KURL url = getSaveURL();
        if (url.isEmpty()) return false;
        if (!saveAsURL(url)) return false;
      }
      if (!saveURL()) return false;
      break;
    case KMessageBox::No :
      break;
    default : // case KMessageBox::Cancel :
      return false;
    }
  }

  mCalendarView->closeCalendar();
  mURL="";
  mFile="";
  
  KIO::NetAccess::removeTempFile( mFile );

  return true;
}

bool KOrganizer::saveURL()
{
  if (!mCalendarView->saveCalendar(mFile)) {
    qDebug("KOrganizer::saveURL(): calendar view save failed.");
    return false;
  } else {
    qDebug("KOrganizer::saveURL(): Notify alarm daemon");
    if (!kapp->dcopClient()->send("alarmd","ad","reloadCal()","")) {
      qDebug("KOrganizer::saveURL(): dcop send failed");
    }
  }
  if (KIO::NetAccess::upload(mFile,mURL)) {
    qDebug("KOrganizer::saveURL(): upload failed.");
    return false;
  }

  // keep saves on a regular interval
  if (autoSave()) {
    mAutoSaveTimer->stop();
    mAutoSaveTimer->start(1000*60);
  }

  return true;
}

bool KOrganizer::saveAsURL( const KURL & kurl )
{
  if (kurl.isMalformed()) {
    qDebug("KOrganizer::saveAsURL(): Malformed URL.");
    return false;
  }
  mURL = kurl; // Store where to upload

  // Local file
  if ( mURL.isLocalFile() ) {
    // get rid of a possible temp file first
    if ( mTempFile ) {  // (happens if previous url was remote)
      delete mTempFile;
      mTempFile = 0;
    }
    mFile = mURL.path();
  } else { // Remote file
    // We haven't saved yet, or we did but locally - provide a temp file
    if ( mFile.isEmpty() || !mTempFile ) {
      mTempFile = new KTempFile;
      mFile = mTempFile->name();
    }
    // otherwise, we already had a temp file
  }
  return saveURL(); // Save local file and upload local file
}
