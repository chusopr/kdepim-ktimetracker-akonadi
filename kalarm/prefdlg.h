/*
 *  prefdlg.h  -  program preferences dialog
 *  Program:  kalarm
 *  (C) 2001, 2002 by David Jarvie  software@astrojar.org.uk
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  As a special exception, permission is given to link this program
 *  with any edition of Qt, and distribute the resulting executable,
 *  without including the source code for Qt in the source distribution.
 */

#ifndef PREFDLG_H
#define PREFDLG_H

#include <kdialogbase.h>
class Settings;
class AppearancePrefTab;
class DefaultPrefTab;
class MiscPrefTab;

class KAlarmPrefDlg : public KDialogBase
{
		Q_OBJECT
	public:
		KAlarmPrefDlg(Settings*);
		~KAlarmPrefDlg();

		AppearancePrefTab* m_appearancePage;
		DefaultPrefTab*    m_defaultPage;
		MiscPrefTab*       m_miscPage;

	protected slots:
		virtual void slotOk();
		virtual void slotApply();
		virtual void slotHelp();
		virtual void slotDefault();
		virtual void slotCancel();
};

#endif // PREFDLG_H
