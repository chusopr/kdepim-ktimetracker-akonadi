/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#include "akregatorconfig.h"
#include "articlelistview.h"
#include "articlelist.h"
#include "feed.h"
#include "treenode.h"

#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kurl.h>

#include <qdatetime.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qwhatsthis.h>
#include <qheader.h>
#include <qdragobject.h>

using namespace Akregator;

ArticleItem::ArticleItem( QListView *parent, QListViewItem *after, const Article& a, Feed *feed)
    : KListViewItem( parent, after, KCharsets::resolveEntities(a.title()), feed->title(), KGlobal::locale()->formatDateTime(a.pubDate(), true, false) )
{
    m_article = a;
    m_feed = feed;
    if (a.keep())
        setPixmap(0, QPixmap(locate("data", "akregator/pics/akregator_flag.png")));
}
 
ArticleItem::~ArticleItem()
{
}


Article& ArticleItem::article()
{
    return m_article;
}

Feed *ArticleItem::feed()
{
    return m_feed;
}

// paint ze peons
void ArticleItem::paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
    QColorGroup cg2(cg);
    
    // FIXME: make configurable
    if (article().status()==Article::Unread)
        cg2.setColor(QColorGroup::Text, Qt::blue);
    else if (article().status()==Article::New)
        cg2.setColor(QColorGroup::Text, Qt::red);
    
    KListViewItem::paintCell( p, cg2, column, width, align );

}


int ArticleItem::compare(QListViewItem *i, int col, bool ascending) const {
    if (col == 2) {
        ArticleItem *item = static_cast<ArticleItem*>(i);
        if (item) {
            return ascending ?
		    item->m_article.pubDate().secsTo(m_article.pubDate()) :
		    -m_article.pubDate().secsTo(item->m_article.pubDate());
        }
    }
    return KListViewItem::compare(i, col, ascending);
}

/* ==================================================================================== */

ArticleListView::ArticleListView(QWidget *parent, const char *name)
    : KListView(parent, name), m_updated(false), m_doReceive(true), m_node(0), m_columnMode(feedMode)
{
    setMinimumSize(250, 150);
    addColumn(i18n("Article"));
    addColumn(i18n("Feed"));
    addColumn(i18n("Date"));
    setSelectionMode(QListView::Extended);
    setColumnWidthMode(2, QListView::Maximum);
    setColumnWidthMode(1, QListView::Manual);
    setColumnWidthMode(0, QListView::Manual);
    setRootIsDecorated(false);
    setItemsRenameable(false);
    setItemsMovable(false);
    setAllColumnsShowFocus(true);
    setDragEnabled(true); // FIXME before we implement dragging between archived feeds??
    setAcceptDrops(false); // FIXME before we implement dragging between archived feeds??
    setFullWidth(false);
    
    setShowSortIndicator(true);
    setDragAutoScroll(true);
    setDropHighlighter(false);

    int c = Settings::sortColumn();
    setSorting((c >= 0 && c <= 2) ? c : 2, Settings::sortAscending());

    int w;
    w = Settings::titleWidth();
    if (w > 0) {
        setColumnWidth(0, w);
    }
    
    w = Settings::feedWidth();
    if (w > 0) {
        setColumnWidth(1, w);
    }
    
    w = Settings::dateWidth();
    if (w > 0) {
        setColumnWidth(2, w);
    }
    
    m_feedWidth = columnWidth(1);
    hideColumn(1);

    header()->setStretchEnabled(true, 0);

    QWhatsThis::add(this, i18n("<h2>Article list</h2>"
        "Here you can browse articles from the currently selected feed. "
        "You can also manage articles, as marking them as persistent (\"Keep Article\") or delete them, using the right mouse button menu."
        "To view the web page of the article, you can open the article internally in a tab or in an external browser window."));

    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()) );
    connect(this, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),  this, SLOT(slotDoubleClicked(QListViewItem*, const QPoint&, int)) );
    connect(this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
            this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)));
}
void ArticleListView::slotSetFilter(const ArticleFilter& textFilter, const ArticleFilter& statusFilter)
{
    if ( (textFilter != m_textFilter) || (statusFilter != m_statusFilter) )
    {
        m_textFilter = textFilter;
        m_statusFilter = statusFilter;
               
        applyFilters();
    }
}

