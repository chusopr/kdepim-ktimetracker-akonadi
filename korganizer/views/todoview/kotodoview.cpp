/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kotodoview.h"

#include "incidencetreemodel.h"
#include "calprinter.h"
#include "kocorehelper.h"
#include "koglobals.h"
#include "kohelper.h"
#include "koprefs.h"
#include "kotododelegates.h"
#include "kotodomodel.h"
#include "kotodoviewsortfilterproxymodel.h"
#include "kotodoviewquickaddline.h"
#include "kotodoviewquicksearch.h"
#include "kotodoviewview.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <libkdepim/kdatepickerpopup.h>

#include <Akonadi/EntityMimeTypeFilterModel>
#include <Akonadi/ETMViewStateSaver>

#include <KCalCore/CalFormat>

#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QToolButton>

// We share this struct between all views, for performance and memory purposes
struct ModelStack
{
  ModelStack( QObject *parent_ )
    : todoModel( new KOTodoModel() ),
      parent( parent_ ),
      calendar( 0 ),
      todoTreeModel( 0 ),
      todoFlatModel( 0 )
  {
  }

  ~ModelStack()
  {
    delete todoModel;
    delete todoTreeModel;
    delete todoFlatModel;
  }

  void registerView( KOTodoView *view )
  {
    views << view;
  }

  void unregisterView( KOTodoView *view )
  {
    views.removeAll( view );
  }

  void setFlatView( bool flat )
  {
    const QString todoMimeType = QLatin1String( "application/x-vnd.akonadi.calendar.todo" );
    if ( flat ) {
      foreach ( KOTodoView *view, views ) {
        // In flatview dropping confuses users and it's very easy to drop into a child item
        view->mView->setDragDropMode( QAbstractItemView::DragOnly );
        view->setFlatView( flat, /**propagate=*/false ); // So other views update their toggle icon

        if ( todoTreeModel ) {
          view->saveViewState(); // Save the tree state before it's gone
        }
      }

      delete todoFlatModel;
      todoFlatModel = new Akonadi::EntityMimeTypeFilterModel( parent );
      todoFlatModel->addMimeTypeInclusionFilter( todoMimeType );
      todoFlatModel->setSourceModel( calendar ? calendar->model() : 0 );
      todoModel->setSourceModel( todoFlatModel );

      delete todoTreeModel;
      todoTreeModel = 0;
    } else {
      delete todoTreeModel;
      todoTreeModel = new IncidenceTreeModel( QStringList() << todoMimeType, parent );
      foreach ( KOTodoView *view, views ) {
        QObject::connect( todoTreeModel, SIGNAL(indexChangedParent(QModelIndex)),
                          view, SLOT(expandIndex(QModelIndex)) );
        QObject::connect( todoTreeModel, SIGNAL(batchInsertionFinished()),
                          view, SLOT(restoreViewState()) );
        view->mView->setDragDropMode( QAbstractItemView::DragDrop );
        view->setFlatView( flat, /**propagate=*/false ); // So other views update their toggle icon
      }
      todoTreeModel->setSourceModel( calendar ? calendar->model() : 0 );
      todoModel->setSourceModel( todoTreeModel );
      delete todoFlatModel;
      todoFlatModel = 0;
    }

    foreach ( KOTodoView *view, views ) {
      view->mFlatViewButton->blockSignals( true );
      // We block signals to avoid recursion, we have two KOTodoViews and mFlatViewButton is synchronized
      view->mFlatViewButton->setChecked( flat );
      view->mFlatViewButton->blockSignals( false );
      view->mView->setRootIsDecorated( !flat );
      view->restoreViewState();
    }

    KOPrefs::instance()->setFlatListTodo( flat );
    KOPrefs::instance()->writeConfig();
  }

  void setCalendar( CalendarSupport::Calendar *newCalendar )
  {
    calendar = newCalendar;
    todoModel->setCalendar( calendar );
    if ( todoTreeModel ) {
      todoTreeModel->setSourceModel( calendar ? calendar->model() : 0 );
    }
  }

  bool isFlatView() const
  {
    return todoFlatModel != 0;
  }

  KOTodoModel *todoModel;
  QList<KOTodoView*> views;
  QObject *parent;

  CalendarSupport::Calendar *calendar;
  IncidenceTreeModel *todoTreeModel;
  Akonadi::EntityMimeTypeFilterModel *todoFlatModel;
};

// Don't use K_GLOBAL_STATIC, see QTBUG-22667
static ModelStack *sModels = 0;

