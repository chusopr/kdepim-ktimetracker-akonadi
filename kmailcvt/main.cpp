/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Wed Aug  2 11:23:04 CEST 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <dcopclient.h>

#include "kmailcvt.h"

static const KCmdLineOptions options[] =
{
  KCmdLineLastOption
};

int main(int argc, char *argv[])
{
  KLocale::setMainCatalogue("kmailcvt");

  KAboutData aboutData( "kmailcvt", I18N_NOOP("KMailCVT"),
    "3", I18N_NOOP("KMail Import Filters"), KAboutData::License_GPL_V2,
    "(c) 2000-2003, The KMailCVT developers");
  aboutData.addAuthor("Hans Dijkema","Original author", "kmailcvt@hum.org", "http://www.hum.org/kmailcvt.html");
  aboutData.addAuthor("Laurence Anderson","New GUI & cleanups", "l.d.anderson@warwick.ac.uk");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication a;
  KMailCVT *kmailcvt = new KMailCVT();
  a.setMainWidget(kmailcvt);
  kmailcvt->show();  

  DCOPClient *client=a.dcopClient();
  if (!client->attach()) {
    return 1;
  }

  return a.exec();
}
