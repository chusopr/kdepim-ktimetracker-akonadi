/***************************************************************************
                          knarticle.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "knarticle.h"
#include "mimelib/datetime.h"
#include "knhdrviewitem.h"
#include <stdio.h>
#include <qdatetime.h>

KNArticle::KNArticle()
{
	i_d=-1;
	t_imeT=0;
	i_tem=0;
	t_imeString=0;
}



KNArticle::~KNArticle()
{
	delete i_tem;
	delete[] t_imeString;
}



void KNArticle::clear()
{
	KNMimeContent::clear();
	s_ubject.resize(0);	
}



void KNArticle::parse()
{
	QCString tmp;
	if(s_ubject.isEmpty()) s_ubject=decodeRFC1522String(headerLine("Subject"));
	
	if(t_imeT==0) {
		tmp=headerLine("Date");
		if(!tmp.isEmpty()) parseDate(tmp);
	}	
	
	if(r_eferences.isEmpty()) {
	  tmp=headerLine("References");
	  if(!tmp.isEmpty())
	    r_eferences.setLine(tmp);
	}
	
	KNMimeContent::parse();
}



void KNArticle::parseDate(const QCString &s)
{
  DwDateTime dt;
	dt.FromString(s.data());
	dt.Parse();
	t_imeT=dt.AsUnixTime();
}



void KNArticle::assemble()
{
  DwDateTime dt;
  QCString tmp;

  dt.FromUnixTime(t_imeT);
  dt.Assemble();

  KNMimeContent::assemble();

  if(!r_eferences.isEmpty())
    setHeader(HTreferences, r_eferences.line(), false);

  tmp=dt.AsString().c_str();
  setHeader(HTdate, tmp, false);

  setHeader(HTsubject, s_ubject, !allow8bit);
}



const QCString& KNArticle::fromName()
{
	static QCString ret;
	ret="";
		
	FromLineParser flp(headerLine("From"));
	flp.parse();
	
	if(flp.hasValidFrom()) ret=flp.from();	
	else if(flp.hasValidEmail()) ret=flp.email();
	else ret="nobody";
	
	return ret;	
	
}



const QCString& KNArticle::fromEmail()
{
	static QCString ret;
	ret="";
	
	FromLineParser flp(headerLine("From"));
	flp.parse();
	if(flp.hasValidEmail()) ret=flp.email();
	else ret="no email";	
	
	return ret;	
}



const QCString& KNArticle::replyToEmail()
{
	static QCString str;
	
	str=headerLine("Reply-To");
	if(!str.isEmpty()) {
		FromLineParser flp(str);
		flp.parse();
		if(flp.hasValidEmail()) str=flp.email();
	}	
	return str;	
}



void KNArticle::setTimeT(time_t t)
{
	t_imeT=t;
	if(t_imeString) {
		delete[] t_imeString;
		t_imeString=0;
	}
}



const char* KNArticle::timeString()
{
	if(!t_imeString) {
		t_imeString=new char[18];
		DwDateTime dt;
	  dt.FromUnixTime(t_imeT);
		sprintf(t_imeString,"%.2d.%.2d.%.2d (%.2d:%.2d)",dt.Day(),dt.Month(),(dt.Year()%100),
 				dt.Hour(),dt.Minute());
	}
	
	return t_imeString;
}



int KNArticle::age()
{
	static QDate today=QDate::currentDate();
	static QDateTime artDate;
	int a=0;
	artDate.setTime_t(t_imeT);
	a=artDate.date().daysTo(today);
	return a;
}



int KNArticle::lines()
{
	QCString tmp=headerLine("Lines");
	if(tmp.isEmpty()) return 0;
	else return tmp.toInt();
}



void KNArticle::setListItem(KNHdrViewItem *it)
{
	i_tem=it;
	if(it) it->art=this;
}








