/*
    signencryptjob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004, 2007 Klarälvdalens Datakonsult AB

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

#ifndef __KLEO_SIGNENCRYPTJOB_H__
#define __KLEO_SIGNENCRYPTJOB_H__

#include "job.h"

#include <gpgme++/global.h>

#include <boost/shared_ptr.hpp>

#include <vector>
#include <utility>

class QByteArray;
class QIODevice;

namespace GpgME {
  class Error;
  class Key;
  class SigningResult;
  class EncryptionResult;
}


namespace Kleo {

  /**
     @short An abstract base class for asynchronous combined signing and encrypting

     To use a SignEncryptJob, first obtain an instance from the
     CryptoBackend implementation, connect the progress() and result()
     signals to suitable slots and then start the operation with a
     call to start(). This call might fail, in which case the
     SignEncryptJob instance will have scheduled it's own destruction
     with a call to QObject::deleteLater().

     After result() is emitted, the SignEncryptJob will schedule it's
     own destruction by calling QObject::deleteLater().
  */
  class SignEncryptJob : public Job {
    Q_OBJECT
  protected:
    explicit SignEncryptJob( QObject * parent );
  public:
    ~SignEncryptJob();

    /**
       Starts the combined signing and encrypting operation. \a signers
       is the list of keys to sign \a plainText with. \a recipients is
       a list of keys to encrypt the signed \a plainText to. In both
       lists, empty (null) keys are ignored.

       If \a alwaysTrust is true, validity checking for the
       \em recipient keys will not be performed, but full validity
       assumed for all \em recipient keys without further checks.
    */
    virtual KDE_DEPRECATED GpgME::Error start( const std::vector<GpgME::Key> & signers,
                                               const std::vector<GpgME::Key> & recipients,
                                               const QByteArray & plainText,
                                               bool alwaysTrust=false ) = 0;

    /*!
      \overload

      If \a cipherText is non-null, the ciphertext is written
      there. Otherwise, it will be delivered in the third argument of
      result().

      \throws GpgME::Exception if starting fails
    */
    virtual void start( const std::vector<GpgME::Key> & signers,
                        const std::vector<GpgME::Key> & recipients,
                        const boost::shared_ptr<QIODevice> & plainText,
                        const boost::shared_ptr<QIODevice> & cipherText=boost::shared_ptr<QIODevice>(),
                        bool alwaysTrust=false ) = 0;

    virtual KDE_DEPRECATED std::pair<GpgME::SigningResult,GpgME::EncryptionResult>
      exec( const std::vector<GpgME::Key> & signers,
	    const std::vector<GpgME::Key> & recipients,
	    const QByteArray & plainText,
	    bool alwaysTrust, QByteArray & cipherText ) = 0;

    /*!
      This is a hack to request BASE64 output (instead of whatever
      comes out normally).
    */
    virtual void setOutputIsBase64Encoded( bool ) = 0;

  Q_SIGNALS:
    void result( const GpgME::SigningResult & signingresult,
		 const GpgME::EncryptionResult & encryptionresult,
		 const QByteArray & cipherText, const QString & auditLogAsHtml=QString() );
  };

}

#endif // __KLEO_SIGNENCRYPTJOB_H__
