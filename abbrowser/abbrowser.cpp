/*
 * pab.cpp
 *
 * Copyright (C) 1999 Don Sanders <dsanders@kde.org>
 */
#include "abbrowser.h"
#include "browserentryeditor.h"

#include <qkeycode.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmenubar.h>
#include <kconfig.h>
#include <kaccel.h>
#include <kdebug.h>
#include <kfiledialog.h>

#include "undo.h"
#include "browserwidget.h"
#include "contactentry.h"
#include "contactentrylist.h"

Pab::Pab() : KMainWindow(0), DCOPObject("AbBrowserIface")
{
  setCaption( i18n( "Address Book Browser" ));
  document = new ContactEntryList();
  view = new PabWidget( document, this, "Abbrowser" );

  // tell the KMainWindow that this is indeed the main widget
  setCentralWidget(view);

  // create a popup menu -- in this case, the File menu
  QPopupMenu* p = new QPopupMenu;
  p->insertItem(i18n("&Sync"), this, SLOT(save()), CTRL+Key_S);
  p->insertItem(i18n("&New Contact"), this, SLOT(newContact()), CTRL+Key_N);
  /*  p->insertItem(i18n("New &Group"), this, SLOT(newGroup()), CTRL+Key_G); */
  p->insertSeparator();
  p->insertItem(i18n("Send &Mail"), view, SLOT(sendMail()), CTRL+Key_Return);
  p->insertSeparator();
  p->insertItem(i18n("&Properties"), view, SLOT(properties()));
  p->insertItem(i18n("&Delete\tDel"));
  p->insertSeparator();
  p->insertItem(i18n("&Export List"), this, SLOT(exportCSV()));
  p->insertSeparator();
  p->insertItem(i18n("&Quit"), kapp, SLOT(quit()), CTRL+Key_Q);
  
  edit = new QPopupMenu;
  undoId = edit->insertItem(i18n("Undo"), this, SLOT(undo()), 
			    KStdAccel::key(KStdAccel::Undo));
  redoId = edit->insertItem(i18n("Redo"), this, SLOT(redo()),
			    KStdAccel::key(KStdAccel::Redo));
  edit->insertSeparator();
  edit->insertItem(i18n("Cut"), view, SLOT(cut()), CTRL+Key_X);
  edit->insertItem(i18n("Copy"), view, SLOT(copy()), CTRL+Key_C);
  edit->insertItem(i18n("Paste"), view, SLOT(paste()), CTRL+Key_V);
  edit->insertSeparator();
  edit->insertItem(i18n("Select All"), view, SLOT(selectAll()), CTRL+Key_A);
  edit->setItemEnabled( undoId, false );
  edit->setItemEnabled( redoId, false );
  QObject::connect( edit, SIGNAL( aboutToShow() ), this, SLOT( updateEditMenu() ));

  QPopupMenu* v = new QPopupMenu;
  v->insertItem(i18n("Choose Fields..."), view, SLOT(showSelectNameDialog()) );
  v->insertItem(i18n("Options..."), view, SLOT(viewOptions()) );
  v->insertSeparator();
  v->insertItem(i18n("Restore defaults"), view, SLOT(defaultSettings()) );
  //  v->insertSeparator();
  //  v->insertItem(i18n("Refresh"), view, SLOT(refresh()), Key_F5 );

  // put our newly created menu into the main menu bar
  menuBar()->insertItem(i18n("&File"), p);
  menuBar()->insertItem(i18n("&Edit"), edit);
  menuBar()->insertItem(i18n("&View"), v);
  menuBar()->insertSeparator();

  // KDE will generate a short help menu automagically
  p = helpMenu( i18n("Abbrowser --- KDE Address Book\n\n"
		     "(c) 2000AD The KDE PIM Team \n"));

  menuBar()->insertItem(i18n("&Help"), p);
  
  toolBar()->insertButton(BarIcon("filenew"),   // icon
			  0,                  // button id
			  SIGNAL(clicked()),  // action
			  this, SLOT(newContact()), // result
			  true, i18n("Add a new entry"));      // tooltip text
  toolBar()->insertButton(BarIcon("edit"),   // icon
			  1,                  // button id
			  SIGNAL(clicked()),  // action
			  view, SLOT(properties()), // result
			  true, i18n("Change this entry"));      // tooltip text
  toolBar()->insertButton(BarIcon("editdelete"),   // icon
			  2,                  // button id
			  SIGNAL(clicked()),  // action
			  view, SLOT(clear()), // result
			  true, i18n("Remove this entry"));      // tooltip text
  toolBar()->insertSeparator();
  toolBar()->insertButton(BarIcon("mail_send"),   // icon
			  3,                  // button id
			  SIGNAL(clicked()),  // action
			  view, SLOT(sendMail()), // result
			  true, i18n("Send email"));      // tooltip text
  toolBar()->setFullSize(true);
  
  // we do want a status bar
  statusBar()->show();
  connect( kapp, SIGNAL( aboutToQuit() ), this, SLOT( saveConfig() ) );
  resize( sizeHint() );
  readConfig();
}

