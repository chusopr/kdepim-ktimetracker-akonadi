/*
    This file is part of Kontact.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <QWidget>
#include <QDropEvent>
#include <QtDBus/QtDBus>

#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kicon.h>
#include <ktemporaryfile.h>

#include <kcal/calendarlocal.h>
#include <kcal/icaldrag.h>

#include <libkdepim/maillistdrag.h>
#include <libkdepim/kdepimprotocols.h>
#include <libkdepim/kvcarddrag.h>
#include <libkdepim/kpimprefs.h>

#include "core.h"

#include "todoplugin.h"
#include "todosummarywidget.h"
#include "korg_uniqueapp.h"
#include "calendarinterface.h"

typedef KGenericFactory< TodoPlugin, Kontact::Core > TodoPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_todoplugin,
                            TodoPluginFactory( "kontact_todoplugin" ) )

TodoPlugin::TodoPlugin( Kontact::Core *core, const QStringList& )
  : Kontact::Plugin( core, core, "korganizer" ),
    mIface( 0 )
{
  setComponentData( TodoPluginFactory::componentData() );
  KIconLoader::global()->addAppDir("korganizer");


  KAction *action  = new KAction(KIcon("newtodo"), i18n("New To-do..."), this);
  actionCollection()->addAction("new_todo", action );
  action->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_T));
  connect(action, SIGNAL(triggered(bool)), SLOT( slotNewTodo() ));
  insertNewAction(action);

  KAction* syncAction = new KAction(KIcon("view-refresh"), i18n( "Synchronize To-do List" ), this);
  actionCollection()->addAction("todo_sync", syncAction);
  connect(action, SIGNAL(triggered(bool)), SLOT( slotSyncTodos()));
  insertSyncAction( syncAction );

  mUniqueAppWatcher = new Kontact::UniqueAppWatcher(
      new Kontact::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this );
}

TodoPlugin::~TodoPlugin()
{
}

Kontact::Summary *TodoPlugin::createSummaryWidget( QWidget *parent )
{
  return new TodoSummaryWidget( this, parent );
}

KParts::ReadOnlyPart *TodoPlugin::createPart()
{
  KParts::ReadOnlyPart *part = loadPart();

  if ( !part )
    return 0;

#ifdef __GNUC__
  #warning "Once we have a running korganizer, make sure that this dbus call really does what it should! Where is it needed, anyway?"
#endif
  mIface = new OrgKdeKorganizerCalendarInterface( "org.kde.korganizer", "/Calendar", QDBusConnection::sessionBus(), this );

  return part;
}

void TodoPlugin::select()
{
  interface()->showTodoView();
}

QStringList TodoPlugin::invisibleToolbarActions() const
{
  QStringList invisible;
  invisible += "new_event";
  invisible += "new_todo";
  invisible += "new_journal";

  invisible += "view_day";
  invisible += "view_list";
  invisible += "view_workweek";
  invisible += "view_week";
  invisible += "view_nextx";
  invisible += "view_month";
  invisible += "view_journal";
  return invisible;
}

OrgKdeKorganizerCalendarInterface *TodoPlugin::interface()
{
  if ( !mIface ) {
    part();
  }
  Q_ASSERT( mIface );
  return mIface;
}

void TodoPlugin::slotNewTodo()
{
  interface()->openTodoEditor( "" );
}

void TodoPlugin::slotSyncTodos()
{
  QDBusMessage message = QDBusMessage::createSignal( "/Groupware", "org.kde.kmail", "triggerSync(QString)");
  message << QString( "Todo" );
  QDBusConnection::sessionBus().send( message );
}

bool TodoPlugin::createDBUSInterface( const QString& serviceType )
{
  kDebug(5602) << k_funcinfo << serviceType;
#ifdef __GNUC__
  #warning "What is this needed for, and do we still need it with DBUS?"
#endif
  if ( serviceType == "DBUS/Organizer" || serviceType == "DBUS/Calendar" ) {
    if ( part() )
      return true;
  }

  return false;
}

bool TodoPlugin::canDecodeMimeData( const QMimeData *mimeData )
{
  return mimeData->hasText() || KPIM::MailList::canDecode( mimeData );
}

bool TodoPlugin::isRunningStandalone()
{
  return mUniqueAppWatcher->isRunningStandalone();
}

void TodoPlugin::processDropEvent( QDropEvent *event )
{
  const QMimeData *md = event->mimeData();

  if ( KVCardDrag::canDecode( md ) ) {
    KABC::Addressee::List contacts;

    KVCardDrag::fromMimeData( md, contacts );

    KABC::Addressee::List::Iterator it;

    QStringList attendees;
    for ( it = contacts.begin(); it != contacts.end(); ++it ) {
      QString email = (*it).fullEmail();
      if ( email.isEmpty() )
        attendees.append( (*it).realName() + "<>" );
      else
        attendees.append( email );
    }

    interface()->openTodoEditor( i18n( "Meeting" ), QString(), QStringList(),
                                 attendees );
    return;
  }

  if ( KCal::ICalDrag::canDecode( event->mimeData() ) ) {
    KCal::CalendarLocal cal( KPimPrefs::timeSpec() );
    if ( KCal::ICalDrag::fromMimeData( event->mimeData(), &cal ) ) {
      KCal::Journal::List journals = cal.journals();
      if ( !journals.isEmpty() ) {
        event->accept();
        KCal::Journal *j = journals.first();
        interface()->openTodoEditor( i18n("Note: %1", j->summary() ), j->description(), QStringList() );
        return;
      }
      // else fall through to text decoding
    }
  }

  if ( md->hasText() ) {
    QString text = md->text();
    interface()->openTodoEditor( text );
    return;
  }

  if ( KPIM::MailList::canDecode( md ) ) {
    KPIM::MailList mails = KPIM::MailList::fromMimeData( md );
    event->accept();
    if ( mails.count() != 1 ) {
      KMessageBox::sorry( core(),
                          i18n("Drops of multiple mails are not supported." ) );
    } else {
      KPIM::MailSummary mail = mails.first();
      QString txt = i18n("From: %1\nTo: %2\nSubject: %3", mail.from() ,
                      mail.to(), mail.subject() );
      QString uri = KDEPIMPROTOCOL_EMAIL +
                    QString::number( mail.serialNumber() ) + '/' +
                    mail.messageId();
      KTemporaryFile tf;
      tf.setAutoRemove( true );
      tf.write( event->encodedData( "message/rfc822" ) );
      interface()->openTodoEditor( i18n("Mail: %1", mail.subject() ), txt,
                                   uri, tf.name(), QStringList(), "message/rfc822" );
      tf.close();
    }
    return;
  }

  KMessageBox::sorry( core(), i18n("Cannot handle drop events of type '%1'.",
                                event->format() ) );
}

#include "todoplugin.moc"
