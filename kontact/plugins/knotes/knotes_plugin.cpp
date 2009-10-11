/*
  This file is part of Kontact
  Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "knotes_plugin.h"
#include "knotes_part.h"
#include "summarywidget.h"

#include <libkdepim/kvcarddrag.h>
#include <libkdepim/maillistdrag.h>
using namespace KPIM;

#include <KCal/CalendarLocal>
#include <KCal/ICalDrag>

#include <KontactInterface/Core>

#include <KAboutData>
#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KMessageBox>
#include <KSystemTimeZones>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDropEvent>

EXPORT_KONTACT_PLUGIN( KNotesPlugin, knotes )

KNotesPlugin::KNotesPlugin( KontactInterface::Core *core, const QVariantList & )
  : KontactInterface::Plugin( core, core, "knotes" ), mAboutData( 0 )
{
  setComponentData( KontactPluginFactory::componentData() );

  KAction *action = new KAction( KIcon( "knotes" ), i18n( "New Popup Note..." ), this );
  actionCollection()->addAction( "new_note", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(slotNewNote()) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_N ) );
  insertNewAction( action );

  KAction *syncAction = new KAction( KIcon( "view-refresh" ), i18n( "Sync Popup Notes" ), this );
  actionCollection()->addAction( "knotes_sync", syncAction );
  connect( syncAction, SIGNAL(triggered(bool)), SLOT(slotSyncNotes()) );
  insertSyncAction( syncAction );
}

KNotesPlugin::~KNotesPlugin()
{
}

QString KNotesPlugin::tipFile() const
{
  // TODO: tips file
  //QString file = KStandardDirs::locate("data", "knotes/tips");
  QString file;
  return file;
}

KParts::ReadOnlyPart *KNotesPlugin::createPart()
{
  return new KNotesPart( this );
}

KontactInterface::Summary *KNotesPlugin::createSummaryWidget( QWidget *parentWidget )
{
  return new KNotesSummaryWidget( this, parentWidget );
}

const KAboutData *KNotesPlugin::aboutData() const
{
  if ( !mAboutData ) {
    mAboutData = new KAboutData( "knotes", 0, ki18n( "KNotes" ),
                                 "0.5", ki18n( "Popup Notes" ),
                                 KAboutData::License_GPL_V2,
                                 ki18n( "(c) 2003-2004 The Kontact developers" ) );
    mAboutData->addAuthor( ki18n( "Michael Brade" ),
                           ki18n( "Current Maintainer" ), "brade@kde.org" );
    mAboutData->addAuthor( ki18n( "Tobias Koenig" ),
                           KLocalizedString(), "tokoe@kde.org" );
  }

  return mAboutData;
}

bool KNotesPlugin::canDecodeMimeData( const QMimeData *mimeData ) const
{
  return
    mimeData->hasText() ||
    MailList::canDecode( mimeData ) ||
    KVCardDrag::canDecode( mimeData ) ||
    ICalDrag::canDecode( mimeData );
}

void KNotesPlugin::processDropEvent( QDropEvent *event )
{
  const QMimeData *md = event->mimeData();

  if ( KVCardDrag::canDecode( md ) ) {
    KABC::Addressee::List contacts;

    KVCardDrag::fromMimeData( md, contacts );

    KABC::Addressee::List::ConstIterator it;

    QStringList attendees;
    for ( it = contacts.constBegin(); it != contacts.constEnd(); ++it ) {
      QString email = (*it).fullEmail();
      if ( email.isEmpty() ) {
        attendees.append( (*it).realName() + "<>" );
      } else {
        attendees.append( email );
      }
    }
    event->accept();
    static_cast<KNotesPart *>( part() )->newNote( i18n( "Meeting" ), attendees.join( ", " ) );
    return;
  }

  if ( ICalDrag::canDecode( event->mimeData() ) ) {
    CalendarLocal cal( KSystemTimeZones::local() );
    if ( ICalDrag::fromMimeData( event->mimeData(), &cal ) ) {
      Journal::List journals = cal.journals();
      if ( !journals.isEmpty() ) {
        event->accept();
        Journal *j = journals.first();
        static_cast<KNotesPart *>( part() )->
          newNote( i18n( "Note: %1", j->summary() ), j->description() );
        return;
      }
      // else fall through to text decoding
    }
  }

  if ( md->hasText() ) {
    static_cast<KNotesPart *>( part() )->newNote( i18n( "New Note" ), md->text() );
    return;
  }

  if ( MailList::canDecode( md ) ) {
    MailList mails = MailList::fromMimeData( md );
    event->accept();
    if ( mails.count() != 1 ) {
      KMessageBox::sorry( core(),
                          i18n( "Dropping multiple mails is not supported." ) );
    } else {
      MailSummary mail = mails.first();
      QString txt = i18n( "From: %1\nTo: %2\nSubject: %3", mail.from(), mail.to(), mail.subject() );
      static_cast<KNotesPart *>( part() )->newNote( i18n( "Mail: %1", mail.subject() ), txt );
    }
    return;
  }

  kWarning() << QString( "Cannot handle drop events of type '%1'." ).arg( event->format() );
}

// private slots

void KNotesPlugin::slotNewNote()
{
  if ( part() ) {
    static_cast<KNotesPart *>( part() )->newNote();
  }
}

void KNotesPlugin::slotSyncNotes()
{
  QDBusMessage message = QDBusMessage::createMethodCall(
    "org.kde.kmail", "/Groupware", "org.kde.kmail.groupware", "triggerSync" );
  message << QString( "Note" );
  QDBusConnection::sessionBus().send( message );
}

#include "knotes_plugin.moc"
