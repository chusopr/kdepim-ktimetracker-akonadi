/* vcal-conduit.c		VCalendar Conduit 
**
**
** Copyright (C) 1998-2000 by Dan Pilone, Preston Brown, and
**	Herwin Jan Steehouwer
**
** A program to synchronize KOrganizer's date book with the Palm
** Pilot / KPilot. This program is part of KPilot.
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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _KPILOT_VCALCONDUIT_H
#define _KPILOT_VCALCONDUIT_H

#include <time.h>

#ifndef _PILOT_DATEBOOK_H_
#include <pi-datebook.h>
#endif

#include "vcc.h"

#ifndef _KPILOT_VCALBASE_H
#include "vcalBase.h"
#endif

class PilotRecord;
class PilotDateEntry;

	
class VCalConduit : public VCalBaseConduit
{
public:
  VCalConduit(BaseConduit::eConduitMode mode);
  virtual ~VCalConduit();
  
  virtual void doSync();
  virtual void doBackup();
  virtual QWidget* aboutAndSetup();
  virtual void doTest();

  virtual const char* dbInfo() { return "DatebookDB"; }
  


protected:
	void doLocalSync();
	void updateVObject(PilotRecord *rec);

private:

	/**
	* Set the event to repeat forever, with repeat
	* frequency @arg rFreq. This function also
	* warns the user that this is probably not
	* *quite* the behavior intented but there's
	* no fix for that.
	*/
	void repeatForever(PilotDateEntry *p,int rFreq,VObject *v=0L);

	/**
	* The following enums distinguish various repeat-by
	* possiblities. Sometimes the specific value of the
	* enum (like DailyPeriod) encodes something special,
	* so these shouldn't be changed at whim without
	* changing @ref repeatUntil as well.
	*/
	typedef enum { DailyPeriod=60*60*24, 	/* seconds per day */
		WeeklyPeriod=60*60*24*7,	/* seconds per week */
		MonthlyByPosPeriod=1,		/* just a constant */
		MonthlyByDayPeriod=2,
		YearlyByDayPeriod=3
		} PeriodConstants;

	/**
	* Set the date entry to repeat every rFreq periods,
	* rDuration times, starting at start. 
	*
	* This function contains code by Dag Nygren.
	*/
	void repeatUntil(PilotDateEntry *dateEntry,
		struct tm *start,
		int rFreq,
		int rDuration,
		PeriodConstants period);
};

#endif


// $Log$
// Revision 1.13  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.12  2001/02/07 15:46:32  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
