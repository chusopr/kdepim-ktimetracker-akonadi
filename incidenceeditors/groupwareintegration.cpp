/*
  Copyright (c) 2010 Kevin Ottens <ervin@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "groupwareintegration.h"
#include "editorconfig.h"
#include "incidenceeditor-ng/incidencedialog.h"
#include "incidenceeditor-ng/incidencedialogfactory.h"
#include "journaleditor.h"

#include <kcalcore/visitor.h>

#include <kcalprefs.h>

#include <akonadi/kcal/calendar.h>  //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/calendarmodel.h> //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/groupware.h> //krazy:exclude=camelcase since kdepim/akonadi
#include <akonadi/kcal/utils.h>     //krazy:exclude=camelcase since kdepim/akonadi

#include <Akonadi/ChangeRecorder>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Session>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include <KSystemTimeZones>

#include "korganizereditorconfig.h"

using namespace KCalCore;
using namespace IncidenceEditors;

class EditorDialogVisitor : public Visitor
{
  public:
    EditorDialogVisitor() : Visitor(), mEditor( 0 ), mDialog( 0 ) {}
    IncidenceEditor *editor() const { return mEditor; }

    // TODO: Once the JournalEditor is ported, we can just use the factory method
    //       and get rid of this visitor.

  protected:
    bool visit( Event::Ptr  )
    {
      return true;
    }
    bool visit( Todo::Ptr  )
    {
      return true;
    }
    bool visit( Journal::Ptr  )
    {
      mEditor = new JournalEditor( 0 );
      return mEditor;
    }
    bool visit( FreeBusy::Ptr  ) // to inhibit hidden virtual compile warning
    {
      return 0;
    }

    IncidenceEditor *mEditor;
    IncidenceEditorsNG::IncidenceDialog *mDialog;
};

class GroupwareUiDelegate : public QObject, public Akonadi::GroupwareUiDelegate
{
  public:
    GroupwareUiDelegate()
    {
      Akonadi::Session *session = new Akonadi::Session( "GroupwareIntegration", this );
      Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( this );

      Akonadi::ItemFetchScope scope;
      scope.fetchFullPayload( true );

      monitor->setSession( session );
      monitor->setCollectionMonitored( Akonadi::Collection::root() );
      monitor->fetchCollection( true );
      monitor->setItemFetchScope( scope );
      monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::eventMimeType(), true );
      monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::todoMimeType(), true );
      monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::journalMimeType(), true );

      Akonadi::CalendarModel *calendarModel = new Akonadi::CalendarModel( monitor, this );

      mCalendar = new Akonadi::Calendar( calendarModel, calendarModel,
                                         KSystemTimeZones::local() );
      mCalendar->setOwner( Person( KCalPrefs::instance()->fullName(),
                                   KCalPrefs::instance()->email() ) );
    }

    void requestIncidenceEditor( const Akonadi::Item &item )
    {
      const Incidence::Ptr incidence = Akonadi::incidence( item );
      if ( !incidence ) {
        return;
      }

      EditorDialogVisitor v;
      if ( !incidence->accept( v, incidence ) ) {
        return;
      }

      if ( v.editor() ) {
        // TODO: Get rid of this as soon as the JournalEditor is ported as well.
        IncidenceEditor *editor = v.editor();
        editor->editIncidence( item, QDate::currentDate() );
        editor->selectInvitationCounterProposal( true );
        editor->setIncidenceChanger( new Akonadi::IncidenceChanger( mCalendar, this, -1 ) );
        editor->show();
      } else {
        IncidenceEditorsNG::IncidenceDialog *dialog = IncidenceEditorsNG::IncidenceDialogFactory::create( incidence->type() );
        dialog->setIsCounterProposal( true );
        dialog->load( item, QDate::currentDate() );
      }
    }

    Akonadi::Calendar *mCalendar;
};

K_GLOBAL_STATIC( GroupwareUiDelegate, globalDelegate )

bool GroupwareIntegration::sActivated = false;

bool GroupwareIntegration::isActive()
{
  return sActivated;
}

void GroupwareIntegration::activate()
{
  EditorConfig::setEditorConfig( new KOrganizerEditorConfig );
  Akonadi::Groupware::create( globalDelegate->mCalendar, &*globalDelegate );
  sActivated = true;
}

