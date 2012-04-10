/*
  This file is part of KTnef.

  Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef KTNEFVIEW_H
#define	KTNEFVIEW_H

#include <QTreeWidget>

namespace KTnef {
  class KTNEFAttach;
}
using namespace KTnef;

class KTNEFView : public QTreeWidget
{
  Q_OBJECT

  public:
    KTNEFView( QWidget *parent = 0 );
    ~KTNEFView();

    void setAttachments( QList<KTNEFAttach *> list );
    QList<KTNEFAttach *> getSelection();

  signals:
    void dragRequested( const QList<KTNEFAttach *> &list );

  protected:
    void resizeEvent( QResizeEvent *e );
    void startDrag( Qt::DropActions dropAction );

  private slots:
    void adjustColumnWidth();

  private:
    QList<KTNEFAttach *> mAttachments;
};

#endif
