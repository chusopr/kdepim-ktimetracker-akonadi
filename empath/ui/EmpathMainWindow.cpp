/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Qt includes
#include <qapplication.h>
#include <qmsgbox.h>

// KDE includes
#include <klocale.h>
#include <kfiledialog.h>
#include <kmsgbox.h>
#include <kapp.h>

// Local includes
#include "EmpathUIUtils.h"
#include "HeapPtr.h"
#include "EmpathDefines.h"
#include "EmpathMainWidget.h"
#include "EmpathMainWindow.h"
#include "EmpathMessageListWidget.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathMessageViewWindow.h"
#include "EmpathMailboxList.h"
#include "EmpathMessageWidget.h"
#include "EmpathMessageList.h"
#include "EmpathFolderWidget.h"
#include "EmpathAboutBox.h"
#include "EmpathStatusWidget.h"
#include "EmpathMessageSourceView.h"
#include "EmpathConfig.h"
#include "EmpathFindDialog.h"
#include "EmpathFilterManagerDialog.h"
#include "EmpathFolderChooserDialog.h"
#include "Empath.h"
#include "EmpathMenuMaker.h"
#include "EmpathDebugDialog.h"

EmpathMainWindow::EmpathMainWindow(const char * name)
	:	KTMainWindow(name)
{
	empathDebug("ctor");

	menu_	= menuBar();
	status_	= statusBar();
		
	mainWidget_						=
		new EmpathMainWidget(this, "mainWidget");
	CHECK_PTR(mainWidget_);
	
	messageListWidget_ = mainWidget_->messageListWidget();
	
	statusWidget_	=
		new EmpathStatusWidget(0, "statusWidget");
	
	CHECK_PTR(statusWidget_);
	
	setView(mainWidget_, false);

	setupMenuBar();
	setupToolBar();
	setupStatusBar();
	
	setCaption(kapp->getCaption());
	
	updateRects();

	qApp->processEvents();
	empathDebug("About to show...");
	show();
	messageListWidget_->update();
}

EmpathMainWindow::~EmpathMainWindow()
{
	empathDebug("dtor");
}

	void
EmpathMainWindow::setupMenuBar()
{
	empathDebug("setting up menu bar");

	fileMenu_		= new QPopupMenu;
	CHECK_PTR(fileMenu_);

	editMenu_		= new QPopupMenu;
	CHECK_PTR(editMenu_);

	folderMenu_		= new QPopupMenu;
	CHECK_PTR(folderMenu_);

	messageMenu_	= new QPopupMenu;
	CHECK_PTR(messageMenu_);
	
	optionsMenu_	= new QPopupMenu;
	CHECK_PTR(optionsMenu_);

	helpMenu_		= new QPopupMenu;
	CHECK_PTR(helpMenu_);

	// File menu
	
	fileMenu_->insertItem(empathIcon("send.xpm"), i18n("&Send Pending Mail"),
		this, SLOT(s_fileSendNew()));

//	fileMenu_->insertSeparator();

//	fileMenu_->insertItem(i18n("Address&book..."),
//		this, SLOT(s_fileAddressBook()));
	
	fileMenu_->insertSeparator();

	fileMenu_->insertItem(i18n("E&xit"),
		this, SLOT(s_fileQuit()));

	// Edit menu
	
	editMenu_->insertItem(empathIcon("empath-cut.xpm"), i18n("Cu&t"),
		this, SLOT(s_editCut()));
	
	editMenu_->insertItem(empathIcon("empath-copy.xpm"),i18n("&Copy"),
		this, SLOT(s_editCopy()));
	
	editMenu_->insertItem(empathIcon("empath-paste.xpm"), i18n("&Paste"),
		this, SLOT(s_editPaste()));
	
	editMenu_->insertItem(i18n("&Delete"),
		this, SLOT(s_editDelete()));

	editMenu_->insertSeparator();
	
	editMenu_->insertItem(i18n("&Select") + "...",
		this, SLOT(s_editSelect()));
	
	editMenu_->insertSeparator();
	
	editMenu_->insertItem(i18n("&Find In Message") + "...",
		this, SLOT(s_editFindInMessage()));
	
	editMenu_->insertItem(i18n("Find In A&ll Messages") + "...",
		this, SLOT(s_editFind()));

	editMenu_->insertItem(i18n("Find &Again"),
		this, SLOT(s_editFindAgain()));
	
	// Folder menu

	folderMenu_->insertItem(empathIcon("mini-folder-grey.xpm"),
		i18n("&New") + "...",
		this, SLOT(s_folderNew()));

	folderMenu_->insertItem(
		i18n("&Properties") + "...",
		this, SLOT(s_folderEdit()));

	folderMenu_->insertItem(empathIcon("mini-folder-outbox.xpm"),
		i18n("&Clear") + "...",
		this, SLOT(s_folderClear()));

	folderMenu_->insertItem(
		i18n("Delete") + "...",
		this, SLOT(s_folderDelete()));

	// Message Menu
	setupMessageMenu(this, 0, messageMenu_);
	
	// Options menu
	setupOptionsMenu(this, 0, optionsMenu_);
	
	// Help menu
	setupHelpMenu(this, 0, helpMenu_);

	menu_->insertItem(i18n("&File"), fileMenu_);
	menu_->insertItem(i18n("&Edit"), editMenu_);
	menu_->insertItem(i18n("F&older"), folderMenu_);
	menu_->insertItem(i18n("&Message"), messageMenu_);
	menu_->insertItem(i18n("&Options"), optionsMenu_);
	menu_->insertSeparator();
	menu_->insertItem(i18n("&Help"), helpMenu_);
}

	void
