/*

 KNotes -- Notes for the KDE project

 Copyright (C) Bernd Johannes Wuebben
               wuebben@math.cornell.edu
	       wuebben@kde.org

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 (i18n stuff added by didier Belot <dib@avo.fr>)

  */

//
// 1999-12-29 Espen Sand
// Cleanup + Undo/Redo
//

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <qdir.h>
#include <qdrawutil.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qtimer.h>

#include <kapp.h>
#include <kaccel.h>
#include <kcolordlg.h>
#include <kconfig.h>
#include <kfontdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio_netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kstddirs.h>
#include <kurl.h>
#include <kwm.h>
#include <kwin.h>

#include "alarm.h"
#include "docking.h"
#include "knotes.h"
#include "mail.h"
#include "optiondialog.h"
#include "renamedlg.h"
#include "timer.h"
#include "version.h"

#include "knotes.moc"

void findPostitFiles();

QList<KPostit> 	  KPostit::PostitList;      // pointers to all postit objects
QStringList 	  KPostit::PostitFilesList; // names of all postit files
QList<AlarmEntry> KPostit::AlarmList;
bool KPostit::dock;

DockWidget*     docker;
DefStruct 	postitdefaults;

static void 	cleanup(int);
void 		catchSignals();
void 	        readSettings();
void 	        writeSettings();

QString 	pidFile;
long 		window_id;
QString 	tmpFile;

MyTimer* 	mytimer;
SaveTimer* 	savetimer;
volatile sig_atomic_t cleaning_up = false;

extern bool     savealarms();
extern bool 	readalarms();



KPostitMultilineEdit::KPostitMultilineEdit(QWidget *parent, const char *myname)
  : QMultiLineEdit(parent, myname){

  setAcceptDrops(TRUE);
}

void KPostitMultilineEdit::keyPressEvent(QKeyEvent *e){

  if (e->key() == Key_Tab) {
    int line, col;
    cursorPosition(&line, &col);
    QMultiLineEdit::insertAt("\t", line, col);
  }

  if(e->key() == Key_Return || e->key() == Key_Enter){

    mynewLine();
    return;
  }

  QMultiLineEdit::keyPressEvent(e);

}

void KPostitMultilineEdit::mouseDoubleClickEvent ( QMouseEvent * e ){

  QMultiLineEdit::mouseDoubleClickEvent(e);

  int line, column = 0;
  getCursorPosition(&line,&column);

  QString text = markedText();
  //  printf("%d %d %s\n",line,column,text.ascii());

  if (text.isEmpty())
    return;

  KURL kurl(text);

  if (kurl.isMalformed())
    return;

  (void) new KRun ( text );
}


void KPostitMultilineEdit::mynewLine(){

  if (isReadOnly())
    return;

  if(!autoIndentMode){
    newLine();
    return;
  }

  int line,col;
  bool found_one = false;

  getCursorPosition(&line,&col);

  QString string, string2;

  while(line >= 0){

    string  = textLine(line);
    string2 = string.stripWhiteSpace();

    if(!string2.isEmpty()){
      string = prefixString(string);
      found_one = TRUE;
      break;
    }

    line --;
  }

  // string will now contain those whitespace characters that I need to insert
  // on the next line.

  if(found_one){

    // don't ask my why I programmed it this way. I am quite sick of the Qt 1.2
    // MultiLineWidget -- It is anoyingly buggy.
    // I have to put in obscure workarounds all over the place.

#ifdef FocusOut
#undef FocusOut
#endif
    QFocusEvent tmpFEOut(QEvent::FocusOut);

    focusOutEvent(&tmpFEOut);
    newLine();

    int line, col;
    cursorPosition(&line, &col);
    QMultiLineEdit::insertAt(string, line, col);

    // this doesn't work. in Qt 1.2:
    // insertAt(string.ascii(),line + 1,0);

#ifdef FocusIn
#undef FocusIn
#endif

    QFocusEvent tmpFEIn(QEvent::FocusIn);

    focusInEvent(&tmpFEIn);

  }
  else{
    newLine();
  }
}

QString KPostitMultilineEdit::prefixString(QString string){

  // This routine returns the whitespace before the first non white space
  // character in string. This is  used in mynewLine() for indent mode.
  // It is assumed that string contains at least one non whitespace character
  // ie \n \r \t \v \f and space

  QString returnstring;

  int len = string.length();

  int i = 0;
  while(i < len && string.at(i).isSpace())
    returnstring += string.at(i++);

  return returnstring;

}


void KPostitMultilineEdit::dragEnterEvent( QDragEnterEvent* event )
{
  event->accept(QUrlDrag::canDecode(event) || QTextDrag::canDecode(event));
}


void KPostitMultilineEdit::dragMoveEvent( QDragMoveEvent* event )
{
  if( QUrlDrag::canDecode(event) )
  {
    event->accept();
  }
  else if ( QTextDrag::canDecode(event) )
  {
    QMultiLineEdit::dragMoveEvent(event);
  }
}


void KPostitMultilineEdit::dropEvent( QDropEvent* event )
{
  QStringList list;

  if ( QUrlDrag::decodeToUnicodeUris( event, list ) )
  {
    emit gotUrlDrop(list.first() );
  }
  else if ( QTextDrag::canDecode( event ) )
  {
    QMultiLineEdit::dropEvent( event );
  }
}


