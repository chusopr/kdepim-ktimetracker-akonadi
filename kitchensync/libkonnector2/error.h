/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSYNC_ERROR_H
#define KSYNC_ERROR_H

#include <qstring.h>

#include "notify.h"

namespace KSync {
    /**
     * Errors
     */
    class Error : public Notify {
    public:
        enum ErrorCodes{
            ConnectionLost, WrongPassword,  Authentication, WrongUser,
            WrongIP, CouldNotConnect, DownloadError, UploadError, UserDefined,
            KonnectorNotExist, CouldNotDisconnect, BackupNotSupported, RestoreNotSupported,
            DownloadNotSupported
        };
        Error( int number,  const QString& text );
        Error( const QString& text = QString::null);

        bool operator==( const Error& rhs);

    private:
        class Private;
        Private* d;

    };
}

#endif
