/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef PAGEVIEWER_H
#define PAGEVIEWER_H

#include "viewer.h"

namespace Akregator
{
    class PageViewer : public Viewer
    {
        Q_OBJECT
        public:
            PageViewer(QWidget* parent, const char* name);
    };
}

#endif // PAGEVIEWER_H