KPostit::KPostit( QWidget *parent, const char *myname, int  _number, 
		  QString pname)
  : QFrame(parent, myname), mOptionDialog(0)
{
  XSetTransientForHint(qt_xdisplay(), winId(), winId());
  KWM::setWmCommand(winId(), "");
  KWM::setDecoration(winId(), KWM::tinyDecoration);
  KWM::setIcon(winId(), kapp->icon());
  KWM::setMiniIcon(winId(), kapp->miniIcon());

  label = new QLabel(this);
  label->setText(i18n("Hello"));
  label->setAlignment( AlignHCenter);
  label->installEventFilter(this);
  dragging = false;

  mybutton = new myPushButton(this);
  mybutton->setGeometry(200-30,0,30,30);
  mybutton->setPixmap(BarIcon("knotesclose"));
  connect(mybutton,SIGNAL(clicked()),this,SLOT(hideKPostit()));

  edit = new KPostitMultilineEdit(this);
  edit->setGeometry(0,30,200,100);
  edit->installEventFilter(this);
  edit->setFocus();
  edit->setFrameStyle(QFrame::NoFrame);
  connect(edit, SIGNAL(gotUrlDrop(const char*)), 
	  this, SLOT(insertNetFile(const char*)));

  hidden = false;
  number = _number;  // index in popup. Not used anymore, but I'll leave it in
                     // the structure for now.
  name   = pname;    // name of postit and name on the popup

  //set the defaults
  forecolor 		= postitdefaults.forecolor;
  backcolor 		= postitdefaults.backcolor;
  edit->autoIndentMode 	= postitdefaults.autoindent;
  font                  = postitdefaults.font;
  frame3d 		= postitdefaults.frame3d;

  resize(postitdefaults.width,postitdefaults.height + 30);
  edit->resize(postitdefaults.width,postitdefaults.height);

  loadnotes();

  colors =  new QPopupMenu();
  colors->insertItem(i18n("Text Color..."),
		     this, SLOT(set_foreground_color()));
  colors->insertItem(i18n("Background Color..."),
		     this, SLOT(set_background_color()));

  operations = new QPopupMenu();
  operations->insertItem (i18n("New Note"), this,SLOT(newKPostit()));
  operations->insertItem (i18n("Delete Note"), this,SLOT(deleteKPostit()));
  operations->insertItem (i18n("Rename Note..."),this,SLOT(renameKPostit()));
  operations->insertSeparator();
  operations->insertItem (i18n("Alarm ..."), this,SLOT(setAlarm()));
  operations->insertSeparator();
  operations->insertItem (i18n("Insert Date"), this, SLOT(insertDate()));
  operations->insertItem (i18n("Insert Calendar"),this,SLOT(insertCalendar()));
  operations->insertItem (i18n("Mail Note ..."), this,SLOT(mail()));
  operations->insertItem (i18n("Print Note"), this,SLOT(print()));
  operations->insertItem (i18n("Save Notes"), this,SLOT(save_all()));

  options = new QPopupMenu();
  options->setCheckable(TRUE);
  frame3dID = options->insertItem(i18n("3D Frame"),this,SLOT(toggleFrame()));
  onTopID = options->insertItem(i18n("Always visible"),
				this,SLOT(toggleOnTopMode()));
  edit->autoIndentID = options->insertItem(i18n("Auto Indent"),this,
					   SLOT(toggleIndentMode()));
  
  options->insertItem(i18n("Font..."),this, SLOT(selectFont()));
  options->insertItem(i18n("Colors"),colors);
  options->insertSeparator();
  options->insertItem(i18n("Default Configuration..."),this, SLOT(defaults()));
  dockID = options->insertItem(i18n("Dock in panel"), this,
			       SLOT(toggleDock()));

  operations->insertSeparator();
  operations->insertItem (i18n("Help"),this,SLOT(help()));
  operations->insertItem ( i18n("Exit"), this, SLOT(quit()));


  right_mouse_button = new QPopupMenu;
  for ( uint i = 0; i < PostitFilesList.count(); i++)
  {
    int k = right_mouse_button->insertItem (PostitFilesList[i]);
    k = k;
  }
  connect( right_mouse_button, SIGNAL(activated( int )),
	   SLOT(RMBActivated(int)) );
  right_mouse_button->insertSeparator();
    
  right_mouse_button->insertItem (i18n("Undo"), this,
				  SLOT(undo()) );
  right_mouse_button->insertItem (i18n("Redo"), this,
				  SLOT(redo()) );
  right_mouse_button->insertSeparator();
  right_mouse_button->insertItem(i18n("Clear"),this, SLOT(clear_text()));
  right_mouse_button->insertSeparator();
  right_mouse_button->insertItem (i18n("Operations"),operations);
  right_mouse_button->insertItem (i18n("Options"),options);

  //right_mouse_button->insertSeparator();
  //sticky_id = right_mouse_button->insertItem("", this, SLOT(toggleSticky()));

  desktops = new QPopupMenu;
  connect( desktops, SIGNAL(activated( int )), SLOT(toDesktop(int)) );
  right_mouse_button->insertItem(i18n("&To desktop"), desktops );

  installEventFilter( this );
  window_id = winId();

  set3DFrame(frame3d);
  setOnTop(postitdefaults.onTop);

  if(edit->autoIndentMode==false)
  {
    setNoAutoIndent();
    options->setItemChecked(edit->autoIndentID,FALSE);
  }
  else
  {
    setAutoIndent();
    options->setItemChecked(edit->autoIndentID,TRUE);
  }

  options->setItemChecked(dockID,dock);

  set_colors();
  connect(kapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(set_colors()));

  edit->setFont(font);

  bool have_alarm = false;
  QListIterator<AlarmEntry> it( AlarmList );
  AlarmEntry *entry;
  while( (entry=it.current()) ) 
  {
    if(entry->name == name)
    {
      have_alarm = TRUE;
      break;
    }
    ++it;
  }

  if (have_alarm)
  {
    label->setText(name + " (A)");
    setCaption(i18n("Note: ") + name + " (A)");
  }
  else 
  {
    label->setText(name);
    setCaption(i18n("Note: ") + name);
  }


  // add some keyboard accelerators (pfeiffer)
  KAccel *accel = new KAccel(this, "kaccel");
  accel->connectItem(KAccel::New, this, SLOT(newKPostit()));
  accel->connectItem(KAccel::Undo, this, SLOT(undo()));
  accel->connectItem(KAccel::Redo, this, SLOT(redo()));
  accel->connectItem(KAccel::Print, this, SLOT(print()));
  accel->connectItem(KAccel::Save, this, SLOT(save_all()));
  accel->connectItem(KAccel::Close, this, SLOT(hideKPostit()));
  accel->connectItem(KAccel::Quit, this, SLOT(quit()));
  accel->connectItem(KAccel::Help, this, SLOT(help()));
}


KPostit::~KPostit( void )
{
  delete mOptionDialog;
}


void KPostit::clear_text( void )
{
  edit->clear();
}


