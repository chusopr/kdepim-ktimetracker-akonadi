/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. <gamaral@amaral.com.mx>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#ifndef LSN_GLOBAL_H
#define LSN_GLOBAL_H

#include <StickyNotes/config-stickynotes.h>

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef LSN_EXPORT
# if defined(MAKE_STICKYNOTES_LIB)
   /* We are building this library */
#  define LSN_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define LSN_EXPORT KDE_IMPORT
# endif
#endif

# ifndef LSN_EXPORT_DEPRECATED
#  define LSN_EXPORT_DEPRECATED KDE_DEPRECATED LSN_EXPORT
# endif

#endif // !LSN_GLOBAL_H

