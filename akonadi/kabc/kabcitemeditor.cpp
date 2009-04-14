/*
    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

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

#include "kabcitemeditor.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>

#include <kabc/addressee.h>
#include <kabc/phonenumber.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/monitor.h>
#include <akonadi/session.h>
#include <klocale.h>

#include "ui_kabcitemeditor.h"
#include "waitingoverlay.h"

using namespace Akonadi;

class KABCItemEditor::Private
{
  public:
    Private( KABCItemEditor *parent )
      : mParent( parent ), mMonitor( 0 )
    {
    }

    ~Private()
    {
      delete mMonitor;
    }

    void fetchDone( KJob* );
    void storeDone( KJob* );
    void itemChanged( const Akonadi::Item &item, const QSet<QByteArray>& );

    void loadContact( const KABC::Addressee &addr );
    void storeContact( KABC::Addressee &addr );
    void setupMonitor();

    KABCItemEditor *mParent;
    KABCItemEditor::Mode mMode;
    Item mItem;
    Monitor *mMonitor;
    Collection mDefaultCollection;
    Ui::KABCItemEditor gui;
};

void KABCItemEditor::Private::fetchDone( KJob *job )
{
  if ( job->error() )
    return;

  ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( !fetchJob )
    return;

  if ( fetchJob->items().isEmpty() )
    return;

  mItem = fetchJob->items().first();

  const KABC::Addressee addr = mItem.payload<KABC::Addressee>();
  loadContact( addr );
}

void KABCItemEditor::Private::storeDone( KJob *job )
{
  if ( job->error() ) {
    emit mParent->error( job->errorString() );
    return;
  }

  if ( mMode == EditMode )
    emit mParent->contactStored( mItem );
  else if ( mMode == CreateMode )
    emit mParent->contactStored( static_cast<ItemCreateJob*>( job )->item() );
}

void KABCItemEditor::Private::itemChanged( const Item&, const QSet<QByteArray>& )
{
  QMessageBox dlg( mParent );

  dlg.setInformativeText( QLatin1String( "The contact has been changed by anyone else\nWhat shall be done?" ) );
  dlg.addButton( i18n( "Take over changes" ), QMessageBox::AcceptRole );
  dlg.addButton( i18n( "Ignore and Overwrite changes" ), QMessageBox::RejectRole );

  if ( dlg.exec() == QMessageBox::AcceptRole ) {
    ItemFetchJob *job = new ItemFetchJob( mItem );
    job->fetchScope().fetchFullPayload();

    mParent->connect( job, SIGNAL( result( KJob* ) ), mParent, SLOT( fetchDone( KJob* ) ) );
  }
}

void KABCItemEditor::Private::loadContact( const KABC::Addressee &addr )
{
  // Names
  gui.mGivenName->setText( addr.givenName() );
  gui.mFamilyName->setText( addr.familyName() );
  gui.mFormattedName->setText( addr.formattedName() );
  gui.mNickName->setText( addr.nickName() );

  // Internet
  const QStringList emails = addr.emails();
  if ( emails.count() > 0 )
    gui.mEMail->setText( emails.at( 0 ) );
  else
    gui.mEMail->setText( QString() );

  if ( emails.count() > 1 )
    gui.mAdditionalEMail->setText( emails.at( 1 ) );
  else
    gui.mAdditionalEMail->setText( QString() );

  gui.mHomepage->setText( addr.url().url() );

  // Phones
  gui.mWorkPhone->setText( addr.phoneNumber( KABC::PhoneNumber::Work ).number() );
  gui.mHomePhone->setText( addr.phoneNumber( KABC::PhoneNumber::Home ).number() );
  gui.mMobilePhone->setText( addr.phoneNumber( KABC::PhoneNumber::Cell ).number() );
  gui.mFaxPhone->setText( addr.phoneNumber( KABC::PhoneNumber::Fax ).number() );
  gui.mPagerPhone->setText( addr.phoneNumber( KABC::PhoneNumber::Pager ).number() );

  // Address Home
  KABC::Address homeAddress = addr.address( KABC::Address::Home );
  gui.mHomeStreet->setText( homeAddress.street() );
  gui.mHomeCity->setText( homeAddress.locality() );
  gui.mHomeState->setText( homeAddress.region() );
  gui.mHomePostalCode->setText( homeAddress.postalCode() );
  gui.mHomeCountry->setText( homeAddress.country() );

  // Address Work
  gui.mTitle->setText( addr.title() );
  gui.mDepartment->setText( addr.department() );
  gui.mOrganization->setText( addr.organization() );

  KABC::Address workAddress = addr.address( KABC::Address::Work );
  gui.mWorkStreet->setText( workAddress.street() );
  gui.mWorkCity->setText( workAddress.locality() );
  gui.mWorkState->setText( workAddress.region() );
  gui.mWorkPostalCode->setText( workAddress.postalCode() );
  gui.mWorkCountry->setText( workAddress.country() );

  // Customs

  // Notes
  gui.mNote->setPlainText( addr.note() );
}

void KABCItemEditor::Private::storeContact( KABC::Addressee &addr )
{
  // Names
  addr.setGivenName( gui.mGivenName->text() );
  addr.setFamilyName( gui.mFamilyName->text() );
  if ( !gui.mFormattedName->text().isEmpty() )
    addr.setFormattedName( gui.mFormattedName->text() );
  else
    addr.setFormattedName( QString::fromLatin1( "%1 %2" ).arg( addr.givenName() ).arg( addr.familyName() ) );
  addr.setNickName( gui.mNickName->text() );

  // Internet
  QStringList emails;
  if ( !gui.mEMail->text().isEmpty() )
    emails.append( gui.mEMail->text() );
  if ( !gui.mAdditionalEMail->text().isEmpty() )
    emails.append( gui.mAdditionalEMail->text() );
  addr.setEmails( emails );
  addr.setUrl( gui.mHomepage->text() );

  // Phones
  addr.removePhoneNumber( addr.phoneNumber( KABC::PhoneNumber::Work ) );
  if ( !gui.mWorkPhone->text().isEmpty() )
    addr.insertPhoneNumber( KABC::PhoneNumber( gui.mWorkPhone->text(), KABC::PhoneNumber::Work ) );

  addr.removePhoneNumber( addr.phoneNumber( KABC::PhoneNumber::Home ) );
  if ( !gui.mHomePhone->text().isEmpty() )
    addr.insertPhoneNumber( KABC::PhoneNumber( gui.mHomePhone->text(), KABC::PhoneNumber::Home ) );

  addr.removePhoneNumber( addr.phoneNumber( KABC::PhoneNumber::Cell ) );
  if ( !gui.mMobilePhone->text().isEmpty() )
    addr.insertPhoneNumber( KABC::PhoneNumber( gui.mMobilePhone->text(), KABC::PhoneNumber::Cell ) );

  addr.removePhoneNumber( addr.phoneNumber( KABC::PhoneNumber::Fax ) );
  if ( !gui.mFaxPhone->text().isEmpty() )
    addr.insertPhoneNumber( KABC::PhoneNumber( gui.mFaxPhone->text(), KABC::PhoneNumber::Fax ) );

  addr.removePhoneNumber( addr.phoneNumber( KABC::PhoneNumber::Pager ) );
  if ( !gui.mPagerPhone->text().isEmpty() )
    addr.insertPhoneNumber( KABC::PhoneNumber( gui.mPagerPhone->text(), KABC::PhoneNumber::Pager ) );

  // Address Home
  addr.removeAddress( addr.address( KABC::Address::Home ) );
  if ( !(gui.mHomeStreet->text().isEmpty() &&
         gui.mHomeCity->text().isEmpty() &&
         gui.mHomeState->text().isEmpty() &&
         gui.mHomePostalCode->text().isEmpty() &&
         gui.mHomeCountry->text().isEmpty()) ) {
    KABC::Address homeAddress( KABC::Address::Home );
    homeAddress.setStreet( gui.mHomeStreet->text() );
    homeAddress.setLocality( gui.mHomeCity->text() );
    homeAddress.setRegion( gui.mHomeState->text() );
    homeAddress.setPostalCode( gui.mHomePostalCode->text() );
    homeAddress.setCountry( gui.mHomeCountry->text() );

    addr.insertAddress( homeAddress );
  }

  // Address Work
  addr.setTitle( gui.mTitle->text() );
  addr.setDepartment( gui.mDepartment->text() );
  addr.setOrganization( gui.mOrganization->text() );

  addr.removeAddress( addr.address( KABC::Address::Work ) );
  if ( !(gui.mWorkStreet->text().isEmpty() &&
         gui.mWorkCity->text().isEmpty() &&
         gui.mWorkState->text().isEmpty() &&
         gui.mWorkPostalCode->text().isEmpty() &&
         gui.mWorkCountry->text().isEmpty()) ) {
    KABC::Address workAddress( KABC::Address::Work );
    workAddress.setStreet( gui.mWorkStreet->text() );
    workAddress.setLocality( gui.mWorkCity->text() );
    workAddress.setRegion( gui.mWorkState->text() );
    workAddress.setPostalCode( gui.mWorkPostalCode->text() );
    workAddress.setCountry( gui.mWorkCountry->text() );

    addr.insertAddress( workAddress );
  }

  // Customs

  // Notes
  addr.setNote( gui.mNote->toPlainText() );
}

void KABCItemEditor::Private::setupMonitor()
{
  delete mMonitor;
  mMonitor = new Monitor;
  mMonitor->ignoreSession( Session::defaultSession() );

  connect( mMonitor, SIGNAL( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ),
           mParent, SLOT( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ) );
}


KABCItemEditor::KABCItemEditor( Mode mode, QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  d->mMode = mode;
  d->gui.setupUi( this );
}


KABCItemEditor::~KABCItemEditor()
{
  delete d;
}

void KABCItemEditor::loadContact( const Akonadi::Item &item )
{
  if ( d->mMode == CreateMode )
    Q_ASSERT_X( false, "KABCItemEditor::loadContact", "You are calling loadContact in CreateMode!" );

  ItemFetchJob *job = new ItemFetchJob( item );
  job->fetchScope().fetchFullPayload();

  connect( job, SIGNAL( result( KJob* ) ), SLOT( fetchDone( KJob* ) ) );

  d->setupMonitor();
  d->mMonitor->setItemMonitored( item );

  new WaitingOverlay( job, this );
}

void KABCItemEditor::saveContact()
{
  if ( d->mMode == EditMode ) {
    if ( !d->mItem.isValid() )
      return;

    KABC::Addressee addr = d->mItem.payload<KABC::Addressee>();

    d->storeContact( addr );

    d->mItem.setPayload<KABC::Addressee>( addr );

    ItemModifyJob *job = new ItemModifyJob( d->mItem );
    connect( job, SIGNAL( result( KJob* ) ), SLOT( storeDone( KJob* ) ) );
  } else if ( d->mMode == CreateMode ) {
    Q_ASSERT_X( d->mDefaultCollection.isValid(), "KABCItemEditor::saveContact", "Using invalid default collection for saving!" );

    KABC::Addressee addr;
    d->storeContact( addr );

    Item item;
    item.setPayload<KABC::Addressee>( addr );
    item.setMimeType( KABC::Addressee::mimeType() );

    ItemCreateJob *job = new ItemCreateJob( item, d->mDefaultCollection );
    connect( job, SIGNAL( result( KJob* ) ), SLOT( storeDone( KJob* ) ) );
  }
}

void KABCItemEditor::setDefaultCollection( const Akonadi::Collection &collection )
{
  d->mDefaultCollection = collection;
}

#include "kabcitemeditor.moc"
