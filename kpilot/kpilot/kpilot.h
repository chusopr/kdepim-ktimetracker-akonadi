#ifndef _KPILOT_KPILOT_H
#define _KPILOT_KPILOT_H
/* kpilot.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the main program in KPilot.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <kmainwindow.h>


class QPopupMenu;
class QComboBox;
class KAction;
class KProgress;
class KJanusWidget;

class PilotDaemonDCOP_stub;
class PilotComponent;
class FileInstallWidget;
class LogWidget;


#include "kpilotDCOP.h"



class KPilotInstaller : public KMainWindow, public KPilotDCOP
{
Q_OBJECT

public:
	KPilotInstaller();
	KPilotInstaller(QStrList& fileList);
	~KPilotInstaller();

	/**
	* Return a string with the version identifier (ie.
	* "KPilot v3.1b11") if kind == 0; otherwise return
	* a "long" string about KPilot -- currently the
	* id of kpilot.o
	*/
	static const char *version(int kind);
	

	// Adds 'name' to the pull down menu of components
	void addComponentPage(PilotComponent *, const QString &name);


	KPilotStatus status() const { return fStatus; } ;


protected:
	void closeEvent(QCloseEvent *e);
	KJanusWidget *getManagingWidget() { return fManagingWidget; }

	/**
	* Provide access to the daemon's DCOP interface
	* through an object of the stub class.
	*/
protected:
	PilotDaemonDCOP_stub &getDaemon() { return *fDaemonStub; } ;
private:
	PilotDaemonDCOP_stub *fDaemonStub;

	/**
	* Handle the functionality of kill-daemon-on-exit and
	* kill-daemon-if-started-by-my by killing it in those
	* cases.
	*/
protected:
	void killDaemonIfNeeded();
public:
	void startDaemonIfNeeded();

public slots:
	void slotRestoreRequested();
	void slotBackupRequested();
	void slotHotSyncRequested();
	void slotListSyncRequested();


	/**
	* These are slots for the standard Configure ...
	* actions and not interesting. The show toolbar
	* functionality is in kdelibs starting with KDE 3.1,
	* but we need to remain backwards compatible.
	*/
	void optionsShowToolbar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();


public:
	/**
	* This is the DCOP interface from the daemon to KPilot.
	*/
	virtual ASYNC daemonStatus(int);
	virtual int kpilotStatus();

	/**
	* This is the DCOP interface from the daemon to KPilot
	* to configure KPilot.
	*/
	virtual ASYNC configure();

protected:
	void readConfig();


	/**
	* Run all the internal conduits' presync functions.
	*/
	bool componentPreSync();
	void setupSync(int kind,const QString& msg);
	void componentPostSync();

	void initIcons();
	void initMenu();
	void setupWidget();
	void initComponents();

	/**
	* This is the private-d-pointer, KPilot style. Not everything
	* has moved there yet.
	*/
	class KPilotPrivate;
	KPilotPrivate *fP;

private:
	bool            fQuitAfterCopyComplete; // Used for GUI-less interface
	KJanusWidget    *fManagingWidget;
	bool            fKillDaemonOnExit;
	bool fDaemonWasRunning;

	KPilotStatus fStatus;

	FileInstallWidget *fFileInstallWidget;
	LogWidget *fLogWidget;

	// Used to track if dialog is visible - needed for new DCOP calls
	bool fConfigureKPilotDialogInUse;


protected slots:
	void quit();
	void slotConfigureKPilot();
	void fileInstalled(int which);
	void slotNewToolbarConfig();

	/**
	 * Indicate that a particular component has been selected (through
	 * whatever mechanism). This will make that component visible and
	 * adjust any other user-visible state to indicate that that component
	 * is now active.
	 *
	 * This should be called (possibly by the component itself!)
	 * or activated through the signal mechanism.
	 * */
	void slotSelectComponent( PilotComponent *c );
	void slotAboutToShowComponent( QWidget *c );

	/**
	* Delayed initialization of the components.
	* This improves perceived startup time.
	*/
	void initializeComponents();

signals:
	void modeSelected(int selected);
};




#endif