KOTodoView::KOTodoView( bool sidebarView, QWidget *parent )
  : BaseView( parent ), mTreeStateRestorer( 0 )
{
  if ( !sModels ) {
    sModels = new ModelStack( parent );
  }
  sModels->registerView( this );

  mProxyModel = new KOTodoViewSortFilterProxyModel( this );
  mProxyModel->setSourceModel( sModels->todoModel );
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setFilterKeyColumn( KOTodoModel::SummaryColumn );
  mProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel->setSortRole( Qt::EditRole );

  mSidebarView = sidebarView;
  if ( !mSidebarView ) {
    mQuickSearch = new KOTodoViewQuickSearch( calendar(), this );
    mQuickSearch->setVisible( KOPrefs::instance()->enableTodoQuickSearch() );
    connect( mQuickSearch, SIGNAL(searchTextChanged(QString)),
             mProxyModel, SLOT(setFilterRegExp(QString)) );
    connect( mQuickSearch, SIGNAL(searchTextChanged(QString)),
             SLOT(restoreViewState()) );
    connect( mQuickSearch, SIGNAL(filterCategoryChanged(QStringList)),
             mProxyModel, SLOT(setCategoryFilter(QStringList)) );
    connect( mQuickSearch, SIGNAL(filterCategoryChanged(QStringList)),
             SLOT(restoreViewState()) );
    connect( mQuickSearch, SIGNAL(filterPriorityChanged(QStringList)),
             mProxyModel, SLOT(setPriorityFilter(QStringList)) );
    connect( mQuickSearch, SIGNAL(filterPriorityChanged(QStringList)),
             SLOT(restoreViewState()) );
  }

  mView = new KOTodoViewView( this );
  mView->setModel( mProxyModel );

  mView->setContextMenuPolicy( Qt::CustomContextMenu );

  mView->setSortingEnabled( true );

  mView->setAutoExpandDelay( 250 );
  mView->setDragDropMode( QAbstractItemView::DragDrop );

  mView->setExpandsOnDoubleClick( false );
  mView->setEditTriggers( QAbstractItemView::SelectedClicked |
                          QAbstractItemView::EditKeyPressed );

  KOTodoRichTextDelegate *richTextDelegate = new KOTodoRichTextDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::SummaryColumn, richTextDelegate );
  mView->setItemDelegateForColumn( KOTodoModel::DescriptionColumn, richTextDelegate );

  KOTodoPriorityDelegate *priorityDelegate = new KOTodoPriorityDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::PriorityColumn, priorityDelegate );

  KOTodoDueDateDelegate *dueDateDelegate = new KOTodoDueDateDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::DueDateColumn, dueDateDelegate );

  KOTodoCompleteDelegate *completeDelegate = new KOTodoCompleteDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::PercentColumn, completeDelegate );

  mCategoriesDelegate = new KOTodoCategoriesDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::CategoriesColumn, mCategoriesDelegate );

  connect( mView, SIGNAL(customContextMenuRequested(QPoint)),
           this, SLOT(contextMenu(QPoint)) );
  connect( mView, SIGNAL(doubleClicked(QModelIndex)),
           this, SLOT(itemDoubleClicked(QModelIndex)) );
  connect( mView->selectionModel(),
           SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this,
           SLOT(selectionChanged(QItemSelection,QItemSelection)) );

  mQuickAdd = new KOTodoViewQuickAddLine( this );
  mQuickAdd->setClearButtonShown( true );
  mQuickAdd->setVisible( KOPrefs::instance()->enableQuickTodo() );
  connect( mQuickAdd, SIGNAL(returnPressed(Qt::KeyboardModifiers)),
           this, SLOT(addQuickTodo(Qt::KeyboardModifiers)) );

  mFullViewButton = 0;
  if ( !mSidebarView ) {
    mFullViewButton = new QToolButton( this );
    mFullViewButton->setAutoRaise( true );
    mFullViewButton->setCheckable( true );

    mFullViewButton->setToolTip(
      i18nc( "@info:tooltip",
             "Display to-do list in a full window" ) );
    mFullViewButton->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Checking this option will cause the to-do view to use the full window." ) );
  }
  mFlatViewButton = new QToolButton( this );
  mFlatViewButton->setAutoRaise( true );
  mFlatViewButton->setCheckable( true );
  mFlatViewButton->setToolTip(
    i18nc( "@info:tooltip",
           "Display to-dos in flat list instead of a tree" ) );
  mFlatViewButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Checking this option will cause the to-dos to be displayed as a "
           "flat list instead of a hierarchical tree; the parental "
           "relationships are removed in the display." ) );

  connect( mFlatViewButton, SIGNAL(toggled(bool)), SLOT(setFlatView(bool)) );
  if ( mFullViewButton ) {
    connect( mFullViewButton, SIGNAL(toggled(bool)), SLOT(setFullView(bool)) );
  }

  QGridLayout *layout = new QGridLayout( this );
  layout->setMargin( 0 );
  if ( !mSidebarView ) {
    layout->addWidget( mQuickSearch, 0, 0, 1, 2 );
  }
  layout->addWidget( mView, 1, 0, 1, 2 );
  layout->setRowStretch( 1, 1 );
  layout->addWidget( mQuickAdd, 2, 0 );

  // Dummy layout just to add a few px of right margin so the checkbox is aligned
  // with the QAbstractItemView's viewport.
  QHBoxLayout *dummyLayout = new QHBoxLayout();
  dummyLayout->setContentsMargins( 0, 0, mView->frameWidth()/*right*/, 0 );
  if ( !mSidebarView ) {
    QFrame *f = new QFrame( this );
    f->setFrameShape( QFrame::VLine );
    f->setFrameShadow( QFrame::Sunken );
    dummyLayout->addWidget( f );
    dummyLayout->addWidget( mFullViewButton );
  }
  dummyLayout->addWidget( mFlatViewButton );

  layout->addLayout( dummyLayout, 2, 1 );
  setLayout( layout );

  // ---------------- POPUP-MENUS -----------------------
  mItemPopupMenu = new QMenu( this );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    i18nc( "@action:inmenu show the to-do", "&Show" ),
    this, SLOT(showTodo()) );

  QAction *a = mItemPopupMenu->addAction(
    i18nc( "@action:inmenu edit the to-do", "&Edit..." ),
    this, SLOT(editTodo()) );
  mItemPopupMenuReadWriteEntries << a;
  mItemPopupMenuItemOnlyEntries << a;

  mItemPopupMenu->addSeparator();
  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    KOGlobals::self()->smallIcon( "document-print" ),
    i18nc( "@action:inmenu print the to-do", "&Print..." ),
    this, SLOT(printTodo()) );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    KOGlobals::self()->smallIcon( "document-print-preview" ),
    i18nc( "@action:inmenu print preview the to-do", "Print Previe&w..." ),
    this, SLOT(printPreviewTodo()) );

  mItemPopupMenu->addSeparator();
  a = mItemPopupMenu->addAction(
    KIconLoader::global()->loadIcon( "edit-delete", KIconLoader::NoGroup, KIconLoader::SizeSmall ),
    i18nc( "@action:inmenu delete the to-do", "&Delete" ),
    this, SLOT(deleteTodo()) );
  mItemPopupMenuReadWriteEntries << a;
  mItemPopupMenuItemOnlyEntries << a;

  mItemPopupMenu->addSeparator();

  mItemPopupMenu->addAction(
    KIconLoader::global()->loadIcon(
      "view-calendar-tasks", KIconLoader::NoGroup, KIconLoader::SizeSmall ),
    i18nc( "@action:inmenu create a new to-do", "New &To-do..." ),
    this, SLOT(newTodo()) );

  a = mItemPopupMenu->addAction(
    i18nc( "@action:inmenu create a new sub-to-do", "New Su&b-to-do..." ),
    this, SLOT(newSubTodo()) );
  mItemPopupMenuReadWriteEntries << a;
  mItemPopupMenuItemOnlyEntries << a;

  mMakeTodoIndependent = mItemPopupMenu->addAction(
    i18nc( "@action:inmenu", "&Make this To-do Independent" ),
    this, SIGNAL(unSubTodoSignal()) );

  mMakeSubtodosIndependent =
    mItemPopupMenu->addAction(
      i18nc( "@action:inmenu", "Make all Sub-to-dos &Independent" ),
      this, SIGNAL(unAllSubTodoSignal()) );

  mItemPopupMenuItemOnlyEntries << mMakeTodoIndependent;
  mItemPopupMenuItemOnlyEntries << mMakeSubtodosIndependent;

  mItemPopupMenuReadWriteEntries << mMakeTodoIndependent;
  mItemPopupMenuReadWriteEntries << mMakeSubtodosIndependent;

  mItemPopupMenu->addSeparator();

  mCopyPopupMenu =
    new KPIM::KDatePickerPopup( KPIM::KDatePickerPopup::NoDate |
                                KPIM::KDatePickerPopup::DatePicker |
                                KPIM::KDatePickerPopup::Words,
                                QDate::currentDate(), this );
  mCopyPopupMenu->setTitle( i18nc( "@title:menu", "&Copy To" ) );

  connect( mCopyPopupMenu, SIGNAL(dateChanged(QDate)),
           SLOT(copyTodoToDate(QDate)) );

  connect( mCopyPopupMenu, SIGNAL(dateChanged(QDate)),
           mItemPopupMenu, SLOT(hide()) );

  mMovePopupMenu =
    new KPIM:: KDatePickerPopup( KPIM::KDatePickerPopup::NoDate |
                                 KPIM::KDatePickerPopup::DatePicker |
                                 KPIM::KDatePickerPopup::Words,
                                 QDate::currentDate(), this );
  mMovePopupMenu->setTitle( i18nc( "@title:menu", "&Move To" ) );

  connect( mMovePopupMenu, SIGNAL(dateChanged(QDate)),
           SLOT(setNewDate(QDate)) );

  connect( mMovePopupMenu, SIGNAL(dateChanged(QDate)),
           mItemPopupMenu, SLOT(hide()) );

  mItemPopupMenu->insertMenu( 0, mCopyPopupMenu );
  mItemPopupMenu->insertMenu( 0, mMovePopupMenu );

  mItemPopupMenu->addSeparator();
  mItemPopupMenu->addAction(
    i18nc( "@action:inmenu delete completed to-dos", "Pur&ge Completed" ),
    this, SIGNAL(purgeCompletedSignal()) );

  mPriorityPopupMenu = new QMenu( this );
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu unspecified priority", "unspecified" ) ) ] = 0;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu highest priority", "1 (highest)" ) ) ] = 1;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=2", "2" ) ) ] = 2;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=3", "3" ) ) ] = 3;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=4", "4" ) ) ] = 4;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu medium priority", "5 (medium)" ) ) ] = 5;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=6", "6" ) ) ] = 6;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=7", "7" ) ) ] = 7;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=8", "8" ) ) ] = 8;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu lowest priority", "9 (lowest)" ) ) ] = 9;
  connect( mPriorityPopupMenu, SIGNAL(triggered(QAction*)),
           SLOT(setNewPriority(QAction*)) );

  mPercentageCompletedPopupMenu = new QMenu(this);
  for ( int i = 0; i <= 100; i+=10 ) {
    QString label = QString( "%1 %" ).arg( i );
    mPercentage[mPercentageCompletedPopupMenu->addAction( label )] = i;
  }
  connect( mPercentageCompletedPopupMenu, SIGNAL(triggered(QAction*)),
           SLOT(setNewPercentage(QAction*)) );

  setMinimumHeight( 50 );

  // Initialize our proxy models
  setFlatView( KOPrefs::instance()->flatListTodo() );
  setFullView( KOPrefs::instance()->fullViewTodo() );
}

