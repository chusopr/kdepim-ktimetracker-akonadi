/*
    messagepropertydialog.h

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

#ifndef MESSAGEPROPERTYDIALOG_H
#define MESSAGEPROPERTYDIALOG_H

#include <kdialog.h>
#include <ktnef/ktnefmessage.h>
class K3ListView;

class MessagePropertyDialog : public KDialog
{
	Q_OBJECT
public:
	MessagePropertyDialog( QWidget *parent, KTnef::KTNEFMessage *msg );

protected slots:
	void slotUser1();

private:
        KTnef::KTNEFMessage *m_message;
	K3ListView    *m_listview;
};

#endif /* MESSAGEPROPERTYDIALOG_H */
