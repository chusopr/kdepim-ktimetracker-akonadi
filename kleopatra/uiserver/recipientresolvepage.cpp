/* -*- mode: c++; c-basic-offset:4 -*-
    certificatepickerwidget.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "recipientresolvepage.h"
#include "keyselectiondialog.h"
#include "models/keycache.h"
#include "utils/formatting.h"

#include <gpgme++/key.h>

#include <KLocale>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QGridLayout>
#include <QHash>
#include <QRadioButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>

#include <cassert>
 
using namespace Kleo;

class Kleo::RecipientResolvePage::Private {
    friend class ::RecipientResolvePage;
    RecipientResolvePage * const q;
public:
    explicit Private( RecipientResolvePage * qq );
    ~Private();
    void addWidgetForIdentifier( const QString& identifier );
    void protocolSelected( int );
    void setSelectedProtocol( GpgME::Protocol protocol );

    void updateRadioButtonVisibility();

//    void completionStateChanged( const QString& id );
    void clear();
    QRadioButton* pgpRB;
    QRadioButton* smimeRB;
    QScrollArea* scrollArea;
    QVBoxLayout* lineLayout;
    std::vector<RecipientResolveWidget*> widgets;
    QStringList identifiers;
    GpgME::Protocol presetProtocol;
    GpgME::Protocol selectedProtocol;
    bool allowMultipleProtocols;
};


RecipientResolvePage::Private::Private( RecipientResolvePage * qq )
    : q( qq ), presetProtocol( GpgME::UnknownProtocol ), selectedProtocol( presetProtocol ), allowMultipleProtocols( false )
{
}

RecipientResolvePage::Private::~Private() {}

RecipientResolvePage::RecipientResolvePage( QWidget * parent )
    : WizardPage( parent ), d( new Private( this ) )
{
    QVBoxLayout* const top = new QVBoxLayout( this );
    QButtonGroup* const buttonGroup = new QButtonGroup( this );
    d->pgpRB = new QRadioButton;
    d->pgpRB->setText( i18n( "OpenPGP" ) );
    d->pgpRB->setChecked( true );
    top->addWidget( d->pgpRB );
    buttonGroup->addButton( d->pgpRB, GpgME::OpenPGP );
    d->smimeRB = new QRadioButton;
    d->smimeRB->setText( i18n( "S/MIME" ) );
    top->addWidget( d->smimeRB );
    buttonGroup->addButton( d->smimeRB, GpgME::CMS );
    connect( buttonGroup, SIGNAL( buttonClicked( int ) ), 
             this, SLOT( protocolSelected( int ) ) );
    d->scrollArea = new QScrollArea( this );
    d->scrollArea->setFrameShape( QFrame::NoFrame );
    d->scrollArea->setWidgetResizable( true );
    QWidget* const container = new QWidget;
    d->lineLayout = new QVBoxLayout( container );
    QWidget* container2 = new QWidget;
    QVBoxLayout* const layout = new QVBoxLayout( container2 );
    layout->addWidget( container );
    layout->addStretch();
    d->scrollArea->setWidget( container2 );
    top->addWidget( d->scrollArea );

    d->updateRadioButtonVisibility();
}

RecipientResolvePage::~RecipientResolvePage() {}

bool RecipientResolvePage::isComplete() const
{
    Q_FOREACH ( RecipientResolveWidget* const i, d->widgets )
    {
        if ( !i->isComplete() )
            return false;
    }
    return true;
}

void RecipientResolvePage::setIdentifiers( const QStringList& ids )
{
    d->clear();
    Q_FOREACH ( const QString& i, ids )
        d->addWidgetForIdentifier( i );
}

void RecipientResolvePage::Private::protocolSelected( int protocol )
{
    assert( !allowMultipleProtocols );
    setSelectedProtocol( static_cast<GpgME::Protocol>( protocol ) );
    emit q->selectedProtocolChanged();
}

void RecipientResolvePage::Private::setSelectedProtocol( GpgME::Protocol protocol )
{
    assert( protocol != GpgME::UnknownProtocol );
    selectedProtocol = protocol;
    for ( uint i = 0; i < widgets.size(); ++i )
    {
        widgets[i]->setProtocol( selectedProtocol );
    }


}

void RecipientResolvePage::Private::clear()
{
    qDeleteAll( widgets );
    widgets.clear();
}

void RecipientResolvePage::Private::addWidgetForIdentifier( const QString& id )
{
    RecipientResolveWidget* const line = new RecipientResolveWidget;
    q->connect( line, SIGNAL( changed() ), q, SIGNAL( completeChanged() ) );
    line->setIdentifier( id );
    line->setProtocol( selectedProtocol );
    widgets.push_back( line );
    identifiers.push_back( id );
    assert( scrollArea->widget() );
    lineLayout->addWidget( line );
    line->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
    line->show();
    emit q->completeChanged();
}

QStringList RecipientResolvePage::identifiers() const
{
    return d->identifiers;
}

void RecipientResolvePage::ensureIndexAvailable( unsigned int idx )
{
    if ( idx < d->widgets.size() )
        return;
    for ( int i = 0; idx - d->widgets.size() + 1; ++i )
        d->addWidgetForIdentifier( QString() );
    assert( d->widgets.size() == idx + 1 );
}

unsigned int RecipientResolvePage::numRecipientResolveWidgets() const
{
    return d->widgets.size();
}

RecipientResolveWidget * RecipientResolvePage::recipientResolveWidget( unsigned int idx ) const
{
    return d->widgets[idx];
}

class Kleo::RecipientResolveWidget::Private {
    friend class Kleo::RecipientResolveWidget;
    RecipientResolveWidget * const q;
public:
    explicit Private( RecipientResolveWidget * qq );

    void selectAnotherCertificate();
    void currentIndexChanged( int );

private:

    QVariant currentData() const;
    void addCertificate( const GpgME::Key& key );
 
    QString m_identifier;
    QComboBox* m_combo;
    QLabel* m_recipientLabel;
    QPushButton* m_selectButton;
    QCheckBox* m_rememberChoiceCO;
    GpgME::Protocol m_protocol;
};

RecipientResolveWidget::Private::Private( RecipientResolveWidget * qq ) : q( qq ), m_identifier(), m_protocol( GpgME::UnknownProtocol )
{
    QGridLayout* const layout = new QGridLayout( q );
    m_recipientLabel = new QLabel;
    layout->addWidget( m_recipientLabel, 0, 0, /*rowSpan=*/1, /*columnSpan=*/-1 );
    QLabel* const certificateLabel = new QLabel;
    certificateLabel->setText( i18n( "Certificate:" ) );
    layout->addWidget( certificateLabel, 1, 0 ); 
    m_combo = new QComboBox;
    m_combo->setSizeAdjustPolicy( QComboBox::AdjustToMinimumContentsLengthWithIcon );
    m_combo->setMinimumContentsLength( 40 );
    certificateLabel->setBuddy( m_combo );
    layout->addWidget( m_combo, 1, 1 );
    m_selectButton = new QPushButton;
    m_selectButton->setText( i18n( "..." ) );
    m_selectButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ); 
    q->connect( m_selectButton, SIGNAL( clicked() ), SLOT( selectAnotherCertificate() ) );
    layout->addWidget( m_selectButton, 1, 2 );
    m_rememberChoiceCO = new QCheckBox;
    m_rememberChoiceCO->setText( i18n( "Remember choice" ) );
    layout->addWidget( m_rememberChoiceCO, 2, 0, /*rowSpan=*/1, /*columnSpan=*/-1 );
}

