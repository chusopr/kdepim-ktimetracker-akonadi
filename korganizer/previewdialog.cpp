/*
    This file is part of KOrganizer.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    Author: Sergio Martins, <sergio.martins@kdab.com>    

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "previewdialog.h"

#include "kolistview.h"
#include "koprefs.h"
#include "stdcalendar.h"

#include <klocale.h>

#include <libkcal/calendarlocal.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>

using namespace KCal;

PreviewDialog::PreviewDialog( const KURL &url, QWidget *parent )
  : KDialogBase( Plain, i18n("Import Calendar/Event"), User1 | User2 | Cancel, User1, parent,
                 0, true, true, KGuiItem( i18n("&Merge into to existing calendar"), "merge" ), KGuiItem( i18n("&Add as new calendar"), "add" ) ),
    mUrl( url )
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame, 0, spacingHint() );

  mCalendar = new CalendarLocal( KOPrefs::instance()->mTimeZoneId );
  mCalendar->load( url.path() );
  mListView = new KOListView( mCalendar, topFrame, "PreviewDialog::ListView", true );
  topLayout->addWidget( mListView );

  mListView->showAll();

  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( marginHint() );

  connect( this, SIGNAL(user1Clicked()), SLOT(slotMerge()) );
  connect( this, SIGNAL(user2Clicked()), SLOT(slotAdd()) );  
}

PreviewDialog::~PreviewDialog()
{
  delete mCalendar;
}

void PreviewDialog::slotMerge()
{
  emit openURL( mUrl, true );
  emit dialogFinished( this );
  accept();
}

void PreviewDialog::slotAdd()
{
  emit addResource( mUrl );
  emit dialogFinished( this );
  accept();
}

#include "previewdialog.moc"
