#ifndef FINDBARSOURCEVIEW_H
#define FINDBARSOURCEVIEW_H
/* Copyright (C) 2011 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "findbarbase.h"

class KTextBrowser;

namespace MessageViewer
{
class FindBarSourceView : public FindBarBase
{
  Q_OBJECT

  public:
    explicit FindBarSourceView( KTextBrowser * view, QWidget * parent = 0 );
    virtual ~FindBarSourceView();

  private:
    FindBarSourceView( QWidget *parent) { Q_UNUSED(parent); }
    void clearSelections();
    void searchText( bool backward, bool isAutoSearch );
    void updateHighLight(bool);
    void updateSensitivity(bool);

  private:
    KTextBrowser * m_view;
};
}



#endif /* FINDBARSOURCEVIEW_H */

