/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "groupwareresourcejob.h"

#include <qtimer.h>

using namespace KPIM;

GroupwareJob::GroupwareJob( GroupwareDataAdaptor *adaptor )
  : mAdaptor( adaptor )
{
  QTimer::singleShot( 0, this, SLOT( run() ) );
}

bool GroupwareJob::error() const
{
  return !mErrorString.isNull();
}

QString GroupwareJob::errorString() const
{
  return mErrorString;
}

void GroupwareJob::success()
{
  mErrorString.clear();
  emit( result( this ) );
}

void GroupwareJob::error( const QString &errorString )
{
  mErrorString = errorString;
  emit( result( this ) );
}


#include "groupwareresourcejob.moc"
