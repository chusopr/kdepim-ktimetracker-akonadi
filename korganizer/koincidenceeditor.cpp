/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qtooltip.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qdatetime.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

#include <libkdepim/categoryselectdialog.h>
#include <libkdepim/designerfields.h>
#include <libkdepim/embeddedurlpage.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/incidence.h>
#include <libkcal/icalformat.h>

#include "koprefs.h"
#include "koglobals.h"
#include "koeditordetails.h"
#include "koeditorattachments.h"
#include "koeditoralarms.h"

#include "koincidenceeditor.h"

KOIncidenceEditor::KOIncidenceEditor( const QString &caption,
                                      Calendar *calendar, QWidget *parent )
  : KDialogBase( Tabbed, caption, Ok | Apply | Cancel | Default | User1, Ok,
                 parent, 0, false, false ),
    mDetails( 0 ), mAttachments( 0 ), mAlarms( 0 )
{
  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setWFlags( getWFlags() | WGroupLeader );

  mCalendar = calendar;

  setButtonText( Default, i18n("Load &Template...") );
  setButtonWhatsThis( Default,
                      i18n("Allows you to load a pre-saved "
                           "template for the event. You "
                           "can save this event or to-do as a "
                           "template by clicking 'Save as Template...'.") );

  QString saveTemplateText;
  if ( KOPrefs::instance()->mCompactDialogs ) {
    showButton( User1, false );
    showButton( Apply, false );
  } else {
    saveTemplateText = i18n("&Save as Template...");
  }
  setButtonText( User1, saveTemplateText );
  setButtonWhatsThis( User1,
                      i18n("Allows you to save the current settings as a template "
                           "which can later be loaded for other events.") );

  mCategoryDialog = new KPIM::CategorySelectDialog( KOPrefs::instance(), this );
  KOGlobals::fitDialogToScreen( mCategoryDialog );

  connect( mCategoryDialog, SIGNAL( editCategories() ),
           SIGNAL( editCategories() ) );

  connect( this, SIGNAL( defaultClicked() ), SLOT( slotLoadTemplate() ) );
  connect( this, SIGNAL( user1Clicked() ), SLOT( slotSaveTemplate() ) );
  connect( this, SIGNAL( finished() ), SLOT( delayedDestruct() ) );
}

KOIncidenceEditor::~KOIncidenceEditor()
{
  delete mCategoryDialog;
}

void KOIncidenceEditor::setupAttendeesTab()
{
  QFrame *topFrame = addPage( i18n("Atte&ndees") );
  QWhatsThis::add( topFrame,
                   i18n("The Attendees tab allows you to Add or Remove "
                        "Attendees to/from this event or to-do.") );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mDetails = new KOEditorDetails( spacingHint(), topFrame );
  topLayout->addWidget( mDetails );
}

void KOIncidenceEditor::setupAttachmentsTab()
{
  QFrame *topFrame = addPage( i18n("Attach&ments") );
  QWhatsThis::add( topFrame,
                   i18n("The Attachments tab allows you to add or remove "
                        "files, emails, contacts, and other items "
                        "associated with this event or to-do.") );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mAttachments = new KOEditorAttachments( spacingHint(), topFrame );
  topLayout->addWidget( mAttachments );
}

void KOIncidenceEditor::setupAlarmsTab()
{
  QFrame *topFrame = addPage( i18n("Re&minders") );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mAlarms = new KOEditorAlarms( spacingHint(), topFrame );
  topLayout->addWidget( mAlarms );
}

void KOIncidenceEditor::slotApply()
{
  processInput();
}

void KOIncidenceEditor::slotOk()
{
  if ( processInput() ) accept();
}

void KOIncidenceEditor::updateCategoryConfig()
{
  mCategoryDialog->updateCategoryConfig();
}

void KOIncidenceEditor::slotCancel()
{
  processCancel();
  reject();
}

