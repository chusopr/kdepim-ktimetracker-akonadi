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
#ifndef GROUPCONFIG_H
#define GROUPCONFIG_H

#include <QtGui/QWidget>
#include <QtCore/QList>
#include <QtCore/QMap>

class QLabel;
class KPageWidget;
class QFrame;
class KPageWidgetItem;

class GroupConfigCommon;
class MemberConfig;
class SyncProcess;

class GroupConfig : public QWidget
{
  Q_OBJECT

  public:
    GroupConfig( QWidget *parent );

    void setSyncProcess( SyncProcess *process );

    void saveConfig();

  public Q_SLOTS:
    void addMember();
    void removeMember();

    void updateMembers();

  Q_SIGNALS:
    void memberSelected( bool );

  protected Q_SLOTS:
    void memberWidgetSelected( KPageWidgetItem*, KPageWidgetItem* );

  private:
    QLabel *mNameLabel;

    KPageWidget *mMemberView;

    SyncProcess *mProcess;

    GroupConfigCommon *mCommonConfig;
    QMap<QWidget*, MemberConfig*> mMemberConfigs;
    QList<KPageWidgetItem*> mConfigPages;
};

#endif
