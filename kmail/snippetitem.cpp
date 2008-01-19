/*
 *  File : snippetitem.cpp
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */
#include "snippetitem.h"

#include <kaction.h>

#include <qstring.h>

SnippetItem::SnippetItem( QTreeWidget *parent, const QString &name, const QString &text )
 : QTreeWidgetItem( parent, QStringList( name ) ),
   action( 0 )
{
  strName = name;
  strText = text;
  iParent = -1;
}

SnippetItem::SnippetItem( QTreeWidgetItem *parent, const QString &name, const QString &text )
 : QTreeWidgetItem( parent, QStringList( name ) ),
   action( 0 )
{
  strName = name;
  strText = text;
  iParent = ((SnippetGroup *)parent)->getId();
}

SnippetItem::~SnippetItem()
{
  if ( action ) {
    delete action;
  }
}

QString SnippetItem::getName() const
{
  return strName;
}

QString SnippetItem::getText() const
{
  return strText;
}

void SnippetItem::setText(const QString &text)
{
  strText = text;
}

void SnippetItem::setName(const QString &name)
{
  strName = name;
}

void SnippetItem::resetParent()
{
  SnippetGroup * group = dynamic_cast<SnippetGroup*>(parent());
  if (group)
    iParent = group->getId();
}

KAction* SnippetItem::getAction() const
{
  return action;
}

void SnippetItem::setAction(KAction * anAction)
{
  action = anAction;
}

void SnippetItem::slotExecute()
{
  emit execute( this );
}


SnippetItem * SnippetItem::findItemByName( const QString &name, const QList<SnippetItem *> &list)
{
  foreach ( SnippetItem *const item, list ) {
    if (item->getName() == name)
        return item;
  }
  return NULL;
}

SnippetGroup * SnippetItem::findGroupById(int id, const QList<SnippetItem *> &list)
{
  foreach ( SnippetItem *const item, list ) {
    SnippetGroup * group = dynamic_cast<SnippetGroup*>(item);
    if (group && group->getId() == id)
        return group;
  }
  return NULL;
}

int SnippetGroup::iMaxId = 1;

SnippetGroup::SnippetGroup( QTreeWidget *parent, const QString &name, int id )
 : SnippetItem( parent, name, "GROUP" )
{
  if (id > 0) {
    iId = id;
    if (id >= iMaxId)
      iMaxId = id+1;
  } else {
    iId = iMaxId;
    iMaxId++;
  }
}

SnippetGroup::~SnippetGroup()
{
}

void SnippetGroup::setId(int id)
{
    iId = id; 
    if (iId >= iMaxId)
        iMaxId = iId+1;
}

#include "snippetitem.moc"