KOTodoView::~KOTodoView()
{
  saveViewState();

  sModels->unregisterView( this );
  if ( sModels->views.isEmpty() ) {
    delete sModels;
    sModels = 0;
  }
}

void KOTodoView::expandIndex( const QModelIndex &index )
{
  QModelIndex todoModelIndex = sModels->todoModel->mapFromSource( index );
  Q_ASSERT( todoModelIndex.isValid() );
  QModelIndex realIndex = mProxyModel->mapFromSource( todoModelIndex );
  Q_ASSERT( realIndex.isValid() );
  while ( realIndex.isValid() ) {
    mView->expand( realIndex );
    realIndex = mProxyModel->parent( realIndex );
  }
}

void KOTodoView::setCalendar( CalendarSupport::Calendar *calendar )
{
  BaseView::setCalendar( calendar );
  if ( !mSidebarView ) {
    mQuickSearch->setCalendar( calendar );
  }
  mCategoriesDelegate->setCalendar( calendar );
  sModels->setCalendar( calendar );
  restoreViewState();
}

Akonadi::Item::List KOTodoView::selectedIncidences()
{
  Akonadi::Item::List ret;
  const QModelIndexList selection = mView->selectionModel()->selectedRows();
  Q_FOREACH ( const QModelIndex &mi, selection ) {
    ret << mi.data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  }
  return ret;
}

