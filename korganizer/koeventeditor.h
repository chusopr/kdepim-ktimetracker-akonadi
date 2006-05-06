/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOEVENTEDITOR_H
#define KOEVENTEDITOR_H

#include "koincidenceeditor.h"

class KOEditorGeneralEvent;
class KOEditorRecurrence;
class KOEditorFreeBusy;

class SaveTemplateDialog;

class KOEditorFreeBusy;

namespace KCal {
class Calendar;
class Event;
}
using namespace KCal;

/**
  This class provides a dialog for editing an event.
*/
class KOEventEditor : public KOIncidenceEditor
{
    Q_OBJECT
  public:
    /**
      Construct new event editor.
    */
    KOEventEditor( Calendar *calendar, QWidget *parent );
    virtual ~KOEventEditor(void);

    void init();
    /** This event has been modified externally */
    void modified (int change=0);
    void reload();

    /**
      Clear eventwin for new event, and preset the dates and times with hint
    */
    void newEvent( const QDateTime &from, const QDateTime &to, bool allDay = false );
    /**
      Edit new event. Set summary and description from given text.
    */
    void newEvent( const QString & );
    /**
      Edit new event.
    */
    void newEvent( const QString &summary, const QString &description,
                   const QString &attachment );
    /**
      Edit new event.
    */
    void newEvent( const QString &summary, const QString &description,
                   const QString &attachment, const QStringList &attendees );
    /**
      Edit an existing event.
    */
    void editIncidence( Incidence * );

    /**
      Set widgets to default values
    */
    void setDefaults( const QDateTime &from, const QDateTime &to, bool allDay );

    /**
      Read event object and setup widgets accordingly. If tmpl is true, the
      event is read as template, i.e. the time and date information isn't set.
        @param event the event from which the data should be used
        @param tmpl If true, the event is treated as a template, so the currently
                    set time is preserved in the editor dialog.
    */
    void readEvent( Event *event, bool tmpl = false );
    /**
      Write event settings to event object
    */
    void writeEvent( Event * );

    QObject *typeAheadReceiver() const;

  signals:
    void focusReceivedSignal();

  protected slots:
    void loadDefaults();
    void deleteEvent();

    void slotSaveTemplate( const QString & );

  protected:
    QString type() { return "Event"; }
    void setupGeneral();
    void setupRecurrence();
    void setupFreeBusy();

    /** Check if the input is valid. */
    bool validateInput();
    /** Process user input and create or update event. Returns false if input
     * is not valid */
    bool processInput();
    void processCancel();
    int msgItemDelete();
    void loadTemplate( /*const*/ CalendarLocal& );
    QStringList& templates() const;

  private:
    Event *mEvent;

    KOEditorGeneralEvent *mGeneral;
    KOEditorRecurrence   *mRecurrence;
    KOEditorFreeBusy     *mFreeBusy;
};

#endif
