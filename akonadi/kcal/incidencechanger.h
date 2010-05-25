/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef INCIDENCECHANGER_H
#define INCIDENCECHANGER_H

#include "akonadi-kcal_next_export.h"

#include "utils.h"

#include <KCal/Incidence>
#include <KCal/Scheduler>

#include <QtCore/QObject>

class KJob;

namespace Akonadi {

class Groupware;
class Calendar;

class AKONADI_KCAL_NEXT_EXPORT IncidenceChanger : public QObject
{
  Q_OBJECT
  public:
    IncidenceChanger( Akonadi::Calendar *cal,
                      QObject *parent,
                      Akonadi::Entity::Id defaultCollectionId );
    ~IncidenceChanger();

    enum HowChanged {
      INCIDENCEADDED,
      INCIDENCEEDITED,
      INCIDENCEDELETED,
      NOCHANGE
    };

    enum WhatChanged {
      PRIORITY_MODIFIED,
      COMPLETION_MODIFIED,
      CATEGORY_MODIFIED,
      DATE_MODIFIED,
      RELATION_MODIFIED,
      ALARM_MODIFIED,
      DESCRIPTION_MODIFIED,
      SUMMARY_MODIFIED,
      COMPLETION_MODIFIED_WITH_RECURRENCE,
      RECURRENCE_MODIFIED_ONE_ONLY,
      RECURRENCE_MODIFIED_ALL_FUTURE,
      UNKNOWN_MODIFIED,
      NOTHING_MODIFIED
    };

    enum DestinationPolicy {
      USE_DEFAULT_DESTINATION,   // the default collection is used, unless it's invalid
      ASK_DESTINATION          // user is asked in which collection
    };

    void setGroupware( Groupware *groupware );

    bool sendGroupwareMessage( const Akonadi::Item &incidence,
                               KCal::iTIPMethod method,
                               HowChanged action,
                               QWidget *parent );

    // returns true if the add job was created
    bool addIncidence( const KCal::Incidence::Ptr &incidence,
                       QWidget *parent, Akonadi::Collection &selectedCollection,
                       int &dialogCode );

    // returns true if the add job was created
    bool addIncidence( const KCal::Incidence::Ptr &incidence,
                       const Akonadi::Collection &collection, QWidget *parent );

    bool changeIncidence( const KCal::Incidence::Ptr &oldinc,
                          const Akonadi::Item &newItem,
                          WhatChanged,
                          QWidget *parent );

    // returns true if the delete job was created
    bool deleteIncidence( const Akonadi::Item &incidence, QWidget *parent );

    bool cutIncidences( const Akonadi::Item::List &incidences, QWidget *parent );
    bool cutIncidence( const Akonadi::Item &incidence, QWidget *parent );

    void setDefaultCollectionId( Akonadi::Entity::Id );

    static bool incidencesEqual( KCal::Incidence *inc1, KCal::Incidence *inc2 );
    static bool assignIncidence( KCal::Incidence *inc1, KCal::Incidence *inc2 );

    void setDestinationPolicy( DestinationPolicy destinationPolicy );
    DestinationPolicy destinationPolicy() const;

    /*
     * Returns false if the item is being deleted by a job
     * or was deleted already.
     *
     * This is more accurate than querying the ETM because when a delete
     * job ends the ETM still has the item for a short period of time.
     */
    bool wasntDeleted( Akonadi::Item::Id ) const;

  public Q_SLOTS:
    void cancelAttendees( const Akonadi::Item &incidence );

  protected:
    Akonadi::Calendar *mCalendar;

  Q_SIGNALS:
    // Signals emited by the Item*Job, the bool parameter is the success of the operation
    void incidenceAddFinished( const Akonadi::Item &, bool );

    void incidenceChangeFinished( const Akonadi::Item &oldinc,
                                  const Akonadi::Item &newInc,
                                  Akonadi::IncidenceChanger::WhatChanged,
                                  bool );

    void incidenceDeleteFinished( const Akonadi::Item &, bool );


    void incidenceToBeDeleted( const Akonadi::Item & );
    void schedule( KCal::iTIPMethod method, const Akonadi::Item &incidence );

  private Q_SLOTS:
    void addIncidenceFinished( KJob* job );
    void deleteIncidenceFinished( KJob* job );

  private:
    class Private;
    Private * const d;
};
}

#endif