void KPostit::selectFont( void )
{
  QFont myfont = edit->font();
  KFontDialog::getFont(myfont);
  edit->setFont(myfont);
  font = myfont;
}

void KPostit::quit( void )
{
  for( PostitList.first(); PostitList.current() != 0; PostitList.next() )
  {
    if(!PostitList.current()->savenotes())
    {
      QString msg = i18n(""
        "Could not save a note.\n"
	"Exit anyways?");
      int result = KMessageBox::warningYesNo( this, msg );
      if (result != KMessageBox::Yes )
      {
	return;
      }
    }
  }

  remove( pidFile.ascii() );
  writeSettings();
  if( savealarms() == false )
  {
    KMessageBox::sorry(this, i18n("Could not save Alarm(s)\n") );
  }
  QApplication::exit();
}


void KPostit::help( void )
{
  kapp->invokeHTMLHelp("","");
}


void KPostit::undo( void )
{
  edit->undo();
}


void KPostit::redo( void )
{
  edit->redo();
}


void KPostit::insertDate( void )
{
  int line, column;
  edit->getCursorPosition(&line,&column);
  edit->insertAt( QDateTime::currentDateTime().toString(), line, column );
}

void KPostit::mail( void )
{
  Mail *mailDialog = new Mail( this,"maildialog");
  if( mailDialog == 0 ) { return; }

  if( mailDialog->exec() != QDialog::Accepted )
  {
    delete mailDialog;
    return;
  }

  kapp->processEvents();
  kapp->flushX();

  QString cmd = postitdefaults.mailcommand.copy();
  cmd = cmd.sprintf(postitdefaults.mailcommand.ascii(),
		    mailDialog->getSubject().ascii(),
		    mailDialog->getRecipient().ascii());

  delete mailDialog;

  FILE *mailpipe = popen( cmd.ascii(),"w" );
  if(mailpipe == 0)
  {
    QString msg = i18n(""
      "Could not pipe the contents of this KNote into:\n %1").arg(cmd);
    KMessageBox::sorry(this, msg);
    return;
  }

  QTextStream t( mailpipe, IO_WriteOnly );

  int line_count = edit->numLines();
  for(int i = 0 ; i < line_count ; i++)
  {
    t << edit->textLine(i) << '\n';
  }
  pclose(mailpipe);
}

void KPostit::insertCalendar( void )
{
  FILE *calfile = popen("cal","r");

  QTextStream t(calfile,IO_ReadOnly );
  while ( !t.eof() ) 
  {
    QString s = t.readLine();
    edit->insertLine( s );
  }
  repaint();
  // The calfile is closed by the QTextStream.
}

void KPostit::setAlarm( void )
{
  AlarmDialog *alarmDialog = new AlarmDialog( this, "alarm" );
  if( alarmDialog == 0 ) return;

  if( alarmDialog->exec() == QDialog::Rejected )
  {
    delete alarmDialog;
    return;
  }

  QDateTime dt = alarmDialog->getDateTime();
  delete alarmDialog;

  AlarmEntry *entry = new AlarmEntry;
  entry->name = name.copy();
  entry->dt = dt;

  AlarmList.append(entry);
  QString str = QString("%1 (A)").arg(name);
  setCaption( i18n("Note: ") + str );
  label->setText(str);
}

void KPostit::save_all( void )
{
  for( KPostit::PostitList.first(); KPostit::PostitList.current() != 0;
       KPostit::PostitList.next() )
  {
    KPostit::PostitList.current()->savenotes();
  }
}

void KPostit::print(){

  kapp->processEvents();
  kapp->flushX();

  FILE* printpipe;

  QString cmd;
  cmd = postitdefaults.printcommand.copy();
  cmd = cmd.sprintf(postitdefaults.printcommand.ascii(),name.ascii());
  //  printf("%s\n",cmd.ascii());

  printpipe = popen(cmd.ascii(),"w");

  if(printpipe == 0 )
  {
    QString msg = i18n(""
      "Could not pipe the contents of this KNote into:\n %1").arg(cmd);
    KMessageBox::sorry(this, msg);
    return;
  }

  QTextStream t(printpipe,IO_WriteOnly );

  int line_count = edit->numLines();

  for(int i = 0 ; i < line_count ; i++){
    t << edit->textLine(i) << '\n';
  }

  pclose(printpipe);

}

void KPostit::newKPostit(){

  QString pname;
  bool exists = FALSE;

  // lets give it the next available name of the for kpostit %d, with %d
  // and integer. If we have more than fifty of those, we give up.

  for (int i = 1; i < 50; i++){

    exists = FALSE;
    pname = "";
    pname = QString("knote %1").arg(i);

/*    for(PostitFilesList.first();
	(PostitFilesList.current() && (!exists));
	  PostitFilesList.next()){

      if ( QString(PostitFilesList.current()) == pname){ // file exist already
*/
	for(QStringList::Iterator it=PostitFilesList.begin();
		it != PostitFilesList.end() && !exists;it++) {
		if (*it==pname) {
		exists = TRUE;
		break;
    	  }
    }
    if (!exists)
      break;

  }

  if (exists){ // all 50 names are taken
    KMessageBox::sorry(this,
			 i18n("You have exeeded the "
			      "arbitrary and unjustly set "
			      "limit of 50 knotes.\n Please complain to the author."));
    return;
  }

  // update all popup menus
  for(PostitList.first();PostitList.current();PostitList.next()){

    // now remove and then reinsert, in a sorted fashion, the popup menu entries
    for(uint i = 0 ; i < PostitFilesList.count();i++){
      PostitList.current()->right_mouse_button->removeItemAt(0);
    }

  }



  PostitFilesList.append(pname);
  PostitFilesList.sort();

  for(PostitList.first();PostitList.current();PostitList.next()){
    int k = 0;
    /*for(PostitFilesList.first();PostitFilesList.current();PostitFilesList.next()){
      PostitList.current()->right_mouse_button->insertItem(
	     PostitFilesList.current(),k,k);
      k++;
    }*/
    for(QStringList::Iterator it=PostitFilesList.begin();
	it != PostitFilesList.end();it++){
      PostitList.current()->right_mouse_button->insertItem(*it,k,k);
      k++;
    }
  }



  KPostit *t = new KPostit (NULL,NULL,PostitFilesList.count() - 1 ,pname);

  t->show ();
  PostitList.append( t );

  docker->createLeftPopUp();

}