RecipientResolveWidget::RecipientResolveWidget( QWidget* parent ) : QWidget( parent ), d( new Private( this ) )
{
}

bool RecipientResolveWidget::rememberSelection() const
{
    return d->m_rememberChoiceCO->checkState() == Qt::Checked;
}

void RecipientResolveWidget::Private::addCertificate( const GpgME::Key& key )
{
    m_combo->addItem( Formatting::formatForComboBox( key ), QByteArray( key.keyID() ) );
}

void RecipientResolveWidget::setIdentifier( const QString& id )
{
    
    d->m_identifier = id;
    d->m_recipientLabel->setText( i18nc( "%1: email or name", "Recipient: %1", id ) );
}

void RecipientResolveWidget::setCertificates( const std::vector<GpgME::Key>& keys )
{
    d->m_combo->clear();
    if ( keys.empty() )
        return;
    Q_FOREACH ( const GpgME::Key& i, keys )
        d->addCertificate( i );
    emit changed();
}

GpgME::Key RecipientResolveWidget::chosenCertificate() const
{
    const QByteArray id = d->currentData().toByteArray();
    return KeyCache::instance()->findByKeyIDOrFingerprint( id.constData() );
}

void RecipientResolveWidget::Private::selectAnotherCertificate()
{
    QPointer<KeySelectionDialog> dlg = new KeySelectionDialog( q );
    dlg->setSelectionMode( KeySelectionDialog::SingleSelection );
    dlg->setProtocol( m_protocol );
    dlg->setAllowedKeys( KeySelectionDialog::EncryptOnly );
    dlg->addKeys( KeyCache::instance()->keys() );
    if ( dlg->exec() == QDialog::Accepted )
    {
        const std::vector<GpgME::Key> keys = dlg->selectedKeys();
        if ( !keys.empty() )
        {
            addCertificate( keys[0] );
            emit q->changed();
            //TODO: make sure keys[0] gets selected
        }
    }

    delete dlg;
}

