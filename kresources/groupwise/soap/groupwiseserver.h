/*
    This file is part of KDE.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef GROUPWISESERVER_H
#define GROUPWISESERVER_H

#include <qobject.h>
#include <qstring.h>
#include <qthread.h>

#include <string>

#include <kabc/addressee.h>

#include "gwjobs.h"

namespace KCal {
class Calendar;
class Incidence;
class ResourceGroupwise;
}

struct soap;

class ns1__Folder;
class ns1__Item;
class ns1__Appointment;
class ns1__Mail;
class ns1__Task;

class GroupwiseServer : public QObject
{
  Q_OBJECT

  public:
    GroupwiseServer( const QString &url, const QString &user,
                     const QString &password, QObject *parent );
    ~GroupwiseServer();

    bool login();
    bool logout();

    bool readCalendar( KCal::Calendar*, KCal::ResourceGroupwise* );
    bool addIncidence( KCal::Incidence * );
    bool changeIncidence( KCal::Incidence * );
    bool deleteIncidence( KCal::Incidence * );

    QMap<QString, QString> addressBookList();

    bool readAddressBooks( const QStringList &addrBookIds );

    bool insertAddressee( const QString &addrBookId, KABC::Addressee& );
    bool removeAddressee( const KABC::Addressee& );

    bool dumpData();
    void dumpFolderList();

    void getDelta();

    bool getCategoryList();

  signals:
    void readAddressBooksFinished( const KABC::Addressee::List& );
    void readCalendarFinished();

  protected slots:
    void slotReadAddressBooksFinished();

  protected:
    void dumpCalendarFolder( const std::string &id );

    void dumpFolder( ns1__Folder * );
    void dumpItem( ns1__Item * );
    void dumpAppointment( ns1__Appointment * );
    void dumpTask( ns1__Task * );
    void dumpMail( ns1__Mail * );

  private:
    QString mUrl;
    QString mUser;
    QString mPassword;

    std::string mSession;
    std::string mCalendarFolder;
    
    struct soap *mSoap;
    KPIM::ThreadWeaver::Weaver *mWeaver;
    ReadAddressBooksJob *mReadAddressBooksJob;
};

#endif
