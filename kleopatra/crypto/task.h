/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/task.h

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

#ifndef __KLEOPATRA_CRYPTO_TASK_H__
#define __KLEOPATRA_CRYPTO_TASK_H__

#include <QObject>
#include <QString>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>

#include <boost/shared_ptr.hpp>

namespace Kleo {
namespace Crypto {

    class Task : public QObject {
        Q_OBJECT
    public:
        explicit Task( QObject * parent=0 );
        ~Task();

        class Result;

        virtual GpgME::Protocol protocol() const = 0;

        void start();

        virtual QString label() const = 0;

    public Q_SLOTS:
        virtual void cancel() = 0;

    Q_SIGNALS:
        void progress( const QString & what, int current, int total );
        void result( const boost::shared_ptr<const Kleo::Crypto::Task::Result> & );
        void started();

    protected:
        static boost::shared_ptr<Result> makeErrorResult( int errCode, const QString& details );

    private Q_SLOTS:
        void emitError( int errCode, const QString& details );
            
    private:
        virtual void doStart() = 0;
        
    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

    class Task::Result {
        QString m_nonce;
    public:
        Result();
        virtual ~Result();

        const QString & nonce() const { return m_nonce; }

        bool hasError() const;
        
        virtual QString overview() const = 0;
        virtual QString details() const = 0;
        virtual int errorCode() const = 0;
        virtual QString errorString() const = 0;
        
    protected:
        enum ErrorLevel {
            NoError,
            Warning,
            Error
        };
        static QString makeSimpleOverview( const QString& summary, ErrorLevel level );
        QString formatKeyLink( const char * fingerprint, const QString & content ) const;
    };
        
}
}

#endif /* __KLEOPATRA_CRYPTO_TASK_H__ */

