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

// FIXME: The writeNewMail method needs to find a way of telling the
// caller whether message write was successful.

// Qt includes
#include <qfile.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qapplication.h>

// KDE includes
#include <klocale.h>
#include <kapp.h>
#include <kconfig.h>

// Local includes
#include "EmpathMailboxMaildir.h"
#include "EmpathFolderList.h"
#include "EmpathMessageList.h"
#include "EmpathConfig.h"
#include "EmpathEnum.h"
#include "Empath.h"

EmpathMailboxMaildir::EmpathMailboxMaildir(const QString & name)
	:	EmpathMailbox(name)
{
	empathDebug("ctor");
	type_ = Maildir;
	seq_ = 0;
	boxList_.setAutoDelete(true);
	
	// for testing
	
	path_ = QDir::homeDirPath() + "/Maildir/";
	
	QDir d(path_);
	
	if (!d.exists())
		if (!d.mkdir(path_)) {
			empathDebug("Couldn't make " + path_ + " !!!!!");
		}
}

EmpathMailboxMaildir::~EmpathMailboxMaildir()
{
	empathDebug("dtor");
}

	void
EmpathMailboxMaildir::mark(const EmpathURL & message, RMM::MessageStatus msgStat)
{
	EmpathMaildir * m = _box(message);
	if (m == 0) return;
	m->mark(message, msgStat);
}

	void
EmpathMailboxMaildir::syncIndex(const EmpathURL & url)
{
	empathDebug("syncIndex(" + url.asString() + ") called");
	EmpathMaildirListIterator it(boxList_);
	
	for (; it.current(); ++it) {
		empathDebug("Looking at \"" + it.current()->path() + "\"");
		if (it.current()->url() == url)
			it.current()->sync(url);
	}
}
	
	void
EmpathMailboxMaildir::_setupDefaultFolders()
{
	empathDebug("_setupDefaultFolders() called");
	EmpathURL urlInbox	(url_.mailboxName(), i18n("Inbox"),		QString::null);
	EmpathURL urlOutbox	(url_.mailboxName(), i18n("Outbox"),	QString::null);
	EmpathURL urlTrash	(url_.mailboxName(), i18n("Trash"),		QString::null);
	EmpathURL urlSent	(url_.mailboxName(), i18n("Sent"),		QString::null);
	EmpathURL urlDrafts	(url_.mailboxName(), i18n("Drafts"),	QString::null);
	
	EmpathFolder * folder_inbox = new EmpathFolder(urlInbox);
	CHECK_PTR(folder_inbox);
		
	EmpathMaildir * box_inbox =
		new EmpathMaildir(path_, urlInbox);
	CHECK_PTR(box_inbox);
	
	EmpathFolder * folder_outbox = new EmpathFolder(urlOutbox);
	CHECK_PTR(folder_outbox);
	
	EmpathMaildir * box_outbox =
		new EmpathMaildir(path_, urlOutbox);
	CHECK_PTR(box_outbox);
	
	EmpathFolder * folder_drafts = new EmpathFolder(urlDrafts);
	CHECK_PTR(folder_drafts);
	
	EmpathMaildir * box_drafts =
		new EmpathMaildir(path_, urlDrafts);
	CHECK_PTR(box_drafts);
	
	EmpathFolder * folder_sent = new EmpathFolder(urlSent);
	CHECK_PTR(folder_sent);
	
	EmpathMaildir * box_sent =
		new EmpathMaildir(path_, urlSent);
	CHECK_PTR(box_sent);
	
	EmpathFolder * folder_trash = new EmpathFolder(urlTrash);
	CHECK_PTR(folder_trash);
	
	EmpathMaildir * box_trash =
		new EmpathMaildir(path_, urlTrash);
	CHECK_PTR(box_trash);
	
	folderList_.append(folder_trash);
	folderList_.append(folder_sent);
	folderList_.append(folder_drafts);
	folderList_.append(folder_outbox);
	folderList_.append(folder_inbox);
	
	boxList_.append(box_trash);
	boxList_.append(box_sent);
	boxList_.append(box_drafts);
	boxList_.append(box_outbox);
	boxList_.append(box_inbox);
	
	saveConfig();
}
	
	bool
EmpathMailboxMaildir::writeMessage(const EmpathURL & folder, const RMessage & m)
{
	empathDebug("writeMessage called with message " + QString().setNum(m.id()));
	// FIXME: Write writeNewMail method !
	QString s;// = writeNewMail(m);
	
	if (s.isEmpty()) return false;
	
//	m.setFilename(s);
	return true;
}

	bool
EmpathMailboxMaildir::newMail() const
{
	return false;
}

	void
EmpathMailboxMaildir::saveConfig()
{
	empathDebug("saveConfig() called - my name is " + url_.asString());
	KConfig * c = kapp->getConfig();
	c->setGroup(url_.mailboxName());
	
	c->writeEntry(EmpathConfig::KEY_MAILBOX_TYPE, type_);
	
	QStrList l;
	
	EmpathFolderListIterator it(folderList_);
	
	for (; it.current(); ++it)
		l.append(it.current()->url().folderPath());
	
	c->writeEntry(EmpathConfig::KEY_FOLDER_LIST, l);
	
	c->writeEntry(EmpathConfig::KEY_CHECK_MAIL, checkMail_);
	c->writeEntry(EmpathConfig::KEY_CHECK_MAIL_INTERVAL, checkMailInterval_);
}

	void