DateList KOTodoView::selectedIncidenceDates()
{
  // The todo view only lists todo's. It's probably not a good idea to
  // return something about the selected todo here, because it has got
  // a couple of dates (creation, due date, completion date), and the
  // caller could not figure out what he gets. So just return an empty list.
  return DateList();
}

void KOTodoView::saveLayout( KConfig *config, const QString &group ) const
{
  KConfigGroup cfgGroup = config->group( group );
  QHeaderView *header = mView->header();

  QVariantList columnVisibility;
  QVariantList columnOrder;
  QVariantList columnWidths;
  for ( int i = 0; i < header->count(); i++ ) {
    columnVisibility << QVariant( !mView->isColumnHidden( i ) );
    columnWidths << QVariant( header->sectionSize( i ) );
    columnOrder << QVariant( header->visualIndex( i ) );
  }
  cfgGroup.writeEntry( "ColumnVisibility", columnVisibility );
  cfgGroup.writeEntry( "ColumnOrder", columnOrder );
  cfgGroup.writeEntry( "ColumnWidths", columnWidths );

  cfgGroup.writeEntry( "SortAscending", (int)header->sortIndicatorOrder() );
  if ( header->isSortIndicatorShown() ) {
    cfgGroup.writeEntry( "SortColumn", header->sortIndicatorSection() );
  } else {
    cfgGroup.writeEntry( "SortColumn", -1 );
  }

  if ( !mSidebarView ) {
    KOPrefs::instance()->setFullViewTodo( mFullViewButton->isChecked() );
  }
  KOPrefs::instance()->setFlatListTodo( mFlatViewButton->isChecked() );
}

