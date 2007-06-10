/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef AKONADI_EXPORT_H
#define AKONADI_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef AKONADI_SERVER_EXPORT
# if defined(MAKE_AKONADIPRIVATE_LIB)
   /* We are building this library */
#  define AKONADI_SERVER_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define AKONADI_SERVER_EXPORT KDE_IMPORT
# endif
#endif

#ifndef AKONADI_KABC_EXPORT
# if defined(MAKE_KABCAKONADI_LIB)
   /* We are building this library */
#  define AKONADI_KABC_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define AKONADI_KABC_EXPORT KDE_IMPORT
# endif
#endif

#ifndef AKONADI_KMIME_EXPORT
# if defined(MAKE_KMIMEAKONADI_LIB)
   /* We are building this library */
#  define AKONADI_KMIME_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define AKONADI_KMIME_EXPORT KDE_IMPORT
# endif
#endif

//TODO: remove with searchprovider stuff that is going away soon
#ifndef AKONADISEARCHPROVIDER_EXPORT
# if defined(MAKE_AKONADISEARCHPROVIDER_LIB)
   /* We are building this library */
#  define AKONADISEARCHPROVIDER_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define AKONADISEARCHPROVIDER_EXPORT KDE_IMPORT
# endif
#endif

#endif
