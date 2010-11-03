/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef FILTERCONFIGWIDGET_H
#define FILTERCONFIGWIDGET_H

#include <QWidget>
#include <QGraphicsProxyWidget>

namespace MailCommon {
class MailFilter;
class SearchPatternEdit;
class FilterActionWidgetLister;
}

class Ui_FilterConfigWidget;

class FilterConfigWidget : public QWidget
{
  Q_OBJECT
public:
    explicit FilterConfigWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~FilterConfigWidget();

    void loadFilter( MailCommon::MailFilter* filter );
    void save();
    void newFilter();
    void deleteFilter( int filterIndex );
    void renameFilter( int filterIndex );
    void moveUpFilter( int filterIndex );
    void moveDownFilter( int filterIndex );

private:
  Ui_FilterConfigWidget *mUi;
  MailCommon::MailFilter *mFilter;
  MailCommon::SearchPatternEdit *mPatternEdit;
  MailCommon::FilterActionWidgetLister *mActionLister;
};

class DeclarativeFilterConfigWidget : public QGraphicsProxyWidget
{
  Q_OBJECT

  public:
    explicit DeclarativeFilterConfigWidget( QGraphicsItem *parent = 0 );
    ~DeclarativeFilterConfigWidget();

  public Q_SLOTS:
    void loadFilter( int filterIndex );
    void save();
    void newFilter();
    void deleteFilter( int filterIndex );
    void renameFilter( int filterIndex );
    void moveUpFilter( int filterIndex );
    void moveDownFilter( int filterIndex );
    
  private:
    FilterConfigWidget *mFilterConfigWidget;
};

#endif // FILTERCONFIGWIDGET_H