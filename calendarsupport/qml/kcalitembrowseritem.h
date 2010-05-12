/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef MESSAGEVIEWER_MESSAGEVIEWITEM_H
#define MESSAGEVIEWER_MESSAGEVIEWITEM_H

#include <mobile/lib/declarativeakonadiitem.h>
#include <QAbstractItemModel>

namespace Akonadi {
class IncidenceViewer;

namespace KCal {

class KCalItemBrowserItem : public DeclarativeAkonadiItem
{
  Q_OBJECT
  Q_PROPERTY( QObject* attachmentModel READ attachmentModel NOTIFY attachmentModelChanged )
  public:
    explicit KCalItemBrowserItem( QDeclarativeItem *parent = 0 );

    qint64 itemId() const;
    void setItemId(qint64 id);

    QObject *attachmentModel() const;

  signals:
    void attachmentModelChanged();

  private:
    IncidenceViewer *m_viewer;
};

}
}

#endif
