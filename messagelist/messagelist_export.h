/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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

#ifndef MESSAGELIST_MESSAGELIST_EXPORT_H
#define MESSAGELIST_MESSAGELIST_EXPORT_H

#include <kdemacros.h>

#ifndef MESSAGELIST_EXPORT
# if defined(MAKE_MESSAGELIST_LIB)
   /* We are building this library */
#  define MESSAGELIST_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define MESSAGELIST_EXPORT KDE_IMPORT
# endif
#endif

#endif
