/*
    This file is part of libkpimexchange
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
#ifndef KDEPIM_EXCHANGE_DELETE_H
#define KDEPIM_EXCHANGE_DELETE_H

#include <qstring.h>
#include <qwidget.h>

#include <kio/job.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>

namespace KPIM {

class ExchangeAccount;

class ExchangeDelete : public QObject {
    Q_OBJECT
  public:
    ExchangeDelete( KCal::Event* event, ExchangeAccount* account, QWidget* window=0 );
    ~ExchangeDelete();

  private slots:
    void slotDeleteResult( KIO::Job * );
    void slotFindUidResult( KIO::Job * );

  signals:
    void finished( ExchangeDelete* worker );

  private:
    void findUidSingleMaster( QString const& uid );
    void startDelete( KURL& url );
    
    ExchangeAccount* mAccount;
    QWidget* mWindow;
};

}

#endif
