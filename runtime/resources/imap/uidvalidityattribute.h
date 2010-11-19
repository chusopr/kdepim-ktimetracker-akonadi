/*
    Copyright (C) 2008 Omat Holding B.V. <info@omat.nl>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef UIDVALIDITYATTRIBUTE_H
#define UIDVALIDITYATTRIBUTE_H

#include <akonadi/attribute.h>

class UidValidityAttribute : public Akonadi::Attribute
{
public:
    UidValidityAttribute( int uidvalidity = 0 );
    void setUidValidity( int uidvalidity );
    int uidValidity() const;
    virtual QByteArray type() const;
    virtual Attribute* clone() const;
    virtual QByteArray serialized() const;
    virtual void deserialize( const QByteArray &data );

private:
    int mUidValidity;
};

#endif
