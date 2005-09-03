/*
 *  clientinfo.h  -  client application information
 *  Program:  KAlarm's alarm daemon (kalarmd)
 *  Copyright (C) 2001, 2004 by David Jarvie <software@astrojar.org.uk>
 *  Based on the original, (c) 1998, 1999 Preston Brown
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _CALCLIENT_H
#define _CALCLIENT_H

#include <q3cstring.h>
#include <qstring.h>
#include <qmap.h>

class ADCalendar;


/*=============================================================================
=  Class: ClientInfo
=  Details of a KAlarm client application.
=============================================================================*/
class ClientInfo
{
	public:
		typedef QMap<Q3CString, ClientInfo*>::ConstIterator ConstIterator;

		ClientInfo(const Q3CString &appName, const QString &title, const Q3CString &dcopObj,
		           const QString& calendar, bool startClient);
		ClientInfo(const Q3CString &appName, const QString &title, const Q3CString &dcopObj,
			   ADCalendar* calendar, bool startClient);
		~ClientInfo();
		ADCalendar*          setCalendar(const QString& url);
		void                 detachCalendar()            { mCalendar = 0; }
		void                 setStartClient(bool start)  { mStartClient = start; }

		Q3CString             appName() const             { return mAppName; }
		QString              title() const               { return mTitle; }
		Q3CString             dcopObject() const          { return mDcopObject; }
		ADCalendar*          calendar() const            { return mCalendar; }
		bool                 startClient() const         { return mStartClient; }

		static ConstIterator begin()                     { return mClients.begin(); }
		static ConstIterator end()                       { return mClients.end(); }
		static ClientInfo*   get(const Q3CString& appName);
		static ClientInfo*   get(const ADCalendar*);
		static void          remove(const Q3CString& appName);
		static void          clear();

	private:
		static QMap<Q3CString, ClientInfo*> mClients;  // list of all constructed clients
		Q3CString             mAppName;      // client's executable and DCOP name
		QString              mTitle;        // application title for display purposes
		Q3CString             mDcopObject;   // object to receive DCOP messages
		ADCalendar*          mCalendar;     // this client's event calendar
		bool                 mStartClient;  // whether to notify events via command line if client app isn't running
};

#endif
