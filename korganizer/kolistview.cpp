/*
    This file is part of KOrganizer.
    Copyright (c) 1999 Preston Brown
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlistview.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qcursor.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kglobal.h>

#include <libkcal/calendar.h>

#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif
#include "koglobals.h"
#include "koincidencetooltip.h"

#include "kolistview.h"
#include "kolistview.moc"


KOListViewToolTip::KOListViewToolTip( QWidget* parent,
                                      KListView* lv )
  :QToolTip(parent)
{
  eventlist=lv;
}

void KOListViewToolTip::maybeTip( const QPoint & pos)
{
  QRect r;
  QListViewItem *it = eventlist->itemAt(pos);
  KOListViewItem *i = static_cast<KOListViewItem*>(it);

  if( i && KOPrefs::instance()->mEnableToolTips ) {
    /* Calculate the rectangle. */
    r=eventlist->itemRect( it );
    /* Show the tip */
    QString tipText;
    ToolTipVisitor v;
    if (v.act(i->data(), &tipText, true)) {
      tip(r, tipText);
    }
  }

}

ListItemVisitor::ListItemVisitor(KOListViewItem *item)
{
  mItem = item;
}

ListItemVisitor::~ListItemVisitor()
{
}

bool ListItemVisitor::visit(Event *e)
{
  mItem->setText(0,e->summary());
  if ( e->isAlarmEnabled() ) {
    static const QPixmap alarmPxmp = KOGlobals::self()->smallIcon("bell");
    mItem->setPixmap(1,alarmPxmp);
  }

  if ( e->doesRecur() ) {
    static const QPixmap recurPxmp = KOGlobals::self()->smallIcon("recur");
    mItem->setPixmap(2,recurPxmp);
  }

  mItem->setText(3,e->dtStartDateStr());
  mItem->setText(4,e->dtStartTimeStr());
  mItem->setText(5,e->dtEndDateStr());
  mItem->setText(6,e->dtEndTimeStr());
  mItem->setText(7,"---");
  mItem->setText(8,"---");
  mItem->setText(9,e->categoriesStr());

  QString key = e->dtStart().toString(Qt::ISODate);
  mItem->setSortKey(1,key);

  key = e->dtEnd().toString(Qt::ISODate);
  mItem->setSortKey(3,key);

  return true;
}

bool ListItemVisitor::visit(Todo *t)
{
  static const QPixmap todoPxmp = KOGlobals::self()->smallIcon("checkedbox");
  mItem->setPixmap(0,todoPxmp);
  mItem->setText(0,t->summary());
  if ( t->isAlarmEnabled() ) {
    static const QPixmap alarmPxmp = KOGlobals::self()->smallIcon("bell");
    mItem->setPixmap(1,alarmPxmp);
  }
  
  if ( t->doesRecur() ) {
    static const QPixmap recurPxmp = KOGlobals::self()->smallIcon("recur");
    mItem->setPixmap(2,recurPxmp);
  }
  
  mItem->setText(3,"---");
  mItem->setText(4,"---");
  mItem->setText(5,"---");
  mItem->setText(6,"---");
  
  if (t->hasDueDate()) {
    mItem->setText(7,t->dtDueDateStr());
    if (t->doesFloat()) {
      mItem->setText(8,"---");
    } else {
      mItem->setText(8,t->dtDueTimeStr());
    }
  } else {
    mItem->setText(7,"---");
    mItem->setText(8,"---");
  }
  mItem->setText(9,t->categoriesStr());

  QString key = t->dtDue().toString(Qt::ISODate);
  mItem->setSortKey(7,key);

  return true;
}

bool ListItemVisitor::visit(Journal *t)
{
  static const QPixmap jrnalPxmp = KOGlobals::self()->smallIcon("journal");
  mItem->setPixmap(0,jrnalPxmp);
  mItem->setText( 0, t->description() );
  mItem->setText( 3, t->dtStartDateStr() );

  return true;
}

KOListView::KOListView( Calendar *calendar, QWidget *parent,
                        const char *name)
  : KOEventView(calendar, parent, name)
{
  mActiveItem = 0;

  mListView = new KListView(this);
  mListView->addColumn(i18n("Summary"));
  mListView->addColumn(i18n("Alarm")); // alarm set?
  mListView->addColumn(i18n("Recurs")); // recurs?
  mListView->addColumn(i18n("Start Date"));
  mListView->setColumnAlignment(3,AlignHCenter);
  mListView->addColumn(i18n("Start Time"));
  mListView->setColumnAlignment(4,AlignHCenter);
  mListView->addColumn(i18n("End Date"));
  mListView->setColumnAlignment(5,AlignHCenter);
  mListView->addColumn(i18n("End Time"));
  mListView->setColumnAlignment(6,AlignHCenter);
  mListView->addColumn(i18n("Due Date"));
  mListView->setColumnAlignment(7,AlignHCenter);
  mListView->addColumn(i18n("Due Time"));
  mListView->setColumnAlignment(8,AlignHCenter);
  mListView->addColumn(i18n("Categories"));
  mListView->setColumnAlignment(9,AlignHCenter);

  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->addWidget(mListView);

  mPopupMenu = eventPopup();
/*
  mPopupMenu->insertSeparator();
  mPopupMenu->insertItem(i18n("Show Dates"), this,
                      SLOT(showDates()));
  mPopupMenu->insertItem(i18n("Hide Dates"), this,
                      SLOT(hideDates()));
*/

  QObject::connect( mListView, SIGNAL( doubleClicked( QListViewItem * ) ),
                    SLOT( defaultItemAction( QListViewItem * ) ) );
  QObject::connect( mListView, SIGNAL( returnPressed( QListViewItem * ) ),
                    SLOT( defaultItemAction( QListViewItem * ) ) );
  QObject::connect( mListView, SIGNAL( rightButtonClicked ( QListViewItem *,
                                                            const QPoint &,
                                                            int ) ),
                    SLOT( popupMenu( QListViewItem *, const QPoint &, int ) ) );
  QObject::connect( mListView, SIGNAL( selectionChanged() ),
                    SLOT( processSelectionChange() ) );

//  setMinimumSize(100,100);
  mListView->restoreLayout(KOGlobals::self()->config(),"KOListView Layout");

  new KOListViewToolTip( mListView->viewport(), mListView );
}

