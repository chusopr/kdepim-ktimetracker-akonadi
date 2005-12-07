/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef LIBSYNDICATION_RSS2_CATEGORY_H
#define LIBSYNDICATION_RSS2_CATEGORY_H

#include <ksharedptr.h>

class QDomDocument;
class QDomElement;
class QString;

namespace LibSyndication {
namespace RSS2 {

    class Category
    {
        public:

        static const Category& null();

        static Category fromXML(const QDomElement& e);

        Category();
        Category(const Category& other);

        virtual ~Category();

        Category& operator=(const Category& other);
        bool operator==(const Category& other) const;

        QString category() const;

        QString domain() const;

        bool isNull() const;

        QString debugInfo() const;

        private:

        Category(const QString& category, const QString& domain);

        static Category* m_null;

        class CategoryPrivate;
        KSharedPtr<CategoryPrivate> d;

    };
} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_CATEGORY_H