void ArticleListView::setReceiveUpdates(bool doReceive, bool remember)
{
    if (m_doReceive && !doReceive)
    {    
        m_updated = false;  
        m_doReceive = false;
        return;
    }
    
    if (!m_doReceive && doReceive)
    {    
        m_doReceive = true;
        if (remember && m_updated)
            slotUpdate();
        m_updated = false;  
    }   
}

void ArticleListView::slotShowNode(TreeNode* node)
{
    if (!node)
    {
        slotClear();
        return;
    }
     
    if (m_node)
    {
        disconnect(m_node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdate()) );
        disconnect(m_node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear()) );
    }
    
    connect(node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdate()) );
    connect(node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear()) );

    m_node = node;
        
    clear();
    
    if ( node->isGroup() && m_columnMode == feedMode )
    {
        setColumnWidth(1, m_feedWidth);
        m_columnMode = groupMode;
    }
    else if ( !node->isGroup() && m_columnMode == groupMode)
    {    
        m_feedWidth = columnWidth(1);
        hideColumn(1);
        m_columnMode = feedMode;
    }

    slotUpdate();
}

void ArticleListView::slotClear()
{
    if (m_node)
    {
        disconnect(m_node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdate()) );
        disconnect(m_node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear()) );
    }
    m_node = 0;
    
    clear();
}

void ArticleListView::slotUpdate()
{
    if (!m_doReceive)
    {
        m_updated = true;
        return;
    }
        
    if (!m_node) 
        return;    
    
    setUpdatesEnabled(false);

    Article oldCurrentArticle;
    
    ArticleItem *li = dynamic_cast<ArticleItem*>(currentItem());
    bool haveOld = false;
    if (li)
    {
        oldCurrentArticle = li->article();
        haveOld = true;
    }

    
    
    QPtrList<QListViewItem> selItems = selectedItems(false);

    ArticleList selectedArticles;

    int haveSelected = 0;    
    for (QListViewItem* i = selItems.first(); i; i = selItems.next() )
    {
        ArticleItem* si = static_cast<ArticleItem*>(i);
        selectedArticles.append(si->article());
        ++haveSelected;
    }
    
    clear();
    
    setShowSortIndicator(false);
    int col = sortColumn();
    SortOrder order = sortOrder();
    setSorting(-1);

    ArticleList articles = m_node->articles();

    ArticleList::ConstIterator end = articles.end();
    ArticleList::ConstIterator it = articles.begin();

    for (; it != end; ++it)
    {
        if (!(*it).isDeleted())
        {
            ArticleItem *ali = new ArticleItem(this, lastChild(), *it, (*it).feed());
            if (haveOld && *it == oldCurrentArticle)
            {
                setCurrentItem(ali);
                haveOld = false;
            }
            if (haveSelected > 0 && selectedArticles.contains(ali->article()))
            {
                setSelected(ali, true);
                --haveSelected;
            }
        }
    }

    setSorting(col, order == Ascending);
    setShowSortIndicator(true);

    applyFilters();        
    setUpdatesEnabled(true);
    triggerUpdate();
}

void ArticleListView::applyFilters()
{
    ArticleItem* ali = 0;
    for (QListViewItemIterator it(this); it.current(); ++it)
    {
        ali = static_cast<ArticleItem*> (it.current());
        ali->setVisible( m_statusFilter.matches( ali->article() ) && m_textFilter.matches( ali->article() ) );
    }
}

QDragObject *ArticleListView::dragObject()
{
    QDragObject *d = new QTextDrag(currentItem()->article().link().prettyURL(), this);
    return d;
}

void ArticleListView::slotPreviousArticle()
{
    QListViewItem *lvi = currentItem();
    
    if (!lvi && firstChild() )
    {
        setCurrentItem(firstChild());
        clearSelection();
        setSelected(firstChild(), true);
    }
    
    if (lvi && lvi->itemAbove()) 
    {
        setCurrentItem(lvi->itemAbove());
        clearSelection();
        setSelected( lvi->itemAbove(), true);
        ensureItemVisible(lvi->itemAbove());
    }
}

