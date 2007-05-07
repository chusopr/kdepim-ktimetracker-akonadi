#ifndef _KPILOT_PROBEDIALOG_H
#define _KPILOT_PROBEDIALOG_H
/* kpilotConfigWizard.h                 KPilot
**
** Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines kpilot's configuration wizard
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "kdialog.h"
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>
#include <Q3ValueList>

class KPilotDeviceLink;
class QLabel;
class Q3GroupBox;
class QTimer;
class Q3GridLayout;
class QProgressBar;
template <class T> class Q3ValueList;
//template<class Qt::Key, class T> class QMap;

typedef Q3ValueList<KPilotDeviceLink*> PilotLinkList;

typedef QMap<QString, KPilotDeviceLink*> PilotLinkMap;

class ProbeDialog : public KDialog
{
Q_OBJECT
public:
	ProbeDialog(QWidget *p=0L,const char *n=0L);
	~ProbeDialog();

	bool detected() const { return mDetected; }
	QString userName() const { return mUserName; }
	QString device() const { return mDevice; }
	QStringList dbs()  const { return mDBs; }


protected slots:
	void startDetection();
	void timeout();
	void connection(KPilotDeviceLink*lnk);
	void retrieveDBList();
	void disconnectDevices();
	void processEvents();
	void progress();
	void detect();
	void detect(int i);
public slots:
	int exec();
	void slotUser1 () { startDetection(); }
protected:
	QLabel* fInfoText;
	Q3GroupBox* fResultsGroup;
	QLabel* fUserLabel;
	QLabel* fDeviceLabel;
	QLabel* fUser;
	QLabel* fDevice;
	Q3GroupBox* fStatusGroup;
	QLabel* fStatus;
	QProgressBar* fProgress;

	QTimer* fProcessEventsTimer;
	QTimer* fTimeoutTimer;
	QTimer* fProgressTimer;
	QTimer* fRotateLinksTimer;
protected:
	Q3GridLayout* fResultsGroupLayout;
	Q3GridLayout* fStatusGroupLayout;

	QStringList mDevicesToProbe[3];
	PilotLinkList mDeviceLinks[3];
	int mProbeDevicesIndex;
	KPilotDeviceLink *mActiveLink;

	bool mDetected;
	QString mUserName;
	QString mDevice;
	QStringList mDBs;
} ;

#endif
