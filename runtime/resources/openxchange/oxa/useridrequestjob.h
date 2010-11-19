/*
    This file is part of oxaccess.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef OXA_USERIDREQUESTJOB_H
#define OXA_USERIDREQUESTJOB_H

#include <kjob.h>

namespace OXA {

class UserIdRequestJob : public KJob
{
  Q_OBJECT

  public:
    UserIdRequestJob( QObject *parent = 0 );

    virtual void start();

    qlonglong userId() const;

  private Q_SLOTS:
    void davJobFinished( KJob* );

  private:
    qlonglong mUserId;
};

}

#endif
