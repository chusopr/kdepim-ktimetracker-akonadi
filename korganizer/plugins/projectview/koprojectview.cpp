/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
*/

#include <qlayout.h>
#include <q3header.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <q3listbox.h>
#include <q3popupmenu.h>
#include <q3strlist.h>
#include <q3listview.h>
//Added by qt3to4:
#include <QList>
#include <QFrame>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QVBoxLayout>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <libkcal/vcaldrag.h>

#include "KGantt.h"

#include "koprojectview.h"

using namespace KOrg;

KOProjectViewItem::KOProjectViewItem(Todo *event,KGanttItem* parentTask,
                                     const QString& text,
                                     const QDateTime& start,
                                     const QDateTime& end) :
  KGanttItem(parentTask,text,start,end)
{
  mEvent = event;
}

KOProjectViewItem::~KOProjectViewItem()
{
}

Todo *KOProjectViewItem::event()
{
  return mEvent;
}


KOProjectView::KOProjectView(Calendar *calendar,QWidget* parent,
                             const char* name) :
  KOrg::BaseView(calendar,parent,name)
{
  QBoxLayout *topLayout = new QVBoxLayout(this);

  QBoxLayout *topBar = new QHBoxLayout;
  topLayout->addLayout(topBar);

  QLabel *title = new QLabel(i18n("Project View"),this);
  title->setFrameStyle(QFrame::Panel|QFrame::Raised);
  topBar->addWidget(title,1);

  QPushButton *zoomIn = new QPushButton(i18n("Zoom In"),this);
  topBar->addWidget(zoomIn,0);
  connect(zoomIn,SIGNAL(clicked()),SLOT(zoomIn()));

  QPushButton *zoomOut = new QPushButton(i18n("Zoom Out"),this);
  topBar->addWidget(zoomOut,0);
  connect(zoomOut,SIGNAL(clicked()),SLOT(zoomOut()));

  QPushButton *menuButton = new QPushButton(i18n("Select Mode"),this);
  topBar->addWidget(menuButton,0);
  connect(menuButton,SIGNAL(clicked()),SLOT(showModeMenu()));

  createMainTask();

  mGantt = new KGantt(mMainTask,this);
  topLayout->addWidget(mGantt,1);

#if 0
  mGantt->addHoliday(2000, 10, 3);
  mGantt->addHoliday(2001, 10, 3);
  mGantt->addHoliday(2000, 12, 24);

  for(int i=1; i<7; ++i)
    mGantt->addHoliday(2001, 1, i);
#endif
}

void KOProjectView::createMainTask()
{
  mMainTask = new KGanttItem(0,i18n("main task"),
                         QDateTime::currentDateTime(),
                         QDateTime::currentDateTime());
  mMainTask->setMode(KGanttItem::Rubberband);
  mMainTask->setStyle(KGanttItem::DrawBorder | KGanttItem::DrawText |
                      KGanttItem::DrawHandle);
}

void KOProjectView::readSettings()
{
  kDebug(5850) << "KOProjectView::readSettings()" << endl;

  //KConfig *config = KGlobal::config();
  KConfig config( "korganizerrc", true, false); // Open read-only, no kdeglobals
  config.setGroup("Views");

  QList<int> sizes = config.readEntry("Separator ProjectView",QList<int>());
  if (sizes.count() == 2) {
    mGantt->splitter()->setSizes(sizes);
  }
}

void KOProjectView::writeSettings(KConfig *config)
{
  kDebug(5850) << "KOProjectView::writeSettings()" << endl;

  config->setGroup("Views");

  QList<int> list = mGantt->splitter()->sizes();
  config->writeEntry("Separator ProjectView",list);
}


