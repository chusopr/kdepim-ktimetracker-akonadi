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

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "HeapPtr.h"
#include "EmpathFilterEventHandler.h"
#include "Empath.h"
#include "EmpathConfig.h"
#include "EmpathDefines.h"

EmpathFilterEventHandler::EmpathFilterEventHandler()
	:	actionType_(MoveFolder)
{
	empathDebug("ctor");
	moveCopyFolder_ = EmpathURL("empath://local/orphaned");
}

EmpathFilterEventHandler::~EmpathFilterEventHandler()
{
	empathDebug("dtor");
}

	ActionType
EmpathFilterEventHandler::actionType() const
{
	return actionType_;
}

	EmpathURL
EmpathFilterEventHandler::moveOrCopyFolder() const
{
	return moveCopyFolder_;
}

	const QString &
EmpathFilterEventHandler::forwardAddress() const
{
	return forwardAddress_;
}
		
	void
EmpathFilterEventHandler::setMoveFolder(const EmpathURL & folder)
{
	empathDebug("setMoveFolder(" + folder.asString() + ")");
	actionType_ = MoveFolder;
	moveCopyFolder_ = folder;
}

	void
EmpathFilterEventHandler::setCopyFolder(const EmpathURL & folder)
{
	empathDebug("setCopyFolder(" + folder.asString() + ")");
	actionType_ = CopyFolder;
	moveCopyFolder_ = folder;
}

	void
EmpathFilterEventHandler::setDelete()
{
	empathDebug("setDelete");
	actionType_ = Delete;
}

	void
EmpathFilterEventHandler::setIgnore()
{
	empathDebug("setIgnore");
	actionType_ = Ignore;
}

	void
EmpathFilterEventHandler::setForward(const QString & address)
{
	empathDebug("setForward(" + QString(address) + ")");
	actionType_ = Forward;
	forwardAddress_ = address;
}

	void
EmpathFilterEventHandler::handleMessage(const EmpathURL & id)
{
	empathDebug("handleMessage() called");
	EmpathFolder * folder = empath->folder(id);
	if (folder == 0) return;
	
	switch (actionType_) {

		case MoveFolder:
			{
				EmpathFolder * mcf = empath->folder(moveCopyFolder_);
				if (mcf == 0) {
					empathDebug("Folder we'd be moving to is 0");
					return;
				}

				empathDebug("Moving message " + QString(id.asString()) +
					" to " + moveCopyFolder_.withoutMessageID().asString());

				RMessage * r = empath->message(id);
				if (r == 0) return;
				
				HeapPtr<RMessage> m(r);
				
				if (!mcf->writeMessage(*m)) return;
				
				empath->removeMessage(id);
			}
			
			break;
			
		case CopyFolder:
			{
				EmpathFolder * mcf = empath->folder(moveCopyFolder_);
				if (mcf == 0) {
					empathDebug("Folder we'd be copying to is 0");
					return;
				}

				empathDebug("Copying message " + QString(id.asString()) +
					" to " + mcf->url().asString());

				RMessage * r = empath->message(id);
				if (r == 0) return;
				
				HeapPtr<RMessage> m(r);
				
				mcf->writeMessage(*m);
			}
		
			break;

		case Forward:
			{
				empathDebug("Forwarding message " + QString(id.asString()));
				
				RMessage * m(empath->message(id));
				
				if (m == 0) return;
				
				empath->forward(m);
			}
			
			break;
			
		case Delete:
			
			empathDebug("Deleting message " + QString(id.asString()));

			if (!empath->removeMessage(id)) {

				empathDebug("Couldn't remove message " +
					id.asString());
			}
			
			break;
	
		case Ignore:
			
			empathDebug("Ignoring message " + id.asString());

			break;
			
		default:
			
			empathDebug("Don't know what to do with " +
				id.asString());
			
			break;
	}
}

	bool
EmpathFilterEventHandler::load(Q_UINT32 filterID)
{
	KConfig * config = kapp->getConfig();
	config->setGroup(GROUP_FILTER + QString().setNum(filterID));
	actionType_ = (ActionType)config->readNumEntry(KEY_FILTER_EVENT_HANDLER_TYPE);

	switch (actionType_) {

		case MoveFolder:

			setMoveFolder(
					EmpathURL(config->readEntry(KEY_FILTER_EVENT_HANDLER_FOLDER)));
			break;
			
		case CopyFolder:

			setCopyFolder(
					EmpathURL(config->readEntry(KEY_FILTER_EVENT_HANDLER_FOLDER)));
			break;

		case Forward:

			setForward(config->readEntry(KEY_FILTER_EVENT_HANDLER_FOLDER));
			break;

		case Delete:
		case Ignore:
			break;
		default:
			return false;
			break;
	}
	return true;
}

	void
EmpathFilterEventHandler::save(Q_UINT32 filterID)
{
	empathDebug("save(" + QString().setNum(filterID) + ") called");
	KConfig * config = kapp->getConfig();
	config->setGroup(GROUP_FILTER + QString().setNum(filterID));
	config->writeEntry(KEY_FILTER_EVENT_HANDLER_TYPE, (int)actionType_);

	switch (actionType_) {

		case MoveFolder:
		case CopyFolder:
			
			config->writeEntry(KEY_FILTER_EVENT_HANDLER_FOLDER,
				moveCopyFolder_.asString());
			
			break;
		
		case Forward:
		
			config->writeEntry(KEY_FILTER_EVENT_HANDLER_ADDRESS, forwardAddress_);
		
			break;
		
		case Delete:
		case Ignore:
		default:
			break;
	}
}

	QString
EmpathFilterEventHandler::description() const
{
	empathDebug("description() called");
	QString action;
	
	switch (actionType_) {
		
		case MoveFolder:
			action = i18n("Move to folder");
			action += " ";
			action += moveCopyFolder_.mailboxName();
			action += "/";
			action += moveCopyFolder_.folderPath();
			break;
			
		case CopyFolder:
			action = i18n("Copy to folder");
			action += " ";
			action += moveCopyFolder_.mailboxName();
			action += "/";
			action += moveCopyFolder_.folderPath();
			break;

		case Delete:
			action = i18n("Delete");
			break;
			
		case Ignore:
			action = i18n("Ignore");
			break;
			
		case Forward:
			action = i18n("Forward to");
			action += " ";
			action += forwardAddress_;
			break;
			
		default:
			action = i18n("No action defined");
			break;
	}
	
	return action;
}

