/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

// $Id$

#include <qfile.h>

#include <kapp.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>

#include "exportwebdialog.h"
#include "calendarview.h"

#include "webexport.h"
#include "webexport.moc"

class WebExportFactory : public KOrg::PartFactory {
  public:
    KOrg::Part *create(KOrg::MainWindow *parent, const char *name)
    {
      return new WebExport(parent,name);
    }
};

extern "C" {
  void *init_libkorg_webexport()
  {
    return (new WebExportFactory);
  }
}


WebExport::WebExport(KOrg::MainWindow *parent, const char *name) :
  KOrg::Part(parent,name)
{
//  KInstance * instance = new KInstance( "korganizer_part" );
//  setInstance( instance );

  setXMLFile("plugins/webexportui.rc");
	   
  new KAction(i18n("Export Web Page.."), 0, this, SLOT(exportWeb()),
              actionCollection(), "export_web");
}

WebExport::~WebExport()
{
}

QString WebExport::info()
{
  return i18n("This plugin provides exporting of calendars as web pages.");
}

void WebExport::exportWeb()
{
  ExportWebDialog *dlg = new ExportWebDialog(mainWindow()->view()->calendar());
  dlg->show();
}
