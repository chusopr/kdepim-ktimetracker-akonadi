/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "feedstree.h"

#include <kurldrag.h>
#include <klocale.h>
#include <kdebug.h>

#include <qwhatsthis.h>
#include <qheader.h>

using namespace Akregator;

FeedsTreeItem::FeedsTreeItem( QListView *parent, QString label)
    : KListViewItem( parent, label )
{

}

FeedsTreeItem::FeedsTreeItem( QListViewItem *parent, QString label)
    : KListViewItem( parent, label )

{

}

FeedsTreeItem::FeedsTreeItem(QListView *parent, QListViewItem *after, QString label)
    : KListViewItem( parent, after, label )
{

}

FeedsTreeItem::FeedsTreeItem(QListViewItem *parent, QListViewItem *after, QString label)
    : KListViewItem( parent, after, label )
{

}


void FeedsTreeItem::paintCell( QPainter * p, const QColorGroup & cg,
                                    int column, int width, int align )

{
    KListViewItem::paintCell(p,cg,column,width,align);
}

FeedsTree::FeedsTree( QWidget *parent, const char *name)
        : KListView(parent, name)
{
    setMinimumSize(150, 150);
    addColumn(i18n("Feeds"));
    setRootIsDecorated(true);
    setItemsRenameable(true);
    setDragEnabled(true);
    setAcceptDrops(true);
    setFullWidth(true);
    setSorting(-1);
    setDragAutoScroll(true);
    setDropVisualizer(true);//FIXME
    setDropHighlighter(true);//FIXME
    QWhatsThis::add(this, i18n("<h2>Feeds tree</h2>"
        "Here you can browse tree of feeds. "
        "You can also add feeds or feed groups (folders) "
        "using right-click menu, or reorganize them using "
        "drag and drop."));
}

FeedsTree::~FeedsTree()
{}

void FeedsTree::takeNode(QListViewItem* item)
{
    if (item->parent())
        item->parent()->takeItem(item);
    else
        takeItem(item);
}

void FeedsTree::insertNode(QListViewItem* parent, QListViewItem* item, QListViewItem* after)
{
    if (parent)
        parent->insertItem(item);
    else
        insertItem(item);
    if (after)
        item->moveItem(after);
}

void FeedsTree::contentsDragEnterEvent (QDragEnterEvent *e)
{
    if (e->source() != viewport())
    {
        if (KURLDrag::canDecode( e ))
        {
            e->acceptAction();
        }
        else
        {
            e->ignore();
        }
    }
    else
    {
        KListView::contentsDragEnterEvent(e);
    }
}

void FeedsTree::contentsDropEvent( QDropEvent *e )
{
    if (e->source() != viewport())
    {
        if (KURLDrag::canDecode( e ))
        {
            QListViewItem *afterme;
            QListViewItem *parent;
            findDrop( e->pos(), parent, afterme );
            KURL::List urls;
            KURLDrag::decode( e, urls );
            e->acceptAction();
            emit dropped( urls, afterme, parent);
        }
        else
        {
            e->ignore();
        }
    }
    else
    {
        KListView::contentsDropEvent(e);
    }
}

void FeedsTree::contentsDragMoveEvent(QDragMoveEvent* event)
{
    QListViewItem* item = itemAt(event->pos());
    /* check if on root decoration */
    if (!item || event->pos().x() > header()->cellPos(header()->mapToActual(0)) +
            treeStepSize() * (item->depth() + 1) + itemMargin() ||
            event->pos().x() < header()->cellPos(header()->mapToActual(0)))
        return;
    if (item && item->childCount() && !item->isOpen())
        item->setOpen(true);
}

void FeedsTree::slotCollapseAll()
{
    QListViewItem* it = firstChild();
    while(it)
    {
        it->setOpen(false);
        it = it->nextSibling();
    }
}

void FeedsTree::slotCollapse()
{
    if (currentItem())
        currentItem()->setOpen(false);
}


void FeedsTree::slotExpandAll()
{
    QListViewItem* it = firstChild();
    while(it)
    {
        it->setOpen(true);
        it = it->nextSibling();
    }
}

void FeedsTree::slotExpand()
{
    if (currentItem())
        currentItem()->setOpen(true);
}

void FeedsTree::slotItemUp()
{
    if (currentItem())
        setCurrentItem(currentItem()->itemAbove());
}

void FeedsTree::slotItemDown()
{
    if (currentItem())
        setCurrentItem(currentItem()->itemBelow());
}

void FeedsTree::slotItemBegin()
{
    setCurrentItem(firstChild());
}

void FeedsTree::slotItemEnd()
{
    QListViewItem* elt = firstChild();
    if (elt)
        while (elt->itemBelow())
            elt = elt->itemBelow();
    setCurrentItem(elt);
}

void FeedsTree::slotItemLeft()
{
    if (currentItem())
        setCurrentItem(currentItem()->parent());
}

void FeedsTree::slotItemRight()
{
    if (currentItem())
        setCurrentItem(currentItem()->firstChild());
}

#include "feedstree.moc"
