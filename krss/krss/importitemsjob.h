/*
 * This file is part of the krss library
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef KRSS_LIB_IMPORTITEMSJOB_H
#define KRSS_LIB_IMPORTITEMSJOB_H

#include "krss_export.h"

#include <KJob>

#include <QVariant>

class KUrl;

namespace KIO {
    class Job;
}

namespace KRss {

class NetResource;

class KRSS_EXPORT ImportItemsJob : public KJob {
    Q_OBJECT

    friend class ::KRss::NetResource;

public:
    ~ImportItemsJob();


    enum Error {
        FileAccessError = KJob::UserDefinedError,
        ImportFailedError
    };

    void setFeedXmlUrl( const QString& url );
    void setSourceFile( const QString& path );

    /* reimp */ void start();

private:
    explicit ImportItemsJob( const QString& dbusService=QString(), QObject* parent=0 );

private:
    class Private;
    Private* const d;
    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void importFinished(QVariantMap) )
    Q_PRIVATE_SLOT( d, void importError(QDBusError) )
};

}

#endif // KRSS_LIB_IMPORTITEMSJOB_H