void  KPostit::RMBActivated(int i){
  // convert absolute id to relative index
  findKPostit(right_mouse_button->indexOf(i));
}

void KPostit::findKPostit(int i){

  if ( i < 0)
    return;

  if ( i >= (int)PostitFilesList.count()){// this is not a kpostit file menue entry
                                          // this should however already been caught
                                          // by the above if (i < 0)
    return;
  }

  // activate the correponding kpostit if a windget already exists for it

  for(PostitList.first(); PostitList.current() ; PostitList.next()){
    if (PostitList.current()->name == QString(right_mouse_button->text(
					      right_mouse_button->idAt( i)))) {
      if( PostitList.current()->hidden == true){
	PostitList.current()->hidden = false;
	if(propertystring != (QString) "")
	    PostitList.current()->setGeometry(KWM::setProperties(PostitList.current()->winId(),
			     PostitList.current()->propertystring));

      }
      PostitList.current()->show();
      KWM::activate(PostitList.current()->winId());
      return;
    }
  }

  KPostit *t = new KPostit (NULL,NULL,i,
			    right_mouse_button->text(
			    right_mouse_button->idAt(i)));
  t->show ();
  PostitList.append( t );

}


void KPostit::renameKPostit(){

  QString newName;
  RenameDialog* dlg = new RenameDialog(this,"renamedlg",true,&newName,
				       &PostitFilesList);
  if(dlg->exec()){

    QString notesfile( locateLocal("appdata", "notes/"+name) );
    QString newnotesfile( locateLocal("appdata", "notes/"+newName) );

    if(rename(notesfile.ascii(),newnotesfile.ascii())){

    }


    /*for(PostitFilesList.first();PostitFilesList.current();PostitFilesList.next()){
      if(name == QString(PostitFilesList.current())){
	PostitFilesList.remove(PostitFilesList.at());*/
	for(QStringList::Iterator it=PostitFilesList.begin();
		it != PostitFilesList.end();it++) {
		if (name == *it) {
			PostitFilesList.remove(it);		
			break;
      }
    }

    // update name in timer list if this postit was registered there

    bool have_alarm = FALSE;

    mytimer->stop();
    for(KPostit::AlarmList.first();KPostit::AlarmList.current();
	KPostit::AlarmList.next()){

      if (KPostit::AlarmList.current()->name == name){
        have_alarm = TRUE;
	KPostit::AlarmList.current()->name = newName.copy();
      }
    }
    mytimer->start();

    PostitFilesList.append(newName);
    PostitFilesList.sort();

    name = newName;


    if (have_alarm){
      label->setText(name + " (A)");
      setCaption(i18n("Note: ") + name + " (A)");
    }
    else {
      label->setText(name);
      setCaption(i18n("Note: ") + name);
    }

    // remove and reinsert the popup menues in a sorted fashion

    for(PostitList.first();PostitList.current();PostitList.next()){
      for(uint i = 0 ; i < PostitFilesList.count();i++){
	PostitList.current()->right_mouse_button->removeItemAt(0);
      }

      int k = 0;
/*      for(PostitFilesList.first();PostitFilesList.current();PostitFilesList.next()){
	PostitList.current()->right_mouse_button->insertItem(
			     PostitFilesList.current(),k,k);*/
	for(QStringList::Iterator it=PostitFilesList.begin();
		it != PostitFilesList.end(); it++) {
		PostitList.current()->right_mouse_button->insertItem(*it,k,k);
		k++;
    }
    docker->createLeftPopUp();
    }
  }
  delete dlg;
}

void KPostit::hideKPostit(){

  int numVisible = 0;

  hidden = true;
  propertystring = KWM::properties(winId());
  this->hide();

  for(PostitList.first(); PostitList.current() ; PostitList.next())
    if(PostitList.current()->hidden == false)
      numVisible++;
  if ((numVisible == 0) && !dock)
    // no more visible notes and not docked = no menu !
    toggleDock();
}


void KPostit::deleteKPostit()
{
  QString msg = i18n(""
    "Are you sure you want to delete this\n"
    "note permanently?");
  int result = KMessageBox::warningYesNo( this, msg );
  if( result != KMessageBox::Yes )
  {
    return;
  }

  QString notesfile( locateLocal("appdata", "notes/"+name) );
  if( remove(notesfile.ascii()) )
  {
  }


  for(PostitList.first();PostitList.current();PostitList.next()){
    // remove popup entries
    for(uint i = 0 ; i < PostitFilesList.count();i++){
      PostitList.current()->right_mouse_button->removeItemAt(0);
    }
  }



/*  for(PostitFilesList.first();PostitFilesList.current();PostitFilesList.next()){
    if(name == QString(PostitFilesList.current())){
      PostitFilesList.remove(PostitFilesList.at());
      break;
    }
  }
*/
  for(QStringList::Iterator it=PostitFilesList.begin();
	it != PostitFilesList.end(); it++) {
	if (name == *it) {
		PostitFilesList.remove(it);
		break;
	}
  }
		
  if ((PostitFilesList.count()==0) && !dock)
    // no more notes and non docked = no menu !
    toggleDock();

  // reinsert PostitFilesList into popus in a sorted fashion
  for(PostitList.first();PostitList.current();PostitList.next()){
    int k = 0;
    /*for(PostitFilesList.first();PostitFilesList.current();PostitFilesList.next()){
      PostitList.current()->right_mouse_button->insertItem(
					      PostitFilesList.current(),k,k);
      k++;
    }*/
	for(QStringList::Iterator it=PostitFilesList.begin();
		it != PostitFilesList.end() ; it++) {
		PostitList.current()->right_mouse_button->insertItem(*it,k,k);
		k++;
	}
  }

  docker->createLeftPopUp();

  mytimer->stop();
  for(KPostit::AlarmList.first();KPostit::AlarmList.current();KPostit::AlarmList.next()){

    if (KPostit::AlarmList.current()->name == name){
      delete KPostit::AlarmList.current();
      KPostit::AlarmList.remove(KPostit::AlarmList.current());
    }
  }
  mytimer->start();

  PostitList.remove( this );
  delete this;
}

