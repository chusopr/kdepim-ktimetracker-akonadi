/*
    knapplication.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNAPPLICATION_H
#define KNAPPLICATION_H

#include <kuniqueapplication.h>

class KNApplication : public KUniqueApplication
{

  public:
    KNApplication(): KUniqueApplication() { };


    /** Create new instance of KNode. Make the existing
        main window active if KNode is already running */
    int newInstance();

};
#endif
