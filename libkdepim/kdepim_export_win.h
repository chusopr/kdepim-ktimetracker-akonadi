/*
    This file is part of libkdepim.
    Copyright (c) 2006 Allen Winter <winter@kde.org>

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

#ifndef KHOLIDAYS_EXPORT
# ifdef MAKE_KHOLIDAYS_LIB
#  define KHOLIDAYS_EXPORT KDE_EXPORT
# else
#  define KHOLIDAYS_EXPORT KDE_IMPORT
# endif
#endif

#ifndef AKONADI_EXPORT
# ifdef MAKE_AKONADI_LIB
#  define AKONADI_EXPORT KDE_EXPORT
# else
#  define AKONADI_EXPORT KDE_IMPORT
# endif
#endif

#ifndef AKONADI_RESOURCES_EXPORT
# ifdef MAKE_AKONADI_RESOURCES_LIB
#  define AKONADI_RESOURCES_EXPORT KDE_EXPORT
# else
#  define AKONADI_RESOURCES_EXPORT KDE_IMPORT
# endif
#endif

#ifndef AKONADICOMPONENTS_EXPORT
# ifdef MAKE_AKONADICOMPONENTS_LIB
#  define AKONADICOMPONENTS_EXPORT KDE_EXPORT
# else
#  define AKONADICOMPONENTS_EXPORT KDE_IMPORT
# endif
#endif

#ifndef AKONADISEARCHPROVIDER_EXPORT
# ifdef MAKE_AKONADISEARCHPROVIDER_LIB
#  define AKONADISEARCHPROVIDER_EXPORT KDE_EXPORT
# else
#  define AKONADISEARCHPROVIDER_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KDEPIM_EXPORT
# ifdef MAKE_KDEPIM_LIB
#  define KDEPIM_EXPORT KDE_EXPORT
# else
#  define KDEPIM_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KXMLCOMMON_EXPORT
# ifdef MAKE_KXMLCOMMON_LIB
#  define KXMLCOMMON_EXPORT KDE_EXPORT
# else
#  define KXMLCOMMON_EXPORT KDE_IMPORT
# endif
#endif

#ifndef QGPGME_EXPORT
# ifdef MAKE_QGPGME_LIB
#  define QGPGME_EXPORT KDE_EXPORT
# else
#  define QGPGME_EXPORT KDE_IMPORT
# endif
#endif