void KOTodoView::restoreLayout( KConfig *config, const QString &group, bool minimalDefaults )
{
  KConfigGroup cfgGroup = config->group( group );
  QHeaderView *header = mView->header();

  QVariantList columnVisibility = cfgGroup.readEntry( "ColumnVisibility", QVariantList() );
  QVariantList columnOrder = cfgGroup.readEntry( "ColumnOrder", QVariantList() );
  QVariantList columnWidths = cfgGroup.readEntry( "ColumnWidths", QVariantList() );

  if ( columnVisibility.isEmpty() ) {
    // if config is empty then use default settings
    mView->hideColumn( KOTodoModel::RecurColumn );
    mView->hideColumn( KOTodoModel::DescriptionColumn );
    mView->hideColumn( KOTodoModel::CalendarColumn );

    if ( minimalDefaults ) {
      mView->hideColumn( KOTodoModel::PriorityColumn );
      mView->hideColumn( KOTodoModel::PercentColumn );
      mView->hideColumn( KOTodoModel::DescriptionColumn );
      mView->hideColumn( KOTodoModel::CategoriesColumn );
    }

    // We don't have any incidences (content) yet, so we delay resizing
    QTimer::singleShot( 0, this, SLOT(resizeColumnsToContent()) );

  } else {
      for ( int i = 0;
            i < header->count() &&
            i < columnOrder.size() &&
            i < columnWidths.size() &&
            i < columnVisibility.size();
            i++ ) {
      bool visible = columnVisibility[i].toBool();
      int width = columnWidths[i].toInt();
      int order = columnOrder[i].toInt();

      header->resizeSection( i, width );
      header->moveSection( header->visualIndex( i ), order );
      if ( i != 0 && !visible ) {
        mView->hideColumn( i );
      }
    }
  }

  int sortOrder = cfgGroup.readEntry( "SortAscending", (int)Qt::AscendingOrder );
  int sortColumn = cfgGroup.readEntry( "SortColumn", -1 );
  if ( sortColumn >= 0 ) {
    mView->sortByColumn( sortColumn, (Qt::SortOrder)sortOrder );
  }

  mFlatViewButton->setChecked( cfgGroup.readEntry( "FlatView", false ) );
}

void KOTodoView::setIncidenceChanger( CalendarSupport::IncidenceChanger *changer )
{
  BaseView::setIncidenceChanger( changer );
  sModels->todoModel->setIncidenceChanger( changer );
}

void KOTodoView::showDates( const QDate &start, const QDate &end, const QDate & )
{
  // There is nothing to do here for the Todo View
  Q_UNUSED( start );
  Q_UNUSED( end );
}

void KOTodoView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
}

void KOTodoView::updateView()
{
  // View is always updated, it's connected to ETM.
}

void KOTodoView::updateCategories()
{
  if ( !mSidebarView ) {
    mQuickSearch->updateCategories();
  }
  // TODO check if we have to do something with the category delegate
}

void KOTodoView::changeIncidenceDisplay( const Akonadi::Item &, int )
{
  // Don't do anything, model is connected to ETM, it's up to date

}
void KOTodoView::updateConfig()
{
  if ( !mSidebarView ) {
    mQuickSearch->setVisible( KOPrefs::instance()->enableTodoQuickSearch() );
  }
  mQuickAdd->setVisible( KOPrefs::instance()->enableQuickTodo() );

  updateView();
}

void KOTodoView::clearSelection()
{
  mView->selectionModel()->clearSelection();
}

