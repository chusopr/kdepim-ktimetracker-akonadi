/**
 * kmstylelistselectaction.h
 *
 * Copyright (C)  2007  Laurent Montel <montel@kde.org>
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

#ifndef KMSTYLELISTSELECTACTION_H
#define KMSTYLELISTSELECTACTION_H

#include "kdepim_export.h"
#include <KSelectAction>
#include <qtextformat.h>

namespace KPIM {

class KDEPIM_EXPORT KMStyleListSelectAction : public KSelectAction
{
    Q_OBJECT

  public:
    /**
     * Constructs a KMStyleListSelectAction
     */
    explicit KMStyleListSelectAction( const QString& text, QWidget *parent = 0 );

    /**
     * Sets the current item of the action the the specified style.
     * Call this whenever the cursor position of the editor changes and therefore
     * the list style might have changed.
     *
     * @param style the new current style of the action
     */
    void setCurrentStyle( QTextListFormat::Style style );

  Q_SIGNALS:
    //emit style will be applyed.
    void applyStyle( QTextListFormat::Style );
  protected Q_SLOTS:
    void slotChangeStyle( int );
  protected:
    void init();
};

}

#endif

