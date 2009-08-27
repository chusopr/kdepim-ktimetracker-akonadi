/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#ifndef __MESSAGELIST_CORE_CONFIGUREAGGREGATIONSDIALOG_H__
#define __MESSAGELIST_CORE_CONFIGUREAGGREGATIONSDIALOG_H__

#include <KDialog>

#include <QListWidget>

#include <messagelist/messagelist_export.h>

class QPushButton;

namespace MessageList
{

namespace Core
{

class Manager;

/**
 * The dialog used for configuring MessageList::Aggregation sets.
 *
 * This is managed by MessageList::Manager. Take a look at it first
 * if you want to display this dialog.
 */
class MESSAGELIST_EXPORT ConfigureAggregationsDialog : public KDialog
{
  friend class Manager;

  Q_OBJECT
protected:
  ConfigureAggregationsDialog( QWidget *parent = 0 );
  ~ConfigureAggregationsDialog();

public:
  /**
   * The one and only ConfigureAggregationsDialog. May be 0.
   *
   * See MessageList::Manager if you want to display this dialog.
   */
  static ConfigureAggregationsDialog * instance();

private:
  Q_PRIVATE_SLOT(d, void aggregationListCurrentItemChanged( QListWidgetItem*, QListWidgetItem* ))
  Q_PRIVATE_SLOT(d, void newAggregationButtonClicked())
  Q_PRIVATE_SLOT(d, void cloneAggregationButtonClicked())
  Q_PRIVATE_SLOT(d, void deleteAggregationButtonClicked())
  Q_PRIVATE_SLOT(d, void editedAggregationNameChanged())
  Q_PRIVATE_SLOT(d, void okButtonClicked())

  class Private;
  Private * const d;
};

} // namespace Core

} // namespace MessageList

#endif //!__MESSAGELIST_CORE_CONFIGUREAGGREGATIONSDIALOG_H__