bool KPostit::loadnotes(){


  QString notesfile ( locateLocal("appdata", "notes/"+name) );

  QFile file(notesfile);


  if( !file.open( IO_ReadOnly )) {
    return FALSE;
  }

  edit->setAutoUpdate(FALSE);
  QTextStream t(&file);

  // get the name
  QString name = t.readLine();

  // get the geomtery

  QString geom = t.readLine();
  propertystring = geom;

  setGeometry(KWM::setProperties(winId(), geom));

  // get the backcolor

  QString red = t.readLine();
  QString green = t.readLine();
  QString blue = t.readLine();

  backcolor.setRgb(red.toUInt(),green.toUInt(),blue.toUInt());

  // get the forecolor

  red = t.readLine();
  green = t.readLine();
  blue = t.readLine();

  forecolor.setRgb(red.toUInt(),green.toUInt(),blue.toUInt());

  // get the font

  QString fontfamily = t.readLine();

  int 	pointsize;
  int 	weight;
  bool 	italic = FALSE;

  QString pointsizestr = t.readLine();
  QString weightstr    = t.readLine();
  QString italicstr    = t.readLine();

  pointsize = pointsizestr.toUInt();
  weight   = weightstr.toUInt();
  int italicint	 = italicstr.toUInt();

  if(italicint == 1)
    italic = TRUE;
  else
    italic = FALSE;

  font = QFont(fontfamily,pointsize,weight,italic);


  int int3d;
  QString int3dstr = t.readLine();
  int3d = int3dstr.toUInt();
  if(int3d == 1)
    frame3d = TRUE;
  else
    frame3d = FALSE;

  int indent;
  QString autostr = t.readLine();
  indent = autostr.toUInt();
  if(indent == 1)
    edit->autoIndentMode = TRUE;
  else
    edit->autoIndentMode = FALSE;

  QString hiddenstring = t.readLine();
  int hiddenint = hiddenstring.toUInt();
  if(hiddenint == 1)
    hidden = true;
  else
    hidden = false;

  // get the text body

  while ( !t.eof() ) {
    QString s = t.readLine();
//    if(!t.eof())
      edit->insertLine( s );
  }

  edit->setAutoUpdate(TRUE);
  file.close();
  return TRUE;

}

bool KPostit::insertFile(const QString &filename){


  QFile file(filename);


  if( !file.open( IO_ReadOnly )) {
    QString string;
    string = i18n("Could not load:\n %1").arg(filename);
    KMessageBox::sorry(this, string);
    return FALSE;
  }

  edit->setAutoUpdate(FALSE);
  QTextStream t(&file);


  while ( !t.eof() ) {
    QString s = t.readLine();
//    if(!t.eof())
      edit->insertLine( s );
  }

  edit->setAutoUpdate(TRUE);
  file.close();
  edit->repaint();
  return TRUE;

}



void KPostit::resizeEvent( QResizeEvent * ){
  label->adjustSize();
  label->setGeometry(1,1,width()-label->height()-2,label->height());
  edit->setGeometry(1, label->height()+1, width()-2, height()-label->height()-2);
  mybutton->setGeometry(this->width()-label->height()-1 ,1,label->height(),label->height());
}

void KPostit::closeEvent( QCloseEvent * ){
  deleteKPostit();
};

bool KPostit::savenotes(){

  QString notesfile( locateLocal("appdata", "notes/"+name) );

  QFile file(notesfile);
  //  QFile file2("/home/wuebben/knotes.txt");

  if( !file.open( IO_WriteOnly)) {
    return FALSE;
  }

  /* if( !file2.open( IO_WriteOnly | IO_Truncate )) {
    return FALSE;
  }*/

  QTextStream t(&file);
// QTextStream t2(&file2);

  t << name <<'\n';
  //  t2 << name <<'\n';

  if( this->hidden){

    t << propertystring << '\n';
    //    t2 << propertystring << '\n';
  }
  else{
    t << KWM::properties(winId()) <<'\n';
    //    t2 << KWM::properties(winId()) <<'\n';
  }

  t << backcolor.red() <<'\n';
  t << backcolor.green()<< '\n';
  t << backcolor.blue() <<'\n';

  t << forecolor.red()<< '\n';
  t << forecolor.green() <<'\n';
  t << forecolor.blue()<< '\n';


  t << font.family() <<'\n';
  t << font.pointSize()<< '\n';
  t << font.weight() <<'\n';
  t << (font.italic() ? 1 : 0) << endl;
  t << (frame3d ? 1 : 0) << endl;
  t << (edit->autoIndentMode ? 1 : 0) << endl;
  t << (hidden ? 1 : 0) << endl;

  int line_count = edit->numLines();

  for(int i = 0 ; i < line_count ; i++){
    t << edit->textLine(i) << '\n';
  }

  file.close();
  //  file2.close();

  return TRUE;
}

bool KPostit::eventFilter(QObject *o, QEvent *ev){

  static QPoint tmp_point;

  QMouseEvent *e = (QMouseEvent *)ev;


  if (o == label){
    if (ev->type() == QEvent::MouseButtonRelease){
      if (e->button() == LeftButton){
        dragging = false;
        label->releaseMouse();
	raise();
      }
      if (e->button() == MidButton)
	lower();
      return TRUE;
    }

    if (ev->type() == QEvent::MouseButtonPress
	&& e->button() == LeftButton){
      pointerOffset = e->pos();
      label->grabMouse(sizeAllCursor);
      return TRUE;
    }
    if (ev->type() == QEvent::MouseMove
	&& label == mouseGrabber()){
	if (dragging) {
	    move(QCursor::pos()-pointerOffset);
	}
      else {
	dragging = (
	  (e->pos().x() - pointerOffset.x())
	  *
	  (e->pos().x() - pointerOffset.x())
	  +
	  (e->pos().y() - pointerOffset.y())
	    *
	  (e->pos().y() - pointerOffset.y())
	  >= 9);
      }
      return TRUE;
    }

    if (ev->type() == QEvent::MouseMove)
      return TRUE;
  }

  (void) o;

  if(ev->type() != QEvent::MouseButtonPress)
    return FALSE;

  if(e->button() != RightButton)
    return FALSE;

  tmp_point = QCursor::pos();

  if(right_mouse_button){
    desktops->clear();
    int i;
    int n = KWin::numberOfDesktops();
    for (i=1; i <= n; i++){
      QString b = "&";
      //b.append(KWM::desktopName(i));
      b.append(QString::number(i));
      desktops->insertItem(b, i);
    }
    desktops->setItemChecked(KWin::currentDesktop(), TRUE);
    /* 
    if (KWM::isSticky(winId()))
      right_mouse_button->changeItem(KWM::unStickyString(), sticky_id);
    else
      right_mouse_button->changeItem(KWM::stickyString(), sticky_id);
    */
    right_mouse_button->popup(tmp_point);
  }

  return TRUE;

}

