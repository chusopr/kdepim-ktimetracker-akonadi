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

#ifndef EMPATHMESSAGEDESCRIPTIONLIST
#define EMPATHMESSAGEDESCRIPTIONLIST

// Qt includes
#include <qdict.h>

// Local includes
#include "RMM_MessageID.h"
#include "EmpathIndexRecord.h"

class EmpathFolder;
class EmpathMessageList;

typedef QDictIterator<EmpathIndexRecord> EmpathIndexIterator;

class EmpathIndex : public QDict<EmpathIndexRecord>
{
	public:
		
		EmpathIndex();
		~EmpathIndex();
		const EmpathIndexRecord * messageDescription(const RMessageID & id) const;
		
		void setFolder(EmpathFolder * parent) { folder_ = parent; }

		/**
		 * This hunts down the message in the local mailbox, and finds the
		 * corresponding message in the internal list, which is also destroyed.
		 */
		bool remove(const RMessageID & messageId);
		/**
		 * Count the number of messages stored.
		 */
		Q_UINT32 countUnread() const;

		/**
		 * Sync up the message list with the mailbox.
		 */
		void sync();

		/**
		 * Append all messages in a list of messages.
		 */
		void parseNewMail(EmpathMessageList * tempMessageList);

		QString asString() const;
		
		EmpathFolder * folder() { return folder_; }

		const char * className() const { return "EmpathMessageList"; }

	protected:
		
		virtual int compareItems(
			EmpathIndexRecord * item1,
			EmpathIndexRecord * item2);
		
		EmpathFolder * folder_;
		
		QDateTime mtime_;
};

#endif

