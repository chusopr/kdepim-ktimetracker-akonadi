#ifndef _KPILOT_MEMOWIDGET_H
#define _KPILOT_MEMOWIDGET_H
/* memoWidget.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the memo viewer widget.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <time.h>

#ifndef _PILOT_MEMO_H_
#include <pi-memo.h>
#endif

#ifndef KDE_VERSION
#include <kdeversion.h>
#endif

#if KDE_VERSION >= 0x30100
#include <ktextedit.h>
#else
#define KTextEdit QTextEdit
#include <qtextedit.h>
#endif

#include <qptrlist.h>

class KPilotInstaller;
class QListBox;
class QComboBox;
class QPushButton;

class PilotMemo;
class PilotListItem;

#ifndef _KPILOT_PILOTCOMPONENT_H
#include "pilotComponent.h"
#endif

class MemoWidget : public PilotComponent
{
Q_OBJECT
  
public:
	MemoWidget(QWidget* parent, const QString& dbpath);
	virtual ~MemoWidget();
  
	// Pilot Component Methods:
	/* virtual */ void showComponent();
	/* virtual */ void hideComponent();
	/* virtual */ bool preHotSync(QString &);
	/* virtual */ void postHotSync();

	// Added by David Bishop, please move to correct location!
	bool saveAsXML(const QString &fileName,const QPtrList<PilotListItem> &menu_item );
	bool saveAsText(const QString &fileName,const QPtrList<PilotListItem> &menu_item );
	
	typedef enum { 
		MAX_MEMO_LEN = 8192 
		} Constants ;

protected:
	void initializeCategories(PilotDatabase *);
	void initializeMemos(PilotDatabase *);

	// TODO: Ugh, I have no idea if this fix is in BRANCH as well.
	void saveChangedMemo();
  
public slots:
	/**
	* Called whenever the selected memo changes in order to:
	*   - display it if necessary
	*   - update which buttons are active, to prevent the delete
	*     button from being active when it can't do anything.
	*
	*/
	void slotShowMemo(int);
	void slotUpdateButtons();
	// void slotTextChanged();
	void slotImportMemo();
	void slotExportMemo();
	void slotDeleteMemo(); // Delets the currently selected memo(s)
	void slotSetCategory(int);

private:
	void setupWidget();
	void updateWidget(); // Called with the lists have changed..
	void writeMemo(PilotMemo* which);
	QComboBox* fCatList;
  
	KTextEdit*		fTextWidget;
	struct MemoAppInfo	fMemoAppInfo;
	QPtrList<PilotMemo>	fMemoList;
	QListBox*		fListBox;

	QPushButton *fExportButton,*fDeleteButton;
	
	int lastSelectedMemo;
};


#endif