void KPostit::set3DFrame(bool enable)
{
  frame3d = enable;
  options->setItemChecked(frame3dID, enable);

  long oldDeco = KWM::decoration(winId());

  if (enable)
    KWM::setDecoration(winId(),
		       oldDeco & ~KWM::noDecoration | KWM::tinyDecoration);
  else
    KWM::setDecoration(winId(),
		       oldDeco & ~KWM::tinyDecoration | KWM::noDecoration);

  edit->repaint();
}


void KPostit::toggleFrame( void )
{
  set3DFrame( !frame3d );
}


void KPostit::toggleDock( void )
{
  if( dock == true )
  {
    dock = false;
    docker->undock();
  }
  else
  {
    dock = true;
    docker->dock();
  }
  options->setItemChecked(dockID,dock);
}


void KPostit::defaults( void )
{
  if( mOptionDialog == 0 )
  {
    mOptionDialog = new OptionDialog( this, "option", false );
    if( mOptionDialog == 0 ) return; 
    mOptionDialog->setState( postitdefaults );
    connect( mOptionDialog, SIGNAL( valueChanged(const DefStruct &)),
	     this, SLOT(configurationChanged(const DefStruct &)));
  }
  mOptionDialog->show();
}


void KPostit::configurationChanged( const DefStruct &state )
{
  bool onTop = postitdefaults.onTop;
  postitdefaults = state;
  postitdefaults.onTop = onTop;
}















void KPostit::set_colors(){


  QPalette mypalette = (edit->palette()).copy();

  QColorGroup cgrp = mypalette.normal();
  QColorGroup ncgrp(forecolor,cgrp.background(),
		    cgrp.light(),cgrp.dark(),cgrp.mid(),forecolor,backcolor);

  mypalette.setNormal(ncgrp);
  mypalette.setDisabled(ncgrp);
  mypalette.setActive(ncgrp);

  edit->setPalette(mypalette);
  edit->setBackgroundColor(backcolor);
  label->setPalette(mypalette);
  label->setBackgroundColor(backcolor.dark(120));
  mybutton->setPalette(mypalette);
  mybutton->setBackgroundColor(backcolor.dark(120));
  setBackgroundColor(backcolor.dark(150));

}


void KPostit::set_foreground_color(){

  QColor color;

  color = forecolor;

  if(KColorDialog::getColor(color) != QDialog::Accepted)
    return;

  forecolor = color;
  set_colors();



}

void KPostit::set_background_color(){

  QColor color;

  color = backcolor;

  if( KColorDialog::getColor(color) != QDialog::Accepted)
    return;

  backcolor = color;
  set_colors();

}


static void siguser1(int sig){
  sig = sig;

  if (KPostit::PostitList.count() == 0){
    KPostit::PostitFilesList.append("knote 1");
    KPostit* postit = new KPostit(NULL,NULL,0,"knote 1");
    KPostit::PostitList.append(postit);
    postit->show();
  }
  else {
    if( KPostit::PostitList.last()->hidden == true){
      KPostit::PostitList.last()->hidden = false;
      if(KPostit::PostitList.last()->propertystring != (QString) "")
	  KPostit::PostitList.last()->setGeometry(KWM::setProperties(KPostit::PostitList.last()->winId(),
			   KPostit::PostitList.last()->propertystring));

    }
    KPostit::PostitList.last()->show();
    KWM::activate(KPostit::PostitList.last()->winId());
    //    KPostit::PostitList.last()->newKPostit();
  }

  signal(SIGUSR1, siguser1);
}

void KPostit::insertNetFile( const char *_url)
{

  QString string;
  QString netFile = _url;

  debug("KPostit::insertNetfile()");

  KURL u( netFile );

  if ( u.isMalformed() )
    {
	KMessageBox::sorry(
			     this,
			     i18n("Malformed URL"));
	return;
    }

    // Just a usual file ?
    if ( u.protocol() == "file" )
    {
      insertFile( u.path() );
      return;
    }

    if (KIONetAccess::download( netFile, tmpFile ))
    {
      insertFile( tmpFile );
      KIONetAccess::removeTempFile( tmpFile );
    }

}


void KPostit::close( void )
{
  if( savenotes() == false )
  {
    QString msg = i18n(""
      "Could not save the notes.\n"
      "Close anyways?");
    int result = KMessageBox::warningYesNo( this, msg );
    if( result != KMessageBox::Yes )
    {
      return;
    }
  }

  if ( PostitList.count() > 1 )
  {
    PostitList.remove( this );
    delete this;
  }
  else
  {
    quit();
  }
}


void KPostit::toggleIndentMode(){
  if(edit->autoIndentMode)
    setNoAutoIndent();
  else
    setAutoIndent();
}

void KPostit::toDesktop(int d){
  KWM::moveToDesktop(winId(), d);
}

void KPostit::toggleSticky(){
  KWM::setSticky(winId(), !KWM::isSticky(winId()));
}

void KPostit::setAutoIndent(){

  edit->autoIndentMode = TRUE;
  options->setItemChecked(edit->autoIndentID,TRUE);
}

void KPostit::setNoAutoIndent(){

  edit->autoIndentMode = FALSE;
  options->setItemChecked(edit->autoIndentID,FALSE);

}

