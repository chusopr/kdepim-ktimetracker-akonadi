/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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
#ifndef FREEBUSYMANAGER_H
#define FREEBUSYMANAGER_H

#include "akonadi-kcal_next_export.h"

#include <kcalcore/icalformat.h>
#include <kcalcore/freebusycache.h>

#include <KUrl>

#include <QPointer>
#include <QByteArray>
#include <QObject>
#include <QString>

class KJob;
class QTimerEvent;

namespace KIO {
  class Job;
}
namespace KCalCore {
  class FreeBusy;
}
namespace Akonadi {
class FreeBusyManager;
class Calendar;

/**
 * Class for downloading FreeBusy Lists
 */
class AKONADI_KCAL_NEXT_EXPORT FreeBusyDownloadJob : public QObject
{
  Q_OBJECT
  public:
    FreeBusyDownloadJob( const QString &email, const KUrl &url,
                         FreeBusyManager *manager, QWidget *parentWidget = 0 );

    virtual ~FreeBusyDownloadJob();

  protected slots:
    void slotResult( KJob * );
    void slotData( KIO::Job *, const QByteArray &data );

  signals:
    void freeBusyDownloaded( KCalCore::FreeBusy::Ptr , const QString & );

  private:
    FreeBusyManager *mManager;
    QString mEmail;

    QByteArray mFreeBusyData;
};

class AKONADI_KCAL_NEXT_EXPORT FreeBusyManager : public QObject, public KCalCore::FreeBusyCache
{
  Q_OBJECT
  public:
    FreeBusyManager( QObject *parent );

    void setCalendar( Akonadi::Calendar * );

    /// KOrganizer publishes the free/busy list
    void publishFreeBusy( QWidget *parentWidget = 0 );

    /**
      KOrganizer downloads somebody else's free/busy list
      The call is asynchronous, and upon download, the
      receivers slot specified by member will be called.
      The slot should be of type "member(const QString&, KCalCore::FreeBusy::Ptr*)"
        @param email Address of the person for which the F/B list should be
                     retrieved.
        @return true if a download is initiated, and false otherwise
    */
    bool retrieveFreeBusy( const QString &email, bool forceDownload,
                           QWidget *parentWidget = 0 );

    void cancelRetrieval();

    KCalCore::FreeBusy::Ptr iCalToFreeBusy( const QByteArray &data );

    /**
      Load freebusy information belonging to email.
    */
    KCalCore::FreeBusy::Ptr loadFreeBusy( const QString &email );
    /**
      Store freebusy information belonging to email.
    */
    bool saveFreeBusy( const KCalCore::FreeBusy::Ptr &freebusy,
                       const KCalCore::Person::Ptr &person );

    /**
      Return URL of freeBusy information for given email address.
    */
    KUrl freeBusyUrl( const QString &email ) const;

    /**
      Return directory used for stroing free/busy information.
    */
    QString freeBusyDir();

    /**
      Change the broken Url status
      mBrokenUrl is used to show the 'broken url popup' only once
    */
    void setBrokenUrl( bool isBroken );

  public slots:
    // When something changed in the calendar, we get this called
    void slotPerhapsUploadFB();

  signals:
    /**
      This signal is emitted to return results of free/busy requests.
    */
    void freeBusyRetrieved( KCalCore::FreeBusy::Ptr , const QString &email );

  protected:
    void timerEvent( QTimerEvent * );

    /**
      Return free/busy list of calendar owner as iCalendar string.
    */
    QString ownerFreeBusyAsString();

    /**
      Return free/busy list of calendar owner.
    */
    KCalCore::FreeBusy::Ptr ownerFreeBusy();

    /**
      Convert free/busy object to iCalendar string.
    */
    QString freeBusyToIcal( KCalCore::FreeBusy::Ptr  );

  protected slots:
    bool processRetrieveQueue();

  private slots:
    void slotUploadFreeBusyResult( KJob * );

  private:
    Akonadi::Calendar *mCalendar;
    KCalCore::ICalFormat mFormat;

    QStringList mRetrieveQueue;

    // Free/Busy uploading
    QDateTime mNextUploadTime;
    int mTimerID;
    bool mUploadingFreeBusy;
    bool mBrokenUrl;

    // the parentWidget to use while doing our "recursive" retrieval
    QPointer<QWidget>  mParentWidgetForRetrieval;
};

}

#endif
