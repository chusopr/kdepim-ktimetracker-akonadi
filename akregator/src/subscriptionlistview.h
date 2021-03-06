/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_SUBSCRIPTIONLISTVIEW_H
#define AKREGATOR_SUBSCRIPTIONLISTVIEW_H

#include <QTreeView>
#include <QMap>
#include <QAction>
#include <QByteArray>

class KMenu;

namespace Akregator {

class TreeNode;

class SubscriptionListView : public QTreeView {
Q_OBJECT
public:
    explicit SubscriptionListView( QWidget* parent = 0 );
    ~SubscriptionListView();
// the following is all transitional, for easier porting from the item-based views

    void startNodeRenaming( TreeNode* node );

    void ensureNodeVisible( TreeNode* node );

    //override
    void setModel( QAbstractItemModel* model );

    void triggerUpdate() {}

    enum Column {
        TitleColumn=0,
        UnreadColumn=1,
        TotalColumn=2
    };


public Q_SLOTS:

    void slotPrevFeed();
    void slotNextFeed();

    void slotPrevUnreadFeed();
    void slotNextUnreadFeed();

    void slotItemBegin();
    void slotItemEnd();
    void slotItemLeft();
    void slotItemRight();
    void slotItemUp();
    void slotItemDown();

Q_SIGNALS:
    void userActionTakingPlace();


private:
    void saveHeaderSettings();
    void loadHeaderSettings();
    void restoreHeaderState();


private Q_SLOTS:
    void showHeaderMenu( const QPoint& pos );
    void headerMenuItemTriggered( QAction* action );

private:
    QByteArray m_headerState;
};

}

#endif // AKREGATOR_SUBSCRIPTIONLISTVIEW_H
