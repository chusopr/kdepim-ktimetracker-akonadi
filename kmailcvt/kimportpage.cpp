/***************************************************************************
                          kimportpage.cpp  -  description
                             -------------------
    begin                : Fri Jan 17 2003
    copyright            : (C) 2003 by Laurence Anderso
    email                : l.d.anderson@warwick.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "kimportpage.h"

KImportPage::KImportPage(QWidget *parent, const char *name ) : KImportPageDlg(parent,name) {

	px_introSidebar->setPixmap(locate("data", "kmailcvt/pics/step2.png"));
	kdDebug() << "KImportPage::KImportPage" << endl;
}

KImportPage::~KImportPage() {
}

#include "kimportpage.moc"

