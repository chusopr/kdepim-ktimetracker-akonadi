/* pilotTodoEntry.h	-*- C++ -*-		KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
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
#ifndef _KPILOT_PILOTTODOENTRY_H
#define _KPILOT_PILOTTODOENTRY_H

#include <time.h>
#include <string.h>

#ifndef QBITARRAY_H
#include <qbitarray.h>
#endif

#ifndef _PILOT_MACROS_H_
#include <pi-macros.h>
#endif

#ifndef _PILOT_TODO_H_
#include <pi-todo.h>
#endif

#ifndef _KPILOT_PILOTAPPCATEGORY_H
#include "pilotAppCategory.h"
#endif

#ifndef _KPILOT_PILOTRECORD_H
#include "pilotRecord.h"
#endif



class PilotTodoEntry : public PilotAppCategory
{
public:
  PilotTodoEntry(void);
  PilotTodoEntry(const PilotTodoEntry &e);
  PilotTodoEntry(PilotRecord* rec);
  ~PilotTodoEntry() { free_ToDo(&fTodoInfo); }

  PilotTodoEntry& operator=(const PilotTodoEntry &e);
  
  PilotRecord* pack() { return PilotAppCategory::pack(); }
  
  struct tm getDueDate() const { return fTodoInfo.due; }
  void setDueDate(struct tm& d) { fTodoInfo.due = d; }
  const struct tm *getDueDate_p() const { return &fTodoInfo.due; } 
  
  int getIndefinite() const { return fTodoInfo.indefinite; }
  void setIndefinite(int i) { fTodoInfo.indefinite = i; }

  int getPriority() const { return fTodoInfo.priority; }
  void setPriority(int p) { fTodoInfo.priority = p; }

  int getComplete() const { return fTodoInfo.complete; }
  void setComplete(int c) { fTodoInfo.complete = c; }

  void  setDescription(const char* desc);
  const char* getDescription() const { return fTodoInfo.description; }

  void  setNote(const char* note);
  const char* getNote() const { return fTodoInfo.note; }
  
protected:
  void *pack(void *, int *);
  void unpack(const void *, int = 0) { }
  
private:
  struct ToDo fTodoInfo;
};



#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif


// $Log$
// Revision 1.1  2001/12/27 23:08:30  adridg
// Restored some deleted wrapper files
//
// Revision 1.9  2001/05/24 10:31:38  adridg
// Philipp Hullmann's extensive memory-leak hunting patches
//
// Revision 1.8  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.7  2001/04/01 17:32:06  adridg
// Fiddling around with date properties
//
// Revision 1.6  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.5  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