EmpathMainWindow::setupToolBar()
{
	empathDebug("setting up tool bar");

	QPixmap p = empathIcon("compose.xpm");
	int i = QMAX(p.width(), p.height());

	KToolBar * tb = new KToolBar(this, "tooly", i + 4 );
	CHECK_PTR(tb);

	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_GENERAL);
	
	KToolBar::BarPosition pos =
		(KToolBar::BarPosition)
		c->readNumEntry(EmpathConfig::KEY_MAIN_WINDOW_TOOLBAR_POSITION);

	if (pos == KToolBar::Floating) pos = KToolBar::Top;
	tb->setBarPos(pos);
	
	this->addToolBar(tb, 0);

	QObject::connect(tb, SIGNAL(moved(BarPosition)),
			this, SLOT(s_toolbarMoved(BarPosition)));

	tb->insertButton(empathIcon("compose.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageNew()), true, i18n("Compose"));
	
	tb->insertButton(empathIcon("reply.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageReply()), true, i18n("Reply"));
	
	tb->insertButton(empathIcon("forward.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageForward()), true, i18n("Forward"));
	
	tb->insertSeparator();
	
	tb->insertButton(empathIcon("delete.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageDelete()), true, i18n("Delete"));
	
	tb->insertButton(empathIcon("save.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_messageSaveAs()), true, i18n("Save"));
	
	// Debugging
	tb->insertButton(empathIcon("mini-view.xpm"), 0, SIGNAL(clicked()),
			this, SLOT(s_dumpWidgetList()), true, i18n("Debug"));
}

	void
EmpathMainWindow::setupStatusBar()
{
	empathDebug("setting up status bar");
	status_->message("Ready");
}

// If the user presses the close button on the title bar, or tries
// to kill us off another way, handle gracefully

	bool
EmpathMainWindow::queryExit()
{
	// FIXME: Check if the user wants to save changes
	s_fileQuit();
	return false;
}

// File menu slots

	void
EmpathMainWindow::s_fileSendNew()
{
	empathDebug("s_fileSendNew called");

}

	void
EmpathMainWindow::s_fileAddressBook()
{
	empathDebug("s_fileAddressBook called");
}
	
	void
EmpathMainWindow::s_fileQuit()
{
	empathDebug("s_fileQuit called");
	// FIXME: Check if the user wants to save changes
	delete this;
}

// Edit menu slots
	
	void
EmpathMainWindow::s_editCut()
{
	empathDebug("s_editCut called");
}

	void
EmpathMainWindow::s_editCopy()
{
	empathDebug("s_editCopy called");
}

	void
EmpathMainWindow::s_editPaste()
{
	empathDebug("s_editPaste called");
}

	void
EmpathMainWindow::s_editDelete()
{
	empathDebug("s_editDelete called");
}

	void
EmpathMainWindow::s_editSelect()
{
	empathDebug("s_editSelect called");
}

	void
EmpathMainWindow::s_editFindInMessage()
{
	empathDebug("s_editFindInMessage called");
	EmpathFindDialog f(this, "findDialog");
	f.exec();
}

	void
EmpathMainWindow::s_editFind()
{
	empathDebug("s_editFind called");
	EmpathFindDialog f(this, "findDialog");
	f.exec();
}

	void
EmpathMainWindow::s_editFindAgain()
{
	empathDebug("s_editFindAgain called");
	EmpathFindDialog f(this, "findDialog");
	f.exec();
}

// Folder menu slots

	void
EmpathMainWindow::s_folderNew()
{
	empathDebug("s_folderNew called");

}

	void
EmpathMainWindow::s_folderEdit()
{
	empathDebug("s_folderEdit called");

}

	void
EmpathMainWindow::s_folderClear()
{
	empathDebug("s_folderClear called");

}

	void
EmpathMainWindow::s_folderDelete()
{
	empathDebug("s_folderDelete called");

}


// Message menu slots

	void
EmpathMainWindow::s_messageNew()
{
	empathDebug("s_messageNew called");
	empath->s_compose();
}

	void
EmpathMainWindow::s_messageReply()
{
	empathDebug("s_messageReply called");
	empath->s_reply(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageReplyAll()
{
	empathDebug("s_messageReplyAll called");
	empath->s_replyAll(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageForward()
{
	empathDebug("s_messageForward called");
	empath->s_forward(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageBounce()
{
	empathDebug("s_messageBounce called");
	empath->s_bounce(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageDelete()
{
	empathDebug("s_messageDelete called");
	empath->s_remove(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageSaveAs()
{
	empathDebug("s_messageSaveAs called");
	
	RMessage * message = _getFirstSelectedMessage();
	
	if (message == 0) {
		KMsgBox(this, "Empath", i18n("Please select a message first"), KMsgBox::EXCLAMATION, i18n("OK"));
		return;
	}

	QString saveFilePath =
		KFileDialog::getSaveFileName(
			QString::null, QString::null, this, i18n("Empath: Save Message"));
	empathDebug(saveFilePath);
	
	if (saveFilePath.isEmpty()) {
		empathDebug("No filename given");
		delete message; message = 0;
		return;
	}
	
	QFile f(saveFilePath);
	if (!f.open(IO_WriteOnly)) {
		// Warn user file cannot be opened.
		empathDebug("Couldn't open file for writing");
		KMsgBox(this, "Empath", i18n("Sorry I can't write to that file. Please try another filename."), KMsgBox::EXCLAMATION, i18n("OK"));
		delete message; message = 0;
		return;
	}
	empathDebug("Opened " + saveFilePath + " OK");
	
	QDataStream d(&f);
	
	d << message->asString();

	f.close();
		
//	KMsgBox(this, "Empath", i18n("Message saved to") + QString(" ") + saveFilePath + QString(" ") + i18n("OK"), KMsgBox::INFORMATION, i18n("OK"));
	delete message; message = 0;
}

	void
EmpathMainWindow::s_messageCopyTo()
{
	empathDebug("s_messageCopyTo called");

	RMessage * message(_getFirstSelectedMessage());
	
	if (message == 0) {
		KMsgBox(this, "Empath", i18n("Please select a message first"), KMsgBox::EXCLAMATION, i18n("OK"));
		return;
	}
	
	EmpathFolderChooserDialog * fcd =
		new EmpathFolderChooserDialog((QWidget *)0L, "fcd");

	fcd->exec();

	EmpathFolder * copyFolder = fcd->selectedFolder();

	if (copyFolder == 0) {
		empathDebug("No folder selected for copy");
		delete message; message = 0;
		return;
	}

	copyFolder->writeMessage(*message);
	delete message; message = 0;
}

	void
EmpathMainWindow::s_messageMoveTo()
{
	empathDebug("s_messageMoveTo called");

	RMessage * message(_getFirstSelectedMessage());
	
	if (message == 0) {
		KMsgBox(this, "Empath", i18n("Please select a message first"), KMsgBox::EXCLAMATION, i18n("OK"));
		return;
	}
	
	EmpathFolderChooserDialog * fcd =
		new EmpathFolderChooserDialog((QWidget *)0L, "fcd");

	fcd->exec();

	EmpathFolder * copyFolder = fcd->selectedFolder();

	if (copyFolder == 0) {
		empathDebug("No folder selected for move");
		delete message; message = 0;
		return;
	}

	if (copyFolder->writeMessage(*message)) {
		// FIXME: Remove message from its parent folder.
	}
	delete message; message = 0;
}

	void
EmpathMainWindow::s_messagePrint()
{
	empathDebug("s_messagePrint called");
	mainWidget_->messageViewWidget()->s_print();
}

	void
EmpathMainWindow::s_messageFilter()
{
	empathDebug("s_messageFilter called");
	empath->filter(messageListWidget_->firstSelectedMessage());
}

	void
EmpathMainWindow::s_messageView()
{
	empathDebug("s_messageView called");
	
	EmpathMessageViewWindow * messageViewWindow =
		new EmpathMessageViewWindow(
			messageListWidget_->firstSelectedMessage(), "Empath");

	CHECK_PTR(messageViewWindow);

	messageViewWindow->show();
}

	void
EmpathMainWindow::s_messageViewSource()
{
	empathDebug("s_messageViewSource called");
	
	EmpathMessageSourceView * sourceView =
		new EmpathMessageSourceView(
			messageListWidget_->firstSelectedMessage(), 0);

	CHECK_PTR(sourceView);

	sourceView->show();
}

	void
EmpathMainWindow::s_help()
{
	empathInvokeHelp("", "");
}

	void
EmpathMainWindow::s_aboutEmpath()
{
	EmpathAboutBox::create();
}

	void
EmpathMainWindow::s_aboutQt()
{
	QMessageBox::aboutQt(this, "aboutQt");
}

	void
EmpathMainWindow::statusMessage(const QString & messageText, int seconds)
{
	status_->message(messageText, seconds);
}

	void
EmpathMainWindow::clearStatusMessage()
{
	status_->clear();
}

	void
EmpathMainWindow::s_toolbarMoved(BarPosition pos)
{
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_GENERAL);
	c->writeEntry(EmpathConfig::KEY_MAIN_WINDOW_TOOLBAR_POSITION, (int)pos);
}

	void
EmpathMainWindow::newMailArrived()
{
	messageListWidget_->update();
}

	EmpathStatusWidget *
EmpathMainWindow::statusWidget()
{
	return statusWidget_;
}

	EmpathMessageListWidget *
EmpathMainWindow::messageListWidget()
{
	return messageListWidget_;
}

	void
EmpathMainWindow::s_dumpWidgetList()
{
	EmpathDebugDialog d(this, "debugDialog");
	d.exec();
}

	RMessage *
EmpathMainWindow::_getFirstSelectedMessage() const
{
	return empath->message(messageListWidget_->firstSelectedMessage());
}

