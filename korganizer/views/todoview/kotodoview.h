/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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
#ifndef KOTODOVIEW_H
#define KOTODOVIEW_H

#include "korganizer/baseview.h"

#include <kcal/todo.h>

#include <QDate>

class KLineEdit;

namespace KCal {
  class Incidence;
  class Calendar;
}
using namespace KCal;
using namespace KOrg;

class QMenu;
class QContextMenuEvent;
class QItemSelection;
class QTreeView;
class QModelIndex;
class QSortFilterProxyModel;

class KOTodoModel;
class KOTodoCategoriesDelegate;
class KOTodoViewQuickSearch;

class KOTodoView : public BaseView
{
  Q_OBJECT

  public:
    KOTodoView( Calendar *cal, QWidget *parent );
    ~KOTodoView();

    virtual void setCalendar( Calendar *cal );

    virtual Incidence::List selectedIncidences();
    virtual DateList selectedDates();
    virtual int currentDateCount() { return 0; }

    void setDocumentId( const QString & ) {}

    void saveLayout( KConfig *config, const QString &group ) const;
    void restoreLayout( KConfig *config, const QString &group );

  public Q_SLOTS:
    virtual void setIncidenceChanger( IncidenceChangerBase *changer );
    virtual void showDates( const QDate &start, const QDate &end );
    virtual void showIncidences( const Incidence::List &incidenceList );
    virtual void updateView();
    void updateCategories();
    virtual void changeIncidenceDisplay( Incidence *incidence, int action );
    virtual void updateConfig();
    virtual void clearSelection();

  protected Q_SLOTS:
    void addQuickTodo();

    void contextMenu( const QPoint &pos );

    void selectionChanged( const QItemSelection &selected,
                           const QItemSelection &deselected );

    // slots used by popup-menus
    void showTodo();
    void editTodo();
    void printTodo();
    void deleteTodo();
    void newTodo();
    void newSubTodo();
    void copyTodoToDate( const QDate &date );

  private Q_SLOTS:
    void itemDoubleClicked( const QModelIndex &index );

  Q_SIGNALS:
    void purgeCompletedSignal();
    void unSubTodoSignal();
    void unAllSubTodoSignal();

  private:
    QTreeView *mView;
    KOTodoModel *mModel;
    QSortFilterProxyModel *mProxyModel;
    KOTodoCategoriesDelegate *mCategoriesDelegate;

    KOTodoViewQuickSearch *mQuickSearch;
    KLineEdit *mQuickAdd;

    QMenu *mItemPopupMenu;
    QMenu *mPopupMenu;
};

#endif /*KOTODOVIEW_H*/
