/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/decryptverifywizard.h

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

#ifndef __KLEOPATRA_CRYPTO_GUI_DECRYPTVERIFYWIZARD_H__
#define __KLEOPATRA_CRYPTO_GUI_DECRYPTVERIFYWIZARD_H__

#include <crypto/gui/wizard.h>

#include <utils/pimpl_ptr.h>

#include <boost/shared_ptr.hpp>

namespace Kleo {
namespace Crypto {
    class Task;
    class TaskCollection;
namespace Gui {

    class DecryptVerifyOperationWidget;
    class ResultDisplayWidget;

    class DecryptVerifyWizard : public Wizard {
        Q_OBJECT
    public:
        enum Page {
            OperationsPage=0,
            ResultPage
        };

        explicit DecryptVerifyWizard( QWidget * parent=0, Qt::WindowFlags f=0 );
        ~DecryptVerifyWizard();

        void setOutputDirectory( const QString & dir );
        QString outputDirectory() const;

        void setTaskCollection( const boost::shared_ptr<TaskCollection> & coll );

        DecryptVerifyOperationWidget * operationWidget( unsigned int idx );

    public Q_SLOTS:
        void setOperationCompleted();

    Q_SIGNALS:
        void operationPrepared();

    private:
        /* reimpl */ void onNext( int id );

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

}
}
}

#endif /* __KLEOPATRA_CRYPTO_GUI_DECRYPTVERIFYWIZARD_H__ */
