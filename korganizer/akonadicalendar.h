/*
    This file is part of Akonadi.

    Copyright (c) 2009 Sebastian Sauer <sebsauer@kdab.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef AKONADICALENDAR_H
#define AKONADICALENDAR_H

#include <kcal/calendar.h>

namespace Akonadi {
  class Collection;
}

namespace KCal {

class CalFormat;

class KCAL_EXPORT AkonadiCalendar : public Calendar
{
    Q_OBJECT
  public:
    explicit AkonadiCalendar( const KDateTime::Spec &timeSpec );
    ~AkonadiCalendar();

    Akonadi::Collection collection() const;
    void setCollection( const Akonadi::Collection &collection );

    bool reload();
    bool save();
    void close();

    bool addEvent( Event *event );
    bool deleteEvent( Event *event );
    void deleteAllEvents();

    Event::List rawEvents( EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    Event::List rawEvents( const QDate &start, const QDate &end, const KDateTime::Spec &timeSpec = KDateTime::Spec(), bool inclusive = false );
    Event::List rawEventsForDate( const QDate &date, const KDateTime::Spec &timeSpec = KDateTime::Spec(), EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    Event::List rawEventsForDate( const KDateTime &dt );

    Event *event( const QString &uid );

    bool addTodo( Todo *todo );
    bool deleteTodo( Todo *todo );
    void deleteAllTodos();

    Todo::List rawTodos( TodoSortField sortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    Todo::List rawTodosForDate( const QDate &date );

    Todo *todo( const QString &uid );

    bool addJournal( Journal *journal );
    bool deleteJournal( Journal *journal );
    void deleteAllJournals();

    Journal::List rawJournals( JournalSortField sortField = JournalSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    Journal::List rawJournalsForDate( const QDate &date );

    Journal *journal( const QString &uid );

    Alarm::List alarms( const KDateTime &from, const KDateTime &to );
    Alarm::List alarmsTo( const KDateTime &to );

    void incidenceUpdated( IncidenceBase *incidenceBase );

    using QObject::event;   // prevent warning about hidden virtual method

  public Q_SLOTS:
    void deleteIncidenceProxyMethod( Incidence *incidence ) { deleteIncidence(incidence); }

  private:
    Q_DISABLE_COPY( AkonadiCalendar )
    class Private;
    Private *const d;
};

}

#endif