void KOIncidenceEditor::cancelRemovedAttendees( Incidence *incidence )
{
  if ( !incidence ) return;

  // cancelAttendeeEvent removes all attendees from the incidence,
  // and then only adds those that need to be cancelled (i.e. a mail needs to be sent to them).
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) ) {
    Incidence *ev = incidence->clone();
    ev->registerObserver( 0 );
    mDetails->cancelAttendeeEvent( ev );
    if ( ev->attendeeCount() > 0 ) {
      emit deleteAttendee( ev );
    }
    delete( ev );
  }

}

void KOIncidenceEditor::slotLoadTemplate()
{
  kdDebug(5850) << "KOIncidenceEditor::loadTemplate()" << endl;
}

void KOIncidenceEditor::slotSaveTemplate()
{
  kdDebug(5850) << "KOIncidenceEditor::saveTemplate()" << endl;
  QString tp = type();
  QStringList templates;

  if ( tp == "Event" ) {
    templates = KOPrefs::instance()->mEventTemplates;
  } else if( tp == "ToDo" ) {
    templates = KOPrefs::instance()->mTodoTemplates;
  } else if ( tp == "Journal" ) {
    templates = KOPrefs::instance()->mJournalTemplates;
  }
  bool ok = false;
  QString templateName = KInputDialog::getItem( i18n("Save Template"),
      i18n("Please enter a name for the template:"), templates,
      -1, true, &ok, this );
  if ( ok && templateName.isEmpty() ) {
    KMessageBox::error( this, i18n("You did not give a valid template name, "
                                   "no template will be saved") );
    ok = false;
  }

  if ( ok && templates.contains( templateName ) ) {
    int res = KMessageBox::warningYesNo( this,
                                         i18n("The selected template "
                                              "already exists. Overwrite it?"),
                                         i18n("Template Already Exists") );
    if ( res == KMessageBox::No ) {
      ok = false;
    }
  }

  if ( ok ) {
    saveTemplate( templateName );

    // Add template to list of existing templates
    if ( !templates.contains( templateName ) ) {
      templates.append( templateName );
      if ( tp == "Event" ) {
        KOPrefs::instance()->mEventTemplates = templates;
      } else if ( tp == "ToDo" ) {
        KOPrefs::instance()->mTodoTemplates = templates;
      } else if ( tp == "Journal" ) {
        KOPrefs::instance()->mJournalTemplates = templates;
      }
    }

  }
}

void KOIncidenceEditor::saveAsTemplate( Incidence *incidence,
                                        const QString &templateName )
{
  if ( !incidence || templateName.isEmpty() ) return;

  QString fileName = "templates/" + incidence->type();
  fileName.append( "/" + templateName );
  fileName = locateLocal( "data", "korganizer/" + fileName );

  CalendarLocal cal( KOPrefs::instance()->mTimeZoneId );
  cal.addIncidence( incidence );
  ICalFormat format;
  format.save( &cal, fileName );
}

QString KOIncidenceEditor::loadTemplate( Calendar *cal, const QString &type,
                                         const QStringList &templates )
{
  bool ok = false;
  QString templateName = KInputDialog::getItem( i18n("Load Template"),
      i18n("Select a template to load:"), templates, 0, false, &ok, this );

  if ( !ok || templateName.isEmpty() ) return QString::null;

  QString fileName = locateLocal( "data", "korganizer/templates/" + type + "/" +
                                  templateName );

  if ( fileName.isEmpty() ) {
    KMessageBox::error( this, i18n("Unable to find template '%1'.")
                              .arg( fileName ) );
    return QString::null;
  } else {
    ICalFormat format;
    if ( !format.load( cal, fileName ) ) {
      KMessageBox::error( this, i18n("Error loading template file '%1'.")
                                .arg( fileName ) );
      return QString::null;
    }
  }

  return templateName;
}