void KOTodoView::addTodo( const QString &summary,
                          const KCalCore::Todo::Ptr &parent,
                          const QStringList &categories )
{
  if ( !mChanger || summary.trimmed().isEmpty() ) {
    return;
  }

  KCalCore::Todo::Ptr todo( new KCalCore::Todo );
  todo->setSummary( summary.trimmed() );
  todo->setOrganizer(
    Person::Ptr( new Person( CalendarSupport::KCalPrefs::instance()->fullName(),
                             CalendarSupport::KCalPrefs::instance()->email() ) ) );

  todo->setCategories( categories );

  Q_UNUSED( parent );
  /*  if ( parent ) {
    todo->setRelatedTo( parent );
  }
  TODO: review
  */

  bool result = false;
  CalendarSupport::CollectionSelection *selection =
    EventViews::EventView::globalCollectionSelection();

  // If we only have one collection, don't ask in which collection to save the to-do.
  if ( selection && selection->model()->model()->rowCount() == 1 ) {
    QModelIndex index = selection->model()->model()->index( 0, 0 );
    if ( index.isValid() ) {
      Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );
      if ( collection.isValid() ) {
        result = mChanger->addIncidence( todo, collection, this );
      }
    }
  } else {
    Akonadi::Collection selectedCollection;
    int dialogCode = 0;
    result = mChanger->addIncidence( todo, this, selectedCollection, dialogCode );
    result = result || dialogCode == QDialog::Rejected;
  }

  if ( !result ) {
    KOHelper::showSaveIncidenceErrorMsg( this, todo );
  }
}

void KOTodoView::addQuickTodo( Qt::KeyboardModifiers modifiers )
{
  if ( modifiers == Qt::NoModifier ) {
    /*const QModelIndex index = */ addTodo( mQuickAdd->text(), KCalCore::Todo::Ptr(),
                                            mProxyModel->categories() );

#ifdef AKONADI_PORT_DISABLED
    // the todo is added asynchronously now, so we have to wait until
    // the new item is actually added before selecting the item

    QModelIndexList selection = mView->selectionModel()->selectedRows();
    if ( selection.size() <= 1 ) {
      // don't destroy complex selections, not applicable now (only single
      // selection allowed), but for the future...
      mView->selectionModel()->select( mProxyModel->mapFromSource( index ),
                                       QItemSelectionModel::ClearAndSelect |
                                       QItemSelectionModel::Rows );
    }
#else
    kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
  } else if ( modifiers == Qt::ControlModifier ) {
    QModelIndexList selection = mView->selectionModel()->selectedRows();
    if ( selection.size() != 1 ) {
      return;
    }
    const QModelIndex idx = mProxyModel->mapToSource( selection[0] );
    const Akonadi::Item parent = sModels->todoModel->data( idx,
                      Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    addTodo( mQuickAdd->text(), CalendarSupport::todo( parent ), mProxyModel->categories() );
  } else {
    return;
  }
  mQuickAdd->setText( QString() );
}

void KOTodoView::contextMenu( const QPoint &pos )
{
  const bool hasItem = mView->indexAt( pos ).isValid();
  Incidence::Ptr incidencePtr;

  Q_FOREACH ( QAction *entry, mItemPopupMenuItemOnlyEntries ) {
    bool enable;

    if ( hasItem ) {
      const Akonadi::Item::List incidences = selectedIncidences();

      if ( incidences.isEmpty() ) {
        enable = false;
      } else {
        Akonadi::Item item = incidences.first();
        incidencePtr = CalendarSupport::incidence( item );

        // Action isn't RO, it can change the incidence, "Edit" for example.
        const bool actionIsRw = mItemPopupMenuReadWriteEntries.contains( entry );

        const bool incidenceIsRO = !calendar()->hasChangeRights( item );

        enable = hasItem && ( !actionIsRw ||
                              ( actionIsRw && !incidenceIsRO ) );

      }
    } else {
      enable = false;
    }

    entry->setEnabled( enable );
  }
  mCopyPopupMenu->setEnabled( hasItem );
  mMovePopupMenu->setEnabled( hasItem );

  if ( hasItem ) {
    if ( incidencePtr ) {
      if ( calendar() ) {
        mMakeSubtodosIndependent->setEnabled( !calendar()->findChildren( incidencePtr ).isEmpty() );
      }
      mMakeTodoIndependent->setEnabled( !incidencePtr->relatedTo().isEmpty() );
    }

    switch ( mView->indexAt( pos ).column() ) {
    case KOTodoModel::PriorityColumn:
      mPriorityPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    case KOTodoModel::PercentColumn:
      mPercentageCompletedPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    case KOTodoModel::DueDateColumn:
      mMovePopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    case KOTodoModel::CategoriesColumn:
      createCategoryPopupMenu()->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    default:
      mItemPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    }
  } else {
    mItemPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
  }
}

void KOTodoView::selectionChanged( const QItemSelection &selected,
                                   const QItemSelection &deselected )
{
  Q_UNUSED( deselected );
  QModelIndexList selection = selected.indexes();
  if ( selection.isEmpty() || !selection[0].isValid() ) {
    emit incidenceSelected( Akonadi::Item(), QDate() );
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();

  if ( selectedIncidenceDates().isEmpty() ) {
    emit incidenceSelected( todoItem, QDate() );
  } else {
    emit incidenceSelected( todoItem, selectedIncidenceDates().first() );
  }
}

void KOTodoView::showTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();

  emit showIncidenceSignal( todoItem );
}

void KOTodoView::editTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  emit editIncidenceSignal( todoItem );
}

