/*
  This file is part of the KOrganizer interfaces.

  Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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

#include <QtCore/QString>
#include <QtCore/QFlags>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtGui/QPixmap>

#include <klibloader.h>

#include "plugin.h"

namespace KOrg {

namespace CalendarDecoration {

/**
  @class FlexibleElement

  @brief Class for calendar decoration elements

  It provides entities like texts and pictures for a given date.
  Implementations can implement all functions or only a subset.
 */
class FlexibleElement
{
  public:
    FlexibleElement() {}
    virtual ~FlexibleElement() {}

    /**
    Return a name for easy identification.
    This will be used for example for internal configuration (position, etc.),
    so don't i18n it and make it unique for your decoration.
     */
    virtual QString elementName() const = 0;
    /**
    Return a name for easy identification.
    This will be used for example for configuration (position, etc.).
     */
    virtual QString elementInfo() const = 0;
    
    /**
      Return a short text for a given date,
      usually only a few words.
     */
    virtual QString shortText( const QDate & ) const
      { return QString(); }
    /**
      Return a long text for a given date.
      This text can be of any length,
      but usually it will have one or a few lines.
     */
    virtual QString longText( const QDate & ) const
      { return QString(); }
    /**
      Return an extensive text for a given date.
      This text can be of any length,
      but usually it will have one or a few paragraphs.
     */
    virtual QString extensiveText( const QDate & ) const
      { return QString(); }

    /**
      Return a pixmap for a give date and a given size.
     */
    virtual QPixmap pixmap( const QDate &, const QSize & ) const
      { return QPixmap(); }

    /**
      Return a tooltip.
     */
    virtual QString toolTip( const QDate & ) const
      { return QString(); }

    /**
      Return a tooltip.
     */
    virtual KUrl url( const QDate & ) const
      { return KUrl(); }

/*    // TODO: think about this:
  signals:
    virtual void pixmapReady( const QPixmap & ) const;
    */
};



/******************** <DEPRECATED> ************************/

/**
  The various places a decoration can be shown at.
  Note that a particular view only implements a subset of those available here.
*/
enum Position {
  // "Standard" positions (for all views)
  Panel       =     1,
  Top         =     2,
  Left        =     4,
  Bottom      =     8,
  Right       =    16,
  // Agenda view - Around the top label (T=top, L=left, B=bottom, R=right)
  DayTopT     =    32,
  DayTopL     =    64,
  DayTopB     =   128,
  DayTopR     =   256,
  // Agenda view - Under the event list
  DayBottomC  =   512,
  // Month view - At the bottom of each cell, near the day label
  CellBottom  =  1024
};
Q_DECLARE_FLAGS(Positions, Position)
Q_DECLARE_OPERATORS_FOR_FLAGS(Positions)

/**
  @class Element

  @brief Class for calendar decoration elements

  It provides entities like texts and pictures for a given date.
  Implementations can implement all functions or only a subset.
 */
class Element
{
  public:
    Element() {}
    virtual ~Element() {}

    virtual QFlags<Positions> availablePositions() const = 0;

    /**
      This function returns the positions the decoration element can accept
      (by default, all available positions)
     */
    QFlags<Positions> acceptablePositions() const
      { return availablePositions(); }

    /**
      Returns the decoration element's current position.
     */
    Position position() const { return mPosition; }

    /**
      Return a short text for a given date, usually only a few words.
     */
    virtual QString shortText( const QDate & ) const { return QString(); }
    /**
      Return a long text for a given date. This text can be of any length, but
      usually it will have one or a few paragraphs.
     */
    virtual QString longText( const QDate & ) const { return QString(); }

    /**
      Return a small pixmap. The size should be something like 30x30 pixels.
     */
    virtual QPixmap smallPixmap( const QDate &) const { return QPixmap(); }
    /**
      Return a large pixmap. The size should be something like 300x300 pixels.
     */
    virtual QPixmap largePixmap( const QDate &) const { return QPixmap(); }

    /**
      The widget to be shown for a given @param date,
      with @param parent as parent widget.
     */
    virtual QWidget *widget( QWidget *parent, const QDate &date ) const
      { Q_UNUSED(parent); Q_UNUSED(date); return 0; }

  protected:
    Position mPosition;

  public Q_SLOTS:
    /**
      Slot to use to allow the widget to adapt to a @param newPosition
      when it changed.
     */
    void widgetPositionChanged( const Position &newPosition )
      { Q_UNUSED(newPosition); }

};

/**
  @class AgendaElement

  @brief Class for calendar decoration elements in the agenda view
 */
class AgendaElement : public Element
{
  public:
    AgendaElement() {}
    virtual ~AgendaElement() {}

    QFlags<Positions> availablePositions() const {
      return Panel|Top|Left|Bottom|Right|
             DayTopT|DayTopL|DayTopB|DayTopR|DayBottomC; }

};

/**
  @class MonthElement

  @brief Class for calendar decoration elements in the month view
 */
class MonthElement : public Element
{
  public:
    MonthElement() {}
    virtual ~MonthElement() {}

    QFlags<Positions> availablePositions() const {
      return Panel|Top|Left|Bottom|Right|CellBottom; }

};

/******************** </DEPRECATED> ************************/

/**
  @class Decoration

  @brief This class provides the interface for a date dependent decoration.

  The decoration is made of various decoration elements,
  which show a defined text/picture/widget for a given date.
 */
class Decoration : public Plugin
{
  public:
    static int interfaceVersion() { return 2; }
    static QString serviceType() { return QLatin1String("Calendar/Decoration"); }

    typedef QList<Decoration*> List;

    Decoration() {}
    virtual ~Decoration() {}

    QList<FlexibleElement*> elements() { return flexibleElements; }
/******************** <DEPRECATED> ************************/
    /**
      Returns the various decoration elements of this decoration
      for the agenda view.
     */
    QList<AgendaElement*> agenda() { return agendaElements; }
    /**
      Returns the various decoration elements of this decoration
      for the month view.
     */
    QList<MonthElement*> month() { return monthElements; }
/******************** </DEPRECATED> ************************/

  protected:
/******************** <DEPRECATED> ************************/
    QList<AgendaElement*> agendaElements;
    QList<MonthElement*> monthElements;
/******************** </DEPRECATED> ************************/
    QList<FlexibleElement*> flexibleElements;

};


class DecorationFactory : public PluginFactory
{
  public:
    virtual Decoration *create() = 0;
};

}

}

#endif
