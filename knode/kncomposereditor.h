/**
 * kncomposereditor.h
 *
 * Copyright (C)  2008 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifndef KNCOMPOSEREDITOR_H
#define KNCOMPOSEREDITOR_H

#include <kmeditor.h>
using namespace KPIM;

class KNComposerEditor : public KMeditor
{
    Q_OBJECT

  public:
    /**
     * Constructs a KNComposerEditor object.
     */
    explicit KNComposerEditor( QWidget *parent );

    virtual ~KNComposerEditor();

     virtual void changeHighlighterColors(KPIM::KEMailQuotingHighlighter * highlighter);

};

#endif
