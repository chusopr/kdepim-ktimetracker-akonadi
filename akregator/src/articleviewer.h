/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef ARTICLEVIEWER_H
#define ARTICLEVIEWER_H

#include <khtml_part.h>
#include <qcolor.h>
#include <qfont.h>
#include <qpaintdevicemetrics.h>
#include "librss/librss.h"

using namespace RSS;

namespace Akregator
{
    class ArticleViewer : public KHTMLPart
    {
        Q_OBJECT
        public:
            ArticleViewer(QWidget* parent, const char* name);
            void openDefault();
            void show(Article);
        private:
            QString htmlHead() const;
            QString cssDefinitions() const;
            QFont m_bodyFont;
            QPaintDeviceMetrics m_metrics;
    };
}

#endif // ARTICLEVIEWER_H