void Pab::newContact()
{
 qDebug("Pab::newContact() start"); 
  ContactDialog *cd = new PabNewContactDialog( i18n( "Address Book Entry Editor" ), this, 0 );
  connect( cd, SIGNAL( add( ContactEntry* ) ), 
	   view, SLOT( addNewEntry( ContactEntry* ) ));
  cd->show();
  qDebug("Pab::newContact() stop"); 

}

void Pab::addEmail( QString addr )
{
  view->addEmail( addr );
  return;
}

void Pab::addEntry(ContactEntry newEntry)
    {
    view->addNewEntry(new ContactEntry(newEntry) );
    }

QStringList  Pab::getKeys() const
    {
    return document->keys();
    }

QDict<ContactEntry> Pab::getEntryDict() const
    {
    return document->getDict();
    }

void  Pab::changeEntry( QString key, ContactEntry changeEntry)
    {
    document->replace(key, new ContactEntry(changeEntry));
    view->pabListView()->getItem(key)->refresh();
    }

void  Pab::removeEntry( QString key )
    {
    document->remove(key);
    delete view->pabListView()->getItem(key);
    }

void Pab::save()
{
  document->commit();
  document->refresh();
  view->saveConfig();
  view->readConfig();
  view->reconstructListView();
  //xxx  document->save( "entries.txt" );
    UndoStack::instance()->clear();
    RedoStack::instance()->clear();
}

void Pab::readConfig()
{
  KConfig *config = kapp->config();
  int w, h;
  config->setGroup("Geometry");
   QString str = config->readEntry("Browser", "");
   if (!str.isEmpty() && str.find(',')>=0)
   {
     sscanf(str.local8Bit(),"%d,%d",&w,&h);
     resize(w,h);
   }
  applyMainWindowSettings( config );
}

void Pab::saveConfig()
{
  view->saveConfig();
  KConfig *config = kapp->config();

  config->setGroup("Geometry");
  QRect r = geometry();
  QString s;
  s.sprintf("%i,%i", r.width(), r.height());
  config->writeEntry("Browser", s);
  saveMainWindowSettings( config );
  config->sync();
}

Pab::~Pab()
{
  saveConfig();
  delete document;
}

void Pab::saveCe() {
  kdDebug() << "saveCe()" << endl;
  //xxx  ce->save( "entry.txt" );
}

void Pab::saveProperties(KConfig *)
{
	// the 'config' object points to the session managed
	// config file.  anything you write here will be available
	// later when this app is restored
  //what I want to save
  //windowsize
  //background image/underlining color/alternating color1,2
  //chosen fields
  //chosen fieldsWidths
  
	
	// e.g., config->writeEntry("key", var); 
}

void Pab::readProperties(KConfig *)
{
	// the 'config' object points to the session managed
	// config file.  this function is automatically called whenever
	// the app is being restored.  read in here whatever you wrote
	// in 'saveProperties'

	// e.g., var = config->readEntry("key"); 
}