void KOTodoView::printTodo()
{
  printTodo( false );
}

void KOTodoView::printPreviewTodo()
{
  printTodo( true );
}

void KOTodoView::printTodo( bool preview )
{
   QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  KOCoreHelper helper;
  CalPrinter printer( this, BaseView::calendar(), &helper, true );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  Incidence::List selectedIncidences;
  selectedIncidences.append( todo );

  KDateTime todoDate;
  if ( todo->hasStartDate() ) {
    todoDate = todo->dtStart();
  } else {
    todoDate = todo->dtDue();
  }

  printer.print( KOrg::CalPrinterBase::Incidence,
                 todoDate.date(), todoDate.date(), selectedIncidences, preview );

}

void KOTodoView::deleteTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() == 1 ) {
    const Akonadi::Item todoItem =
      selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();

    if ( mChanger->isNotDeleted( todoItem.id() ) ) {
      emit deleteIncidenceSignal( todoItem );
    }
  }
}

void KOTodoView::newTodo()
{
  emit newTodoSignal( QDate::currentDate().addDays( 7 ) );
}

void KOTodoView::newSubTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() == 1 ) {
    const Akonadi::Item todoItem =
      selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();

    emit newSubTodoSignal( todoItem );
  } else {
    // This never happens
    kWarning() << "Selection size isn't 1";
  }
}

void KOTodoView::copyTodoToDate( const QDate &date )
{
  if ( !mChanger ) {
    return;
  }

  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const QModelIndex origIndex = mProxyModel->mapToSource( selection[0] );
  Q_ASSERT( origIndex.isValid() );

  const Akonadi::Item origItem =
    sModels->todoModel->data( origIndex,
                              Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  const KCalCore::Todo::Ptr orig = CalendarSupport::todo( origItem );
  if ( !orig ) {
    return;
  }

  KCalCore::Todo::Ptr todo( orig->clone() );

  todo->setUid( KCalCore::CalFormat::createUniqueId() );

  KDateTime due = todo->dtDue();
  due.setDate( date );
  todo->setDtDue( due );

  Akonadi::Collection selectedCollection;
  int dialogCode = 0;
  if ( !mChanger->addIncidence( todo, this, selectedCollection, dialogCode ) ) {
    if ( dialogCode != QDialog::Rejected ) {
      KOHelper::showSaveIncidenceErrorMsg( this, todo );
    }
  }
}

void KOTodoView::itemDoubleClicked( const QModelIndex &index )
{
  if ( index.isValid() ) {
    QModelIndex summary = index.sibling( index.row(), KOTodoModel::SummaryColumn );
    if ( summary.flags() & Qt::ItemIsEditable ) {
      editTodo();
    } else {
      showTodo();
    }
  }
}

QMenu *KOTodoView::createCategoryPopupMenu()
{
  QMenu *tempMenu = new QMenu( this );

  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return tempMenu;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  QStringList checkedCategories = todo->categories();

  QStringList::Iterator it;
  CalendarSupport::CategoryConfig cc( KOPrefs::instance() );
  Q_FOREACH ( const QString &i, cc.customCategories() ) {
    QAction *action = tempMenu->addAction( i );
    action->setCheckable( true );
    mCategory[ action ] = i;
    if ( checkedCategories.contains( i ) ) {
      action->setChecked( true );
    }
  }

  connect( tempMenu, SIGNAL(triggered(QAction*)),
           SLOT(changedCategories(QAction*)) );
  connect( tempMenu, SIGNAL(aboutToHide()),
           tempMenu, SLOT(deleteLater()) );
  return tempMenu;
}

void KOTodoView::setNewDate( const QDate &date )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  if ( calendar()->hasChangeRights( todoItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );

    KDateTime dt( date );

    if ( !todo->allDay() ) {
      dt.setTime( todo->dtDue().time() );
    }

    if ( date.isNull() ) {
      todo->setHasDueDate( false );
    } else if ( !todo->hasDueDate() ) {
      todo->setHasDueDate( true );
    }
    todo->setDtDue( dt );

    mChanger->changeIncidence( oldTodo, todoItem,
                               CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED, this );
  } else {
    kDebug() << "Item is readOnly";
  }
}