void RecipientResolveWidget::Private::currentIndexChanged( int )
{
    emit q->changed();
}

QVariant RecipientResolveWidget::Private::currentData() const
{
    return m_combo->itemData( m_combo->currentIndex() ); 
}

bool RecipientResolveWidget::isComplete() const
{
    return !d->currentData().isNull();
}

void RecipientResolveWidget::setProtocol( GpgME::Protocol prot )
{
    d->m_protocol = prot; 
}

GpgME::Protocol RecipientResolveWidget::protocol() const
{
    return d->m_protocol;
}


void RecipientResolvePage::setPresetProtocol( GpgME::Protocol prot )
{
    d->presetProtocol = prot;
    if ( d->selectedProtocol == GpgME::UnknownProtocol )
        d->selectedProtocol = prot;
    for ( uint i = 0; i < d->widgets.size(); ++i )
    {
        d->widgets[i]->setProtocol( d->selectedProtocol );
    }
    d->updateRadioButtonVisibility();
}

GpgME::Protocol RecipientResolvePage::presetProtocol() const
{
    return d->presetProtocol;
}


GpgME::Protocol RecipientResolvePage::selectedProtocol() const
{
    return d->selectedProtocol;
}

bool RecipientResolvePage::isMultipleProtocolsAllowed() const
{
    return d->allowMultipleProtocols;
}

void RecipientResolvePage::setMultipleProtocolsAllowed( bool allowed )
{
    d->allowMultipleProtocols = allowed;
    d->updateRadioButtonVisibility();
}


void RecipientResolvePage::Private::updateRadioButtonVisibility()
{
    const bool rbsVisible = !allowMultipleProtocols && presetProtocol == GpgME::UnknownProtocol;
    pgpRB->setVisible( rbsVisible ); 
    smimeRB->setVisible( rbsVisible );
    if ( rbsVisible )
        setSelectedProtocol( pgpRB->isChecked() ? GpgME::OpenPGP : GpgME::CMS );
}

#include "moc_recipientresolvepage.cpp"