void KPostit::setOnTop(bool enable)
{
  long oldDeco = KWM::decoration(winId());

  if (enable)
    KWM::setDecoration(winId(),oldDeco | KWM::staysOnTop);
  else
    KWM::setDecoration(winId(), oldDeco & ~KWM::staysOnTop);

  options->setItemChecked(onTopID, enable);
  postitdefaults.onTop = enable;
}

void KPostit::toggleOnTopMode()
{
  setOnTop(!postitdefaults.onTop);
}

void findPostitFiles(){

  QString filesdir = locateLocal( "appdata", "notes/" );
  QString alarmdir = locateLocal( "appdata", "xyalarms/" );

  QDir d(filesdir);
  d.setSorting( QDir::Name );

  QStringList list = d.entryList();
  QStringList::Iterator it = list.begin(); // create list iterator
  QString fi;                           // pointer for traversing

  while ( !(fi=*it).isNull() ) {           // for each file...
    if ("." == fi || ".." == fi || "xyalarms" == fi){
      ++it;
      continue;
    }

    KPostit::PostitFilesList.append(fi);
    KPostit::PostitFilesList.sort();
    ++it;
  }

}

void alarmConsistencyCheck(){

  for(KPostit::AlarmList.first();KPostit::AlarmList.current();
      KPostit::AlarmList.next()){

    if (KPostit::PostitFilesList.find(KPostit::AlarmList.current()->name) == 
	KPostit::PostitFilesList.end()){

      QString str;
      str = i18n("Found an alarm to which the underlying\n"\
		  "KNotes file:\n"\
		  "%1\n no longer exists.\n\n"\
		  "I will correct the Problem for you.")
		  .arg(KPostit::AlarmList.current()->name);

      KMessageBox::sorry(0,
			 str,
			 i18n("Inconsistency"));

      KPostit::AlarmList.remove(KPostit::AlarmList.current());

    }
  }
}

sessionWidget::sessionWidget() {
  // the only function of this widget is to catch & forward the
  // saveYourself() signal from the session manager
  connect(kapp, SIGNAL(saveYourself()), SLOT(wm_saveyourself()));
}

void sessionWidget::wm_saveyourself() {

  remove( pidFile.ascii() );
  savealarms();
  writeSettings();

  for(KPostit::PostitList.first();KPostit::PostitList.current();
      KPostit::PostitList.next())

    KPostit::PostitList.current()->savenotes();
}

int main( int argc, char **argv ) {

  FILE *fp;
  int pid;
  KPostit* postit;

  KApplication a(argc, argv, "knotes");

  pidFile = locateLocal( "appdata", "knotes.pid");

  //
  // if there is a pidFile then this is not the first instance of kpostit
  //
  if ( ( fp = fopen( pidFile.ascii(), "r" ) ) != NULL )
    {

      fscanf( fp, "%d", &pid);
      printf("pid %d\n",pid);

      // if this fails I assume that the pid file is left over from a bad exit
      // and continue on
      //
      if ( kill( pid, SIGUSR1 ) == 0){

	// the kpostit is still alive
	// it could however be zombi or a recycled pid -- use IPC instead.

	exit(0);
      }

      fclose( fp );
    }

  catchSignals();

  fp = fopen( pidFile.ascii(), "w" );
  fprintf( fp, "%d\n", getpid());
  fclose( fp );

  readSettings();

  findPostitFiles();
  readalarms();
  alarmConsistencyCheck();

  mytimer = new MyTimer();
  savetimer = new SaveTimer();

  bool restoring = false;

  if (QString("-knotes_restore") == (QString)argv[1]){
    restoring = true;
  }


  if(KPostit::PostitFilesList.count() == 0 && !restoring){
    KPostit::PostitFilesList.append("knote 1");
  }

  //unsigned int i;
  bool one_is_visible = false;

  /*for (i=0; i<KPostit::PostitFilesList.count(); i++){
    postit = new KPostit(NULL,NULL,0,KPostit::PostitFilesList.at(i));*/
	for(QStringList::Iterator it=KPostit::PostitFilesList.begin();
		it != KPostit::PostitFilesList.end(); it++) {
	postit = new KPostit(NULL,NULL,0,*it);
    KPostit::PostitList.append(postit);
    if(!postit->hidden){
      postit->show();
      one_is_visible = true;
    }
  }


  // manual session management (knotes alredy stores everything)
  kapp->setTopWidget(new sessionWidget);
#if 0  
  kapp->enableSessionManagement(true);
  kapp->setWmCommand("knotes -knotes_restore");
#endif  

  docker = new DockWidget();
  if (KPostit::dock)
      docker->dock();

  return a.exec();

}


void readSettings()
{

  QString str;

  KConfig *config = kapp->config();
  config->setGroup( "Font" );
  QFont defaultFont("helvetica",12);
  postitdefaults.font = config->readFontEntry("Font", &defaultFont);

  config->setGroup("Colors");

  QColor blackC(Qt::black);
  QColor yellowC(Qt::yellow);

  postitdefaults.forecolor = config->readColorEntry("ForeColor",&blackC);
  postitdefaults.backcolor = config->readColorEntry("BackColor",&yellowC);

  config->setGroup("Geometry");

  postitdefaults.width = config->readNumEntry("width",230);
  postitdefaults.height = config->readNumEntry("height",165);

  config->setGroup("Misc");

  postitdefaults.frame3d = (bool) config->readNumEntry("frame3d",(int)FALSE);
  postitdefaults.onTop = config->readBoolEntry("always visible", FALSE);
  postitdefaults.autoindent = (bool) config->readNumEntry("autoindent",(int)TRUE);
  KPostit::dock = config->readBoolEntry("dock",TRUE);
  postitdefaults.playSound = config->readBoolEntry("play sound", FALSE);

  config->setGroup("Commands");

  postitdefaults.mailcommand = config->readEntry("mailCmd");
  if (postitdefaults.mailcommand.isEmpty())
    postitdefaults.mailcommand = "mail -s\"%s\" %s";

  postitdefaults.printcommand = config->readEntry("printCmd");
  if (postitdefaults.printcommand.isEmpty())
    postitdefaults.printcommand = "a2ps -1 --center-title=\"%s\" "\
      "--underlay=\"KDE\"";

  postitdefaults.soundcommand = config->readEntry("soundCmd");
  if (postitdefaults.soundcommand.isEmpty()) {
    QString soundfile = locate("appdata", "knotes_alarm.wav");
    if (soundfile.isNull())
      soundfile = "knotes_alarm.wav";
    postitdefaults.soundcommand = ("kplayaudio " + soundfile);
  }
}

