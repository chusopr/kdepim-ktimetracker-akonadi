/* vcal-factory.cc                      KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the vcal-conduit plugin.
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
#include "vcal-factorybase.moc"


// Configuration keys
//
//
const char * const VCalConduitFactoryBase::calendarFile = "CalFile" ;
const char * const VCalConduitFactoryBase::firstTime = "FirstTime" ;
const char * const VCalConduitFactoryBase::deleteOnPilot = "DeleteOnPilot" ;
const char * const VCalConduitFactoryBase::fullSyncOnPCChange = "FullSyncOnPCChange";
const char * const VCalConduitFactoryBase::alwaysFullSync = "AlwaysFullSync";


// $Log$
// Revision 1.1.2.1  2002/04/28 12:58:54  kainhofe
// Calendar conduit now works, no memory leaks, timezone still shifted. Todo conduit mostly works, for my large list it crashes when saving the calendar file.
//
// Revision 1.6  2002/04/20 14:21:26  kainhofe
// Alarms are now written to the palm. Some bug fixes, extensive testing. Exceptions still crash the palm ;-(((
//
// Revision 1.5  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//
// Revision 1.4  2001/12/31 09:25:05  adridg
// Cleanup, various fixes for runtime loading
//
// Revision 1.3  2001/12/28 12:56:46  adridg
// Added SyncAction, it may actually do something now.
//
// Revision 1.2  2001/12/27 16:43:36  adridg
// Fixup configuration
//
// Revision 1.1  2001/12/13 21:40:40  adridg
// New files for move to .so
//

