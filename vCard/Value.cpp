/*
	libvcard - vCard parsing library for vCard version 3.0

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

#ifdef __GNUG__
# pragma implementation "Value.h"
#endif

#include <Entity.h>
#include <Value.h>

namespace VCARD
{

Value::Value()
	:	Entity()
{
}

Value::Value(const Value & x)
	:	Entity(x)
{
}

Value::Value(const QCString & s)
	:	Entity(s)
{
}

	Value &
Value::operator = (Value & x)
{
	if (*this == x) return *this;

	Entity::operator = (x);
	return *this;
}

	Value &
Value::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
Value::operator == (Value & x)
{
	x.parse();
	return false;
}

Value::~Value()
{
}

	void
Value::_parse()
{
}

	void
Value::_assemble()
{
}

}

