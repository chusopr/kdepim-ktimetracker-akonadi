/*
 *  kalarm.h  -  global header file
 *  Program:  kalarm
 *  (C) 2001 - 2005 by David Jarvie <software@astrojar.org.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KALARM_H
#define KALARM_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define KALARM_VERSION "1.3.1"

#include <kdeversion.h>
extern int  marginKDE2;   // KDE2 compatibility

#define OLD_DCOP    // retain DCOP pre-1.2 compatibility

#endif // KALARM_H

