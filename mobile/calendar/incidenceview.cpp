/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

#include "incidenceview.h"

#include <QtGui/QMessageBox>

#include <KDebug>
#include <KDialog>
#include <KLocalizedString>

#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/KCal/IncidenceMimeTypeVisitor>

#include <akonadi/kcal/utils.h>

#include "declarativeeditors.h"

#include <incidenceeditors/incidenceeditor-ng/incidencegeneral.h>
#include <incidenceeditors/incidenceeditor-ng/incidencedatetime.h>

using namespace Akonadi;
using namespace IncidenceEditorsNG;
using namespace KCal;

IncidenceView::IncidenceView( QWidget* parent )
  : KDeclarativeFullScreenView( QLatin1String( "incidence-editor" ), parent )
  , mItemManager( new EditorItemManager( this ) )
  , mCollectionCombo( 0 )
  , mEditor( new CombinedIncidenceEditor( parent ) )
{
  qmlRegisterType<DCollectionCombo>( "org.kde.incidenceeditors", 4, 5, "CollectionCombo" );
  qmlRegisterType<DIEGeneral>( "org.kde.incidenceeditors", 4, 5, "GeneralEditor" );

  mItem.setPayload<KCal::Incidence::Ptr>( KCal::Incidence::Ptr( new KCal::Event ) );
  mItem.setMimeType( IncidenceMimeTypeVisitor::eventMimeType() );

  connect( mItemManager, SIGNAL(itemSaveFinished()),
           SLOT(slotSaveFinished() ) );
  connect( mItemManager, SIGNAL(itemSaveFailed(QString)),
           SLOT(slotSaveFailed(QString) ) );
}

IncidenceView::~IncidenceView()
{
  delete mEditor;
}

void IncidenceView::load( const Akonadi::Item &item, const QDate &date )
{
  Q_ASSERT( item.hasPayload() ); // TODO: Fetch payload if there is no payload set.

  mItem = item;
  mItemManager->load( mItem );
  mActiveDate = date;

  if ( mCollectionCombo )
    mCollectionCombo->setDefaultCollection( mItem.parentCollection() );
}

void IncidenceView::setCollectionCombo( Akonadi::CollectionComboBox *combo )
{
  mCollectionCombo = combo;
  mCollectionCombo->setMimeTypeFilter( QStringList() << Akonadi::IncidenceMimeTypeVisitor::eventMimeType() );
  mCollectionCombo->setDefaultCollection( mItem.parentCollection() );
}

void IncidenceView::setGeneralEditor( MobileIncidenceGeneral *editor )
{
  IncidenceEditorsNG::IncidenceGeneral *editorGeneral = new IncidenceEditorsNG::IncidenceGeneral( editor->mUi );
  editorGeneral->load( mItem.payload<Incidence::Ptr>() );
  mEditor->combine( editorGeneral );

  IncidenceEditorsNG::IncidenceDateTime *editorDateTime = new IncidenceEditorsNG::IncidenceDateTime( editor->mUi );
  editorDateTime->setActiveDate( mActiveDate );
  editorDateTime->load( mItem.payload<Incidence::Ptr>() );
  mEditor->combine( editorDateTime );
}

/// ItemEditorUi methods

bool IncidenceView::containsPayloadIdentifiers( const QSet<QByteArray> &partIdentifiers ) const
{
  return partIdentifiers.contains( QByteArray( "PLD:RFC822" ) );
}

bool IncidenceView::hasSupportedPayload( const Akonadi::Item &item ) const
{
  return item.hasPayload() && item.hasPayload<KCal::Incidence::Ptr>()
    && ( item.hasPayload<KCal::Event::Ptr>() || item.hasPayload<KCal::Todo::Ptr>() );
}

bool IncidenceView::isDirty() const
{
  return mEditor->isDirty();
}

bool IncidenceView::isValid()
{
  return mEditor->isValid();
}

void IncidenceView::load( const Akonadi::Item &item )
{
  Q_ASSERT( hasSupportedPayload( item ) );
  mItem = item;
  mEditor->load( mItem.payload<Incidence::Ptr>() );
}

Akonadi::Item IncidenceView::save( const Akonadi::Item &item )
{
  // TODO: Add support for todos
  KCal::Event::Ptr event( new KCal::Event );
  event->setUid( Akonadi::incidence( mItem )->uid() );
  mEditor->save( event );

  Akonadi::Item result = item;
  result.setMimeType( Akonadi::IncidenceMimeTypeVisitor::eventMimeType() );
  result.setPayload<KCal::Event::Ptr>( event );
  return result;
}

Akonadi::Collection IncidenceView::selectedCollection() const
{
  return mCollectionCombo->currentCollection();
}

void IncidenceView::reject( RejectReason /*reason*/, const QString &errorMessage )
{
  kDebug() << "Rejecting:" << errorMessage;
  deleteLater();
}

/// IncidenceView slots

void IncidenceView::save()
{
  mItemManager->save();
}

void IncidenceView::slotSaveFinished()
{
  deleteLater();
}

void IncidenceView::slotSaveFailed( const QString &message )
{
  QPointer<QMessageBox> dlg = new QMessageBox; //krazy:exclude=qclasses
  dlg->setIcon( QMessageBox::Warning );
  dlg->setWindowTitle( i18n( "Saving the event failed." ) );
  dlg->setInformativeText( i18n( "Reason:\n\n" ) + message );
  dlg->addButton( i18n( "Ok" ), QMessageBox::AcceptRole );
  dlg->exec();
}

void IncidenceView::cancel()
{
  deleteLater();
}

#include "incidenceview.moc"
