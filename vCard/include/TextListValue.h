/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
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
# pragma interface "TextListValue.h"
#endif

#ifndef  TEXTLISTVALUE_H
#define  TEXTLISTVALUE_H

#include <qstrlist.h>

#include <qcstring.h>

#include <Value.h>

namespace VCARD
{

class TextListValue : public Value
{
	
#include "TextListValue-generated.h"
	
	unsigned int numValues();
	QCString value(unsigned int);
	
	private:
		
		QStrList valueList_;
};

}

#endif
