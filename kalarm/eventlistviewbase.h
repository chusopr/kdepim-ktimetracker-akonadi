/*
 *  eventlistviewbase.h  -  base classes for widget showing list of events
 *  Program:  kalarm
 *  (C) 2004 by David Jarvie <software@astrojar.org.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  In addition, as a special exception, the copyright holders give permission
 *  to link the code of this program with any edition of the Qt library by
 *  Trolltech AS, Norway (or with modified versions of Qt that use the same
 *  license as Qt), and distribute linked combinations including the two.
 *  You must obey the GNU General Public License in all respects for all of
 *  the code used other than Qt.  If you modify this file, you may extend
 *  this exception to your version of the file, but you are not obligated to
 *  do so. If you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef EVENTLISTVIEWBASE_H
#define EVENTLISTVIEWBASE_H

#include "kalarm.h"

#include <qvaluelist.h>
#include <klistview.h>

#include "alarmevent.h"

class QPixmap;
class EventListViewItemBase;


class EventListViewBase : public KListView
{
		Q_OBJECT
	public:
		typedef QValueList<EventListViewBase*>              InstanceList;
		typedef QValueListIterator<EventListViewBase*>      InstanceListIterator;
		typedef QValueListConstIterator<EventListViewBase*> InstanceListConstIterator;

		EventListViewBase(QWidget* parent = 0, const char* name = 0);
		virtual ~EventListViewBase()  { }
		void                   refresh();
		EventListViewItemBase* getEntry(const QString& eventID);
		void                   addEvent(const KAEvent& e)  { addEvent(e, instances(), this); }
		void                   modifyEvent(const KAEvent& e)
		                                              { modifyEvent(e.id(), e, instances(), this); }
		void                   modifyEvent(const QString& oldEventID, const KAEvent& newEvent)
		                                              { modifyEvent(oldEventID, newEvent, instances(), this); }
		void                   deleteEvent(const QString& eventID)  { deleteEvent(eventID, instances()); }
		static void            addEvent(const KAEvent&, const InstanceList&, EventListViewBase* selectionView);
		static void            modifyEvent(const KAEvent& e, const InstanceList& list, EventListViewBase* selectionView)
		                                              { modifyEvent(e.id(), e, list, selectionView); }
		static void            modifyEvent(const QString& oldEventID, const KAEvent& newEvent, const InstanceList&, EventListViewBase* selectionView);
		static void            deleteEvent(const QString& eventID, const InstanceList&);
		static void            undeleteEvent(const QString& oldEventID, const KAEvent& event, const InstanceList& list, EventListViewBase* selectionView)
		                                              { modifyEvent(oldEventID, event, list, selectionView); }
		void                   resizeLastColumn();
		int                    itemHeight();
		bool                   anySelected() const;    // are any items selected?
		const KAEvent*         selectedEvent() const;
		EventListViewItemBase* selectedItem() const   { return (EventListViewItemBase*)KListView::selectedItem(); }
		EventListViewItemBase* currentItem() const    { return (EventListViewItemBase*)KListView::currentItem(); }
		EventListViewItemBase* firstChild() const     { return (EventListViewItemBase*)KListView::firstChild(); }
		EventListViewItemBase* singleSelectedItem() const;
		QValueList<EventListViewItemBase*> selectedItems() const;
		int                    selectedCount() const;
		int                    lastColumn() const     { return mLastColumn; }
		virtual QString        whatsThisText(int column) const = 0;
		virtual InstanceList   instances() = 0; // return all instances

	signals:
		void                   itemDeleted();

	protected:
		virtual void           populate() = 0;         // populate the list with all desired events
		virtual EventListViewItemBase* createItem(const KAEvent&) = 0;   // only used by default addEntry() method
		virtual bool           shouldShowEvent(const KAEvent&) const  { return true; }
		EventListViewItemBase* addEntry(const KAEvent&, bool setSize = false, bool reselect = false);
		EventListViewItemBase* addEntry(EventListViewItemBase*, bool setSize, bool reselect);
		EventListViewItemBase* updateEntry(EventListViewItemBase*, const KAEvent& newEvent, bool setSize = false, bool reselect = false);
		void                   addLastColumn(const QString& title);
		virtual void           showEvent(QShowEvent*);
		virtual void           resizeEvent(QResizeEvent*);

	private:
		void                   deleteEntry(EventListViewItemBase*, bool setSize = false);

		int                    mLastColumn;           // index to last column
		int                    mLastColumnHeaderWidth;
};


class EventListViewItemBase : public QListViewItem
{
	public:
		EventListViewItemBase(EventListViewBase* parent, const KAEvent&);
		const KAEvent&         event() const             { return mEvent; }
		QPixmap*               eventIcon() const;
		int                    lastColumnWidth() const   { return mLastColumnWidth; }
		EventListViewItemBase* nextSibling() const       { return (EventListViewItemBase*)QListViewItem::nextSibling(); }
		static int             iconWidth()               { return mIconWidth; }

	protected:
		void                   setLastColumnText();
		virtual QString        lastColumnText() const = 0;   // get the text to display in the last column

	private:
		static QPixmap*        mTextIcon;
		static QPixmap*        mFileIcon;
		static QPixmap*        mCommandIcon;
		static QPixmap*        mEmailIcon;
		static int             mIconWidth;        // maximum width of any icon

		KAEvent                mEvent;            // the event for this item
		int                    mLastColumnWidth;  // width required to display message column
};

#endif // EVENTLISTVIEWBASE_H