void KOProjectView::updateView()
{
  kDebug(5850) << "KOProjectView::updateView()" << endl;

  // Clear Gantt view
  QList<KGanttItem*> subs = mMainTask->getSubItems();
  QList<KGanttItem*>::iterator it;
  for ( it = subs.begin(); it != subs.end(); ++it ) {
  while (!subs.isEmpty())
    delete subs.takeFirst();

#if 0
  KGanttItem* t1 = new KGanttItem(mGantt->getMainTask(), "task 1, no subtasks",
                             QDateTime::currentDateTime().addDays(10),
                             QDateTime::currentDateTime().addDays(20) );

  KGanttItem* t2 = new KGanttItem(mGantt->getMainTask(), "task 2, subtasks, no rubberband",
                             QDateTime(QDate(2000,10,1)),
                             QDateTime(QDate(2000,10,31)) );
#endif

  Todo::List todoList = calendar()->todos();

/*
  kDebug(5850) << "KOProjectView::updateView(): Todo List:" << endl;
  Event *t;
  for(t = todoList.first(); t; t = todoList.next()) {
    kDebug(5850) << "  " << t->getSummary() << endl;

    if (t->getRelatedTo()) {
      kDebug(5850) << "      (related to " << t->getRelatedTo()->getSummary() << ")" << endl;
    }

    QPtrList<Event> l = t->getRelations();
    Event *c;
    for(c=l.first();c;c=l.next()) {
      kDebug(5850) << "    - relation: " << c->getSummary() << endl;
    }
  }
*/

  // Put for each Event a KOProjectViewItem in the list view. Don't rely on a
  // specific order of events. That means that we have to generate parent items
  // recursively for proper hierarchical display of Todos.
  mTodoMap.clear();
  Todo::List::ConstIterator it;
  for( it = todoList.begin(); it != todoList.end(); ++it ) {
    if ( !mTodoMap.contains( *it ) ) {
      insertTodoItem( *it );
    }
  }
}

QMap<Todo *,KGanttItem *>::ConstIterator
    KOProjectView::insertTodoItem(Todo *todo)
{
//  kDebug(5850) << "KOProjectView::insertTodoItem(): " << todo->getSummary() << endl;
  Todo *relatedTodo = dynamic_cast<Todo *>(todo->relatedTo());
  if (relatedTodo) {
//    kDebug(5850) << "  has Related" << endl;
    QMap<Todo *,KGanttItem *>::ConstIterator itemIterator;
    itemIterator = mTodoMap.find(relatedTodo);
    if (itemIterator == mTodoMap.end()) {
//      kDebug(5850) << "    related not yet in list" << endl;
      itemIterator = insertTodoItem (relatedTodo);
    }
    KGanttItem *task = createTask(*itemIterator,todo);
    return mTodoMap.insert(todo,task);
  } else {
//    kDebug(5850) << "  no Related" << endl;
    KGanttItem *task = createTask(mMainTask,todo);
    return mTodoMap.insert(todo,task);
  }
}

KGanttItem *KOProjectView::createTask(KGanttItem *parent,Todo *todo)
{
  QDateTime startDt;
  QDateTime endDt;

  if (todo->hasStartDate() && !todo->hasDueDate()) {
    // start date but no due date
    startDt = todo->dtStart();
    endDt = QDateTime::currentDateTime();
  } else if (!todo->hasStartDate() && todo->hasDueDate()) {
    // due date but no start date
    startDt = todo->dtDue();
    endDt = todo->dtDue();
  } else if (!todo->hasStartDate() || !todo->hasDueDate()) {
    startDt = QDateTime::currentDateTime();
    endDt = QDateTime::currentDateTime();
  } else {
    startDt = todo->dtStart();
    endDt = todo->dtDue();
  }

  KGanttItem *task = new KOProjectViewItem(todo,parent,todo->summary(),startDt,
                                       endDt);
  connect(task,SIGNAL(changed(KGanttItem*, KGanttItem::Change)),
          SLOT(taskChanged(KGanttItem*,KGanttItem::Change)));
  if (todo->relations().count() > 0) {
    task->setBrush(QBrush(QColor(240,240,240), Qt::Dense4Pattern));
  }

  return task;
}

void KOProjectView::updateConfig()
{
  // FIXME: to be implemented.
}

Incidence::List KOProjectView::selectedIncidences()
{
  Incidence::List selected;

/*
  KOProjectViewItem *item = (KOProjectViewItem *)(mTodoListView->selectedItem());
  if (item) selected.append(item->event());
*/

  return selected;
}

DateList KOProjectView::selectedDates()
{
  DateList selected;
  return selected;
}

void KOProjectView::changeIncidenceDisplay(Incidence *, int)
{
  updateView();
}

void KOProjectView::showDates(const QDate &, const QDate &)
{
  updateView();
}

void KOProjectView::showIncidences( const Incidence::List & )
{
  kDebug(5850) << "KOProjectView::showIncidences( const Incidence::List & ): not yet implemented" << endl;
}

#if 0
void KOProjectView::editItem(Q3ListViewItem *item)
{
  emit editIncidenceSignal(((KOProjectViewItem *)item)->event());
}

void KOProjectView::showItem(Q3ListViewItem *item)
{
  emit showIncidenceSignal(((KOProjectViewItem *)item)->event());
}

void KOProjectView::popupMenu(Q3ListViewItem *item,const QPoint &,int)
{
  mActiveItem = (KOProjectViewItem *)item;
  if (item) mItemPopupMenu->popup(QCursor::pos());
  else mPopupMenu->popup(QCursor::pos());
}

void KOProjectView::newTodo()
{
  emit newTodoSignal();
}

void KOProjectView::newSubTodo()
{
  if (mActiveItem) {
    emit newSubTodoSignal(mActiveItem->event());
  }
}

void KOProjectView::itemClicked(Q3ListViewItem *item)
{
  if (!item) return;

  KOProjectViewItem *todoItem = (KOProjectViewItem *)item;
  int completed = todoItem->event()->isCompleted();  // Completed or not?

  if (todoItem->isOn()) {
    if (!completed) {
      todoItem->event()->setCompleted(true);
    }
  } else {
    if (completed) {
      todoItem->event()->setCompleted(false);
    }
  }
}
#endif

void KOProjectView::showModeMenu()
{
  mGantt->menu()->popup(QCursor::pos());
}

void KOProjectView::taskChanged(KGanttItem *task,KGanttItem::Change change)
{
  if (task == mMainTask) return;

  KOProjectViewItem *item = (KOProjectViewItem *)task;

  if (change == KGanttItem::StartChanged) {
    item->event()->setDtStart(task->getStart());
  } else if (change == KGanttItem::EndChanged) {
    item->event()->setDtDue(task->getEnd());
  }
}

void KOProjectView::zoomIn()
{
  mGantt->zoom(2);
}

void KOProjectView::zoomOut()
{
  mGantt->zoom(0.5);
}

#include "koprojectview.moc"
