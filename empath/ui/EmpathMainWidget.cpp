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
#include <qheader.h>

// Local includes
#include "EmpathMainWidget.h"
#include "EmpathFolderWidget.h"
#include "EmpathMessageViewWidget.h"
#include "EmpathMessageListWidget.h"

EmpathMainWidget::EmpathMainWidget(QWidget * parent, const char * name)
	: QWidget(parent, name)
{
	empathDebug("ctor");
	setBackgroundMode(QWidget::PaletteLight);
	
	hSplit = new KNewPanner(this, "hSplit", KNewPanner::Vertical, KNewPanner::Absolute);
	
	CHECK_PTR(hSplit);
	
	folderWidget_ =
		new EmpathFolderWidget(hSplit, "folderWidget");
	CHECK_PTR(folderWidget_);

	
	vSplit = new KNewPanner(hSplit, "vSplit", KNewPanner::Horizontal, KNewPanner::Absolute);
	CHECK_PTR(vSplit);
		
	messageListWidget_ =
		new EmpathMessageListWidget(vSplit, "messageListWidget");
	CHECK_PTR(messageListWidget_);

	messageViewWidget_ =
		new EmpathMessageViewWidget(EmpathURL(),
				vSplit, "messageViewWidget");
	CHECK_PTR(messageViewWidget_);

	empathDebug("Updating message list widget");
	messageListWidget_->update();
	
	QObject::connect(folderWidget_, SIGNAL(showFolder(const EmpathURL &)),
			messageListWidget_, SLOT(s_showFolder(const EmpathURL &)));
	
	QObject::connect(messageListWidget_, SIGNAL(changeView(const EmpathURL &)),
			this, SLOT(s_displayMessage(const EmpathURL &)));
	
	QObject::connect(folderWidget_->header(), SIGNAL(sizeChange(int, int, int)),
			this, SLOT(s_folderWidgetSizeChange(int, int, int)));
	
	messageListWidget_->setSignalUpdates(true);
	
	vSplit->activate(messageListWidget_, messageViewWidget_);
	hSplit->activate(folderWidget_, vSplit);
}

EmpathMainWidget::~EmpathMainWidget()
{
	empathDebug("dtor");
	ASSERT(messageViewWidget_ != 0);
	ASSERT(messageListWidget_ != 0);
	delete messageViewWidget_;
	messageViewWidget_ = 0;
	delete messageListWidget_;
	messageListWidget_ = 0;
}

	void
EmpathMainWidget::resizeEvent(QResizeEvent * e)
{
	resize(e->size());
	hSplit->resize(e->size());
	hSplit->setAbsSeparatorPos(folderWidget_->header()->sizeHint().width());
	vSplit->setAbsSeparatorPos(height() / 2);
}

	EmpathMessageListWidget *
EmpathMainWidget::messageListWidget()
{
	return messageListWidget_;
}

	EmpathMessageViewWidget *
EmpathMainWidget::messageViewWidget()
{
	return messageViewWidget_;
}

	void
EmpathMainWidget::s_displayMessage(const EmpathURL & url)
{
	messageViewWidget_->s_setMessage(url);
	messageViewWidget_->go();
}

	void
EmpathMainWidget::s_folderWidgetSizeChange(int a, int b, int c)
{
	hSplit->setAbsSeparatorPos(folderWidget_->header()->sizeHint().width());
}

