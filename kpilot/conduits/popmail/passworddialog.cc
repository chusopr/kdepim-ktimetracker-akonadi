/* passworddialog.cc			KPilot
**
** Copyright (C) is unclear. Given that the comments are
** in German, I don't think Dan wrote this. The .h file
** is (C) 1997 Micael Roth.
**
** This file is part of the popmail conduit, a conduit for KPilot that
** synchronises the Pilot's email application with the outside world,
** which currently means:
**	-- sendmail or SMTP for outgoing mail
**	-- POP or mbox for incoming mail
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to groot@kde.org
*/

// This is an old trick so you can determine what revisions
// make up a binary distribution.
//
//
static const char *passworddialog_id="$Id$";





#include <string.h>
#include <stdio.h>

#include <qapp.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qaccel.h>
#include <qmsgbox.h>
#include <qchkbox.h>

#include "passworddialog.h"
#include "passworddialog.moc"

#include <klocale.h>
#include <kapplication.h>

PasswordDialog::PasswordDialog(QString head, QWidget* parent, const char* name, bool modal, WFlags wflags)
   : QDialog(parent, name, modal, wflags)
{
    
   _head = head;

   //
   // Bei Bedarf einen kleinen Kommentar als Label einfuegen
   //
   if (!_head.isEmpty())
   {
      QLabel *l;
      
      l = new QLabel(_head, this);
      l->setGeometry( 10, 10, 200, 20 );
   }
   
   //
   // Die eine oder zwei Zeile(n) mit der Passwortabfrage
   //
   QLabel *l_password = new QLabel(i18n("Password"), this);
   l_password->setGeometry( 10, 40, 80, 30 );
   
   _w_password = new QLineEdit( this );
   _w_password->setGeometry( 90, 40, 100, 30 );
   _w_password->setEchoMode( QLineEdit::Password );
   
   //
   // Connect vom LineEdit herstellen und Accelerator
   //
   QAccel *ac = new QAccel(this);
   ac->connectItem( ac->insertItem(Key_Escape), this, SLOT(reject()) );
   
   connect( _w_password, SIGNAL(returnPressed()), SLOT(accept()) );
   
   //
   // Eine vertikale Linie erzeugen
   //
   QFrame *f = new QFrame(this);
   f->setLineWidth(1);
   f->setMidLineWidth(1);
   f->setFrameStyle( QFrame::HLine|QFrame::Raised);
   f->setGeometry( 10, 80, 180, 2 );
   
   //
   // Die Buttons "Ok" & "Cancel" erzeugen
   //
   QPushButton *b1, *b2;
   b1 = new QPushButton(i18n("OK"), this);
   b1->setGeometry( 10, 90, 80, 30 );
   
   b2 = new QPushButton(i18n("Cancel"), this);
   b2->setGeometry( 110, 90, 80, 30 );
   
   // Buttons mit Funktionaliataet belegen
   connect( b1, SIGNAL(clicked()), SLOT(accept()) );
   connect( b2, SIGNAL(clicked()), SLOT(reject()) );
   
   // Fenstertitel
   setCaption(i18n("Password"));
   
   // Focus
   _w_password->setFocus();
   
   setGeometry( x(), y(), 200, 130 );

	(void) passworddialog_id;
}

const char * PasswordDialog::password()
{
   if ( _w_password )
      return _w_password->text().latin1();
   else
      return "";
}





// $Log$
// Revision 1.10  2002/05/15 16:58:02  gioele
// kapp.h -> kapplication.h
//
// Revision 1.9  2001/12/28 13:01:16  adridg
// Add SyncAction
//
// Revision 1.8  2001/02/09 15:59:28  habenich
// replaced "char *id" with "char *<filename>_id", because of --enable-final in configure
//
// Revision 1.7  2001/02/07 15:46:31  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
