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

#include <RMM_ContentType.h>
#include <RMM_Token.h>

RContentType::RContentType()
	:	RHeaderBody()
{
	rmmDebug("ctor");
}

RContentType::RContentType(const RContentType & cte)
	:	RHeaderBody(cte)
{
	rmmDebug("ctor");
	assembled_	= false;
}

RContentType::~RContentType()
{
	rmmDebug("dtor");
}

	RContentType &
RContentType::operator = (const RContentType & ct)
{
	rmmDebug("operator =");
    if (this == &ct) return *this; // Don't do a = a.

	type_			= ct.type_;
	subType_		= ct.subType_;
	parameterList_	= ct.parameterList_;
	
	RHeaderBody::operator = (ct);

	return *this;
}

	void
RContentType::parse()
{
	if (parsed_) return;

	QCString ts;
	int i = strRep_.find(";");
	
	if (i == -1)
	
		ts = strRep_;
	
	else {
	
		ts = strRep_.left(i);
		parameterList_ = strRep_.right(strRep_.length() - i + 1);
		parameterList_.parse();
	}
	
	int slash = ts.find('/');
	
	if (slash == -1) {
		rmmDebug("Invalid Content-Type");
		return;
	}
	
	type_		= ts.left(slash).stripWhiteSpace();
	subType_	= ts.right(ts.length() - slash - 1).stripWhiteSpace();
	
	parsed_		= true;
	assembled_	= false;
}

	void
RContentType::assemble()
{
	if (assembled_) return;
	
	strRep_ = type_ + "/" + subType_;
	
	parameterList_.assemble();
	
	if (parameterList_.count() == 0) return;
	
	strRep_ += QCString(";\n    ");
	
	strRep_ += parameterList_.asString();

	rmmDebug("assembled to: \"" + strRep_ + "\"");
	
	assembled_ = true;
}

	void
RContentType::createDefault()
{
	rmmDebug("createDefault() called");
	type_ = "text";
	subType_ = "plain";
	parsed_		= true;
	assembled_	= false;
}

	void
RContentType::setType(const QCString & t)
{
	parse();
	type_ = t;
	assembled_	= false;
}

	void
RContentType::setSubType(const QCString & t)
{
	parse();
	subType_ = t;
	assembled_	= false;
}

	void
RContentType::setParameterList(RParameterList & p)
{
	parse();
	parameterList_ = p;
	assembled_	= false;
}
	
	QCString
RContentType::type()
{
	parse();
	return type_;
}

	QCString
RContentType::subType()
{
	parse();
	return subType_;
}
	
	RParameterList &
RContentType::parameterList()	
{
	parse();
	return parameterList_;
}

