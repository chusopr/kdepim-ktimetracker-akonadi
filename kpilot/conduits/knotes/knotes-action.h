#ifndef _KPILOT_KNOTES_ACTION_H
#define _KPILOT_KNOTES_ACTION_H
/* knotes-action.h                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the SyncAction that the KNotes conduit performs.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "plugin.h"

class KNotesAction : public ConduitAction
{
Q_OBJECT
public:
	KNotesAction(
		KPilotDeviceLink *o,
		const char *n = 0L,
		const QStringList &a = QStringList() );
	virtual ~KNotesAction();

	enum Status { Init,
		ModifiedNotesToPilot,
		NewNotesToPilot,
		MemosToKNotes,
		Cleanup,
		Done } ;
	virtual QString statusString() const;

protected:
	virtual bool exec();

protected:
	/**
	* For test mode -- just list the notes KNotes has.
	*/
	void listNotes();

	/**
	* For actual processing. These are called by process
	* and it is critical that fP->fIndex is set properly.
	*
	* Each returns true when it is completely finished processing,
	* if it returns a bool. Void functions need only be called once.
	*/
	void getAppInfo();
	void getConfigInfo();
	bool modifyNoteOnPilot();
	bool addNewNoteToPilot();
	bool syncMemoToKNotes();
	void cleanupMemos();

	void resetIndexes();

	static const char * const noteIdsKey;
	static const char * const memoIdsKey;

protected slots:
	void process();

private:
	class KNotesActionPrivate;
	KNotesActionPrivate *fP;
} ;

// $Log$
// Revision 1.5  2002/05/19 15:01:49  adridg
// Patches for the KNotes conduit
//
// Revision 1.4  2002/01/20 22:42:43  adridg
// CVS_SILENT: Administrative
//
// Revision 1.3  2001/12/20 22:55:44  adridg
// Making conduits save their configuration and doing syncs
//
// Revision 1.2  2001/10/31 23:46:51  adridg
// CVS_SILENT: Ongoing conduits ports
//
// Revision 1.1  2001/10/16 21:44:53  adridg
// Split up some files, added behavior
//
//

#endif