EmpathMailboxMaildir::readConfig()
{
	empathDebug("readConfig() called");
	KConfig * c = kapp->getConfig();
	empathDebug("Setting config group to \"" + url_.mailboxName() + "\""); 
	c->setGroup(url_.mailboxName());
	
	QStrList l;
	c->readListEntry(EmpathConfig::KEY_FOLDER_LIST, l);
	
	empathDebug("There are " + QString().setNum(l.count()) + " folders to load");
	
	QStrListIterator it(l);
	
	folderList_.clear();
	boxList_.clear();
	
	for (; it.current(); ++it) {
		
		empathDebug("Loading folder: \"" + QString(it.current()) + "\"");

		EmpathURL url(url_.mailboxName(), it.current(), QString::null);

		EmpathFolder * f = new EmpathFolder(url);
		CHECK_PTR(f);

		folderList_.append(f);
		
		EmpathMaildir * m = new EmpathMaildir(path_, url);
		CHECK_PTR(m);
		
		boxList_.append(m);
	}
	
	checkMail_ = c->readUnsignedNumEntry(EmpathConfig::KEY_CHECK_MAIL);
	checkMailInterval_ = c->readUnsignedNumEntry(EmpathConfig::KEY_CHECK_MAIL_INTERVAL);
}

	bool
EmpathMailboxMaildir::getMail()
{
	return false;
}

	void
EmpathMailboxMaildir::s_checkNewMail()
{
}
	
	void
EmpathMailboxMaildir::s_getNewMail()
{
}

	Q_UINT32
EmpathMailboxMaildir::sizeOfMessage(const EmpathURL & id)
{
	EmpathMaildir * m = _box(id);
	if (m == 0) return 0;
	return m->sizeOfMessage(id.messageID());
}

	QString
EmpathMailboxMaildir::plainBodyOfMessage(const EmpathURL & id)
{
	EmpathMaildir * m = _box(id);
	if (m == 0) return QString::null; 
	return m->plainBodyOfMessage(id.messageID());
}

	REnvelope *
EmpathMailboxMaildir::envelopeOfMessage(const EmpathURL & id)
{
	EmpathMaildir * m = _box(id);
	if (m == 0) return 0;
	return m->envelopeOfMessage(id.messageID());
}

	RMessage::MessageType
EmpathMailboxMaildir::typeOfMessage(const EmpathURL & id)
{	
	EmpathMaildir * m = _box(id);
	if (m == 0) return RMessage::BasicMessage;
	return m->typeOfMessage(id.messageID());
}

	void
EmpathMailboxMaildir::init()
{
	empathDebug("init() called");
	readConfig();
}

	RMessage *
EmpathMailboxMaildir::message(const EmpathURL & id)
{
	empathDebug("message(" + id.asString() + ") called");
	EmpathMaildir * m = _box(id);
	if (m == 0) {
		empathDebug("Can't find box \"" + id.asString() + "\"");
		return 0;
	}
	return m->message(id.messageID());
}

	bool
EmpathMailboxMaildir::removeMessage(const EmpathURL & id)
{
	EmpathMaildir * m = _box(id);
	if (m == 0) return 0;
	return m->removeMessage(id.messageID());
}

	void
EmpathMailboxMaildir::setPath(const QString & path)
{
	path_ = path;
}

	EmpathMaildir *
EmpathMailboxMaildir::_box(const EmpathURL & id)
{
	empathDebug("_box(" + id.asString() + ") called");
	EmpathMaildirListIterator it(boxList_);
	
	for (; it.current(); ++it) {
		empathDebug("Looking at \"" + it.current()->url().folderPath() + "\"");
		if (it.current()->url().folderPath() == id.folderPath())
			return it.current();
	}
	
	empathDebug("Can't find box with id \"" + id.asString() + "\"");
	return 0;
}

	bool
EmpathMailboxMaildir::addFolder(const EmpathURL & id)
{
	empathDebug("addFolder(" + id.asString() + ") called");
	EmpathFolder * f = new EmpathFolder(id);
	CHECK_PTR(f);
	
	EmpathMaildir * m = new EmpathMaildir(path_, id);
	CHECK_PTR(m);
	
	folderList_.append(f);
	boxList_.append(m);
	
	emit(updateFolderLists());
	return true;
}

	bool
EmpathMailboxMaildir::removeFolder(const EmpathURL & id)
{
	bool removedFromFolderList = false;
	bool removedFromMaildirList = false;
	
	EmpathFolderListIterator fit(folderList_);
	
	for (; fit.current(); ++fit)
		if (fit.current()->url().folderPath() == id.folderPath()) {
			folderList_.remove(fit.current());
			removedFromFolderList = true;
			break;
		}
	
	EmpathMaildirListIterator mit(boxList_);
	
	for (; mit.current(); ++mit)
		if (mit.current()->path() == id.folderPath()) {
			boxList_.remove(mit.current());
			removedFromMaildirList = true;
			break;
		}
	
	emit(updateFolderLists());
	return (removedFromFolderList && removedFromMaildirList);
}

