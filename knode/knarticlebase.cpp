/***************************************************************************
                          knarticlebase.cpp  -  description
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

#include <qregexp.h>
#include <kconfig.h>
#include <mimelib/mimepp.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>


#include "knarticlebase.h"
#include "utilities.h"
#include "knstringsplitter.h"

bool KNArticleBase::allow8bit;
static char chars[] = "0123456789abcdefghijklmnopqrstuvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";


QCString KNArticleBase::uniqueString()
{
	time_t now;
	QCString ret;
	char p[11];
	int pos, ran;
	unsigned int timeval;
	
	p[10]='\0';
	now=time(0);
	ran=1+(int) (1000.0*rand()/(RAND_MAX+1.0));
	timeval=(now/ran)+getpid();
	
	for(int i=0; i<10; i++){
		pos=(int) (61.0*rand()/(RAND_MAX+1.0));
		//qDebug("%d", pos);
		p[i]=chars[pos];
	}
	ret.sprintf("%d.%s", timeval, p);
	
	return ret;	
}



// THIS PART IS TAKEN FROM KMAIL : START

QCString KNArticleBase::decodeQuotedPrintable(const QCString aStr)
{
    qDebug("decoding %s", aStr.data());

  DwString dwsrc(aStr.data());
  DwString dwdest;

  DwDecodeQuotedPrintable(dwsrc, dwdest);
  return QCString(dwdest.c_str());
}



QCString KNArticleBase::encodeQuotedPrintable(const QCString aStr)
{
  DwString dwsrc(aStr.data(), aStr.length());
  DwString dwdest;
  QCString result;

  DwEncodeQuotedPrintable(dwsrc, dwdest);
  result = dwdest.c_str();
  return result;
}



QCString KNArticleBase::decodeQuotedPrintableString(const QCString aStr)
{
  return decodeRFC1522String(aStr);
}



QCString KNArticleBase::decodeBase64(const QCString aStr)
{
  DwString dwsrc(aStr.data(), aStr.length());
  DwString dwdest;
  QCString result;

  DwDecodeBase64(dwsrc, dwdest);
  result = dwdest.c_str();
  return result;
}



QCString KNArticleBase::encodeBase64(const QCString aStr)
{
  DwString dwsrc(aStr.data(), aStr.length());
  DwString dwdest;
  QCString result;

  DwEncodeBase64(dwsrc, dwdest);
  result = dwdest.c_str();
  return result;
}



QCString KNArticleBase::decodeRFC1522String(const QCString aStr)
{
  static QCString result;
  char *pos, *dest, *beg, *end, *mid;
  QCString str;
  char encoding, ch;
  bool valid;
  const int maxLen=400;
  int i;

  if (aStr.find("=?") < 0) return aStr;

  result.truncate(aStr.length());
  for (pos=aStr.data(), dest=result.data(); *pos; pos++)
  {
    if (pos[0]!='=' || pos[1]!='?')
    {
      *dest++ = *pos;
      continue;
    }
    beg = pos+2;
    end = beg;
    valid = TRUE;
    // parse charset name
    for (i=2,pos+=2; i<maxLen && (*pos!='?'&&(ispunct(*pos)||isalnum(*pos))); i++)
      pos++;
    if (*pos!='?' || i<4 || i>=maxLen) valid = FALSE;
    else
    {
      // get encoding and check delimiting question marks
      encoding = toupper(pos[1]);
      if (pos[2]!='?' || (encoding!='Q' && encoding!='B'))
      	valid = FALSE;
      pos+=3;
      i+=3;
    }
    if (valid)
    {
      mid = pos;
      // search for end of encoded part
      while (i<maxLen && *pos && !(*pos=='?' && *(pos+1)=='='))
      {
      	i++;
      	pos++;
      }
      end = pos+2;//end now points to the first char after the encoded string
      if (i>=maxLen || !*pos) valid = FALSE;
    }
    if (valid)
    {
      ch = *pos;
      *pos = '\0';
      str = QCString(mid, (int)(mid - pos - 1));
      if (encoding == 'Q')
      {
      	// decode quoted printable text
      	for (i=str.length()-1; i>=0; i--)
      	  if (str[i]=='_') str[i]=' ';
          	str = decodeQuotedPrintable(str);
      }
      else
      {
       	// decode base64 text
      	str = decodeBase64(str);
      }
      *pos = ch;
      for (i=0; str[i]; i++)
    	*dest++ = str[i];

      pos = end -1;
    }
    else
    {
      //result += "=?";
      //pos = beg -1; // because pos gets increased shortly afterwards
      pos = beg - 2;
      *dest++ = *pos++;
      *dest++ = *pos;
    }
  }
  *dest = '\0';
  return result;
}
// THIS PART IS TAKEN FROM KMAIL : END


QCString KNArticleBase::encodeRFC1522String(const QCString aStr)
{
	QCString result, tmp, chset;
	bool usascii, isFirst;
	KNStringSplitter split;
	
	split.init(aStr, " ");
	
	isFirst=true;
	
	CONF()->setGroup("POSTNEWS");
	chset=CONF()->readEntry("Charset", "ISO-8859-1").upper().local8Bit();
	if(chset=="US-ASCII") chset="ISO-8859-1";
	
	if(!split.first()) tmp=aStr;
	else tmp=split.string();
	
	while(1) {
	
	  usascii=true;
	  	
		for(unsigned int i=0; i<tmp.length(); i++)
			if(tmp[i]<0) {
				usascii=false;
				break;
			}
		
		if(!isFirst) result+=" ";
					
		if(!usascii) {
			
			result+="=?"+chset+"?Q?"+encodeQuotedPrintable(tmp)+="?=";
		}
		else result+=tmp;
		
		isFirst=false;
		
		if(split.next()) tmp=split.string();
		else break;
		
	}
	
	
	return result;		
}



bool KNArticleBase::stripCRLF(char *str)
{
	int pos=strlen(str)-1;
	while(str[pos]!='\n' && pos>0) pos--;
	if(pos>0) {
		str[pos--]='\0';
		if(str[pos]=='\r') str[pos]='\0';
		return true;
	}
	else return false;
}



void KNArticleBase::removeQuots(QCString &str)
{
	int pos1=0, pos2=0;
	unsigned int idx=0;
	char firstChar, lastChar;
		
	do {
		pos1=idx;
		firstChar=str[idx++];
	} while(firstChar==' ' && idx<str.length());
		
	idx=str.length();
		
	do {
		lastChar=str[--idx];
		pos2=idx-1;
	} while(lastChar==' ' && idx>0);
		
	if(firstChar=='"' && lastChar=='"') {
		str.remove(pos1,1);
		str.remove(pos2,1);
	}
	
}



QCString KNArticleBase::headerTypeToString(headerType t)
{
	QCString s;
	
	switch(t) {
		case HTmessageId: 		s="Message-ID"; break;
		case HTfrom:					s="From"; break;
		case HTsubject:				s="Subject"; break;
		case HTcontrol:				s="Control"; break;
		case HTto:						s="To"; break;
		case HTnewsgroups:		s="Newsgroups"; break;
		case HTfup2:					s="Followup-To2"; break;
		case HTreplyTo:				s="Reply-To"; break;
		case HTdate:          s="Date"; break;
		case HTreferences:		s="References"; break;
		case HTlines:         s="Lines"; break;
		case HTorga:         	s="Organization"; break;
		case HTmimeVersion:   s="Mime-Version"; break;
		case HTcontentType:   s="Content-Type"; break;
		case HTencoding:      s="Content-Transfer-Encoding"; break;
		case HTdisposition:   s="Content-Disposition"; break;
		case HTuserAgent:   	s="User-Agent"; break;
		case HTxknstatus:     s="X-KNode-Status"; break;
		default:              s="X-Unknown"; break;
	}
	return s;
}



int KNArticleBase::stringToHeaderType(const char *s)
{
	headerType t;
	
	if(strncasecmp(s, "Message-ID", 10)==0) 												t=HTmessageId;
	else if(strncasecmp(s, "From", 4)==0) 													t=HTfrom;
	else if(strncasecmp(s, "Subject", 7)==0)                        t=HTsubject;
	else if(strncasecmp(s, "Control", 7)==0)                        t=HTcontrol;
	else if(strncasecmp(s, "To", 2)==0)                             t=HTto;
	else if(strncasecmp(s, "Newsgroups", 10)==0)                    t=HTnewsgroups;
	else if(strncasecmp(s, "Followup-To", 11)==0)                   t=HTfup2;
	else if(strncasecmp(s, "Reply-To", 8)==0)                   		t=HTreplyTo;
	else if(strncasecmp(s, "Date", 4)==0)                           t=HTdate;
	else if(strncasecmp(s, "References", 10)==0)                    t=HTreferences;
	else if(strncasecmp(s, "Lines", 5)==0)                          t=HTlines;
	else if(strncasecmp(s, "Organization", 12)==0)									t=HTorga;
	else if(strncasecmp(s, "Mime-Version", 12)==0)                  t=HTmimeVersion;
	else if(strncasecmp(s, "Content-Type", 12)==0)                  t=HTcontentType;
	else if(strncasecmp(s, "Content-Transfer-Encoding", 25)==0)     t=HTencoding;
	else if(strncasecmp(s, "Content-Disposition", 19)==0)           t=HTdisposition;
	else if(strncasecmp(s, "User-Agent", 10)==0)                    t=HTuserAgent;
	else if(strncasecmp(s, "X-KNode-Status", 14)==0)                t=HTxknstatus;
	else                                                            t=HTunknown;
	
	return t;
}



//===================================================================================


KNArticleBase::FromLineParser::FromLineParser(const QCString &fLine)
{
	src=fLine.stripWhiteSpace();
	is_broken=false;
}



KNArticleBase::FromLineParser::FromLineParser(const char *fLine)
{
	src=fLine;
	src=src.stripWhiteSpace();
	is_broken=false;
}



KNArticleBase::FromLineParser::~FromLineParser()
{
}



void KNArticleBase::FromLineParser::parse()
{
	int pos1=0, pos2=0, type=0, idx;
	QRegExp 	t0("*@*", false, true),
						t1("*<*@*>", false, true),
						t2("*@*(*)", false, true);
	
	idx=src.length();
	
	
	if(src.find(t2)!=-1) type=2;
	else if(src.find(t1)!=-1) type=1;
	else if(src.find(t0)!=-1) type=0;
	else {
		is_broken=true;
		return;
	}
	
	
	switch(type) {
	
		case 0:
			e_mail=src.copy();
		break;
		
		case 1:
			pos1=0;
			pos2=src.find('<');
			if(pos2!=-1) {
				f_rom=decodeRFC1522String(src.mid(pos1, pos2-pos1)).stripWhiteSpace();
				pos1=pos2+1;
				pos2=src.find('>', pos1);
				if(pos2==-1) is_broken=true;
				else e_mail=src.mid(pos1, pos2-pos1);
			}
			else is_broken=true;
		break;
		
		case 2:
			pos1=0;
			pos2=src.find('(');
			if(pos2!=-1) {
				e_mail=src.mid(pos1, pos2-pos1).stripWhiteSpace();
				pos1=pos2+1;
				pos2=src.find(')', pos1);
				if(pos2==-1) is_broken=true;
				else f_rom=decodeRFC1522String(src.mid(pos1, pos2-pos1));
			}
			else is_broken=true;
		break;
	
		default: is_broken=true; break;
	}
	
	if(!is_broken && !f_rom.isEmpty()) removeQuots(f_rom);
	
}



bool KNArticleBase::FromLineParser::hasValidEmail()
{
	static QRegExp match("*@*", true, true);
	
	return (e_mail.find(match)!=-1);
	
}



bool KNArticleBase::FromLineParser::hasValidFrom()
{
	return (!f_rom.isEmpty());
}



//===================================================================================



KNArticleBase::MultiPartParser::MultiPartParser(QStrList *l, const QCString &b)
{
	src=l;
	startBoundary="--"+b;
	endBoundary="--"+b+"--";
	p_art=0;
	pos=0;
}



KNArticleBase::MultiPartParser::~MultiPartParser()
{
}


			
QStrList* KNArticleBase::MultiPartParser::nextPart()
{
	p_art=0;
	
	if(pos==-1) return 0;
	
	char *line=src->at(pos);
	
	while(line && !isStartBoundary(line)) line=src->next();
	if(!line) {
		pos=-1;
		return 0;
	}
	else line=src->next();
	
	if(line) {
		p_art=new QStrList(false);
		p_art->setAutoDelete(false);
		
		while(line) {
			if(isStartBoundary(line) || isEndBoundary(line)) break;
			else {
				p_art->append(line);
				line=src->next();
			}
		}
		pos=src->at();
	}
	
	return p_art;
}



bool KNArticleBase::MultiPartParser::isStartBoundary(const char *line)
{
	return (	(strncmp(startBoundary, line, startBoundary.length())==0)	
           	&&  (strncmp(endBoundary, line, endBoundary.length())!=0) );
}


bool KNArticleBase::MultiPartParser::isEndBoundary(const char *line)
{
	return (strncmp(endBoundary, line, endBoundary.length())==0);
}



//===================================================================================




KNArticleBase::UUParser::UUParser(QStrList *l)
{
	src=l;
	text=0;
	bin=0;
}



KNArticleBase::UUParser::~UUParser()
{
	delete text;
	delete bin;
}



void KNArticleBase::UUParser::parse()
{
	int beginPos=-1, endPos=-1;
	char *line;
	int MCnt=0, lineCnt=0, pos1=0;
	QCString tmp;
	
	for(line=src->first(); line; line=src->next()) {
		if(strncasecmp(line, "begin", 5)==0) {
			beginPos=src->at();
			tmp=line;
			pos1=tmp.findRev(' ');
			if(pos1!=-1) fName=tmp.right(tmp.length()-(++pos1));
			else beginPos=-1;
			break;
		}
	}
	if(beginPos==-1) return;
	
	line=src->next();
	while(line!=0 && endPos==-1) {
		lineCnt++;
		if(strncmp(line, "M", 1)==0) MCnt++;
		if(strncasecmp(line, "end", 3)==0) endPos=src->at();
		line=src->next();
	}	
	if(beginPos==-1 || endPos==-1 || (lineCnt-MCnt)>5) return;
	
	text=new QStrList(false);
  text->setAutoDelete(false);
  for(int idx=0; idx<beginPos; idx++)
    text->append(src->at(idx));

	bin=new QStrList(false);
	bin->setAutoDelete(false);
	for(int idx=beginPos; idx<=endPos; idx++)
		bin->append(src->at(idx));
	
  if(!fName.isEmpty()) {
    pos1=fName.findRev('.');
    if(pos1++ != -1) {
      tmp=fName.mid(pos1, fName.length()-pos1).upper();
      if(tmp=="JPG" || tmp=="JPEG")       mimeType="image/jpeg";
      else if(tmp=="GIF")                 mimeType="image/gif";
      else if(tmp=="PNG")                 mimeType="image/png";
      else if(tmp=="TIFF" || tmp=="TIF")  mimeType="image/tiff";
      else if(tmp=="XPM")                 mimeType="image/x-xpm";
      else if(tmp=="XBM")                 mimeType="image/x-xbm";
      else if(tmp=="BMP")                 mimeType="image/x-bmp";
      else if(tmp=="TXT" ||
              tmp=="ASC" ||
              tmp=="H" ||
              tmp=="C" ||
              tmp=="CC" ||
              tmp=="CPP")                 mimeType="text/plain";
      else if(tmp=="HTML" || tmp=="HTM")  mimeType="text/html";
      else                                mimeType="application/octet-stream";
    }
  }
}
