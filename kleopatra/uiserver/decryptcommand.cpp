/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptemailcommand.cpp

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

#include "decryptcommand.h"

#include <QObject>
#include <QIODevice>

#include <kleo/decryptjob.h>
#include <kleo/cryptobackendfactory.h>

#include <gpgme++/error.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/verificationresult.h>

#include <gpg-error.h>

#include <cassert>

class Kleo::DecryptCommand::Private : public QObject
{
    Q_OBJECT
public:
    Private( DecryptCommand * qq )
        :q( qq ), backend(0)
    {}

    DecryptCommand *q;
    const CryptoBackend::Protocol *backend;
    void findCryptoBackend();

public Q_SLOTS:
    void slotDecryptionResult( const GpgME::DecryptionResult &, const QByteArray & plainText );
    void slotProgress( const QString& what, int current, int total );

};

Kleo::DecryptCommand::DecryptCommand()
    : AssuanCommandMixin<DecryptCommand>(),
      d( new Private( this ) )
{
}

Kleo::DecryptCommand::~DecryptCommand() {}

void Kleo::DecryptCommand::Private::findCryptoBackend()
{
    // FIXME this could be either SMIME or OpenPGP, find out from headers
    const bool isSMIME = false;
    if ( isSMIME )
        backend = Kleo::CryptoBackendFactory::instance()->smime();
    else
        backend = Kleo::CryptoBackendFactory::instance()->openpgp();
}


void Kleo::DecryptCommand::Private::slotProgress( const QString& what, int current, int total )
{
    // FIXME report progress, via sendStatus()
}

void Kleo::DecryptCommand::Private::slotDecryptionResult( const GpgME::DecryptionResult & decryptionResult, const QByteArray & plainText )
{
    const GpgME::Error decryptionError = decryptionResult.error();
    if ( decryptionError )
    {
        q->done( decryptionError );
        return;
    }
    
    //handle result, send status
    q->bulkOutputDevice( "OUT" )->write( plainText );
    q->done();
}

int Kleo::DecryptCommand::doStart()
{
    // FIXME check options

    if ( !bulkInputDevice( "IN" ) )
        done( GPG_ERR_ASS_NO_INPUT );

    if ( !bulkOutputDevice( "OUT" ) )
        done( GPG_ERR_ASS_NO_OUTPUT );

    d->findCryptoBackend(); // decide on smime or openpgp
    assert(d->backend);

    //fire off appropriate kleo decrypt verify job
    Kleo::DecryptJob * const job = d->backend->decryptJob();
    assert(job);

    QObject::connect( job, SIGNAL( result( GpgME::DecryptionResult, QByteArray ) ),
                      d.get(), SLOT( slotDecryptionResult( GpgME::DecryptionResult, QByteArray ) ) );
    QObject::connect( job, SIGNAL( progress( QString, int, int ) ),
                      d.get(), SLOT( slotProgress( QString, int, int ) ) );

    const QByteArray encrypted = bulkInputDevice( "IN" )->readAll(); // FIXME safe enough?

    // FIXME handle cancelled, let job show dialog? both done and return error?
    const GpgME::Error error = job->start( encrypted );
    if ( error )
        done( error );
    return error;
}

void Kleo::DecryptCommand::doCanceled()
{
}

#include "decryptcommand.moc"

