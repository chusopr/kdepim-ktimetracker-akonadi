/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef KORG_EXCHANGE_H
#define KORG_EXCHANGE_H

#include <qstring.h>

#include <korganizer/part.h>

#include <libkcal/event.h>

#include <exchangeaccount.h>
#include <exchangeclient.h>

// using namespace KOrg;

class Exchange : public KOrg::Part {
    Q_OBJECT
  public:
    Exchange( KOrg::MainWindow *, const char * );
    ~Exchange();

    QString info();

  private slots:
    void download();
    void upload();
    void configure();
    void test();

  private:
    void test2();

    KPIM::ExchangeClient *mClient;
    KPIM::ExchangeAccount* mAccount;
};

#endif

