/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003 by Reinhold Kainhofer <reinhold@kainhofer.com>
**
** Program description
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifndef _KPILOT_LISTITEMS_H
#include "listItems.h"
#endif

#include <q3listbox.h>
#include <q3listview.h>

#include "options.h"

#ifdef DEBUG
/* static */ int PilotListItem::crt = 0;
/* static */ int PilotListItem::del = 0;
/* static */ int PilotListItem::count = 0;

/* static */ void PilotListItem::counts()
{
	FUNCTIONSETUP;
	DEBUGKPILOT << "created=" << crt << " deletions=" << del;
}
#endif

PilotListItem::PilotListItem(const QString & text,
	recordid_t pilotid, void *r) :
	Q3ListBoxText(text),
	fid(pilotid),
	fr(r)
{
	// FUNCTIONSETUP;
#ifdef DEBUG
	crt++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
}

PilotListItem::~PilotListItem()
{
	// FUNCTIONSETUP;
#ifdef DEBUG
	del++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
}




#ifdef DEBUG
/* static */ int PilotCheckListItem::crt = 0;
/* static */ int PilotCheckListItem::del = 0;
/* static */ int PilotCheckListItem::count = 0;

/* static */ void PilotCheckListItem::counts()
{
	FUNCTIONSETUP;
	DEBUGKPILOT << "created=" << crt << " deletions=" << del;
}
#endif

PilotCheckListItem::PilotCheckListItem(Q3ListView * parent, const QString & text, recordid_t pilotid, void *r) :
	Q3CheckListItem(parent, text, Q3CheckListItem::CheckBox),
	fid(pilotid),
	fr(r)
{
	// FUNCTIONSETUP;
#ifdef DEBUG
	crt++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
}

PilotCheckListItem::~PilotCheckListItem()
{
	// FUNCTIONSETUP;
#ifdef DEBUG
	del++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
}

void PilotCheckListItem::stateChange ( bool on)
{
	// FUNCTIONSETUP;
	Q3CheckListItem::stateChange(on);

}




#ifdef DEBUG
/* static */ int PilotListViewItem::crt = 0;
/* static */ int PilotListViewItem::del = 0;
/* static */ int PilotListViewItem::count = 0;

/* static */ void PilotListViewItem::counts()
{
	FUNCTIONSETUP;
	DEBUGKPILOT << "created=" << crt << " deletions=" << del;
}
#endif

PilotListViewItem::PilotListViewItem( Q3ListView * parent,
	const QString &label1, const QString &label2, const QString &label3
	, const QString &label4, recordid_t pilotid, void *r):
	Q3ListViewItem(parent, label1, label2, label3, label4,
		QString(), QString(), QString(), QString()),
	fid(pilotid),
	fr(r),
	d(new PilotListViewItemData)
{
	// FUNCTIONSETUP;
	if (d) d->valCol=-1;
#ifdef DEBUG
	crt++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
}

PilotListViewItem::~PilotListViewItem()
{
	// FUNCTIONSETUP;
#ifdef DEBUG
	del++;
	count++;
	if (!(count & 0xff))
		counts();
#endif
}
void PilotListViewItem::setNumericCol(int col, bool numeric)
{
	// FUNCTIONSETUP;
	if (numeric)
	{
		if (!numericCols.contains(col))
			numericCols.append(col);
	}
	else
	{
		if (numericCols.contains(col))
			numericCols.remove(col);
	}
}

unsigned long PilotListViewItem::colValue(int col, bool *ok) const
{
//	FUNCTIONSETUP;
/*	if (!d)
	{
		d=new PilotListViewItemData;
		d->valCol=-1;
	}*/
	if (d->valCol!=col)
	{
		// Use true for ascending for now...
		d->val=key(col, true).toULong(&d->valOk);
		d->valCol=col;
	}
	if (ok) (*ok)=d->valOk;
	return d->val;
}

int PilotListViewItem::compare( Q3ListViewItem *i, int col, bool ascending ) const
{
	PilotListViewItem*item=dynamic_cast<PilotListViewItem*>(i);
	if (item && numericCols.contains(col))
	{
		bool ok1, ok2;
		/// Do the toULong call just once if the sorting column changed:
		unsigned long l1=colValue(col, &ok1);
		unsigned long l2=item->colValue(col, &ok2);
		if (ok1 && ok2)
		{
			// Returns -1 if this item is less than i, 0 if they are
			// equal and 1 if this item is greater than i.
			int res=0;
			if (l1<l2) res=-1;
			else if (l1>l2) res=1;
			//else res=0;
			return res;
		}
	}
	return Q3ListViewItem::compare(i, col, ascending);
}

