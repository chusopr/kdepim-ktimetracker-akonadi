/***************************************************************************
                          kselfilterpage.cpp  -  description
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

#include <kdebug.h>
#include <kstandarddirs.h>

#include "kselfilterpage.h"

KSelFilterPage::KSelFilterPage(QWidget *parent, const char *name ) : KSelFilterPageDlg(parent,name) {

	px_introSidebar->setPixmap(locate("data", "kmailcvt/pics/step1.png"));
	kdDebug() << "KSelFilterPage::KSelFilterPage" << endl;
}

KSelFilterPage::~KSelFilterPage() {
	int i;
	for(i=0;i<F.len();i++) {
		delete F[i];
	}
}

void KSelFilterPage::addFilter(filter *f)
{
	F[F.len()]=f;
	_filters->insertItem(f->name());
}

filter * KSelFilterPage::getSelectedFilter(void)
{
	return F[_filters->currentItem()];
}

void KSelFilterPage::setAuthors(KAboutData& data)
{
	int i;
	for(i=0;i<F.len();i++) {
		data.addAuthor(F[i]->author().latin1(), F[i]->name().latin1());
	}
}

#include "kselfilterpage.moc"

