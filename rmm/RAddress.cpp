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

#include <qstring.h>

#include <RMM_Enum.h>
#include <RMM_Address.h>
#include <RMM_Group.h>
#include <RMM_Mailbox.h>

RAddress::RAddress()
	:	mailbox_(0),
		group_(0)
{
	rmmDebug("ctor");
}

RAddress::RAddress(const RAddress & addr)
	:	RHeaderBody(),
		mailbox_(addr.mailbox_),
		group_(addr.group_)
{
	rmmDebug("ctor");
}

RAddress::RAddress(const QCString & addr)
	:	RHeaderBody(addr.data()),
		mailbox_(0),
		group_(0)
{
	rmmDebug("ctor");
}

RAddress::~RAddress()
{
	rmmDebug("dtor");

        delete mailbox_;
        delete group_;

	mailbox_	= 0;
	group_		= 0;
}

	const RAddress &
RAddress::operator = (const RAddress & addr)
{
	rmmDebug("operator =");
    if (this == &addr) return *this; // Don't do a = a.

        delete mailbox_;
        delete group_;

	mailbox_	= 0;
	group_		= 0;

	if (addr.mailbox_ != 0)
		mailbox_ = new RMailbox(*(addr.mailbox_));
	else
		group_ = new RGroup(*(addr.group_));

	RHeaderBody::operator = (addr);

	return *this;
}

	bool
RAddress::isValid() const
{
	return (group_->isValid() && mailbox_->isValid());
}

	RGroup *
RAddress::group()
{
	return group_;
}

	RMailbox *
RAddress::mailbox()
{
	return mailbox_;
}

	void
RAddress::parse()
{
	rmmDebug("parse() called");

        delete mailbox_;
        delete group_;

	mailbox_	= 0;
	group_		= 0;

	rmmDebug("Done my deletions. Should be safe if ctors work");

	QCString s = strRep_.stripWhiteSpace();

	// RFC822: group: phrase ":" [#mailbox] ";"
	// -> If a group, MUST end in ";".

	if (s.right(1) == ";") { // This is a group !

		rmmDebug("I'm a group.");

		group_ = new RGroup;
		CHECK_PTR(group_);
		if (!group_) rmmDebug("!mailbox!");
		group_->set(s);
		group_->parse();
		dirty_ = group_->isDirty() ? true : dirty_;

	} else {

		rmmDebug("I'm a mailbox.");

		mailbox_ = new RMailbox;
		CHECK_PTR(mailbox_);
		if (!mailbox_) rmmDebug("!mailbox!");
		rmmDebug(s);
		mailbox_->set(s);
		mailbox_->parse();
		dirty_ = mailbox_->isDirty() ? true : dirty_;
	}
	rmmDebug("parsed");
}

	void
RAddress::assemble()
{
	rmmDebug("assemble() called");

	if (mailbox_ != 0) {

		mailbox_->assemble();
		strRep_ = mailbox_->asString();

	} else if (group_ != 0) {

		group_->assemble();
		strRep_ = group_->asString();

	} else {
			strRep_ = "foo@bar";
	}
}

	void
RAddress::createDefault()
{
	rmmDebug("createDefault() called");
	if (mailbox_ == 0 && group_ == 0) {
		rmmDebug("I have no mailbox or group yet");
		mailbox_ = new RMailbox;
		mailbox_->createDefault();
	}
	else if (mailbox_ == 0) {
		rmmDebug("I have no mailbox");
		group_ = new RGroup;
		group_->createDefault();
	} else {
		rmmDebug("I have no group");
		mailbox_ = new RMailbox;
		mailbox_->createDefault();
	}
}

