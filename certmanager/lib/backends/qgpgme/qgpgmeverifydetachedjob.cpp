/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmeverifydetachedjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "qgpgmeverifydetachedjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

//#include <gpgmepp/key.h>
#include <gpgmepp/context.h>
#include <gpgmepp/verificationresult.h>
#include <gpgmepp/data.h>

#include <assert.h>

Kleo::QGpgMEVerifyDetachedJob::QGpgMEVerifyDetachedJob( GpgME::Context * context )
  : VerifyDetachedJob( QGpgME::EventLoopInteractor::instance(), "Kleo::QGpgMEVerifyDetachedJob" ),
    GpgME::ProgressProvider(),
    mCtx( context ),
    mSignedDataDataProvider( 0 ),
    mSignedData( 0 ),
    mSignatureDataProvider( 0 ),
    mSignature( 0 )
{
  assert( context );
}

Kleo::QGpgMEVerifyDetachedJob::~QGpgMEVerifyDetachedJob() {
  // YES, WE own it!
  delete mCtx; mCtx = 0;
  delete mSignature; mSignature = 0;
  delete mSignatureDataProvider; mSignatureDataProvider = 0;
  delete mSignedData; mSignedData = 0;
  delete mSignedDataDataProvider; mSignedDataDataProvider = 0;
}

GpgME::Error Kleo::QGpgMEVerifyDetachedJob::start( const QByteArray & signature,
						   const QByteArray & signedData ) {
  assert( !mSignature );
  assert( !mSignedData );

  // set up data object for signature
  mSignatureDataProvider = new QGpgME::QByteArrayDataProvider( signature );
  mSignature = new GpgME::Data( mSignatureDataProvider );
  assert( !mSignature->isNull() );

  // set up data object for signedData
  mSignedDataDataProvider = new QGpgME::QByteArrayDataProvider( signedData );
  mSignedData = new GpgME::Data( mSignedDataDataProvider );
  assert( !mSignedData->isNull() );

  // hook up the context to the eventloopinteractor:
  mCtx->setManagedByEventLoopInteractor( true );
  connect( QGpgME::EventLoopInteractor::instance(),
	   SIGNAL(operationDoneEventSignal(GpgME::Context*,const GpgME::Error&)),
	   SLOT(slotOperationDoneEvent(GpgME::Context*,const GpgME::Error&)) );
  mCtx->setProgressProvider( this );

  const GpgME::Error err = mCtx->startDetachedSignatureVerification( *mSignature, *mSignedData );
						  
  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMEVerifyDetachedJob::slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & ) {
  if ( context == mCtx ) {
    emit done();
    emit result( mCtx->verificationResult() );
    deleteLater();
  }
}

void Kleo::QGpgMEVerifyDetachedJob::slotCancel() {
  mCtx->cancelPendingOperation();
}

void Kleo::QGpgMEVerifyDetachedJob::showProgress( const char * what, int type, int current, int total ) {
  emit progress( what ? QString::fromUtf8( what ) : QString::null, type, current, total );
}

#include "qgpgmeverifydetachedjob.moc"