void KOIncidenceEditor::setupDesignerTabs( const QString &type )
{
  QStringList activePages = KOPrefs::instance()->activeDesignerFields();

  QStringList list = KGlobal::dirs()->findAllResources( "data",
    "korganizer/designer/" + type + "/*.ui", true, true );
  for ( QStringList::iterator it = list.begin(); it != list.end(); ++it ) {
    kdDebug() << "Designer tab: " << *it << endl;

    if ( activePages.find( (*it).mid( (*it).findRev('/') + 1 ) ) == activePages.end() )
      continue;

    KPIM::DesignerFields *wid = new KPIM::DesignerFields( *it, 0 );
    mDesignerFields.append( wid );

    QFrame *topFrame = addPage( wid->title() );

    QBoxLayout *topLayout = new QVBoxLayout( topFrame );

    wid->reparent( topFrame, 0, QPoint() );
    topLayout->addWidget( wid );
  }
}

class KCalStorage : public KPIM::DesignerFields::Storage
{
  public:
    KCalStorage( Incidence *incidence )
      : mIncidence( incidence )
    {
    }

    QStringList keys()
    {
      QStringList keys;

      QMap<QCString, QString> props = mIncidence->customProperties();
      QMap<QCString, QString>::ConstIterator it;
      for( it = props.begin(); it != props.end(); ++it ) {
        QString customKey = it.key();
        QStringList parts = QStringList::split( "-", customKey );
        if ( parts.count() != 4 ) continue;
        if ( parts[ 2 ] != "KORGANIZER" ) continue;
        keys.append( parts[ 3 ] );
      }

      return keys;
    }

    QString read( const QString &key )
    {
      return mIncidence->customProperty( "KORGANIZER", key.utf8() );
    }

    void write( const QString &key, const QString &value )
    {
      mIncidence->setCustomProperty( "KORGANIZER", key.utf8(), value );
    }

  private:
    Incidence *mIncidence;
};

void KOIncidenceEditor::readDesignerFields( Incidence *i )
{
  KCalStorage storage( i );
  KPIM::DesignerFields *fields;
  for( fields = mDesignerFields.first(); fields;
       fields = mDesignerFields.next() ) {
    fields->load( &storage );
  }
}

void KOIncidenceEditor::writeDesignerFields( Incidence *i )
{
  kdDebug() << "KOIncidenceEditor::writeDesignerFields()" << endl;

  KCalStorage storage( i );
  KPIM::DesignerFields *fields;
  for( fields = mDesignerFields.first(); fields;
       fields = mDesignerFields.next() ) {
    kdDebug() << "Write Field " << fields->title() << endl;
    fields->save( &storage );
  }
}


void KOIncidenceEditor::setupEmbeddedURLPage( const QString &label,
                                 const QString &url, const QString &mimetype )
{
  kdDebug() << "KOIncidenceEditor::setupEmbeddedURLPage()" << endl;
  kdDebug() << "label=" << label << ", url=" << url << ", mimetype=" << mimetype << endl;
  QFrame *topFrame = addPage( label );
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  KPIM::EmbeddedURLPage *wid = new KPIM::EmbeddedURLPage( url, mimetype,
                                                          topFrame );
  topLayout->addWidget( wid );
  mEmbeddedURLPages.append( wid );
  // TODO: Call this method only when the tab is actually activated!
  wid->loadContents();
}

void KOIncidenceEditor::createEmbeddedURLPages( Incidence *i )
{
  kdDebug() << "KOIncidenceEditor::createEmbeddedURLPages()" << endl;

  if ( !i ) return;
  if ( !mEmbeddedURLPages.isEmpty() ) {
    mEmbeddedURLPages.setAutoDelete( true );
    mEmbeddedURLPages.clear();
    mEmbeddedURLPages.setAutoDelete( false );
  }
  Attachment::List att = i->attachments();
  for ( Attachment::List::Iterator it = att.begin(); it != att.end(); ++it ) {
    Attachment *a = (*it);
    kdDebug() << "Iterating over the attachments " << endl;
    kdDebug() << "label=" << a->label() << ", url=" << a->uri() << ", mimetype=" << a->mimeType() << endl;
    if ( a && a->showInline() && a->isUri() ) {
      // TODO: Allow more mime-types, but add security checks!
//       if ( a->mimeType() == "text/html" ) {
        setupEmbeddedURLPage( a->label(), a->uri(), a->mimeType() );
//       }
    }
  }
}

#include "koincidenceeditor.moc"
