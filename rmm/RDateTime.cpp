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

#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#include <qdatetime.h>

#include <RMM_DateTime.h>
#include <RMM_Enum.h>
#include <RMM_Token.h>

RDateTime::RDateTime()
	:	RHeaderBody(),
		zone_("")
{
	rmmDebug("ctor");
	parsed_ = false;
	assembled_ = false;
}

RDateTime::~RDateTime()
{ 
	rmmDebug("dtor");
}

RDateTime::RDateTime(const QCString & s)
	:	RHeaderBody(s)
{
	parsed_ = false;
	assembled_ = false;
}

RDateTime::RDateTime(const RDateTime & t)
	:	RHeaderBody(t),
		qdate_		(t.qdate_),
		zone_		(t.zone_),
		parsed_		(t.parsed_),
		assembled_	(t.assembled_)
{
	rmmDebug("copy ctor");
}

	RDateTime &
RDateTime::operator = (RDateTime & t)
{
	rmmDebug("operator =");
    if (this == &t) return *this; // Don't do a = a.
	t.parse();
	qdate_	= t.qdate_;
	zone_	= t.zone_;
	
	RHeaderBody::operator = (t);
	
	parsed_ = true;
	assembled_ = false;
	return *this;
}

	QDataStream &
operator >> (QDataStream & s, RDateTime & dt)
{
	s	>> dt.qdate_
		>> dt.zone_;
	dt.parsed_		= true;
	dt.assembled_	= false;
	return s;
}

	QDataStream &
operator << (QDataStream & s, RDateTime & dt)
{
	dt.parse();
	s	<< dt.qdate_
		<< dt.zone_;
	return s;
}

	QCString
RDateTime::timeZone()
{
	parse();
	return zone_;
}

	void
RDateTime::parse()
{
	if (parsed_) return;

	rmmDebug("parse() called");
	
	Q_UINT32	dayOfMonth_;
	Q_UINT32	month_;
	Q_UINT32	year_;
	Q_UINT32	hour_;
	Q_UINT32	min_;
	Q_UINT32	sec_;

	QStrList tokens;
	RTokenise(strRep_, " :", tokens);

	// date-time = [day ","] date time

	if (tokens.count() < 6 || tokens.count() > 9) {
		// Invalid date-time !
		rmmDebug("Invalid date-time");
		parsed_		= true;
		assembled_	= false;
		return;
	}
	
	int i = 0;
	
	// If the first token can be identified as a day name, then ignore it.
	
	bool haveDay = false;
	if (isalpha(tokens.at(i)[0])) { haveDay = true; i++; }

	if (tokens.at(i)[0] == '0')
		dayOfMonth_ = tokens.at(i++)[1] - '0';
	else
		dayOfMonth_ = atoi(tokens.at(i++));

	month_ = RMM::strToMonth(tokens.at(i++));

	if (strlen(tokens.at(i)) == 2)
		year_ = atoi(tokens.at(i++)) + 1900;
	else
		year_ = atoi(tokens.at(i++));
	
	hour_	= atoi(tokens.at(i++));
	min_	= atoi(tokens.at(i++));
	
	// If the earlier token for day of week was there, and the total token
	// count is 8, then we must also have a seconds field next
	// OR if the earlier token for day was NOT there, and the total token count
	// is 7, then again, we must have a seconds field.
	
	if (( haveDay && (tokens.count() == 7)) ||
		(!haveDay && (tokens.count() == 6)))
		sec_ = strtol(tokens.at(i++), NULL, 10);
	else
		sec_ = 0;

	if (tokens.count() - 1 == (unsigned)i)
		zone_ = tokens.at(i);
	
	QDate d;
	d.setYMD(year_, month_, dayOfMonth_);
	qdate_.setDate(d);
	
	QTime t;
	t.setHMS(hour_, min_, sec_);
	qdate_.setTime(t);
	
	parsed_		= true;
	assembled_	= false;
}

	void
RDateTime::assemble()
{
	parse();
	if (assembled_) return;
	
	rmmDebug("assemble() called");
	if (!qdate_.isValid()) {
		rmmDebug("I'm not VALID !");
		return;
	}

	QDate d = qdate_.date();
	QTime t = qdate_.time();
	
	strRep_ = d.dayName(d.dayOfWeek()).ascii();
	strRep_ += ',';
	strRep_ += ' ';
	strRep_ += QCString().setNum(d.day());
	strRep_ += ' ';
	strRep_ += d.monthName(d.month()).ascii();
	strRep_ += ' ';
	strRep_ += QCString().setNum(d.year());
	strRep_ += ' ';
	strRep_ += t.toString().ascii();
	if (!zone_.isEmpty())
		strRep_ += ' ' + zone_;
	
	rmmDebug("assembled to \"" + strRep_ + "\"");
	
	assembled_ = true;
}

	void
RDateTime::createDefault()
{
	qdate_ = QDateTime::currentDateTime();
	zone_ = "";
	parsed_ = true;
	assembled_ = false;
}

	Q_UINT32
RDateTime::asUnixTime()
{
	parse();
	struct tm timeStruct;
	
	rmmDebug(QCString("asUnixTime: date: ") + qdate_.toString().ascii());
	QDate d = qdate_.date();
	QTime t = qdate_.time();
	
	timeStruct.tm_sec	= t.second();
	timeStruct.tm_min	= t.minute();
	timeStruct.tm_hour	= t.hour();
	timeStruct.tm_mday	= d.day();
	timeStruct.tm_mon	= d.month() - 1;
	timeStruct.tm_year	= d.year() - 1900;
	timeStruct.tm_isdst	= -1; // Unknown
	
	time_t timeT = mktime(&timeStruct);	

	return (Q_UINT32)timeT;	
}

