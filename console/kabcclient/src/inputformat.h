//
//  Copyright (C) 2005 Kevin Krammer <kevin.krammer@gmx.at>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef INPUTFORMAT_H
#define INPUTFORMAT_H

// standard includes
#include <istream>

// forward declarations
class QTextCodec;

namespace KABC
{
    class Addressee;
};

class InputFormat
{
public:
    virtual ~InputFormat() {}

    virtual QString description() const { return QString::null; }

    virtual KABC::Addressee readAddressee(std::istream& stream) = 0;

    virtual bool setOptions(const QCString& options) = 0;

    virtual QString optionUsage() const { return QString::null; }

    virtual bool setCodec(QTextCodec* codec) = 0;
};

#endif

// End of file
