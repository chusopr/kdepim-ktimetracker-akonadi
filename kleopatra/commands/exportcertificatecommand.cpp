/* -*- mode: c++; c-basic-offset:4 -*-
    exportcertificatecommand.cpp

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

#include "exportcertificatecommand.h"
#include "exportcertificatesdialog.h"

#include <kleo/cryptobackend.h>
#include <kleo/cryptobackendfactory.h>
#include <kleo/exportjob.h>

#include <gpgme++/key.h>

#include <KLocale>
#include <KMessageBox>
#include <KSaveFile>

#include <QDataStream>
#include <QFileDialog>
#include <QMap>
#include <QPointer>
#include <QTextStream>

#include <boost/bind.hpp>
#include <algorithm>
#include <vector>
#include <cassert>

using namespace boost;
using namespace GpgME;
using namespace Kleo;

class ExportCertificateCommand::Private {
    friend class ::ExportCertificateCommand;
    ExportCertificateCommand * const q;
public:
    explicit Private( ExportCertificateCommand * qq );
    ~Private();
    void startExportJob( GpgME::Protocol protocol, const std::vector<Key>& keys );
    void cancelJobs();
    void exportResult( const GpgME::Error&, const QByteArray& );
    void showError( const GpgME::Error& error );

    bool requestFileNames( Protocol prot );
    void finishedIfLastJob();

private:
    bool textArmor;
    QWidget* parent;
    QMap<GpgME::Protocol, QString> fileNames;
    std::vector<GpgME::Key> certificates;
    uint jobsPending;
    QMap<QObject*, QString> outFileForSender;
    QPointer<ExportJob> cmsJob;
    QPointer<ExportJob> pgpJob;
};


ExportCertificateCommand::Private::Private( ExportCertificateCommand * qq )
    : q( qq ), textArmor( true ), parent( 0 ), jobsPending( 0 )
{
    
}

ExportCertificateCommand::Private::~Private() 
{
    cancelJobs();
}


ExportCertificateCommand::ExportCertificateCommand( KeyListController* controller )
  : Command( controller ), d( new Private( this ) )
{
    
}

ExportCertificateCommand::~ExportCertificateCommand() {}

void ExportCertificateCommand::doStart()
{
    std::vector<Key> keys = d->certificates;
    if ( keys.empty() )
        return;

    const std::vector<Key>::iterator firstCms = std::partition( keys.begin(), keys.end(), bind( &GpgME::Key::protocol, _1 ) != CMS );

    std::vector<Key> openpgp, cms; 
    std::copy( keys.begin(), firstCms, std::back_inserter( openpgp ) ); 
    std::copy( firstCms, keys.end(), std::back_inserter( cms ) ); 
    assert( !openpgp.empty() || !cms.empty() );
    const bool haveBoth = !cms.empty() && !openpgp.empty();
    const GpgME::Protocol prot = haveBoth ? UnknownProtocol : ( !cms.empty() ? CMS : OpenPGP );
    if ( !d->requestFileNames( prot ) )
    {
        emit canceled();
        return;
    }

    if ( !openpgp.empty() )
        d->startExportJob( GpgME::OpenPGP, openpgp );
    if ( !cms.empty() )
        d->startExportJob( GpgME::CMS, cms );
}
    
bool ExportCertificateCommand::Private::requestFileNames( GpgME::Protocol protocol )
{
    fileNames[OpenPGP] = QString();
    fileNames[CMS] = QString();
    if ( protocol == UnknownProtocol )
    {
        QPointer<ExportCertificatesDialog> dlg( new ExportCertificatesDialog( parent ) );
        const bool accepted = dlg->exec() == QDialog::Accepted;
        if ( accepted )
        {
            fileNames[OpenPGP] = dlg->openPgpExportFileName();
            fileNames[CMS] = dlg->cmsExportFileName();
        }
        delete dlg;
        return accepted;
    }

    const QString fname = QFileDialog::getSaveFileName( parent, i18n( "Export Certificates" ), QString(), protocol == GpgME::OpenPGP ? i18n( "OpenPGP Certificates (.asc)" ) : i18n( "S/MIME Certificates (.pem)" ) );
    fileNames[protocol] = fname;
    return !fname.isNull();
}

void ExportCertificateCommand::Private::startExportJob( GpgME::Protocol protocol, const std::vector<Key>& keys )
{
    assert( protocol != GpgME::UnknownProtocol );

    const CryptoBackend::Protocol* const backend = CryptoBackendFactory::instance()->protocol( protocol );
    assert( backend );
    std::auto_ptr<ExportJob> job( backend->publicKeyExportJob( /*armor=*/true ) );
    assert( job.get() );

    connect( job.get(), SIGNAL( result( GpgME::Error, QByteArray ) ),
             q, SLOT( exportResult( GpgME::Error, QByteArray ) ) );

    connect( job.get(), SIGNAL( progress( QString, int, int ) ),
             q, SIGNAL( progress( QString, int, int ) ) );

    QStringList fingerprints;
    Q_FOREACH ( const Key& i, keys )
        fingerprints << i.primaryFingerprint();

    const GpgME::Error err = job->start( fingerprints );
    if ( err ) {
        showError( err );
        emit q->finished();
        return;
    }
    emit q->info( i18n( "Exporting certificates..." ) );
    ++jobsPending;
    const QPointer<ExportJob> exportJob( job.release() );

    outFileForSender[exportJob] = fileNames[protocol];
    ( protocol == CMS ? cmsJob : pgpJob ) = exportJob;
}

void ExportCertificateCommand::Private::showError( const GpgME::Error& err )
{
    assert( err );
    const QString msg = i18n("<qt><p>An error occurred while trying to export "
                             "the certificate:</p>"
                             "<p><b>%1</b></p></qt>",
                             QString::fromLocal8Bit( err.asString() ) );
    KMessageBox::error( parent, msg, i18n("Certificate Export Failed") );
}

void ExportCertificateCommand::doCancel()
{
    d->cancelJobs();
}

void ExportCertificateCommand::Private::finishedIfLastJob()
{
    if ( jobsPending > 0 )
        return;
    emit q->finished();
}

void ExportCertificateCommand::Private::exportResult( const GpgME::Error& err, const QByteArray& data )
{
    assert( jobsPending > 0 );
    --jobsPending;

    assert( outFileForSender.contains( q->sender() ) );
    const QString outFile = outFileForSender[q->sender()];

    if ( err ) {
        showError( err );
        finishedIfLastJob();
        return;
    }
    KSaveFile savefile( outFile );
    //TODO: use KIO
    const QString writeErrorMsg = i18n( "Could not write to file %1.",  outFile );
    const QString errorCaption = i18n( "Certificate Export Failed" );
    if ( !savefile.open() )
    {
        KMessageBox::error( parent, writeErrorMsg, errorCaption );
        finishedIfLastJob();
        return;
    }
    if ( textArmor )
    {
        QTextStream out( &savefile );
        out << data;
    }
    else
    {
        QDataStream out( &savefile );
        out << data;
    }

    if ( !savefile.finalize() )
        KMessageBox::error( parent, writeErrorMsg, errorCaption );
    finishedIfLastJob();
}

void ExportCertificateCommand::setCertificates( const std::vector<GpgME::Key>& certificates )
{
    d->certificates = certificates;
}

void ExportCertificateCommand::setParentWidget( QWidget* parent )
{
    d->parent = parent;
}

void ExportCertificateCommand::Private::cancelJobs()
{
    if ( cmsJob )
        cmsJob->slotCancel();
    if ( pgpJob )
        pgpJob->slotCancel();
}

#include "moc_exportcertificatecommand.cpp"
