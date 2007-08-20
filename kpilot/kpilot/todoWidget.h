#ifndef _KPILOT_TODOWIDGET_H
#define _KPILOT_TODOWIDGET_H
/* todoWidget.h			KPilot
**
** Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**

** This file defines the todo-viewing widget used in KPilot
** to display the Pilot's todo records.
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

class QListWidget;
class QListWidgetItem;
class QPushButton;

class KComboBox;
class KTextEdit;

class PilotTodoEntry;

#include "pilotComponent.h"

class TodoWidget : public PilotComponent
{
Q_OBJECT

public:
	TodoWidget(QWidget *parent, const QString &dbpath);
	~TodoWidget();

	// Pilot Component Methods:
	virtual bool preHotSync(QString &);
	virtual void postHotSync();
	virtual void showComponent();
	virtual void hideComponent();

public slots:
	/**
	* Called when a particular todo is selected. This slot displays
	* it in the viewer widget.
	*/
	void slotShowTodo(QListWidgetItem *);

	void slotUpdateButtons();	// Enable/disable buttons

protected slots:
	/**
	* Change category. This means that the display should be
	* cleared and that the list should be repopulated.
	*/
	void slotSetCategory(int);

private:
	void setupWidget();
	void updateWidget(); // Called with the lists have changed..

	/**
	* getAllTodos reads the database and places all
	* the todos from the database in the list
	* in memory --- not the list on the screen.
	* @see fTodoList
	*/
	int getAllTodos(PilotDatabase *todoDB);

	/**
	* Create a sensible "title" for an todo, composed
	* of first + last name if possible.
	*/
	QString createTitle(PilotTodoEntry *,int displayMode);

	/**
	* We use a KComboBox fCatList to hold the user-visible names
	* of all the categories. The QTextView fTodoInfo is for
	* displaying the currently selected todo, if any.
	* The QListView fListBox lists all the todoes in the
	* currently selected category.
	*
	* The entire todo database is read into memory in the
	* QList fTodoList. We need the appinfo block from the
	* database to determine which categories there are; this
	* is held in fTodoAppInfo.
	*
	* The two buttons should speak for themselves.
	*/
	KComboBox *fCategoryList;
	QListWidget *fTodoList;
	KTextEdit *fTodoViewer;

	class Private;
	Private *fP;
};

#endif
