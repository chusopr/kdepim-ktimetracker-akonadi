/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#ifndef KSYNC_STDERROR_H
#define KSYNC_STDERROR_H

#include "error.h"

namespace KSync {

struct StdError
{
    static Error connectionLost();
    static Error wrongPassword();
    static Error authenticationError();
    static Error wrongUser( const QString &user = QString::null );
    static Error wrongIP();
    static Error couldNotConnect();
    static Error downloadError( const QString & );
    static Error uploadError( const QString & );
    static Error konnectorDoesNotExist();
    static Error backupNotSupported();
    static Error restoreNotSupported();
    static Error downloadNotSupported();
};

}
#endif
