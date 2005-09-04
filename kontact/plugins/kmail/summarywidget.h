/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef SUMMARYWIDGET_H
#define SUMMARYWIDGET_H

#include <qmap.h>
#include <qtimer.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QLabel>
#include <QGridLayout>
#include <QEvent>
#include <Q3PtrList>

#include <dcopobject.h>
#include <kurllabel.h>
#include <kparts/part.h>

#include "plugin.h"
#include "summary.h"

class QGridLayout;
class QString;

class SummaryWidget : public Kontact::Summary, public DCOPObject
{
  Q_OBJECT
  K_DCOP

  public:
    SummaryWidget( Kontact::Plugin *plugin, QWidget *parent, const char *name = 0 );

    int summaryHeight() const { return 1; }
    QStringList configModules() const;

  k_dcop_hidden:
    void slotUnreadCountChanged();

  protected:
    virtual bool eventFilter( QObject *obj, QEvent* e );

  public slots:
    virtual void updateSummary( bool force );

  private slots:
    void selectFolder( const QString& );

  private:
    void updateFolderList( const QStringList& folders );

    Q3PtrList<QLabel> mLabels;
    QGridLayout *mLayout;
    Kontact::Plugin *mPlugin;
    int mTimeOfLastMessageCountUpdate;
};

#endif
