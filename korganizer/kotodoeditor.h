/*
  This file is part of KOrganizer.

  Copyright (c) 1997,1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KOTODOEDITOR_H
#define KOTODOEDITOR_H

#include "koincidenceeditor.h"
#include <kcal/todo.h>

class QDateTime;
class KOEditorGeneralTodo;
class KOEditorRecurrence;
class KOEditorRecurrenceDialog;

/**
  This class provides a dialog for editing a Todo.
*/
class KOTodoEditor : public KOIncidenceEditor
{
  Q_OBJECT
  public:
    /**
      Constructs a new todo editor.
    */
    KOTodoEditor( Calendar *calendar, QWidget *parent );
    virtual ~KOTodoEditor();

    void init();

    void reload();

    /**
      Edit new todo.
      Use setter methods to set appropriate default values, as needed.
    */
    void newTodo();

    /**
      Sets the given summary and description. If description is empty and the
      summary contains multiple lines, the summary will be used as description
      and only the first line of summary will be used as the summary.
    */
    void setTexts( const QString &summary,
                   const QString &description = QString(),
                   bool richDescription = false );

    /** Edit an existing todo. */
    void editIncidence( Incidence *incidence, const QDate &date, Calendar *calendar );

    /** Set widgets to default values */
    void setDates( const QDateTime &due, bool allDay = true, Todo *relatedTodo = 0 );

   /**
      Read todo object and setup widgets accordingly. If tmpl is true, the
      todo is read as template, i.e. the time and date information isn't set.

      @param todo the todo from which the data should be used
      @param tmpl If true, the todo is treated as a template, so the currently
      set time is preserved in the editor dialog.
    */
    void readTodo( Todo *todo, const QDate &date, bool tmpl = false );

    /** Write To-do settings to todo object */
    void writeTodo( Todo *todo );

    /** Check if the input is valid. */
    bool validateInput();

    /**
      Process user input and create or update event.
      Returns false if input is not valid.
    */
    bool processInput();

    /** This todo has been modified externally */
    void modified();

  protected slots:
    void loadDefaults();
    void deleteTodo();

    void slotSaveTemplate( const QString & );
    void updateRecurrenceSummary();

  protected:
    void loadTemplate( CalendarLocal & );
    QStringList &templates() const;
    QString type() { return "Todo"; }
    void setupGeneral();
    void setupRecurrence();
    int msgItemDelete();

  private:
    Todo *mTodo;
    Calendar *mCalendar;

    Todo *mRelatedTodo;

    KOEditorGeneralTodo *mGeneral;
    KOEditorRecurrenceDialog *mRecurrenceDialog;
    KOEditorRecurrence *mRecurrence;
};

#endif