void ArticleListView::slotNextArticle()
{
    QListViewItem *lvi = currentItem();
    
    if (!lvi && firstChild() )
    {
        setCurrentItem(firstChild());
        clearSelection();
        setSelected(firstChild(), true);
        return;
    }
    if (lvi && lvi->itemBelow()) 
    {
        setCurrentItem(lvi->itemBelow());
        clearSelection();
        setSelected(lvi->itemBelow(), true);
        ensureItemVisible(lvi->itemBelow());
    }
}

void ArticleListView::slotNextUnreadArticle()
{
    ArticleItem *it= static_cast<ArticleItem*>(currentItem());
    if (!it)
        it = static_cast<ArticleItem*>(firstChild());

    for ( ; it; it = static_cast<ArticleItem*>(it->nextSibling()))
    {
        if (it->article().status() != Article::Read)
        {
            setCurrentItem(it);
            clearSelection();
            setSelected(it, true);
            ensureItemVisible(it);
            return;
        }
    }
    // only reached when there is no unread article after the selected one
    if (m_node->unread() > 0)
    {
        it = static_cast<ArticleItem*>(firstChild());
        for ( ; it; it = static_cast<ArticleItem*>(it->nextSibling()))
        {
            if (it->article().status() != Article::Read)
            {
                setCurrentItem(it);
                clearSelection();
                setSelected(it, true);
                ensureItemVisible(it);
                return;
            }
        }
    }
}

void ArticleListView::slotPreviousUnreadArticle()
{
    if ( !currentItem() )
        slotNextUnreadArticle(); 

    QListViewItemIterator it( currentItem() );
    
    for ( ; it.current(); --it )
    {
        ArticleItem* ali = static_cast<ArticleItem*> (it.current());
        if (!ali)
            break;
        if ((ali->article().status()==Article::Unread) ||
             (ali->article().status()==Article::New))
        {
            setCurrentItem(ali);
            clearSelection();
            setSelected(ali, true);
            ensureItemVisible(ali);
            return;
        }
    }
    // only reached when there is no unread article before the selected one
    if (m_node->unread() > 0)
    {
        it = static_cast<ArticleItem*>(lastChild());

        for ( ; it.current(); --it )
        {
            ArticleItem* ali = static_cast<ArticleItem*> (it.current());
            if ((ali->article().status() != Article::Read))
            {
                setCurrentItem(ali);
                clearSelection();
                setSelected(ali, true);
                ensureItemVisible(ali);
                return;
            }
        }
    }
}

void ArticleListView::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
}

void ArticleListView::slotSelectionChanged()
{
    ArticleItem* ai = dynamic_cast<ArticleItem*> (currentItem());
    if (ai)
        emit signalArticleChosen( ai->article() );
}

void ArticleListView::slotCurrentChanged(QListViewItem*/* item*/)
{/*
    ArticleItem* ai = dynamic_cast<ArticleItem*> (item);
    if (ai)
        emit signalCurrentArticleChanged( ai->article() );*/
} 


void ArticleListView::slotDoubleClicked(QListViewItem* item, const QPoint& p, int i)
{
    emit signalDoubleClicked(static_cast<ArticleItem*>(item), p, i);
}

void ArticleListView::slotContextMenu(KListView* list, QListViewItem* item, const QPoint& p)
{
    emit signalContextMenu(list, static_cast<ArticleItem*>(item), p);
}
        
ArticleListView::~ArticleListView()
{
    Settings::setTitleWidth(columnWidth(0));
    Settings::setFeedWidth(columnWidth(1) > 0 ? columnWidth(1) : m_feedWidth);
    Settings::setSortColumn(sortColumn());
    Settings::setSortAscending(sortOrder() == Ascending);
    Settings::writeConfig();
}

QPtrList<ArticleItem> ArticleListView::selectedArticleItems(bool includeHiddenItems) const
{
    QPtrList<QListViewItem> items = selectedItems(includeHiddenItems);
    QPtrList<ArticleItem> ret;
    for (QListViewItem* i = items.first(); i; i = items.next() )
        ret.append(static_cast<ArticleItem*>(i));
    return ret;
}

#include "articlelistview.moc"
// vim: ts=4 sw=4 et
