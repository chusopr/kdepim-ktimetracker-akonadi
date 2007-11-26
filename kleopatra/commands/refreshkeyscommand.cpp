/* -*- mode: c++; c-basic-offset:4 -*-
    refreshkeyscommand.cpp

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

#include "refreshkeyscommand.h"
#include "command_p.h"
#include <models/keycache.h>

#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/keylistjob.h>

#include <QStringList>

#include <cassert>

using namespace Kleo;

class RefreshKeysCommand::Private : public Command::Private {
    friend class ::Kleo::RefreshKeysCommand;
    RefreshKeysCommand * const mq;
public:
    Private( RefreshKeysCommand * qq, KeyListController* controller );
    ~Private();
    enum KeyType {
        PublicKeys,
        SecretKeys
    };
    void startKeyListing( const char* backend, KeyType type );
    void publicKeyListingDone( const GpgME::KeyListResult& result );
    void secretKeyListingDone( const GpgME::KeyListResult& result );

    void addKey( const GpgME::Key& key );

private:
    uint m_pubKeysJobs;
    uint m_secKeysJobs;
};

RefreshKeysCommand::Private * RefreshKeysCommand::d_func() { return static_cast<Private*>( d.get() ); }
const RefreshKeysCommand::Private * RefreshKeysCommand::d_func() const { return static_cast<const Private*>( d.get() ); }


RefreshKeysCommand::Private::Private( RefreshKeysCommand * qq, KeyListController * controller )
    : Command::Private( qq, controller ), mq( qq ), m_pubKeysJobs( 0 ), m_secKeysJobs( 0 )
{

}

RefreshKeysCommand::Private::~Private() {}


void RefreshKeysCommand::Private::startKeyListing( const char* backend, KeyType type )
{
    Kleo::KeyListJob * const job = Kleo::CryptoBackendFactory::instance()->protocol( backend )->keyListJob();
    assert( job );
    if ( type == PublicKeys ) {
        QObject::connect( job, SIGNAL( result( GpgME::KeyListResult ) ),
                          mq, SLOT( publicKeyListingDone( GpgME::KeyListResult ) ) );
    } else {
        QObject::connect( job, SIGNAL( result( GpgME::KeyListResult ) ),
                          mq, SLOT( secretKeyListingDone( GpgME::KeyListResult ) ) );
    }

    QObject::connect( job, SIGNAL( nextKey( GpgME::Key ) ),
             mq, SLOT( addKey( GpgME::Key ) ) );
    job->start( QStringList(), type == SecretKeys ); 
    ++( type == PublicKeys ? m_pubKeysJobs : m_secKeysJobs );
}

void RefreshKeysCommand::Private::publicKeyListingDone( const GpgME::KeyListResult & )
{
    assert( m_pubKeysJobs > 0 );
    --m_pubKeysJobs;
    if ( m_pubKeysJobs == 0 )
    {
        startKeyListing( "openpgp", Private::SecretKeys );
        startKeyListing( "smime", Private::SecretKeys );
    }
}


void RefreshKeysCommand::Private::secretKeyListingDone( const GpgME::KeyListResult & )
{
    assert( m_secKeysJobs > 0 );
    --m_secKeysJobs;
    if ( m_secKeysJobs == 0 )
    {
    }
}

void RefreshKeysCommand::Private::addKey( const GpgME::Key& key )
{
    KeyCache::mutableInstance()->insert( key );
}

RefreshKeysCommand::RefreshKeysCommand( KeyListController * p )
    : Command( p, new Private( this, p ) )
{

}

#define d d_func()

RefreshKeysCommand::~RefreshKeysCommand() {
}

void RefreshKeysCommand::doStart() {
    /* NOTE: first fetch public keys. when done, fetch secret keys. hasSecret() works only
       correctly when the key was retrieved with --list-secret-keys (secretOnly flag
       in gpgme keylist operations) so we overwrite the key from --list-keys (secret
       not set) with the one from --list-secret-keys (with secret set).
    */
    d->startKeyListing( "openpgp", Private::PublicKeys );
    d->startKeyListing( "smime", Private::PublicKeys );
}

void RefreshKeysCommand::doCancel() {
}

#undef d

#include "moc_refreshkeyscommand.cpp"
