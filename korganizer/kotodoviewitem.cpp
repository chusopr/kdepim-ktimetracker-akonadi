/*
    This file is part of KOrganizer.

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

#include <qpainter.h>

#include <klocale.h>
#include <kdebug.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "kotodoviewitem.h"
#include "kotodoview.h"
#include "koprefs.h"

KOTodoViewItem::KOTodoViewItem( QListView *parent, Todo *todo, KOTodoView *kotodo)
  : QCheckListItem( parent , "", CheckBox ), mTodo( todo ), mTodoView( kotodo )
{
  construct();
}

KOTodoViewItem::KOTodoViewItem( KOTodoViewItem *parent, Todo *todo, KOTodoView *kotodo )
  : QCheckListItem( parent, "", CheckBox ), mTodo( todo ), mTodoView( kotodo )
{
  construct();
}

QString KOTodoViewItem::key(int column,bool) const
{
  QMap<int,QString>::ConstIterator it = mKeyMap.find(column);
  if (it == mKeyMap.end()) {
    return text(column);
  } else {
    return *it;
  }
}

void KOTodoViewItem::setSortKey(int column,const QString &key)
{
  mKeyMap.insert(column,key);
}

#if QT_VERSION >= 300
void KOTodoViewItem::paintBranches(QPainter *p,const QColorGroup & cg,int w,
                                   int y,int h)
{
  QListViewItem::paintBranches(p,cg,w,y,h);
}
#else
#endif

void KOTodoViewItem::construct()
{
  m_init = true;
  QString keyd = "==";
  QString keyt = "==";

  setOn(mTodo->isCompleted());
  setText(0,mTodo->summary());
  static const QPixmap recurPxmp = KOGlobals::self()->smallIcon("recur");
  if (mTodo->doesRecur()) {
    setPixmap(1,recurPxmp);
    setSortKey(1, "1");
  }
  else setSortKey(1,"0");
  setText(2,QString::number(mTodo->priority()));
  setText(3,QString::number(mTodo->percentComplete()));
  if (mTodo->percentComplete()<100) {
    if (mTodo->isCompleted()) setSortKey(3,QString::number(999));
    else setSortKey(3,QString::number(mTodo->percentComplete()));
  }
  else {
    if (mTodo->isCompleted()) setSortKey(3,QString::number(999));
    else setSortKey(3,QString::number(99));
  }
  if (mTodo->hasDueDate()) {
    setText(4, mTodo->dtDueDateStr());
    QDate d = mTodo->dtDue().date();
    keyd.sprintf("%04d%02d%02d",d.year(),d.month(),d.day());
    setSortKey(4,keyd);
    if (mTodo->doesFloat()) {
      setText(5,"");
    }
    else {
      setText(5,mTodo->dtDueTimeStr());
      QTime t = mTodo->dtDue().time();
      keyt.sprintf("%02d%02d",t.hour(),t.minute());
      setSortKey(5,keyt);
    }
  } else {
    setText(4,"");
    setText(5,"");
  }
  setSortKey(4,keyd);
  setSortKey(5,keyt);

  QString priorityKey = QString::number( mTodo->priority() ) + keyd + keyt;
  if ( mTodo->isCompleted() ) setSortKey( 2, "1" + priorityKey );
  else setSortKey( 2, "0" + priorityKey );

  setText(6,mTodo->categoriesStr());

#if 0
  // Find sort id in description. It's the text behind the last '#' character
  // found in the description. White spaces are removed from beginning and end
  // of sort id.
  int pos = mTodo->description().findRev('#');
  if (pos < 0) {
    setText(6,"");
  } else {
    QString str = mTodo->description().mid(pos+1);
    str.stripWhiteSpace();
    setText(6,str);
  }
#endif

  m_known = false;
  m_init = false;
}

void KOTodoViewItem::stateChange(bool state)
{
  // do not change setting on startup
  if ( m_init ) return;

  kdDebug(5850) << "State changed, modified " << state << endl;
  QString keyd = "==";
  QString keyt = "==";

  Todo*oldTodo = mTodo->clone();

  if (state)
    mTodoView->emitRecurSignal( mTodo );
  else mTodo->setPercentComplete(0);
  if (isOn()!=state) {
    setOn(state);
  }

  if (mTodo->hasDueDate()) {
    setText(4, mTodo->dtDueDateStr());
    QDate d = mTodo->dtDue().date();
    keyd.sprintf("%04d%02d%02d",d.year(),d.month(),d.day());
    setSortKey(4,keyd);
    if (mTodo->doesFloat()) {
      setText(5,"");
    }
    else {
      setText(5,mTodo->dtDueTimeStr());
      QTime t = mTodo->dtDue().time();
      keyt.sprintf("%02d%02d",t.hour(),t.minute());
      setSortKey(5,keyt);
    }
  }

  QString priorityKey = QString::number( mTodo->priority() ) + keyd + keyt;
  if ( mTodo->isCompleted() ) setSortKey( 2, "1" + priorityKey );
  else setSortKey( 2, "0" + priorityKey );

  setText(3, QString::number(mTodo->percentComplete()));
  if (mTodo->percentComplete()<100) {
    if (mTodo->isCompleted()) setSortKey(3,QString::number(999));
    else setSortKey(3,QString::number(mTodo->percentComplete()));
  }
  else {
    if (mTodo->isCompleted()) setSortKey(3,QString::number(999));
    else setSortKey(3,QString::number(99));
  }
  QListViewItem *myChild = firstChild();
  KOTodoViewItem *item;
  while( myChild ) {
    item = static_cast<KOTodoViewItem*>(myChild);
    item->stateChange(state);
    myChild = myChild->nextSibling();
  }
  mTodoView->setTodoModified( oldTodo, mTodo );
  delete oldTodo;
}

bool KOTodoViewItem::isAlternate()
{
#ifndef KORG_NOLVALTERNATION
  KOTodoListView *lv = static_cast<KOTodoListView *>(listView());
  if (lv && lv->alternateBackground().isValid())
  {
    KOTodoViewItem *above = 0;
    above = dynamic_cast<KOTodoViewItem *>(itemAbove());
    m_known = above ? above->m_known : true;
    if (m_known)
    {
       m_odd = above ? !above->m_odd : false;
    }
    else
    {
       KOTodoViewItem *item;
       bool previous = true;
       if (QListViewItem::parent())
       {
          item = dynamic_cast<KOTodoViewItem *>(QListViewItem::parent());
          if (item)
             previous = item->m_odd;
          item = dynamic_cast<KOTodoViewItem *>(QListViewItem::parent()->firstChild());
       }
       else
       {
          item = dynamic_cast<KOTodoViewItem *>(lv->firstChild());
       }

       while(item)
       {
          item->m_odd = previous = !previous;
          item->m_known = true;
          item = dynamic_cast<KOTodoViewItem *>(item->nextSibling());
       }
    }
    return m_odd;
  }
  return false;
#else
  return false;
#endif
}

void KOTodoViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
#ifndef KORG_NOLVALTERNATION
  if (isAlternate())
        _cg.setColor(QColorGroup::Base, static_cast< KOTodoListView* >(listView())->alternateBackground());
  if (mTodo->hasDueDate()) {
    if (mTodo->dtDue().date()==QDate::currentDate() &&
        !mTodo->isCompleted()) {
      _cg.setColor(QColorGroup::Base, KOPrefs::instance()->mTodoDueTodayColor);
      _cg.setColor(QColorGroup::Text, getTextColor(KOPrefs::instance()->mTodoDueTodayColor));
    }
    if (mTodo->dtDue().date() < QDate::currentDate() &&
        !mTodo->isCompleted()) {
      _cg.setColor(QColorGroup::Base, KOPrefs::instance()->mTodoOverdueColor);
      _cg.setColor(QColorGroup::Text, getTextColor(KOPrefs::instance()->mTodoOverdueColor));
    }
  }
#endif

  // show the progess by a horizontal bar
  if ( column == 3 ) {
    p->save();
    int progress = (int)(( (width-6)*mTodo->percentComplete())/100.0 + 0.5);

    p->fillRect( 0, 0, width, height(), _cg.base() ); // background
    p->setPen( KGlobalSettings::textColor() );  //border
    p->setBrush( KGlobalSettings::baseColor() );  //filling
    p->drawRect( 2, 2, width-4, height()-4);
    p->fillRect( 3, 3, progress, height()-6, 
        KGlobalSettings::highlightColor() );
    p->restore();
  } else {
    QCheckListItem::paintCell(p, _cg, column, width, alignment);
  }
}
