/*
    This file is part of KOrganizer.
    Copyright (c) 1997, 1998 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef _KOTODOEDITOR_H
#define _KOTODOEDITOR_H
// $Id$	

#include <libkcal/calendar.h>

#include "koeditorgeneraltodo.h"
#include "koeditordetails.h"
#include "koincidenceeditor.h"

class QDateTime;

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
    KOTodoEditor(Calendar *calendar);
    virtual ~KOTodoEditor(void);

    void init();

    /**
      Clear editor for new todo, and preset the dates and times with hint.
    */
    void newTodo(QDateTime due,Todo *relatedTodo=0,bool allDay=false);

    /** Edit an existing todo. */
    void editTodo(Todo *);

    /** Set widgets to default values */
    void setDefaults(QDateTime due,Todo *relatedTodo,bool allDay);
    /** Read event object and setup widgets accordingly */
    void readTodo(Todo *);
    /** Write event settings to event object */
    void writeTodo(Todo *);

    /** Check if the input is valid. */
    bool validateInput();
    /** Process user input and create or update event. Returns false if input
     * is not valid */
    bool processInput();

  signals:
    void todoChanged(Todo *);
    void todoAdded(Todo *);
    void todoToBeDeleted(Todo *);
    void todoDeleted();

  protected slots:
    void slotDefault();
    void slotUser1();
  
  protected:
    void setupGeneral();

  private:  
    Todo *mTodo;
    
    Todo *mRelatedTodo;

    KOEditorGeneralTodo *mGeneral;
};

#endif
