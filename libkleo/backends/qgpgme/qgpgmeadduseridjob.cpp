/*
    qgpgmeadduseridjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "qgpgmeadduseridjob.h"

#include <qgpgme/eventloopinteractor.h>
#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/gpgadduserideditinteractor.h>

#include <cassert>
#include <memory>

Kleo::QGpgMEAddUserIDJob::QGpgMEAddUserIDJob( GpgME::Context * context )
  : AddUserIDJob( QGpgME::EventLoopInteractor::instance() ),
    QGpgMEJob( this, context )
{
  assert( context );
}

Kleo::QGpgMEAddUserIDJob::~QGpgMEAddUserIDJob() {
}

GpgME::Error Kleo::QGpgMEAddUserIDJob::start( const GpgME::Key & key, const QString & name, const QString & email, const QString & comment ) {

  assert( !mOutData );

  createOutData();
  hookupContextToEventLoopInteractor();

  std::auto_ptr<GpgME::GpgAddUserIDEditInteractor> gau( new GpgME::GpgAddUserIDEditInteractor );

  gau->setNameUtf8(       name.toUtf8().constData() );
  gau->setEmailUtf8(     email.toUtf8().constData() );
  gau->setCommentUtf8( comment.toUtf8().constData() );

  std::auto_ptr<GpgME::EditInteractor> ei( gau );

  const GpgME::Error err = mCtx->startEditing( key, ei, *mOutData );

  if ( err )
    deleteLater();
  return err;
}

void Kleo::QGpgMEAddUserIDJob::doOperationDoneEvent( const GpgME::Error & error ) {
  getAuditLog();
  emit result( error );
}

#include "qgpgmeadduseridjob.moc"