void Pab::undo()
{
  kdDebug() << "Pab::undo()" << endl;
  UndoStack::instance()->undo();
}

void Pab::redo()
{
  RedoStack::instance()->redo();
}

void Pab::updateEditMenu()
{
  kdDebug() << "UpdateEditMenu()" << endl;
  UndoStack *undo = UndoStack::instance();
  RedoStack *redo = RedoStack::instance();

  if (undo->isEmpty())
    edit->changeItem( undoId, i18n( "Undo" ) );
  else
    edit->changeItem( undoId, i18n( "Undo" ) + " " + undo->top()->name() );
  edit->setItemEnabled( undoId, !undo->isEmpty() );

  if (redo->isEmpty())
    edit->changeItem( redoId, i18n( "Redo" ) );
  else
    edit->changeItem( redoId, i18n( "Redo" ) + " " + redo->top()->name() );
  edit->setItemEnabled( redoId, !redo->isEmpty() );  
}

void Pab::exportCSV()
{
  QString fileName = KFileDialog::getSaveFileName("addressbook.csv");

  QFile outFile(fileName);
  if ( outFile.open(IO_WriteOnly) ) {    // file opened successfully
    QTextStream t( &outFile );        // use a text stream

    QStringList keys = document->keys();
    QStringList::ConstIterator it = keys.begin();
    QStringList::ConstIterator end = keys.end();

    t << "\"First Name\",";
    t << "\"Last Name\",";
    t << "\"Middle Name\",";
    t << "\"Name Prefix\",";
    t << "\"Job Title\",";
    t << "\"Company\",";
    t << "\"Email\",";
    t << "\"Nickname\",";
    t << "\"Note\",";
    t << "\"Business Phone\",";
    t << "\"Home Phone\",";
    t << "\"Mobile Phone\",";
    t << "\"Home Fax\",";
    t << "\"Business Fax\",";
    t << "\"Pager\",";
    t << "\"Street Home Address\",";
    t << "\"City Home Address\",";
    t << "\"State Home Address\",";
    t << "\"Zip Home Address\",";
    t << "\"Country Home Address\",";
    t << "\"Street Business Address\",";
    t << "\"City Business Address\",";
    t << "\"State Business Address\",";
    t << "\"Zip Business Address\",";
    t << "\"Country Business Address\"";
    t << "\n";

    while(it != end) {
      ContactEntry *entry = document->find(*it);

      t << "\"" << entry->getFirstName() << "\",";
      t << "\"" << entry->getLastName() << "\",";
      t << "\"" << entry->getMiddleName() << "\",";
      t << "\"" << entry->getNamePrefix() << "\",";
      t << "\"" << entry->getJobTitle() << "\",";
      t << "\"" << entry->getCompany() << "\",";
      t << "\"" << entry->getEmail() << "\",";
      t << "\"" << entry->getNickname() << "\",";
      t << "\"" << entry->getNote() << "\",";
      t << "\"" << entry->getBusinessPhone() << "\",";
      t << "\"" << entry->getHomePhone() << "\",";
      t << "\"" << entry->getMobilePhone() << "\",";
      t << "\"" << entry->getHomeFax() << "\",";
      t << "\"" << entry->getBusinessFax() << "\",";
      t << "\"" << entry->getPager() << "\",";
      ContactEntry::Address *address;
      address = entry->getHomeAddress();
      t << "\"" << address->getStreet() << "\",";
      t << "\"" << address->getCity() << "\",";
      t << "\"" << address->getState() << "\",";
      t << "\"" << address->getZip() << "\",";
      t << "\"" << address->getCountry() << "\",";
      address = entry->getBusinessAddress();
      t << "\"" << address->getStreet() << "\",";
      t << "\"" << address->getCity() << "\",";
      t << "\"" << address->getState() << "\",";
      t << "\"" << address->getZip() << "\",";
      t << "\"" << address->getCountry(); 
      t << "\n";
      
      ++it;
    }

    outFile.close();
  }
}
