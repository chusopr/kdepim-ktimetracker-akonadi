/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef TIMESPANVIEW_H
#define TIMESPANVIEW_H

#include <qwidget.h>

#include <libkcal/event.h>

class QSplitter;
class QListView;
class LineView;
class TimeLine;

class TimeSpanView : public QWidget
{
    Q_OBJECT
  public:
    TimeSpanView( QWidget *parent=0, const char *name=0 );
    virtual ~TimeSpanView();
    
    void addItem( KCal::Event * );

    QValueList<int> splitterSizes();
    void setSplitterSizes( QValueList<int> );
    
    void clear();
    
    void setDateRange( const QDateTime &start, const QDateTime &end );
    
    QDateTime startDateTime();
    QDateTime endDateTime();

  public slots:
    void updateView();

    void zoomIn();
    void zoomOut();
    void centerView();
    
  signals:
    void dateRangeChanged();
    
  private:
    QSplitter *mSplitter;
    QListView *mList;
    TimeLine *mTimeLine;
    LineView *mLineView;
    
    QDateTime mStartDate;
    QDateTime mEndDate;
    int mSecsPerPixel;
};

#endif
