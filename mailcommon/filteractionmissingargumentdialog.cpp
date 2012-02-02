/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2011 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "filteractionmissingargumentdialog.h"
#include "mailkernel.h"
#include "mailutil.h"
#include "folderrequester.h"
#include "kmfilterdialog.h"

#include <Akonadi/EntityMimeTypeFilterModel>

#include <mailtransport/transportcombobox.h>
#include <mailtransport/transport.h>
#include <mailtransport/transportmanager.h>

#include <kpimidentities/identitycombo.h>

#include <KLocale>
#include <KUrlRequester>

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>

FilterActionMissingCollectionDialog::FilterActionMissingCollectionDialog(
  const Akonadi::Collection::List &list, const QString &filtername, const QString& argStr, QWidget *parent )
  : KDialog( parent ), mListwidget( 0 )
{
  setModal( true );
  setCaption( i18n( "Select Folder" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  QVBoxLayout* lay = new QVBoxLayout( mainWidget() );
  QLabel * lab = new QLabel(i18n("Folder path was \"%1\".", argStr));
  lay->addWidget( lab );
  if ( !list.isEmpty() ) {
    lab =
      new QLabel( i18n( "The following folders can be used for this filter:" ) );
    lay->addWidget( lab );
    mListwidget = new QListWidget( this );
    lay->addWidget( mListwidget );
    const int numberOfItems( list.count() );
    for ( int i = 0; i< numberOfItems; ++i ) {
      Akonadi::Collection col = list.at( i );
      QListWidgetItem *item = new QListWidgetItem( MailCommon::Util::fullCollectionPath( col ) );
      item->setData( FilterActionMissingCollectionDialog::IdentifyCollection, col.id() );
      mListwidget->addItem(  item );
    }
    connect( mListwidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
             SLOT(slotCurrentItemChanged()));
    connect( mListwidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
             SLOT(slotDoubleItemClicked(QListWidgetItem*)));

  }

  QLabel *label = new QLabel( this );
  label->setText( i18n( "Filter folder is missing. "
                        "Please select a folder to use with filter \"%1\"",
                        filtername ) );
  lay->addWidget( label );
  mFolderRequester = new MailCommon::FolderRequester( this );
  connect( mFolderRequester, SIGNAL(folderChanged(Akonadi::Collection)),
           this, SLOT(slotFolderChanged(Akonadi::Collection)) );
  lay->addWidget( mFolderRequester );
  enableButtonOk( false );
}

FilterActionMissingCollectionDialog::~FilterActionMissingCollectionDialog()
{
}

void FilterActionMissingCollectionDialog::slotFolderChanged( const Akonadi::Collection&col )
{
  enableButtonOk( col.isValid() );
}


void FilterActionMissingCollectionDialog::slotDoubleItemClicked(QListWidgetItem*item)
{
  if(!item)
      return;
  const Akonadi::Collection::Id id =  item->data( FilterActionMissingCollectionDialog::IdentifyCollection ).toLongLong();
  mFolderRequester->setCollection(Akonadi::Collection( id ));
  accept();
}

void FilterActionMissingCollectionDialog::slotCurrentItemChanged()
{
  QListWidgetItem * currentItem = mListwidget->currentItem();
  if ( currentItem ) {
    const Akonadi::Collection::Id id =  currentItem->data( FilterActionMissingCollectionDialog::IdentifyCollection ).toLongLong();
    mFolderRequester->setCollection(Akonadi::Collection( id ));
  }
}

Akonadi::Collection FilterActionMissingCollectionDialog::selectedCollection() const
{
  return mFolderRequester->collection();
}

void FilterActionMissingCollectionDialog::getPotentialFolders(  const QAbstractItemModel *model, const QModelIndex& parentIndex, const QString& lastElement, Akonadi::Collection::List& list )
{
  const int rowCount = model->rowCount( parentIndex );
  for ( int row = 0; row < rowCount; ++row ) {
    const QModelIndex index = model->index( row, 0, parentIndex );
    if ( model->rowCount( index ) > 0 ) {
      getPotentialFolders( model, index, lastElement, list );
    }
    if ( model->data( index ).toString() == lastElement )
      list << model->data( index, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
  }
}

Akonadi::Collection::List FilterActionMissingCollectionDialog::potentialCorrectFolders( const QString& path, bool & exactPath )
{
  Akonadi::Collection::List lst;
  const QString realPath = MailCommon::Util::realFolderPath( path );
  if ( realPath.isEmpty() )
    return lst;
  const int lastSlash = realPath.lastIndexOf( QLatin1Char( '/' ) );
  QString lastElement;
  if ( lastSlash == -1 )
    lastElement = realPath;
  else
    lastElement = realPath.right( realPath.length() - lastSlash - 1 );

  if ( KernelIf->collectionModel() ) {
    FilterActionMissingCollectionDialog::getPotentialFolders( KernelIf->collectionModel(),  QModelIndex(), lastElement,lst ) ;
    const int numberOfItems( lst.count() );
    for ( int i = 0; i < numberOfItems; ++i ) {
      if ( MailCommon::Util::fullCollectionPath( lst.at( i ) ) == realPath ) {
        exactPath = true;
        return  Akonadi::Collection::List()<< lst.at( i );
      }
    }
  }
  return lst;
}


FilterActionMissingIdentityDialog::FilterActionMissingIdentityDialog( const QString & filtername, QWidget *parent )
  : KDialog( parent )
{
  setModal( true );
  setCaption( i18n( "Select Identity" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  QVBoxLayout* lay = new QVBoxLayout( mainWidget() );
  QLabel *label = new QLabel( this );
  label->setText( i18n( "Filter identity is missing. "
                        "Please select an identity to use with filter \"%1\"",
                        filtername ) );
  lay->addWidget( label );
  mComboBoxIdentity = new KPIMIdentities::IdentityCombo( KernelIf->identityManager(), this );
  lay->addWidget( mComboBoxIdentity );

}

FilterActionMissingIdentityDialog::~FilterActionMissingIdentityDialog()
{
}

int FilterActionMissingIdentityDialog::selectedIdentity() const
{
  return mComboBoxIdentity->currentIdentity();
}


FilterActionMissingTransportDialog::FilterActionMissingTransportDialog( const QString & filtername, QWidget *parent )
  : KDialog( parent )
{
  setModal( true );
  setCaption( i18n( "Select Transport" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  QVBoxLayout* lay = new QVBoxLayout( mainWidget() );
  QLabel *label = new QLabel( this );
  label->setText( i18n( "Filter transport is missing. "
                        "Please select a transport to use with filter \"%1\"",
                        filtername ) );
  lay->addWidget( label );
  mComboBoxTransport = new MailTransport::TransportComboBox( this );
  lay->addWidget( mComboBoxTransport );

}

FilterActionMissingTransportDialog::~FilterActionMissingTransportDialog()
{
}

int FilterActionMissingTransportDialog::selectedTransport() const
{
  return mComboBoxTransport->currentTransportId();
}

FilterActionMissingTemplateDialog::FilterActionMissingTemplateDialog( const QStringList& templateList, const QString & filtername, QWidget *parent )
  : KDialog( parent )
{
  setModal( true );
  setCaption( i18n( "Select Template" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  QVBoxLayout* lay = new QVBoxLayout( mainWidget() );
  QLabel *label = new QLabel( this );
  label->setText( i18n( "Filter template is missing. "
                        "Please select a template to use with filter \"%1\"",
                        filtername ) );
  lay->addWidget( label );
  mComboBoxTemplate = new KComboBox( this );
  mComboBoxTemplate->addItems( templateList );
  lay->addWidget( mComboBoxTemplate );
}

FilterActionMissingTemplateDialog::~FilterActionMissingTemplateDialog()
{
}

QString FilterActionMissingTemplateDialog::selectedTemplate() const
{
  if ( mComboBoxTemplate->currentIndex() == 0 )
    return QString();
  else
    return mComboBoxTemplate->currentText();
}


FilterActionMissingAccountDialog::FilterActionMissingAccountDialog(const QStringList &lstAccount, const QString& filtername, QWidget * parent )
  : KDialog( parent )
{
  setModal( true );
  setCaption( i18n( "Select Account" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  QVBoxLayout* lay = new QVBoxLayout( mainWidget() );
  QLabel *label = new QLabel( this );
  label->setText( i18n( "Filter account is missing. "
                        "Please select account to use with filter \"%1\"",
                        filtername ) );
  lay->addWidget( label );
  mAccountList = new MailCommon::AccountList( this );
  mAccountList->applyOnAccount(lstAccount);
  lay->addWidget( mAccountList );
}

FilterActionMissingAccountDialog::~FilterActionMissingAccountDialog()
{
}

QStringList FilterActionMissingAccountDialog::selectedAccount() const
{
  return mAccountList->selectedAccount();
}

bool FilterActionMissingAccountDialog::allAccountExist( const QStringList & lst )
{
  const Akonadi::AgentInstance::List lstAgent = MailCommon::Util::agentInstances();

  const int numberOfAccount( lst.count() );
  const int numberOfAgent(  lstAgent.count() );

  for ( int i = 0; i <numberOfAccount; ++i ) {
    bool found = false;
    const QString accountName( lst.at( i ) );
    for ( int j=0; j < numberOfAgent;++j ) {
      if ( lstAgent.at( j ).identifier() ==  accountName ) {
        found = true;
        break;
      }
    }
    if ( !found )
      return false;
  }
  return true;
}

FilterActionMissingTagDialog::FilterActionMissingTagDialog( const QStringList & tagList,  const QString& filtername, const QString& argsStr, QWidget *parent )
  : KDialog( parent )
{
  setModal( true );
  setCaption( i18n( "Select Tag" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  QVBoxLayout* lay = new QVBoxLayout( mainWidget() );
  QLabel *label = new QLabel(i18n("Tag was \"%1\".", argsStr));
  lay->addWidget( label );

  label = new QLabel( this );
  label->setText( i18n( "Filter tag is missing. "
                        "Please select a tag to use with filter \"%1\"",
                        filtername ) );
  lay->addWidget( label );
  mTagList = new QListWidget( this );
  mTagList->addItems( tagList );
  lay->addWidget( mTagList );

}

FilterActionMissingTagDialog::~FilterActionMissingTagDialog()
{
}

QString FilterActionMissingTagDialog::selectedTag() const
{
  if ( mTagList->currentItem() )
    return mTagList->currentItem()->text();
  return QString();
}

FilterActionMissingSoundUrlDialog::FilterActionMissingSoundUrlDialog( const QString & filtername, const QString& argStr,QWidget *parent )
    :KDialog(parent)
{
    setModal( true );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    setCaption(i18n("Select sound"));
    showButtonSeparator( true );
    QVBoxLayout* lay = new QVBoxLayout( mainWidget() );
    QLabel *label = new QLabel( i18n("Sound file was \"%1\".", argStr ));
    lay->addWidget( label );

    label = new QLabel( this );
    label->setText( i18n( "Sound file is missing. Please select a sound to use with filter \"%1\"", filtername ) );
    lay->addWidget( label );
    mUrlWidget = new KUrlRequester( this );
    lay->addWidget( mUrlWidget );
}

FilterActionMissingSoundUrlDialog::~FilterActionMissingSoundUrlDialog()
{
}

QString FilterActionMissingSoundUrlDialog::soundUrl() const
{
    return mUrlWidget->url().path();
}


#include "filteractionmissingargumentdialog.moc"