void writeSettings()
{

  KConfig *config = kapp->config();

  config->setGroup( "Font" );
  config->writeEntry("Font",postitdefaults.font);

  config->setGroup("Colors");
  config->writeEntry("ForeColor",postitdefaults.forecolor);
  config->writeEntry("BackColor",postitdefaults.backcolor);

  config->setGroup("Geometry");
  config->writeEntry("width",  postitdefaults.width);
  config->writeEntry("height",  postitdefaults.height);

  config->setGroup("Misc");
  config->writeEntry("frame3d",postitdefaults.frame3d);
  config->writeEntry("always visible", postitdefaults.onTop);
  config->writeEntry("autoindent",postitdefaults.autoindent);
  config->writeEntry("dock",KPostit::dock);
  config->writeEntry("play sound", postitdefaults.playSound);

  config->setGroup("Commands");
  config->writeEntry("mailCmd",postitdefaults.mailcommand);
  config->writeEntry("printCmd",postitdefaults.printcommand);
  config->writeEntry("soundCmd",postitdefaults.soundcommand);

  config->sync();

}



static void cleanup( int sig )
{

  (void) sig;
  if(cleaning_up)
    return;
  cleaning_up = true;
  //  printf("KPostit: Caught signal %d. Tyring to save state.\n",sig);
  remove( pidFile.ascii() );
  savealarms();
  writeSettings();

  for(KPostit::PostitList.first();KPostit::PostitList.current();
      KPostit::PostitList.next()){

    KPostit::PostitList.current()->savenotes();

  }


  QApplication::exit();
}


// make sure the pid file is cleaned up when exiting unexpectedly.

void catchSignals()
{
	signal(SIGHUP, cleanup);		/* Hangup */
	signal(SIGINT, cleanup);		/* Interrupt */
	signal(SIGTERM, cleanup);		/* Terminate */
//	signal(SIGCHLD, cleanup);

	signal(SIGABRT, cleanup);
	signal(SIGUSR1, siguser1);
	signal(SIGALRM, cleanup);
	signal(SIGFPE, cleanup);
	signal(SIGILL, cleanup);
//	signal(SIGPIPE, cleanup);
	signal(SIGQUIT, cleanup);
//	signal(SIGSEGV, cleanup);

#ifdef SIGBUS
	signal(SIGBUS, cleanup);
#endif
#ifdef SIGPOLL
	signal(SIGPOLL, cleanup);
#endif
#ifdef SIGSYS
	signal(SIGSYS, cleanup);
#endif
#ifdef SIGTRAP
	signal(SIGTRAP, cleanup);
#endif
#ifdef SIGVTALRM
	signal(SIGVTALRM, cleanup);
#endif
#ifdef SIGXCPU
	signal(SIGXCPU, cleanup);
#endif
#ifdef SIGXFSZ
	signal(SIGXFSZ, cleanup);
#endif
}


myPushButton::myPushButton(QWidget *parent, const char* name)
  : QPushButton( parent, name ){
    setFocusPolicy(NoFocus);
    flat = True;
    last_button = 0;
}

void myPushButton::enterEvent( QEvent * ){
  flat = False;
  repaint(FALSE);
}

void myPushButton::leaveEvent( QEvent * ){
  flat = True;
    repaint();
}

void myPushButton::paint(QPainter *painter){
  if ( isDown() || (isOn() && !flat)) {
    if ( style() == WindowsStyle )
      qDrawWinButton( painter, 0, 0, width(),
		      height(), colorGroup(), TRUE );
    else
      qDrawShadePanel( painter, 0, 0, width(),
		       height(), colorGroup(), TRUE, 2, 0L );
  }
  else if (!flat ) {
    if ( style() == WindowsStyle )
      qDrawWinButton( painter, 0, 0, width(), height(),
		      colorGroup(), FALSE );
    else {
      qDrawShadePanel( painter, 0, 0, width(), height(),
		       colorGroup(), FALSE, 2, 0L );
//       painter->setPen(black);
//       painter->drawRect(0,0,width(),height());
    }
  }


   int dx = ( width() - pixmap()->width() ) / 2;
  int dy = ( height() - pixmap()->height() ) / 2;
  if ( isDown() && style() == WindowsStyle ) {
    dx++;
    dy++;
  }

  painter->drawPixmap( dx, dy, *pixmap());

  /*
  QPen pen = painter->pen();
  pen.setWidth(2);
  pen.setColor(backgroundColor().light(180));
  //  pen.setColor(white);
  painter->setPen(pen);

  painter->drawLine(2,2,width()-3,height()-2);
  painter->drawLine(width()-3,2,2,height()-2);
  */
}

void myPushButton::mousePressEvent( QMouseEvent *e){

  if ( isDown())
    return;

  bool hit = hitButton( e->pos() );
  if ( hit ){
    last_button = e->button();
    setDown( TRUE );
    repaint( FALSE );
    emit pressed();
  }
}

void myPushButton::mouseReleaseEvent( QMouseEvent *e){
  if ( !isDown() ){
    last_button = 0;
    return;
  }
  bool hit = hitButton( e->pos() );
  setDown( FALSE );
  if ( hit ){
    if ( isToggleButton() )
      setOn( !isOn() );
    repaint( FALSE );
    if ( isToggleButton() )
      emit toggled( isOn() );
    emit released();
    emit clicked();
  }
  else {
    repaint();
    emit released();
  }
  last_button = 0;
}

void myPushButton::mouseMoveEvent( QMouseEvent *e ){

  if (!last_button)
    return;

  if ( !(e->state() & LeftButton) &&
       !(e->state() & MidButton) &&
       !(e->state() & RightButton))
    return;


  bool hit = hitButton( e->pos() );
  if ( hit ) {
    if ( !isDown() ) {
      setDown(TRUE);
      repaint(FALSE);
      emit pressed();
    }
  } else {
    if ( isDown() ) {
      setDown(FALSE);
      repaint();
      emit released();
    }
  }
}


