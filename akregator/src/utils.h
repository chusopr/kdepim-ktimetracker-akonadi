/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_UTILS_H
#define AKREGATOR_UTILS_H

class QString;

namespace Akregator {

class Utils 
{
    public:
    /** removes HTML/XML tags (everything between < and >, that is) from a string.  "<p><strong>foo</strong> bar</p>" becomes "foo bar" */
    static QString stripTags(const QString& str);

    // /** strips tags, resolves entities and replaces <br/> by new lines */
    //static QString htmlToPlainText(const QString& str);
};

}

#endif // AKREGATOR_UTILS_H
