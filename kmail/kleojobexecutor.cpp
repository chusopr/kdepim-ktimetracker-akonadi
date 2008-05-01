/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kleojobexecutor.h"

#include <kleo/decryptverifyjob.h>
#include <kleo/importjob.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/verifyopaquejob.h>

#include <kdebug.h>

#include <QBuffer>
#include <QEventLoop>

using namespace KMail;
using namespace Kleo;
using namespace GpgME;
using boost::shared_ptr;

KleoJobExecutor::KleoJobExecutor( QObject* parent ) : QObject( parent )
{
  setObjectName( "KleoJobExecutor" );
  mEventLoop = new QEventLoop( this );
}


GpgME::VerificationResult KleoJobExecutor::exec(
    Kleo::VerifyDetachedJob* job,
    const QByteArray & signature,
    const QByteArray & signedData )
{
  kDebug() << "Starting detached verification job";
  connect( job, SIGNAL(result(GpgME::VerificationResult)),
           SLOT(verificationResult(GpgME::VerificationResult)) );
  job->start( signature, signedData );
  mEventLoop->exec();
  return mVerificationResult;
}

GpgME::VerificationResult KleoJobExecutor::exec(
    Kleo::VerifyOpaqueJob * job,
    const QByteArray & signedData,
    QByteArray & plainText )
{
  kDebug() << "Starting opaque verfication job";
  connect( job, SIGNAL(result(GpgME::VerificationResult,QByteArray)),
           SLOT(verificationResult(GpgME::VerificationResult,QByteArray)) );
  job->start( signedData );
  mEventLoop->exec();
  plainText = mData;
  return mVerificationResult;
}

std::pair< GpgME::DecryptionResult, GpgME::VerificationResult > KleoJobExecutor::exec(
    Kleo::DecryptVerifyJob * job,
    const QByteArray & cipherText,
    QByteArray & plainText )
{
  kDebug() << "Starting decryption job";
  connect( job, SIGNAL(result(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)),
           SLOT(decryptResult(GpgME::DecryptionResult,GpgME::VerificationResult,QByteArray)) );
  job->start( cipherText );
  mEventLoop->exec();
  plainText = mData;
  return std::make_pair( mDecryptResult, mVerificationResult );
}

GpgME::ImportResult KleoJobExecutor::exec(Kleo::ImportJob* job, const QByteArray & certData)
{
  connect( job, SIGNAL(result(GpgME::ImportResult)), SLOT(importResult(GpgME::ImportResult)) );
  job->start( certData );
  mEventLoop->exec();
  return mImportResult;
}

void KleoJobExecutor::verificationResult(const GpgME::VerificationResult & result)
{
  kDebug() << "Detached verification job finished";
  mVerificationResult = result;
  mEventLoop->quit();
}

void KleoJobExecutor::verificationResult(const GpgME::VerificationResult & result, const QByteArray & plainText)
{
  kDebug() << "Opaque verification job finished";
  mVerificationResult = result;
  mData = plainText;
  mEventLoop->quit();
}

void KleoJobExecutor::decryptResult(
    const GpgME::DecryptionResult & decryptionresult,
    const GpgME::VerificationResult & verificationresult,
    const QByteArray & plainText )
{
  kDebug() << "Decryption job finished";
  mVerificationResult = verificationresult;
  mDecryptResult = decryptionresult;
  mData = plainText;
  mEventLoop->quit();
}

void KleoJobExecutor::importResult(const GpgME::ImportResult & result)
{
  mImportResult = result;
  mEventLoop->quit();
}

#include "kleojobexecutor.moc"
