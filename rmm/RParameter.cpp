/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
# pragma implementation "RMM_Parameter.h"
#endif

#include <qcstring.h>

#include <RMM_Defines.h>
#include <RMM_Token.h>
#include <RMM_Parameter.h>

using namespace RMM;

RParameter::RParameter()
    :    RMessageComponent()
{
    // Empty.
}

RParameter::RParameter(const RParameter & p)
    :   RMessageComponent(p),
        attribute_(p.attribute_),
        value_(p.value_)
{
    // Empty.
}

RParameter::RParameter(const QCString & s)
    :    RMessageComponent(s)
{
    // Empty.
}

RParameter::~RParameter()
{
    // Empty.
}

    RParameter &
RParameter::operator = (const QCString & s)
{
    RMessageComponent::operator = (s);
    return *this;
}

    RParameter &
RParameter::operator = (const RParameter & p)
{
    if (this == &p) return *this; // Don't do a = a.
    
    attribute_ = p.attribute_;
    value_ = p.value_;

    RMessageComponent::operator = (p);
    return *this;
}

    bool
RParameter::operator == (RParameter & p)
{
    parse();
    p.parse();

    return (
        attribute_    == p.attribute_ &&
        value_        == p.value_);
}
    
    void
RParameter::_parse()   
{
    int split = strRep_.find('=');
    
    if (split == -1) {
        rmmDebug("Invalid parameter");
        return;
    }
    
    attribute_    = strRep_.left(split).stripWhiteSpace();
    value_        = strRep_.right(strRep_.length() - split - 1).stripWhiteSpace();
    
    rmmDebug("attribute == \"" + attribute_ + "\"");
    rmmDebug("value     == \"" + value_ + "\"");
}

    void
RParameter::_assemble()
{
    strRep_ = attribute_ + "=" + value_;
}

    void
RParameter::createDefault()
{
    attribute_ = value_ = "";
}

    QCString
RParameter::attribute()
{
    parse();
    return attribute_;
}

    QCString
RParameter::value()
{
    parse();
    return value_;
}

    void
RParameter::setAttribute(const QCString & attribute)
{
    parse();
    attribute_ = attribute;
}
    void
RParameter::setValue(const QCString & value)    
{
    parse();
    value_ = value;
}

// vim:ts=4:sw=4:tw=78
