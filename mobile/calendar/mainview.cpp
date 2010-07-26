/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Volker Krause <vkrause@kde.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include "mainview.h"
#include "calendarinterface.h"
#include "calendaradaptor.h"

#include <akonadi/entitytreemodel.h>
#include <Akonadi/ItemFetchScope>
#include <akonadi/kcal/incidencemimetypevisitor.h>
#include <akonadi/kcal/calendar.h>

#include <ksystemtimezone.h>

#include <qdeclarativeengine.h>
#include <qdeclarativecontext.h>

#include "incidenceview.h"

#include <QGraphicsItem>
#include <QTimer>
#include <QDBusConnection>

using namespace Akonadi;
using namespace KCal;

MainView::MainView( QWidget* parent ) : KDeclarativeMainView( "korganizer-mobile", 0 /* TODO */, parent )
{
  m_calendar = 0;
  QTimer::singleShot(0, this, SLOT(delayedInit()));
}

MainView::~MainView()
{
  m_calendar->deleteLater();
}

void MainView::delayedInit()
{
  addMimeType( IncidenceMimeTypeVisitor::eventMimeType() );
  addMimeType( IncidenceMimeTypeVisitor::todoMimeType() );
  itemFetchScope().fetchFullPayload();

  m_calendar = new Akonadi::Calendar( entityTreeModel(), regularSelectedItems(), KSystemTimeZones::local() );
  engine()->rootContext()->setContextProperty( "calendarModel", QVariant::fromValue( static_cast<QObject*>( m_calendar ) ) );
  
  QDBusConnection::sessionBus().registerService("org.kde.korganizer"); //register also as the real korganizer, so kmail can communicate with it
  CalendarInterface* calendarIface = new CalendarInterface();
  new CalendarAdaptor(calendarIface);
  QDBusConnection::sessionBus().registerObject("/Calendar", calendarIface);

  //connect Qt signals to QML slots
  connect(calendarIface, SIGNAL(showDateSignal(QVariant)), rootObject(), SLOT(showDate(QVariant)));
  connect(calendarIface, SIGNAL(showEventViewSignal()), rootObject(), SLOT(showEventView()));
}

void MainView::showRegularCalendar()
{
  m_calendar->setUnfilteredModel(regularSelectedItems());
}

void MainView::setCurrentEventItemId(qint64 id)
{
  QModelIndexList list = itemModel()->match(itemModel()->index(0, 0), EntityTreeModel::ItemIdRole, id, 1 );
  if (list.isEmpty())
    return;

  setListSelectedRow(list.first().row());
}

void MainView::newEvent()
{
  IncidenceView *editor = new IncidenceView;
  Item item;
  item.setMimeType( Akonadi::IncidenceMimeTypeVisitor::eventMimeType() );
  KCal::Event::Ptr event( new KCal::Event );

  // make it take one hour from now
  event->setDtStart( KDateTime::currentLocalDateTime() );
  event->setDtEnd( KDateTime::currentLocalDateTime().addSecs( 3600 ) );

  item.setPayload<KCal::Event::Ptr>( event );
  editor->load( item );
  editor->show();
}

void MainView::newTodo()
{
  IncidenceView *editor = new IncidenceView;
  Item item;
  item.setMimeType( Akonadi::IncidenceMimeTypeVisitor::todoMimeType() );
  KCal::Todo::Ptr todo( new KCal::Todo );

  // make it due one day from now
  todo->setDtStart( KDateTime::currentLocalDateTime() );
  todo->setDtDue( KDateTime::currentLocalDateTime().addDays( 1 ) );

  item.setPayload<KCal::Todo::Ptr>( todo );
  editor->load( item );
  editor->show();
}

void MainView::editIncidence( const Akonadi::Item &item, const QDate &date )
{
  IncidenceView *editor = new IncidenceView;
  editor->load( item, date );
  editor->show();
}


#include "mainview.moc"
