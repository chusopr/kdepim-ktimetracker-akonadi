/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

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

#ifndef AKONADI_NEPOMUK_RSS_FEEDER_H
#define AKONADI_NEPOMUK_RSS_FEEDER_H

#include "nepomukfeederagent.h"
#include "rsschannel.h"

#include <QtCore/QList>

class NepomukRssFeeder : public NepomukFeederAgent<NepomukFast::RssChannel>
{
     Q_OBJECT
public:
    explicit NepomukRssFeeder( const QString &id );
    void updateItem( const Akonadi::Item &item, const QUrl &graphUri );
};

#endif //AKONADI_NEPOMUK_RSS_FEEDER_H
