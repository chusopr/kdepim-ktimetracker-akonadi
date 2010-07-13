/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef KODIALOGMANAGER_H
#define KODIALOGMANAGER_H

#include "koeventview.h"

#include <QList>
#include <QPointer>

class ArchiveDialog;
class CalendarView;
class CategoryEditDialog;
class FilterEditDialog;
class KCMultiDialog;
class SearchDialog;

namespace Akonadi {
  class Item;
}

namespace IncidenceEditors {
  class EventEditor;
  class IncidenceEditor;
  class JournalEditor;
  class TodoEditor;
}

namespace KCal {
  class CalFilter;
}

/**
  This class manages the dialogs used by the calendar view. It owns the objects
  and handles creation and selection.
*/
class KODialogManager : public QObject
{
  Q_OBJECT
  public:
    explicit KODialogManager( CalendarView * );
    virtual ~KODialogManager();

    /** Get the appropriate editor for the given incidence */
    IncidenceEditors::IncidenceEditor *getEditor( const Akonadi::Item& item );

    /** Get an editor dialog for an Event. */
    IncidenceEditors::EventEditor *getEventEditor();

    /** Get an editor dialog for a Todo. */
    IncidenceEditors::TodoEditor *getTodoEditor();

    /** Get an editor dialog for a Journal. */
    IncidenceEditors::JournalEditor *getJournalEditor();

    void connectEditor( IncidenceEditors::IncidenceEditor *editor );

    void updateSearchDialog();

    void connectTypeAhead( IncidenceEditors::EventEditor *editor, KOEventView *view );

  public slots:
    void showOptionsDialog();
    void showCategoryEditDialog();
    void showSearchDialog();
    void showArchiveDialog();
    void showFilterEditDialog( QList<KCalCore::CalFilter*> *filters );

  private:
    void createCategoryEditor();
    class DialogManagerVisitor;
    class EditorDialogVisitor;

    CalendarView *mMainView;
    KCMultiDialog *mOptionsDialog;
    QPointer<CategoryEditDialog> mCategoryEditDialog;
    SearchDialog *mSearchDialog;
    ArchiveDialog *mArchiveDialog;
    FilterEditDialog *mFilterEditDialog;
};

#endif
