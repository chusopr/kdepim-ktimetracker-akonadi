/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include "article.h"
#include "articlemodel.h"
#include "feedlist.h"
#include "selectioncontroller.h"
#include "subscriptionlistmodel.h"

#include <QAbstractItemView>
#include <QItemSelectionModel>

namespace {
    static Akregator::Article articleForIndex( const QModelIndex& index, Akregator::FeedList* feedList )
    {
        if ( !index.isValid() )
            return Akregator::Article();

        const QString guid = index.data( Akregator::ArticleModel::GuidRole ).toString();
        const QString feedId = index.data( Akregator::ArticleModel::FeedIdRole ).toString();
        return feedList->findArticle( feedId, guid );
    }

    static Akregator::TreeNode* subscriptionForIndex( const QModelIndex& index, Akregator::FeedList* feedList )
    {
        if ( !index.isValid() )
            return 0L;

         return feedList->findByID( index.data( Akregator::SubscriptionListModel::SubscriptionIdRole ).toInt() );
    }
} // anon namespace 

Akregator::SelectionController::SelectionController( QObject* parent ) : AbstractSelectionController( parent ), m_feedList( 0 ), m_feedSelector( 0 ), m_articleLister( 0 )
{
}

void Akregator::SelectionController::setFeedSelector( QAbstractItemView* feedSelector )
{
    if ( m_feedSelector && m_feedSelector->selectionModel() )
    {
        disconnect( m_feedSelector->selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ),
                    this, SLOT( selectedSubscriptionChanged( QModelIndex ) ) );
    }
    
    m_feedSelector = feedSelector;
}

void Akregator::SelectionController::setArticleLister( Akregator::ArticleLister* lister )
{
    if ( m_articleLister )
    {
        disconnect( m_articleLister->articleSelectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ),
                 this, SLOT( currentArticleIndexChanged( QModelIndex ) ) );
    }

    m_articleLister = lister;
    setUp();
}

Akregator::Article Akregator::SelectionController::currentArticle() const
{
    return ::articleForIndex( m_articleLister->articleSelectionModel()->currentIndex(), m_feedList );
}


Akregator::TreeNode* Akregator::SelectionController::selectedSubscription() const
{
    return ::subscriptionForIndex( m_feedSelector->selectionModel()->currentIndex(), m_feedList );
}

void Akregator::SelectionController::setFeedList( Akregator::FeedList* list )
{
    m_feedList = list;
    setUp();
}

void Akregator::SelectionController::setUp()
{
    if ( m_feedList && m_feedSelector && m_articleLister )
    {
        m_feedSelector->setModel( new SubscriptionListModel( m_feedList, this ) );

        connect( m_feedSelector->selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ),
                 this, SLOT( selectedSubscriptionChanged( QModelIndex ) ) );

    }
}

void Akregator::SelectionController::selectedSubscriptionChanged( const QModelIndex& index )
{
    if ( !index.isValid() )
        return;

    Akregator::TreeNode* selected = selectedSubscription();

    m_articleLister->setArticleModel( new Akregator::ArticleModel( selected, 0 ) );
    connect( m_articleLister->articleSelectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex) ),
             this, SLOT( currentArticleIndexChanged( QModelIndex ) ) );
    emit currentSubscriptionChanged( selected );
}

void Akregator::SelectionController::currentArticleIndexChanged( const QModelIndex& )
{
    const Akregator::Article article = currentArticle();
    emit currentArticleChanged( article );
}

#include "selectioncontroller.moc"
