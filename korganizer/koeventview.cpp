/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koeventview.h"
#include "kocore.h"
#include "koprefs.h"
#include "koeventpopupmenu.h"
#include "komessagebox.h"

#include <KCal/Incidence>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include <QMenu>

//---------------------------------------------------------------------------

KOEventView::KOEventView( Calendar *cal, QWidget *parent )
  : KOrg::BaseView( cal, parent )
{
}

//---------------------------------------------------------------------------

KOEventView::~KOEventView()
{
}

//---------------------------------------------------------------------------

KOEventPopupMenu *KOEventView::eventPopup()
{
  KOEventPopupMenu *eventPopup = new KOEventPopupMenu;

  connect( eventPopup, SIGNAL(editIncidenceSignal(Incidence *)),
           SIGNAL(editIncidenceSignal(Incidence *)));
  connect( eventPopup, SIGNAL(showIncidenceSignal(Incidence *)),
           SIGNAL(showIncidenceSignal(Incidence *)));
  connect( eventPopup, SIGNAL(deleteIncidenceSignal(Incidence *)),
           SIGNAL(deleteIncidenceSignal(Incidence *)));
  connect( eventPopup, SIGNAL(cutIncidenceSignal(Incidence *)),
           SIGNAL(cutIncidenceSignal(Incidence *)));
  connect( eventPopup, SIGNAL(copyIncidenceSignal(Incidence *)),
           SIGNAL(copyIncidenceSignal(Incidence *)));
  connect( eventPopup, SIGNAL(pasteIncidenceSignal()),
           SIGNAL(pasteIncidenceSignal()));
  connect( eventPopup, SIGNAL(toggleAlarmSignal(Incidence *)),
           SIGNAL(toggleAlarmSignal(Incidence*)));
  connect( eventPopup, SIGNAL(toggleTodoCompletedSignal(Incidence *)),
           SIGNAL(toggleTodoCompletedSignal(Incidence *)));
  connect( eventPopup, SIGNAL(dissociateOccurrencesSignal(Incidence *,const QDate &)),
           SIGNAL(dissociateOccurrencesSignal(Incidence *,const QDate &)) );

  return eventPopup;
}

QMenu *KOEventView::newEventPopup()
{
  KXMLGUIClient *client = KOCore::self()->xmlguiClient( this );
  if ( !client ) {
    kError() << "no xmlGuiClient.";
    return 0;
  }
  if ( !client->factory() ) {
    kError() << "no factory";
    return 0; // can happen if called too early
  }

  return static_cast<QMenu*>
      ( client->factory()->container( "rmb_selection_popup", client ) );
}
//---------------------------------------------------------------------------

void KOEventView::popupShow()
{
  emit showIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupEdit()
{
  emit editIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupDelete()
{
  emit deleteIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupCut()
{
  emit cutIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::popupCopy()
{
  emit copyIncidenceSignal(mCurrentIncidence);
}

//---------------------------------------------------------------------------

void KOEventView::showNewEventPopup()
{
  QMenu *popup = newEventPopup();
  if ( !popup ) {
    kError() << "popup creation failed";
    return;
  }

  popup->popup( QCursor::pos() );
}

//---------------------------------------------------------------------------

void KOEventView::defaultAction( Incidence *incidence )
{
  kDebug();

  if ( !incidence ) {
    return;
  }

  kDebug() << "  type:" << incidence->type();

  if ( incidence->isReadOnly() ) {
    emit showIncidenceSignal(incidence);
  } else {
    emit editIncidenceSignal(incidence);
  }
}

//---------------------------------------------------------------------------
int KOEventView::showMoveRecurDialog( Incidence *inc, const QDate &date ) {

  int answer = KMessageBox::Ok;
  KGuiItem itemFuture( i18n( "Also &Future Items" ) );

  KDateTime dateTime( date, KOPrefs::instance()->timeSpec() );
  bool isFirst = !inc->recurrence()->getPreviousDateTime( dateTime ).isValid();
  bool isLast  = !inc->recurrence()->getNextDateTime( dateTime ).isValid();

  QString message;

  if ( !isFirst && !isLast ) {
    itemFuture.setEnabled( true );
    message = i18n( "The item you try to change is a recurring item. "
                    "Shall the changes be applied only to this single occurrence, "
                    "also to future items, or to all items in the recurrence?" );
  } else {
    itemFuture.setEnabled( false );
    message = i18n( "The item you try to change is a recurring item. "
                    "Shall the changes be applied only to this single occurrence "
                    "or to all items in the recurrence?" );
  }

  if ( !(isFirst && isLast) ) {
    answer = KOMessageBox::fourBtnMsgBox( this,
                                          QMessageBox::Question,
                                          message,
                                          i18n( "Changing Recurring Item" ),
                                          KGuiItem( i18n( "Only &This Item" ) ),
                                          itemFuture,
                                          KGuiItem( i18n( "&All Occurrences" ) ) );

  }

  return answer;
}

#include "koeventview.moc"

