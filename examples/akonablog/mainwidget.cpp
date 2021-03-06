/*
    Copyright (c) 2007 Bruno Virlet <bruno.virlet@gmail.com>
    Copyright (c) 2009 Omat Holding B.V. <info@omat.nl>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "mainwidget.h"
#include "mainwindow.h"
#include "blogmodel.h"
#include "microblogdelegate.h"
#include "akonaditabbar.h"

#include <akonadi/agentinstancemodel.h>
#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/collectionfetchjob.h>

#include <QVBoxLayout>
#include <QSplitter>
#include <QListView>
#include <QTreeView>

#include <KVBox>
#include <KTabBar>
#include <KDebug>
#include <KLocale>

using namespace Akonadi;

MainWidget::MainWidget( MainWindow * parent ) :
        QWidget( parent ), mMainWindow( parent )
{
    QVBoxLayout *layout = new QVBoxLayout( this );

    QSplitter *splitter = new QSplitter( Qt::Vertical, this );
    layout->addWidget( splitter );


    // Accounts
    Akonadi::AgentInstanceModel *model = new Akonadi::AgentInstanceModel( this );
    m_resourcesView = new QListView( splitter );
    m_resourcesView->setModel( model );
    connect( m_resourcesView, SIGNAL(clicked(QModelIndex)),
             SLOT(slotCurrentResourceChanged(QModelIndex)) );
    splitter->addWidget( m_resourcesView );
    
    // Filter the collection to only show the blogs
    Akonadi::AgentFilterProxyModel* proxy = new Akonadi::AgentFilterProxyModel( this );
    proxy->addMimeTypeFilter( "application/x-vnd.kde.microblog" );
    proxy->setSourceModel( model );
    m_resourcesView->setModel( proxy );

    // Bottom part
    KVBox* box = new KVBox( splitter );

    // Folders
    m_tabBar = new AkonadiTabBar( box );
    connect( m_tabBar, SIGNAL(currentChanged(Akonadi::Collection)),
             SLOT(slotCurrentTabChanged(Akonadi::Collection)) );

    mMessageList = new QTreeView( box );
    mMessageList->setRootIsDecorated( false );
    mMessageList->setDragEnabled( false );
    mMessageList->setSelectionMode( QAbstractItemView::ExtendedSelection );
    mMessageList->setSortingEnabled( true );

    MicroblogDelegate *delegate = new MicroblogDelegate( mMessageList, this );
    mMessageList->setItemDelegate( delegate );

    mMessageModel = new BlogModel( this );

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel( this );
    proxyModel->setSortRole( Qt::EditRole );
    proxyModel->setDynamicSortFilter( true );
    proxyModel->setSourceModel( mMessageModel );

    mMessageList->setModel( proxyModel );
    splitter->addWidget( box );

    splitter->setSizes( QList<int>() << 30 << 470 );
}

void MainWidget::slotCurrentResourceChanged( const QModelIndex &index )
{
    const Akonadi::AgentInstanceModel *model = static_cast<const AgentInstanceModel*>( m_resourcesView->model() );
    const QString identifier = model->data( index, AgentInstanceModel::InstanceIdentifierRole ).toString();
    m_tabBar->setResource( identifier );
}

void MainWidget::slotCurrentTabChanged( const Akonadi::Collection& col )
{
    mMessageModel->setCollection( col );
}
