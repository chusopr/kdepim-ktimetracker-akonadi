/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
 **
 ** This file may be distributed and/or modified under the terms of the
 ** GNU General Public License version 2 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.
 **
 ** Licensees holding valid commercial KD Gantt licenses may use this file in
 ** accordance with the KD Gantt Commercial License Agreement provided with
 ** the Software.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** See http://www.kdab.net/kdgantt for
 **   information about KD Gantt Commercial License Agreements.
 **
 ** Contact info@kdab.net if any conditions of this
 ** licensing are not clear to you.
 **
 **********************************************************************/
#ifndef KDGANTTABSTRACTROWCONTROLLER_H
#define KDGANTTABSTRACTROWCONTROLLER_H

#include <QPair>
#include "kdganttglobal.h"

class QModelIndex;

namespace KDGantt {
    class KDGANTT_EXPORT AbstractRowController {
    public:
        AbstractRowController();
        virtual ~AbstractRowController();

        virtual int headerHeight() const = 0;
        virtual int maximumItemHeight() const = 0;
	virtual int totalHeight() const = 0;

        virtual bool isRowVisible( const QModelIndex& idx ) const = 0;
	virtual bool isRowExpanded( const QModelIndex& idx ) const = 0;
        virtual Span rowGeometry( const QModelIndex& idx ) const = 0;


        virtual QModelIndex indexAt( int height ) const = 0;
        virtual QModelIndex indexAbove( const QModelIndex& idx ) const = 0;
        virtual QModelIndex indexBelow( const QModelIndex& idx ) const = 0;
    };
}

#endif /* KDGANTTABSTRACTROWCONTROLLER_H */

