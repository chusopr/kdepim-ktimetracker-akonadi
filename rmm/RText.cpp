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

#include <RMM_Text.h>

RText::RText()
	:	RHeaderBody()
{
	rmmDebug("ctor");
}

RText::RText(const RText & r)
	:	RHeaderBody(r)
{
	rmmDebug("ctor with RText(" + r.asString() + ")");
	rmmDebug("my strRep becomes \"" + strRep_ + "\"");
}

RText::RText(const QCString & s)
   	:	RHeaderBody(s)
{
	rmmDebug("ctor with \"" + s + "\"");
}

RText::~RText()
{
	rmmDebug("dtor");
}

	RText &
RText::operator = (const RText & r)
{
    if (this == &r) return *this; // Avoid a = a
	RHeaderBody::operator = (r);
	return *this;
}

	void
RText::parse()
{
	rmmDebug("parse() called");
}

	void
RText::assemble()
{
	rmmDebug("assemble() called");
	rmmDebug("strRep_ = \"" + strRep_ + "\"");
}

	void
RText::createDefault()
{
	rmmDebug("createDefault() called");
}

