/*
    This file is part of Akregator.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_EDITSUBSCRIPTIONCOMMAND_H
#define AKREGATOR_EDITSUBSCRIPTIONCOMMAND_H

#include "command.h"

#include <boost/shared_ptr.hpp>

namespace KRss {
    class FeedList;
    class FeedListView;
    class TreeNode;
    class TagProvider;
}

namespace Akregator {

class EditSubscriptionCommand : public Command
{
    Q_OBJECT
public:
    explicit EditSubscriptionCommand( QObject* parent = 0 );
    ~EditSubscriptionCommand();

    boost::shared_ptr<KRss::FeedList> feedList() const;
    void setFeedList( const boost::shared_ptr<KRss::FeedList>& fl );

    void setNode( const boost::shared_ptr<KRss::TreeNode>& node );
    boost::shared_ptr<KRss::TreeNode> node() const;

    KRss::FeedListView* feedListView() const;
    void setFeedListView( KRss::FeedListView* view );

    boost::shared_ptr<const KRss::TagProvider> tagProvider() const;
    void setTagProvider( const boost::shared_ptr<const KRss::TagProvider>& tp );

private:
    void doStart();

private:
    class Private;
    Private* const d;
    Q_PRIVATE_SLOT( d, void startEdit() )
    Q_PRIVATE_SLOT( d, void feedModifyDone( KJob* ) );
    Q_PRIVATE_SLOT( d, void tagModifyDone( KJob* ) );
};

}

#endif // AKREGATOR_EDITSUBSCRIPTIONCOMMAND_H
