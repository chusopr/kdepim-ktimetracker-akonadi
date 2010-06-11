/*  -*- c++ -*-
    stl_util.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2004 Marc Mutz <mutz@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KDEPIM__KMAIL__STL_UTIL_H__
#define __KDEPIM__KMAIL__STL_UTIL_H__

#include <algorithm>

template <typename T>
struct DeleteAndSetToZero {
  void operator()( const T * & t ) { delete t; t = 0; }
};

template <typename T>
static inline void deleteAll( T & c ) {
  for ( typename T::iterator it = c.begin() ; it != c.end() ; ++it ) {
    delete *it ; *it = 0;
  }
}

namespace kdtools {

    template <typename Iterator, typename UnaryPredicate>
    bool any( Iterator first, Iterator last, UnaryPredicate p )
    {
        while ( first != last )
            if ( p( *first ) )
                return true;
            else
                ++first;
        return false;
    }

} // namespace kdtools

#endif // __KDEPIM__KMAIL__STL_UTIL_H__
