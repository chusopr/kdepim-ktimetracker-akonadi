/*
    This file is part of KDE.

    Copyright (C) 2007 Trolltech ASA. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef SCALIX_H
#define SCALIX_H

#include <kio/job.h>
#include <kio/slavebase.h>

#include <QtCore/QObject>

class Scalix : public QObject, public KIO::SlaveBase
{
  Q_OBJECT

  public:
    Scalix( const QByteArray &protocol, const QByteArray &pool, const QByteArray &app );

    void get( const KUrl &url );
    void put( const KUrl &url, int permissions, bool overwrite, bool resume );

  Q_SIGNALS:
    void leaveModality();

  private Q_SLOTS:
    void slotRetrieveResult( KIO::Job* );
    void slotPublishResult( KIO::Job* );
    void slotInfoMessage( KIO::Job*, const QString& );

  private:
    void enterLoop();
    void retrieveFreeBusy( const KUrl& );
    void publishFreeBusy( const KUrl& );

    QString mFreeBusyData;
};

#endif
