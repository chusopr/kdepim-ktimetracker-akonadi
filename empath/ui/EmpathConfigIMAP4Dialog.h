/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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

#ifdef __GNUG__
# pragma interface "EmpathConfigIMAP4Dialog.h"
#endif

#ifndef EMPATHCONFIGIMAP4DIALOG_H
#define EMPATHCONFIGIMAP4DIALOG_H

// Qt includes
#include <qdialog.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qbuttongroup.h>

// KDE includes
#include <kbuttonbox.h>

// Local includes
#include "EmpathDefines.h"
#include "RikGroupBox.h"

class EmpathMailboxIMAP4;

/**
 * Configure an imap mailbox
 */
class EmpathConfigIMAP4Dialog : public QDialog
{
	Q_OBJECT

	public:
		
		EmpathConfigIMAP4Dialog(
				EmpathMailboxIMAP4 * mailbox,
				QWidget * parent = 0,
				const char * name = 0);

		~EmpathConfigIMAP4Dialog() { empathDebug("dtor"); }

		void fillInSavedData();
		
	protected slots:

		void	s_OK();
		void	s_Cancel();
		void	s_Help();

	protected:

	private:

		EmpathMailboxIMAP4	* mailbox_;

		RikGroupBox		* rgb_server_;
		
		KButtonBox		* buttonBox_;

		QPushButton		* pb_OK_;
		QPushButton		* pb_Cancel_;
		QPushButton		* pb_Help_;
	
		QWidget			* w_server_;
	
		QLabel			* l_notImp_;
		
		QGridLayout		* topLevelLayout_;
		QGridLayout		* serverGroupLayout_;
};

#endif
