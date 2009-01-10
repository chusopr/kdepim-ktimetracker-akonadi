/*
    This file is part of KContactManager.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "mainwidget.h"

#include <QtGui/QAction>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QListView>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QSplitter>

#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectionview.h>
#include <akonadi/itemview.h>

#include <kactioncollection.h>
#include <kabc/kabcmodel.h>
#include <kabc/kabcitembrowser.h>
#include <kicon.h>
#include <klocale.h>
#include <kxmlguiclient.h>
#include <kaction.h>

#include "contacteditordialog.h"

MainWidget::MainWidget( KXMLGUIClient *guiClient, QWidget *parent )
  : QWidget( parent ),
    mGuiClient( guiClient )
{
  setupGui();
  setupActions();

  mCollectionModel = new Akonadi::CollectionModel( this );

  mCollectionFilterModel = new Akonadi::CollectionFilterProxyModel();
  mCollectionFilterModel->addMimeTypeFilter( "text/x-vcard" );
  mCollectionFilterModel->addMimeTypeFilter( "text/directory" );
  mCollectionFilterModel->addMimeTypeFilter( "text/vcard" );
  mCollectionFilterModel->setSourceModel( mCollectionModel );

  // display collections sorted
  QSortFilterProxyModel *sortModel = new QSortFilterProxyModel( this );
  sortModel->setDynamicSortFilter( true );
  sortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  sortModel->setSourceModel( mCollectionFilterModel );

  mCollectionView->setModel( sortModel );
  mCollectionView->header()->setDefaultAlignment( Qt::AlignCenter );
  mCollectionView->header()->setSortIndicatorShown( false );

  mContactModel = new Akonadi::KABCModel( this );
  mContactView->setModel( mContactModel );
  mContactView->header()->setDefaultAlignment( Qt::AlignCenter );
  for ( int column = 1; column < mContactModel->columnCount(); ++column )
    mContactView->setColumnHidden( column, true );

  connect( mCollectionView, SIGNAL( currentChanged( const Akonadi::Collection& ) ),
           mContactModel, SLOT( setCollection( const Akonadi::Collection& ) ) );
  connect( mContactView, SIGNAL( currentChanged( const Akonadi::Item& ) ),
           mContactDetails, SLOT( setItem( const Akonadi::Item& ) ) );
  connect( mContactView, SIGNAL( activated( const Akonadi::Item& ) ),
           this, SLOT( editItem( const Akonadi::Item& ) ) );
}

MainWidget::~MainWidget()
{
}

void MainWidget::setupGui()
{
  QHBoxLayout *layout = new QHBoxLayout( this );

  QSplitter *splitter = new QSplitter;
  layout->addWidget( splitter );

  mCollectionView = new Akonadi::CollectionView();
  splitter->addWidget( mCollectionView );

  mContactView = new Akonadi::ItemView;
  splitter->addWidget( mContactView );

  mContactDetails = new Akonadi::KABCItemBrowser;
  splitter->addWidget( mContactDetails );
}

void MainWidget::setupActions()
{
  KAction *action = 0;
  KActionCollection *collection = mGuiClient->actionCollection();

  action = collection->addAction( "file_new_contact" );
  action->setIcon( KIcon( "contact-new" ) );
  action->setText( i18n( "&New Contact..." ) );
  connect( action, SIGNAL( triggered(bool) ), SLOT( newContact() ));
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
  action->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers.</p>" ) );

  action = collection->addAction( "file_new_group" );
  action->setIcon( KIcon( "user-group-new" ) );
  action->setText( i18n( "&New Group..." ) );
  connect( action, SIGNAL( triggered(bool) ), SLOT( newGroup() ));
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );
  action->setWhatsThis( i18n( "Create a new group<p>You will be presented with a dialog where you can add a new group of contacts.</p>" ) );
}

void MainWidget::newContact()
{
  ContactEditorDialog dlg( ContactEditorDialog::CreateMode, mCollectionFilterModel, this );
  dlg.exec();
}

void MainWidget::newGroup()
{
}

void MainWidget::editItem( const Akonadi::Item &reference )
{
  const QModelIndex index = mContactModel->indexForItem( reference, 0 );
  const Akonadi::Item item = mContactModel->itemForIndex( index );

  if ( item.mimeType() == "text/directory" || item.mimeType() == "text/vcard" ) {
    editContact( reference );
  } else if ( item.mimeType() == "text/distributionlist" ) {
    editGroup( reference );
  }
}

void MainWidget::editContact( const Akonadi::Item &contact )
{
  ContactEditorDialog dlg( ContactEditorDialog::EditMode, mCollectionFilterModel, this );
  dlg.setContact( contact );
  dlg.exec();
}

void MainWidget::editGroup( const Akonadi::Item &group )
{
}

#include "mainwidget.moc"