KOListView::~KOListView()
{
  delete mPopupMenu;
}

int KOListView::maxDatesHint()
{
  return 0;
}

int KOListView::currentDateCount()
{
  return 0;
}

Incidence::List KOListView::selectedIncidences()
{
  Incidence::List eventList;

  QListViewItem *item = mListView->selectedItem();
  if (item) eventList.append(((KOListViewItem *)item)->data());

  return eventList;
}

DateList KOListView::selectedDates()
{
  DateList eventList;
  return eventList;
}

void KOListView::showDates(bool show)
{
  // Shouldn't we set it to a value greater 0? When showDates is called with
  // show == true at first, then the columnwidths are set to zero.
  static int oldColWidth1 = 0;
  static int oldColWidth3 = 0;

  if (!show) {
    oldColWidth1 = mListView->columnWidth(1);
    oldColWidth3 = mListView->columnWidth(3);
    mListView->setColumnWidth(1, 0);
    mListView->setColumnWidth(3, 0);
  } else {
    mListView->setColumnWidth(1, oldColWidth1);
    mListView->setColumnWidth(3, oldColWidth3);
  }
  mListView->repaint();
}

void KOListView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                               const QDate &td)
{
#ifndef KORG_NOPRINTER
  calPrinter->preview(CalPrinter::Day, fd, td);
#endif
}

void KOListView::showDates()
{
  showDates(true);
}

void KOListView::hideDates()
{
  showDates(false);
}

void KOListView::updateView()
{
  kdDebug(5850) << "KOListView::updateView() does nothing" << endl;
}

void KOListView::showDates(const QDate &start, const QDate &end)
{
  clear();

  QDate date = start;
  while( date <= end ) {
    addIncidences( calendar()->incidences(date) );
    date = date.addDays( 1 );
  }

  emit incidenceSelected( 0 );
}

void KOListView::addIncidences( const Incidence::List &incidenceList )
{
  Incidence::List::ConstIterator it;
  for( it = incidenceList.begin(); it != incidenceList.end(); ++it ) {
    addIncidence( *it );
  }
}

void KOListView::addIncidence(Incidence *incidence)
{
  if ( mUidDict.find( incidence->uid() ) ) return;
  
  mUidDict.insert( incidence->uid(), incidence );

  KOListViewItem *item = new KOListViewItem( incidence, mListView );
  ListItemVisitor v(item);
  if (incidence->accept(v)) return;
  else delete item;
}

void KOListView::showIncidences( const Incidence::List &incidenceList )
{
  clear();

  addIncidences( incidenceList );

  // After new creation of list view no events are selected.
  emit incidenceSelected( 0 );
}

void KOListView::changeIncidenceDisplay(Incidence *incidence, int action)
{
  KOListViewItem *item;

  switch(action) {
    case KOGlobals::INCIDENCEADDED:
      addIncidence( incidence );
      break;
    case KOGlobals::INCIDENCEEDITED:
      item = getItemForEvent(incidence);
      if (item) {
        delete item;
        mUidDict.remove( incidence->uid() );
        addIncidence( incidence );
      }
      break;
    case KOGlobals::INCIDENCEDELETED:
      item = getItemForEvent(incidence);
      if (item) {
        delete item;
      }
      break;
    default:
      kdDebug(5850) << "KOListView::changeIncidenceDisplay(): Illegal action " << action << endl;
  }
}

KOListViewItem *KOListView::getItemForEvent(Incidence *incidence)
{
  KOListViewItem *item = (KOListViewItem *)mListView->firstChild();
  while (item) {
//    kdDebug(5850) << "Item " << item->text(0) << " found" << endl;
    if (item->data() == incidence) return item;
    item = (KOListViewItem *)item->nextSibling();
  }
  return 0;
}

void KOListView::defaultItemAction(QListViewItem *i)
{
  KOListViewItem *item = static_cast<KOListViewItem *>( i );
  if ( item ) defaultAction( item->data() );
}

void KOListView::popupMenu(QListViewItem *item,const QPoint &,int)
{
  mActiveItem = (KOListViewItem *)item;
  if (mActiveItem) {
    Incidence *incidence = mActiveItem->data();
    mPopupMenu->showIncidencePopup(incidence);
    // TODO_RK: Use the todo RMB menu from the todo view for todo items!
  }
  else {
    showNewEventPopup();
  }
}

void KOListView::readSettings(KConfig *config)
{
  mListView->restoreLayout(config,"KOListView Layout");
}

void KOListView::writeSettings(KConfig *config)
{
  mListView->saveLayout(config,"KOListView Layout");
}

void KOListView::processSelectionChange()
{
  kdDebug(5850) << "KOListView::processSelectionChange()" << endl;

  KOListViewItem *item =
    static_cast<KOListViewItem *>( mListView->selectedItem() );

  if ( !item ) {
    emit incidenceSelected( 0 );
  } else {
    emit incidenceSelected( item->data() );
  }
}

void KOListView::clearSelection()
{
  mListView->selectAll( false );
}

void KOListView::clear()
{
  mListView->clear();
  mUidDict.clear();
}
