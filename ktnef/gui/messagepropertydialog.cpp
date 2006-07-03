/*
    messagepropertydialog.cpp

    Copyright (C) 2003 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "messagepropertydialog.h"
#include "attachpropertydialog.h"
#include "ktnef/ktnefmessage.h"

#include <k3listview.h>
#include <klocale.h>
#include <kstdguiitem.h>

MessagePropertyDialog::MessagePropertyDialog( QWidget *parent, KTNEFMessage *msg )
	: KDialog( parent)
{
  setCaption( i18n( "Message Properties" ) );
  setButtons( KDialog::Close|KDialog::User1 );
  setDefaultButton( KDialog::Close );
  setButtonGuiItem( KDialog::User1,  KStdGuiItem::save() );
  setModal( true );
	m_message = msg;

	m_listview = new K3ListView( this );
	m_listview->addColumn( i18n( "Name" ) );
	m_listview->addColumn( i18n( "Value" ) );
	m_listview->setAllColumnsShowFocus( true );
	setMainWidget( m_listview );

	formatPropertySet( m_message, m_listview );
}

void MessagePropertyDialog::slotUser1()
{
	saveProperty( m_listview, m_message, this );
}

#include "messagepropertydialog.moc"
