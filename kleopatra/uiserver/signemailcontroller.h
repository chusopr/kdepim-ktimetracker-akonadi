/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signemailcontroller.h

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

#ifndef __KLEOPATRA_UISERVER_SIGNEMAILCONTROLLER_H__
#define __KLEOPATRA_UISERVER_SIGNEMAILCONTROLLER_H__

#include <QObject>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>

#include <boost/shared_ptr.hpp>

#include <vector>

namespace KMime {
namespace Types {
    class Mailbox;
}
}

namespace Kleo {

    class AssuanCommand;

    class SignEMailController : public QObject {
        Q_OBJECT
    public:
        explicit SignEMailController( QObject * parent=0 );
        ~SignEMailController();

        void setProtocol( GpgME::Protocol proto );
        GpgME::Protocol protocol() const;
        //const char * protocolAsString() const;

        void setCommand( const boost::shared_ptr<AssuanCommand> & cmd );

        void startResolveSigners( const std::vector<KMime::Types::Mailbox> & signers );

        void setDetachedSignature( bool detached );

        void importIO();

        void start();

    public Q_SLOTS:
        void cancel();

    Q_SIGNALS:
        void signersResolved();
        void reportMicAlg( const QString & micalg );
        void error( int err, const QString & details );
        void done();

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
        Q_PRIVATE_SLOT( d, void slotWizardSignersResolved() )
        Q_PRIVATE_SLOT( d, void slotWizardCanceled() )
        Q_PRIVATE_SLOT( d, void slotTaskDone() )
        Q_PRIVATE_SLOT( d, void schedule() )
    };
}

#endif /* __KLEOPATRA_UISERVER_SIGNEMAILCONTROLLER_H__ */