void KOTodoView::setNewPercentage( QAction *action )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  if ( calendar()->hasChangeRights( todoItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );

    int percentage = mPercentage.value( action );
    if ( percentage == 100 ) {
      todo->setCompleted( KDateTime::currentLocalDateTime() );
      todo->setPercentComplete( 100 );
    } else {
      todo->setPercentComplete( percentage );
    }
    if ( todo->recurs() && percentage == 100 ) {
      mChanger->changeIncidence(
        oldTodo, todoItem,
        CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED_WITH_RECURRENCE, this );
    } else {
      mChanger->changeIncidence(
        oldTodo, todoItem,
        CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED, this );
    }
  } else {
    kDebug() << "Item is read only";
  }
}

void KOTodoView::setNewPriority( QAction *action )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }
  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( calendar()->hasChangeRights( todoItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );
    todo->setPriority( mPriority[action] );

    mChanger->changeIncidence( oldTodo, todoItem,
                               CalendarSupport::IncidenceChanger::PRIORITY_MODIFIED, this );
  }
}

void KOTodoView::changedCategories( QAction *action )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );
  if ( calendar()->hasChangeRights( todoItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );

    QStringList categories = todo->categories();
    if ( categories.contains( mCategory[action] ) ) {
      categories.removeAll( mCategory[action] );
    } else {
      categories.append( mCategory[action] );
    }
    categories.sort();
    todo->setCategories( categories );
    mChanger->changeIncidence( oldTodo, todoItem,
                               CalendarSupport::IncidenceChanger::CATEGORY_MODIFIED, this );
  } else {
    kDebug() << "No active item, active item is read-only, or locking failed";
  }
}

void KOTodoView::setFullView( bool fullView )
{
  if ( !mFullViewButton ) {
    return;
  }

  mFullViewButton->setChecked( fullView );
  if ( fullView ) {
    mFullViewButton->setIcon( KIcon( "view-restore" ) );
  } else {
    mFullViewButton->setIcon( KIcon( "view-fullscreen" ) );
  }

  mFullViewButton->blockSignals( true );
  // We block signals to avoid recursion; there are two KOTodoViews and
  // also mFullViewButton is synchronized.
  mFullViewButton->setChecked( fullView );
  mFullViewButton->blockSignals( false );

  KOPrefs::instance()->setFullViewTodo( fullView );
  KOPrefs::instance()->writeConfig();

  emit fullViewChanged( fullView );
}

void KOTodoView::setFlatView( bool flatView, bool notifyOtherViews )
{
  if ( flatView ) {
    mFlatViewButton->setIcon( KIcon( "view-list-tree" ) );
  } else {
    mFlatViewButton->setIcon( KIcon( "view-list-details" ) );
  }

  if ( notifyOtherViews ) {
    sModels->setFlatView( flatView );
  }
}

void KOTodoView::getHighlightMode( bool &highlightEvents,
                                   bool &highlightTodos,
                                   bool &highlightJournals )
{
  highlightTodos    = KOPrefs::instance()->mHighlightTodos;
  highlightEvents   = !highlightTodos;
  highlightJournals = false;
}

bool KOTodoView::usesFullWindow()
{
  return KOPrefs::instance()->mFullViewTodo;
}

void KOTodoView::resizeColumnsToContent()
{
  mView->resizeColumnToContents( KOTodoModel::DueDateColumn );
  mView->resizeColumnToContents( KOTodoModel::SummaryColumn );
}

KOrg::CalPrinterBase::PrintType KOTodoView::printType() const
{
  return KOrg::CalPrinterBase::Todolist;
}

void KOTodoView::restoreViewState()
{
  if ( sModels->isFlatView() ) {
    return;
  }

  if ( sModels->todoTreeModel && !sModels->todoTreeModel->sourceModel() )
    return;

  //QElapsedTimer timer;
  //timer.start();
  delete mTreeStateRestorer;
  mTreeStateRestorer = new Akonadi::ETMViewStateSaver();
  KConfigGroup group( KOGlobals::self()->config(), stateSaverGroup() );
  mTreeStateRestorer->setView( mView );
  mTreeStateRestorer->restoreState( group );
  //kDebug() << "Took " << timer.elapsed();
}

QString KOTodoView::stateSaverGroup() const
{
  QString str = QLatin1String( "TodoTreeViewState" );
  if ( mSidebarView ) {
    str += QChar( 'S' );
  }

  return str;
}

void KOTodoView::saveViewState()
{
  Akonadi::ETMViewStateSaver treeStateSaver;
  KConfigGroup group( KOGlobals::self()->config(), stateSaverGroup() );
  treeStateSaver.setView( mView );
  treeStateSaver.saveState( group );
}

#include "kotodoview.moc"
