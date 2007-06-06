/*
    This file is part of the KOrganizer interfaces.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORG_CALENDARDECORATION_H
#define KORG_CALENDARDECORATION_H

#include <QString>
#include <QDateTime>
#include <QPixmap>
#include <QList>

#include <klibloader.h>

#include "plugin.h"

namespace KOrg {

namespace CalendarDecoration {

/* Class for calendar decoration elements
 * It provides entities like texts and pictures for a given date.
 * Implementations can implement all functions or only a subset.
 */
class Element
{
  public:
    Element() {}
    virtual ~Element() {}

    virtual QList<QString> availablePositions() = 0;

    /* The positions the decoration element can accept.
     * By default, they are all available positions.
     */
    QList<QString> acceptablePositions() { return availablePositions(); }

    /* The decoration element's current position.
     */
    QString position() { return m_position; }

    /* The widget to be shown for a given @param date,
     * with @param parent as parent widget.
     */
    virtual QWidget *widget( QWidget *parent, const QDate &date ) { return 0; }

  protected:
    QString m_position;

  public slots:
    /* Slot to use to allow the widget to adapt to a @param newPosition
     * when it changed.
     */
    void positionChanged( const QString &newPosition ) {}

};

/* Class for calendar decoration elements in the agenda view
 */
class AgendaElement : public Element
{
  public:
    AgendaElement() {}
    virtual ~AgendaElement() {}

    QList<QString> availablePositions() {
      QList<QString> l;
      l << "Panel" << "Top" << "Left" << "Bottom" << "Right"
                                                       // "Standard" positions
        << "DayTopT" << "DayTopL" << "DayTopB" << "DayTopR"
                    // Around the top label (T=top, L=left, B=bottom, R=right)
        << "DayBottom"; // Under the event list
      return l; }

};

/* This class provides the interface for a date dependent decoration.
 *
 * The decoration is made of various decoration elements,
 * which show a defined widget for a given date.
 */
class Decoration : public Plugin
{
  public:
    static int interfaceVersion() { return 3; }
    static QString serviceType() { return "Calendar/Decoration"; }

    typedef QList<Decoration*> List;

    Decoration() {}
    virtual ~Decoration() {}

    /* Returns the various decoration elements of this decoration
     * for the agenda view.
     */
    QList<AgendaElement*> agenda() { return agendaElements; }

  protected:
    QList<AgendaElement*> agendaElements;

};


class DecorationFactory : public PluginFactory
{
  public:
    virtual Decoration *create() = 0;
};

}

}

#endif
