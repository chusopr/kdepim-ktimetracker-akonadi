/* -*- mode: c++; c-basic-offset:4 -*-
    decryptverifytask.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include <config-kleopatra.h>

#include "decryptverifytask.h"


#include <kleo/cryptobackendfactory.h>
#include <kleo/verifyopaquejob.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/decryptjob.h>
#include <kleo/decryptverifyjob.h>

#include <models/keycache.h>
#include <models/predicates.h>

#include <utils/detail_p.h>
#include <utils/input.h>
#include <utils/output.h>
#include <utils/classify.h>
#include <utils/formatting.h>
#include <utils/stl_util.h>
#include <utils/kleo_assert.h>
#include <utils/exception.h>

#include <gpgme++/error.h>
#include <gpgme++/key.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/decryptionresult.h>

#include <gpg-error.h>

#include <KIconLoader>
#include <KLocale>
#include <KLocalizedString>

#include <QByteArray>
#include <QColor>
#include <QDateTime>
#include <QTextDocument> // Qt::escape

#include <boost/bind.hpp>

#include <algorithm>

using namespace Kleo::Crypto;
using namespace Kleo;
using namespace GpgME;
using namespace boost;

namespace {

static QString signatureSummaryToString( int summary )
{
    if ( summary & Signature::None )
        return i18n( "Error: Signature not verified" );
    else if ( summary & Signature::Valid || summary & Signature::Green )
        return i18n( "Good signature" );
    else if ( summary & Signature::Red )
        return i18n( "Bad signature" );
    else if ( summary & Signature::KeyRevoked )
        return i18n( "Signing certificate revoked" );
    else if ( summary & Signature::KeyExpired )
        return i18n( "Signing certificate expired" );
        else if ( summary & Signature::KeyMissing )
        return i18n( "No public key to verify the signature" );
    else if ( summary & Signature::SigExpired )
        return i18n( "Signature expired" );
    else if ( summary & Signature::KeyMissing )
        return i18n( "Key missing" );
    else if ( summary & Signature::CrlMissing )
        return i18n( "CRL missing" );
    else if ( summary & Signature::CrlTooOld )
        return i18n( "CRL too old" );
    else if ( summary & Signature::BadPolicy )
        return i18n( "Bad policy" );
    else if ( summary & Signature::SysError )
        return i18n( "System error" ); //### retrieve system error details?
    return QString();
}

static QString formatValidSignatureWithTrustLevel( const Key & key ) {
    assert( !key.isNull() );
    switch ( key.ownerTrust() ) {
        case Key::Marginal:
            return i18n( "The signature is valid but the trust in the certificate's validity is only marginal." );
        case Key::Full:
            return i18n( "The signature is valid and the certificate's validity is fully trusted." );
        case Key::Ultimate:
            return i18n( "The signature is valid and the certificate's validity is ultimately trusted." );
        case Key::Never:
            return i18n( "The signature is valid but the certificate's validity is <em>not trusted</em>." );
        case Key::Unknown:
            return i18n( "The signature is valid but the certificate's validity is unknown." );
        case Key::Undefined:
        default:
            return i18n( "The signature is valid but the certificate's validity is undefined." );
    }
}

static QString renderFingerprint( const char * fpr ) {
    if ( !fpr )
        return QString();
    return QString( "0x%1" ).arg( QString::fromAscii( fpr ).toUpper() );
}

static QString renderKey( const Key & key ) {
    if ( key.isNull() )
        return i18n( "Unknown key" );
    return QString::fromLatin1( "<a href=\"key:%1\">%2</a>" ).arg( key.primaryFingerprint(), Formatting::prettyName( key ) );
}

static QString renderKeyEMailOnlyNameAsFallback( const Key & key ) {
    if ( key.isNull() )
        return i18n( "Unknown key" );
    const QString email = Formatting::prettyEMail( key );
    const QString user = !email.isNull() ? email : Formatting::prettyName( key );
    return QString::fromLatin1( "<a href=\"key:%1\">%2</a>" ).arg( key.primaryFingerprint(), user );
}

static QString formatDate( const QDateTime & dt ) {
    return KGlobal::locale()->formatDateTime( dt );
}
static QString formatSigningInformation( const Signature & sig, const Key & key ) {
    if ( sig.isNull() )
        return QString();
    const QDateTime dt = QDateTime::fromTime_t( sig.creationTime() );
    const QString signer = key.isNull() ? QString() : renderKeyEMailOnlyNameAsFallback( key );
    const bool haveKey = !key.isNull();
    const bool haveSigner = !signer.isEmpty();
    const bool haveDate = dt.isValid();
    if ( !haveKey )
        if ( haveDate )
            return i18n( "Message was signed on %1 with unknown key %2.", formatDate( dt ), sig.fingerprint() );
        else
            return i18n( "Message was signed with unknown key %1.", sig.fingerprint() );
    if ( haveSigner )
        if ( haveDate )
            return i18nc( "date, key owner, key ID",
                          "Message was signed on %1 by %2 (Key ID: %3).", 
                          formatDate( dt ),
                          signer,
                          renderFingerprint( key.keyID() ) );
        else
            return i18n( "Message was signed by %1 with key %2.", signer, renderKey( key ) );
    if ( haveDate )
        return i18n( "Message was signed on %1 with key %2.", formatDate( dt ), renderKey( key ) );
    return i18n( "Message was signed with key %1.", renderKey( key ) );

}

static QString strikeOut( const QString & str, bool strike ) {
    return QString( strike ? "<s>%1</s>" : "%1" ).arg( Qt::escape( str ) );
}

static QString formatInputOutputLabel( const QString & input, const QString & output, bool inputDeleted, bool outputDeleted ) {
    if ( output.isEmpty() )
        return strikeOut( input, inputDeleted );
    return i18nc( "Input file --> Output file (rarr is arrow", "%1 &rarr; %2", 
                  strikeOut( input, inputDeleted ),
                  strikeOut( output, outputDeleted ) );
}

static const char * iconForSignature( const Signature & sig ) {
    if ( sig.summary() & Signature::Green )
        return "dialog-ok";
    if ( sig.summary() & Signature::Red )
        return "dialog-error";
    return "dialog-warning";
}

static QColor color( const DecryptionResult & dr, const VerificationResult & vr ) {
    if ( !dr.isNull() && dr.error() )
        return Qt::red;
    if ( !vr.isNull() && vr.error() )
        return Qt::red;
    return Qt::gray;
}

static QColor color( const Signature & sig ) {
    if ( sig.summary() & GpgME::Signature::Red )
        return Qt::red;
    if ( sig.summary() & GpgME::Signature::Green )
        return Qt::green;
    return Qt::yellow;
}

static bool IsErrorOrCanceled( const GpgME::Error & err )
{
    return err || err.isCanceled();
}

static bool IsErrorOrCanceled( const Result & res )
{
    return IsErrorOrCanceled( res.error() );
}

static bool IsBad( const Signature & sig ) {
    return sig.summary() & Signature::Red;
}
static bool IsValid( const Signature & sig ) {
    return sig.summary() & Signature::Valid;
}

static Task::Result::VisualCode codeForVerificationResult( const VerificationResult & res )
{
    if ( res.isNull() )
        return Task::Result::NeutralSuccess;

    const std::vector<Signature> sigs = res.signatures();
    if ( sigs.empty() )
        return Task::Result::Warning;

    if ( !std::count_if( sigs.begin(), sigs.end(), IsBad ) )
        return Task::Result::AllGood;
    if ( std::find_if( sigs.begin(), sigs.end(), IsBad ) != sigs.end() )
        return Task::Result::Danger;
    return Task::Result::Warning;
}

static QString formatVerificationResultOverview( const VerificationResult & res ) {
    if ( res.isNull() )
        return QString();

    const Error err = res.error();

    if ( err.isCanceled() )
        return i18n("<b>Verification canceled.</b>");
    else if ( err )
        return i18n( "<b>Verification failed: %1.</b>", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );

    const std::vector<Signature> sigs = res.signatures();
    const std::vector<Key> signers = KeyCache::instance()->findSigners( res );

    if ( sigs.empty() )
        return i18n( "<b>No signatures found.</b>" );

    const uint bad = std::count_if( sigs.begin(), sigs.end(), IsBad );
    if ( bad > 0 ) {
        return i18np("<b>Invalid signature.</b>", "<b>%1 invalid signatures.</b>", bad );
    }
    const uint warn = std::count_if( sigs.begin(), sigs.end(), !bind( IsValid, _1 ) );
    if ( warn > 0 )
        return i18np("<b>Not enough information to check signature validity.</b>", "<b>%1 signatures could not be verified.</b>", warn );

    //All good:
    if ( sigs.size() == 1 ) {
        const Key key = DecryptVerifyResult::keyForSignature( sigs[0], signers );
        if ( key.isNull() )
            return i18n( "<b>Signature is valid.</b>" );
        return i18n( "<b>Signed by %1</b>", renderKeyEMailOnlyNameAsFallback( key ) );
    }
    return i18np("<b>Valid signature.</b>", "<b>%1 valid signatures.</b>", sigs.size() );
}

static QString formatDecryptionResultOverview( const DecryptionResult & result )
{
    const Error err = result.error();

    if ( err.isCanceled() )
        return i18n("<b>Decryption canceled.</b>");
    else if ( err )
        return i18n( "<b>Decryption failed: %1.</b>", Qt::escape( QString::fromLocal8Bit( err.asString() ) ) );
    return i18n("<b>Decryption succeeded.</b>" );
}

static QString formatSignature( const Signature & sig, const Key & key ) {
    if ( sig.isNull() )
        return QString();

    QString text = formatSigningInformation( sig, key ) + "<br/>";
    
    const bool red = sig.summary() & Signature::Red;
    if ( sig.summary() & Signature::Valid )
        return text + formatValidSignatureWithTrustLevel( key ); // ### TODO handle key.isNull()?

    if ( red )
        if ( key.isNull() )
            if ( const char * fpr = sig.fingerprint() )
                return text + i18n("Bad signature by unknown key %1: %2", signatureSummaryToString( sig.summary() ) );
            else
                return text + i18n("Bad signature by an unknown key: %1", signatureSummaryToString( sig.summary() ) );
        else
            return text + i18n("Bad signature by %1: %2", renderKey( key ), signatureSummaryToString( sig.summary() ) );
    if ( key.isNull() )
        if ( const char * fpr = sig.fingerprint() )
            return text + i18n("Invalid signature by unknown key %1: %2", renderFingerprint( fpr ), signatureSummaryToString( sig.summary() ) );
        else
            return text + i18n("Invalid signature by an unknown key: %1", signatureSummaryToString( sig.summary() ) );
    else
        return text + i18n("Invalid signature by %1: %2", renderKey( key ), signatureSummaryToString( sig.summary() ) );
}

static QString formatVerificationResultDetails( const VerificationResult & res )
{
    const std::vector<Signature> sigs = res.signatures();
    const std::vector<Key> signers = KeyCache::instance()->findSigners( res );
    QString details;
    Q_FOREACH ( const Signature & sig, sigs ) 
        details += formatSignature( sig, DecryptVerifyResult::keyForSignature( sig, signers ) ) + '\n';
    details = details.trimmed();
    details.replace( '\n', "<br/>" );
    return details;
}

static QString formatDecryptionResultDetails( const DecryptionResult & res, const std::vector<Key> & recipients )
{
    if ( res.isNull() || !res.error() || res.error().isCanceled() )
        return QString();

    if ( recipients.empty() )
        return QString( "<i>" + i18np( "One unknown recipient.", "%1 unknown recipients.", res.numRecipients() ) + "</i>" );

    QString details;
    details += i18np( "Recipients:", "Recipients:", res.numRecipients() );
    if ( res.numRecipients() == 1 )
        return details + renderKey( recipients.front() );

    details += "<ul>";
    Q_FOREACH( const Key & key, recipients )
        details += "<li>" + renderKey( key ) + "</li>";
    if ( recipients.size() < res.numRecipients() )
        details += "<li><i>" + i18np( "One unknown recipient", "%1 unknown recipients",
                                   res.numRecipients() - recipients.size() ) + "</i></li>";

    return details + "</ul>";
}

static QString formatDecryptVerifyResultOverview( const DecryptionResult & dr, const VerificationResult & vr )
{
    if ( IsErrorOrCanceled( dr ) )
        return formatDecryptionResultOverview( dr );
    return formatVerificationResultOverview( vr );
}


static QString formatDecryptVerifyResultDetails( const DecryptionResult & dr,
                                                 const VerificationResult & vr,
                                                 const std::vector<Key> & recipients )
{
    const QString drDetails = formatDecryptionResultDetails( dr, recipients );
    if ( IsErrorOrCanceled( dr ) )
        return drDetails;
    return drDetails + ( drDetails.isEmpty() ? "" : "<br/>" ) + formatVerificationResultDetails( vr );
}

} // anon namespace

class DecryptVerifyResult::Private {
    DecryptVerifyResult* const q;
public:
    Private( DecryptVerifyOperation type,
             const VerificationResult & vr,
             const DecryptionResult & dr,
             const QByteArray & stuff,
             int errCode,
             const QString & errString,
             const QString & input,
             const QString & output,
             DecryptVerifyResult* qq ) :
                 q( qq ),
                 m_type( type ),
                 m_verificationResult( vr ),
                 m_decryptionResult( dr ),
                 m_stuff( stuff ),
                 m_error( errCode ),
                 m_errorString( errString ),
                 m_inputLabel( input ),
                 m_outputLabel( output )
    {
    }

    QString label() const {
        return formatInputOutputLabel( m_inputLabel, m_outputLabel, false, q->hasError() );
    }

    bool isDecryptOnly() const { return m_type == Decrypt; }
    bool isVerifyOnly() const { return m_type == Verify; }
    bool isDecryptVerify() const { return m_type == DecryptVerify; }
    DecryptVerifyOperation m_type;
    VerificationResult m_verificationResult;
    DecryptionResult m_decryptionResult;
    QByteArray m_stuff;
    int m_error;
    QString m_errorString;
    QString m_inputLabel;
    QString m_outputLabel;
};

shared_ptr<DecryptVerifyResult> AbstractDecryptVerifyTask::fromDecryptResult( const DecryptionResult & dr, const QByteArray & plaintext ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        Decrypt,
        VerificationResult(),
        dr,
        plaintext,
        0,
        QString(),
        inputLabel(),
        outputLabel() ) );
}

shared_ptr<DecryptVerifyResult> AbstractDecryptVerifyTask::fromDecryptResult( const GpgME::Error & err, const QString& what ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        Decrypt,
        VerificationResult(),
        DecryptionResult( err ),
        QByteArray(),
        err.code(),
        what,
        inputLabel(),
        outputLabel() ) );
}

shared_ptr<DecryptVerifyResult> AbstractDecryptVerifyTask::fromDecryptVerifyResult( const DecryptionResult & dr, const VerificationResult & vr, const QByteArray & plaintext ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        DecryptVerify,
        vr,
        dr,
        plaintext,
        0,
        QString(),
        inputLabel(),
        outputLabel() ) );
}

shared_ptr<DecryptVerifyResult> AbstractDecryptVerifyTask::fromDecryptVerifyResult( const GpgME::Error & err, const QString & details ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        DecryptVerify,
        VerificationResult(),
        DecryptionResult( err ),
        QByteArray(),
        err.code(),
        details,
        inputLabel(),
        outputLabel() ) );
}

shared_ptr<DecryptVerifyResult> AbstractDecryptVerifyTask::fromVerifyOpaqueResult( const VerificationResult & vr, const QByteArray & plaintext ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        Verify,
        vr,
        DecryptionResult(),
        plaintext,
        0,
        QString(),
        inputLabel(),
        outputLabel() ) );
}
shared_ptr<DecryptVerifyResult> AbstractDecryptVerifyTask::fromVerifyOpaqueResult( const GpgME::Error & err, const QString & details ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        Verify,
        VerificationResult( err ),
        DecryptionResult(),
        QByteArray(),
        err.code(),
        details,
        inputLabel(),
        outputLabel() ) );
}

shared_ptr<DecryptVerifyResult> AbstractDecryptVerifyTask::fromVerifyDetachedResult( const VerificationResult & vr ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult( 
        Verify,
        vr,
        DecryptionResult(),
        QByteArray(),
        0,
        QString(),
        inputLabel(),
        outputLabel() ) );
}
shared_ptr<DecryptVerifyResult> AbstractDecryptVerifyTask::fromVerifyDetachedResult( const GpgME::Error & err, const QString & details ) {
    return shared_ptr<DecryptVerifyResult>( new DecryptVerifyResult(
        Verify,
        VerificationResult( err ),
        DecryptionResult(),
        QByteArray(),
        err.code(),
        details,
        inputLabel(),
        outputLabel() ) );
}

DecryptVerifyResult::DecryptVerifyResult( DecryptVerifyOperation type,
                    const VerificationResult& vr,
                    const DecryptionResult& dr,
                    const QByteArray& stuff,
                    int errCode,
                    const QString & errString,
                    const QString & inputLabel,
                    const QString & outputLabel )
    : Task::Result(), d( new Private( type, vr, dr, stuff, errCode, errString, inputLabel, outputLabel, this ) )
{
}

QString DecryptVerifyResult::overview() const
{
    QString ov;
    if ( d->isDecryptOnly() )
        ov = formatDecryptionResultOverview( d->m_decryptionResult );
    else if ( d->isVerifyOnly() )
        ov = formatVerificationResultOverview( d->m_verificationResult );
    else 
        ov = formatDecryptVerifyResultOverview( d->m_decryptionResult, d->m_verificationResult );
    return i18nc( "label: result example: foo.sig: Verification failed. ", "%1: %2", d->label(), ov );
}

QString DecryptVerifyResult::details() const
{
    if ( d->isDecryptOnly() )
        return formatDecryptionResultDetails( d->m_decryptionResult, KeyCache::instance()->findRecipients( d->m_decryptionResult ) );
    if ( d->isVerifyOnly() )
        return formatVerificationResultDetails( d->m_verificationResult );
    return formatDecryptVerifyResultDetails( d->m_decryptionResult, d->m_verificationResult, KeyCache::instance()->findRecipients( d->m_decryptionResult ) );
}

bool DecryptVerifyResult::hasError() const
{
    return d->m_error != 0;
}

int DecryptVerifyResult::errorCode() const
{
    return d->m_error;
}

QString DecryptVerifyResult::errorString() const
{
    return d->m_errorString;
}

Task::Result::VisualCode DecryptVerifyResult::code() const {
    if ( d->m_type == DecryptVerify || d->m_type == Verify )
        return codeForVerificationResult( d->m_verificationResult );
    return hasError() ? NeutralError : NeutralSuccess;
}

GpgME::VerificationResult DecryptVerifyResult::verificationResult() const
{
    return d->m_verificationResult;
}

const Key & DecryptVerifyResult::keyForSignature( const Signature & sig, const std::vector<Key> & keys ) {
    if ( const char * const fpr = sig.fingerprint() ) {
        const std::vector<Key>::const_iterator it
            = std::lower_bound( keys.begin(), keys.end(), fpr, _detail::ByFingerprint<std::less>() );
        if ( it != keys.end() && _detail::ByFingerprint<std::equal_to>()( *it, fpr ) )
            return *it;
    }
    static const Key null;
    return null;
}

class AbstractDecryptVerifyTask::Private {
    
};

AbstractDecryptVerifyTask::AbstractDecryptVerifyTask( QObject * parent ) : Task( parent ), d( new Private ) {}

AbstractDecryptVerifyTask::~AbstractDecryptVerifyTask() {}

class DecryptVerifyTask::Private {
    DecryptVerifyTask* const q;
public:
    explicit Private( DecryptVerifyTask* qq ) : q( qq ), m_backend( 0 ), m_protocol( UnknownProtocol )  {}

    void slotResult( const DecryptionResult&, const VerificationResult&, const QByteArray& );

    void registerJob( DecryptVerifyJob * job ) {
        q->connect( job, SIGNAL(result(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)),
                    q, SLOT(slotResult(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)) );
        q->connect( job, SIGNAL(progress(QString,int,int)),
                    q, SLOT(setProgress(QString,int,int)) );
    }

    void emitResult( const shared_ptr<DecryptVerifyResult>& result );
    
    shared_ptr<Input> m_input;
    shared_ptr<Output> m_output;
    const CryptoBackend::Protocol* m_backend;
    Protocol m_protocol;
};


void DecryptVerifyTask::Private::emitResult( const shared_ptr<DecryptVerifyResult>& result )
{
    q->emitResult( result );
    emit q->decryptVerifyResult( result );
}

void DecryptVerifyTask::Private::slotResult( const DecryptionResult& dr, const VerificationResult& vr, const QByteArray& plainText )
{
    if ( dr.error().code() || vr.error().code() ) {
        m_output->cancel();
    } else {
        try {
            kleo_assert( !dr.isNull() || !vr.isNull() );
            m_output->finalize();
        } catch ( const GpgME::Exception & e ) {
            emitResult( q->fromDecryptResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
            return;
        }
    }

    emitResult( q->fromDecryptVerifyResult( dr, vr, plainText ) );
}


DecryptVerifyTask::DecryptVerifyTask( QObject* parent ) : AbstractDecryptVerifyTask( parent ), d( new Private( this ) )
{
}

DecryptVerifyTask::~DecryptVerifyTask()
{
}

void DecryptVerifyTask::setInput( const shared_ptr<Input> & input )
{
    d->m_input = input;
    kleo_assert( d->m_input && d->m_input->ioDevice() );
}

void DecryptVerifyTask::setOutput( const shared_ptr<Output> & output )
{
    d->m_output = output;
    kleo_assert( d->m_output && d->m_output->ioDevice() );
}

void DecryptVerifyTask::setProtocol( Protocol prot )
{
    kleo_assert( prot != UnknownProtocol );
    d->m_protocol = prot;
    d->m_backend = CryptoBackendFactory::instance()->protocol( prot );
    kleo_assert( d->m_backend );
}

void DecryptVerifyTask::autodetectProtocolFromInput() 
{
    if ( d->m_input )
        setProtocol( findProtocol( d->m_input->classification() ) );
}

QString DecryptVerifyTask::label() const
{
    return i18n( "Decrypting: %1...", d->m_input->label() );
}

unsigned long long DecryptVerifyTask::inputSize() const
{
    return d->m_input ? d->m_input->size() : 0;
}

QString DecryptVerifyTask::inputLabel() const
{
    return d->m_input ? d->m_input->label() : QString();
}

QString DecryptVerifyTask::outputLabel() const
{
    return d->m_output ? d->m_output->label() : QString();
}

Protocol DecryptVerifyTask::protocol() const
{
    return d->m_protocol;
}

void DecryptVerifyTask::cancel()
{
    
}

void DecryptVerifyTask::doStart()
{
    kleo_assert( d->m_backend );
    try {
        DecryptVerifyJob * const job = d->m_backend->decryptVerifyJob();
        kleo_assert( job );
        d->registerJob( job );
        job->start( d->m_input->ioDevice(), d->m_output->ioDevice() );
    } catch ( const GpgME::Exception & e ) {
        d->emitResult( fromDecryptVerifyResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
    }
}

class DecryptTask::Private {
    DecryptTask* const q;
public:
    explicit Private( DecryptTask* qq ) : q( qq ), m_backend( 0 ), m_protocol( UnknownProtocol )  {}

    void slotResult( const DecryptionResult&, const QByteArray& );

    void registerJob( DecryptJob * job ) {
        q->connect( job, SIGNAL(result(GpgME::DecryptionResult,QByteArray)),
                    q, SLOT(slotResult(GpgME::DecryptionResult,QByteArray)) );
        q->connect( job, SIGNAL(progress(QString,int,int)),
                    q, SLOT(setProgress(QString,int,int)) );
    }

    void emitResult( const shared_ptr<DecryptVerifyResult>& result );
    
    shared_ptr<Input> m_input;
    shared_ptr<Output> m_output;
    const CryptoBackend::Protocol* m_backend;
    Protocol m_protocol;
};


void DecryptTask::Private::emitResult( const shared_ptr<DecryptVerifyResult>& result )
{
    q->emitResult( result );
    emit q->decryptVerifyResult( result );
}

void DecryptTask::Private::slotResult( const DecryptionResult& result, const QByteArray& plainText )
{
    if ( result.error().code() ) {
        m_output->cancel();
    } else {
        try {
            kleo_assert( !result.isNull() );
            m_output->finalize();
        } catch ( const GpgME::Exception & e ) {
            emitResult( q->fromDecryptResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
            return;
        }
    }

    emitResult( q->fromDecryptResult( result, plainText ) );
}

DecryptTask::DecryptTask( QObject* parent ) : AbstractDecryptVerifyTask( parent ), d( new Private( this ) )
{
}

DecryptTask::~DecryptTask()
{
}

void DecryptTask::setInput( const shared_ptr<Input> & input )
{
    d->m_input = input;
    kleo_assert( d->m_input && d->m_input->ioDevice() );
}

void DecryptTask::setOutput( const shared_ptr<Output> & output )
{
    d->m_output = output;
    kleo_assert( d->m_output && d->m_output->ioDevice() );
}

void DecryptTask::setProtocol( Protocol prot )
{
    kleo_assert( prot != UnknownProtocol );
    d->m_protocol = prot;
    d->m_backend = CryptoBackendFactory::instance()->protocol( prot );
    kleo_assert( d->m_backend );
}

void DecryptTask::autodetectProtocolFromInput() 
{
    if ( d->m_input )
        setProtocol( findProtocol( d->m_input->classification() ) );
}

QString DecryptTask::label() const
{
    return i18n( "Decrypting: %1...", d->m_input->label() );
}

unsigned long long DecryptTask::inputSize() const
{
    return d->m_input ? d->m_input->size() : 0;
}

QString DecryptTask::inputLabel() const
{
    return d->m_input ? d->m_input->label() : QString();
}

QString DecryptTask::outputLabel() const
{
    return d->m_output ? d->m_output->label() : QString();
}

Protocol DecryptTask::protocol() const
{
    kleo_assert( !"not implemented" );
    return UnknownProtocol; // ### TODO
}

void DecryptTask::cancel()
{
    
}

void DecryptTask::doStart()
{
    kleo_assert( d->m_backend );

    try {
        DecryptJob * const job = d->m_backend->decryptJob();
        kleo_assert( job );
        d->registerJob( job );
        job->start( d->m_input->ioDevice(), d->m_output->ioDevice() );
    } catch ( const GpgME::Exception & e ) {
        d->emitResult( fromDecryptResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
    }
}

class VerifyOpaqueTask::Private {
    VerifyOpaqueTask* const q;
public:
    explicit Private( VerifyOpaqueTask* qq ) : q( qq ), m_backend( 0 ), m_protocol( UnknownProtocol )  {}

    void slotResult( const VerificationResult&, const QByteArray& );

    void registerJob( VerifyOpaqueJob* job ) {
        q->connect( job, SIGNAL(result(GpgME::VerificationResult,QByteArray)),
                    q, SLOT(slotResult(GpgME::VerificationResult,QByteArray)) );
        q->connect( job, SIGNAL(progress(QString,int,int)),
                    q, SLOT(setProgress(QString,int,int)) );
    }

    void emitResult( const shared_ptr<DecryptVerifyResult>& result );
    
    shared_ptr<Input> m_input;
    shared_ptr<Output> m_output;
    const CryptoBackend::Protocol* m_backend;
    Protocol m_protocol;
};


void VerifyOpaqueTask::Private::emitResult( const shared_ptr<DecryptVerifyResult>& result )
{
    q->emitResult( result );
    emit q->decryptVerifyResult( result );
}

void VerifyOpaqueTask::Private::slotResult( const VerificationResult& result, const QByteArray& plainText )
{
    if ( result.error().code() ) {
        m_output->cancel();
    } else {
        try {
            kleo_assert( !result.isNull() );
            m_output->finalize();
        } catch ( const GpgME::Exception & e ) {
            emitResult( q->fromDecryptResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
            return;
        }
    }

    emitResult( q->fromVerifyOpaqueResult( result, plainText ) );
}

VerifyOpaqueTask::VerifyOpaqueTask( QObject* parent ) : AbstractDecryptVerifyTask( parent ), d( new Private( this ) )
{
}

VerifyOpaqueTask::~VerifyOpaqueTask()
{
}

void VerifyOpaqueTask::setInput( const shared_ptr<Input> & input )
{
    d->m_input = input;
    kleo_assert( d->m_input && d->m_input->ioDevice() );
}

void VerifyOpaqueTask::setOutput( const shared_ptr<Output> & output )
{
    d->m_output = output;
    kleo_assert( d->m_output && d->m_output->ioDevice() );
}

void VerifyOpaqueTask::setProtocol( Protocol prot )
{
    kleo_assert( prot != UnknownProtocol );
    d->m_protocol = prot;
    d->m_backend = CryptoBackendFactory::instance()->protocol( prot );
    kleo_assert( d->m_backend );
}

void VerifyOpaqueTask::autodetectProtocolFromInput() 
{
    if ( d->m_input )
        setProtocol( findProtocol( d->m_input->classification() ) );
}

QString VerifyOpaqueTask::label() const
{
    return i18n( "Verifying: %1...", d->m_input->label() );
}

unsigned long long VerifyOpaqueTask::inputSize() const
{
    return d->m_input ? d->m_input->size() : 0;
}

QString VerifyOpaqueTask::inputLabel() const
{
    return d->m_input ? d->m_input->label() : QString();
}

QString VerifyOpaqueTask::outputLabel() const
{
    return d->m_output ? d->m_output->label() : QString();
}

Protocol VerifyOpaqueTask::protocol() const
{
    return d->m_protocol;
}

void VerifyOpaqueTask::cancel()
{
    
}

void VerifyOpaqueTask::doStart()
{
    kleo_assert( d->m_backend );

    try {
        VerifyOpaqueJob * const job = d->m_backend->verifyOpaqueJob();
        kleo_assert( job );
        d->registerJob( job );
        job->start( d->m_input->ioDevice(), d->m_output ? d->m_output->ioDevice() : shared_ptr<QIODevice>() );
    } catch ( const GpgME::Exception & e ) {
        d->emitResult( fromVerifyOpaqueResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
    }
}

class VerifyDetachedTask::Private {
    VerifyDetachedTask* const q;
public:
    explicit Private( VerifyDetachedTask* qq ) : q( qq ), m_backend( 0 ), m_protocol( UnknownProtocol ) {}

    void slotResult( const VerificationResult& );

    void registerJob( VerifyDetachedJob* job ) {
        q->connect( job, SIGNAL(result(GpgME::VerificationResult)),
                    q, SLOT(slotResult(GpgME::VerificationResult)) );
        q->connect( job, SIGNAL(progress(QString,int,int)),
                    q, SLOT(setProgress(QString,int,int)) );
    }

    void emitResult( const shared_ptr<DecryptVerifyResult>& result );
    
    shared_ptr<Input> m_input, m_signedData;
    const CryptoBackend::Protocol* m_backend;
    Protocol m_protocol;
};


void VerifyDetachedTask::Private::emitResult( const shared_ptr<DecryptVerifyResult>& result )
{
    q->emitResult( result );
    emit q->decryptVerifyResult( result );
}

void VerifyDetachedTask::Private::slotResult( const VerificationResult& result )
{
    try {
        kleo_assert( !result.isNull() );
        emitResult( q->fromVerifyDetachedResult( result ) );
    } catch ( const GpgME::Exception & e ) {
        emitResult( q->fromVerifyDetachedResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
    }
}

VerifyDetachedTask::VerifyDetachedTask( QObject* parent ) : AbstractDecryptVerifyTask( parent ), d( new Private( this ) )
{
}

VerifyDetachedTask::~VerifyDetachedTask()
{
}

void VerifyDetachedTask::setInput( const shared_ptr<Input> & input )
{
    d->m_input = input;
    kleo_assert( d->m_input && d->m_input->ioDevice() );
}

void VerifyDetachedTask::setSignedData( const shared_ptr<Input> & signedData )
{
    d->m_signedData = signedData;
    kleo_assert( d->m_signedData && d->m_signedData->ioDevice() );
}

void VerifyDetachedTask::setProtocol( Protocol prot )
{
    kleo_assert( prot != UnknownProtocol );
    d->m_protocol = prot;
    d->m_backend = CryptoBackendFactory::instance()->protocol( prot );
    kleo_assert( d->m_backend );
}

void VerifyDetachedTask::autodetectProtocolFromInput() 
{
    if ( d->m_input )
        setProtocol( findProtocol( d->m_input->classification() ) );
}

unsigned long long VerifyDetachedTask::inputSize() const
{
    return d->m_signedData ? d->m_signedData->size() : 0;
}

QString VerifyDetachedTask::label() const
{
    return i18n( "Verifying signature: %1...", d->m_input->label() );
}

QString VerifyDetachedTask::inputLabel() const
{
    return d->m_input ? d->m_input->label() : QString();
}

QString VerifyDetachedTask::outputLabel() const
{
    return QString();
}

Protocol VerifyDetachedTask::protocol() const
{
    kleo_assert( !"not implemented" );
    return UnknownProtocol; // ### TODO
}

void VerifyDetachedTask::cancel()
{
    
}

void VerifyDetachedTask::doStart()
{
    kleo_assert( d->m_backend );
    try {
        VerifyDetachedJob * const job = d->m_backend->verifyDetachedJob();
        kleo_assert( job );
        d->registerJob( job );
        job->start( d->m_input->ioDevice(), d->m_signedData->ioDevice() );
    } catch ( const GpgME::Exception & e ) {
        d->emitResult( fromVerifyDetachedResult( e.error(), QString::fromLocal8Bit( e.what() ) ) );
    }
}

#include "decryptverifytask.moc"
