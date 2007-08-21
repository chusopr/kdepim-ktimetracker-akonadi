/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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
*/
#ifndef GROUPCONFIGDIALOG_H
#define GROUPCONFIGDIALOG_H

#include <kdialog.h>

class GroupConfig;
class SyncProcess;

class GroupConfigDialog : public KDialog
{
  Q_OBJECT

  public:
    GroupConfigDialog( QWidget *parent, SyncProcess * );
    ~GroupConfigDialog();

  protected slots:
    void slotOk();
    void slotUser1();
    void slotUser2();

  private slots:
    void memberSelected( bool );

  private:
    GroupConfig *mConfigWidget;
};

#endif
