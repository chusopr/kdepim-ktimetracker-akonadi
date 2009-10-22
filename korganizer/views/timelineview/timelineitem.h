/*
  Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#ifndef KORG_TIMELINEITEM_H
#define KORG_TIMELINEITEM_H

#include <kdgantt1/KDGanttViewTaskItem.h>

#include <Akonadi/Item>

#include <KDateTime>

#include <QMap>
#include <QList>

class KDGanttView;
class KDCanvasPolygon;

namespace KCal {
  class Incidence;
}
using namespace KCal;

namespace KOrg {
class AkonadiCalendar;
class TimelineSubItem;

class TimelineItem : public KDGanttViewTaskItem
{
  public:
    TimelineItem( const QString &label, AkonadiCalendar *calendar, KDGanttView *parent );

    void insertIncidence( const Akonadi::Item &incidence,
                          const KDateTime &start = KDateTime(),
                          const KDateTime &end = KDateTime() );
    void removeIncidence( const Akonadi::Item &incidence );

    void moveItems( const Akonadi::Item &incidence, int delta, int duration );

  private:
    AkonadiCalendar *mCalendar;
    QMap<Akonadi::Item::Id, QList<TimelineSubItem*> > mItemMap;
};

class TimelineSubItem : public KDGanttViewTaskItem
{
  public:
    TimelineSubItem( AkonadiCalendar *calendar, const Akonadi::Item &incidence, TimelineItem *parent );
    ~TimelineSubItem();

    Akonadi::Item  incidence() const { return mIncidence; }

    KDateTime originalStart() const { return mStart; }
    void setOriginalStart( const KDateTime &dt ) { mStart = dt; }

  private:
    void showItem( bool show = true, int coordY = 0 );

  private:
    Akonadi::Item mIncidence;
    KDateTime mStart;
    KDCanvasPolygon *mLeft, *mRight;
    int mMarkerWidth;
};

}

#endif
