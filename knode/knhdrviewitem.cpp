/***************************************************************************
                     knhdrviewitem.cpp - description
 copyright            : (C) 1999 by Christian Thurner
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

#include "knhdrviewitem.h"
#include "knmime.h"
#include <stdio.h>


KNHdrViewItem::KNHdrViewItem(KNListView *ref, KNArticle *a) :
  KNLVItemBase(ref), art(a)
{
}



KNHdrViewItem::KNHdrViewItem(KNLVItemBase *ref, KNArticle *a) :
  KNLVItemBase(ref), art(a)
{
}



KNHdrViewItem::~KNHdrViewItem()
{
  if(art) art->setListItem(0);
}



QString KNHdrViewItem::key(int col, bool) const
{
  if(col==3) {
    QString tmpString;
    return tmpString.sprintf("%08d",(uint)art->date()->unixTime());
  } else
    return text(col);
}



bool KNHdrViewItem::greyOut()
{
  if(art->type()==KNMimeBase::ATremote)
    return (  !((KNRemoteArticle*)art)->hasUnreadFollowUps() &&
              ((KNRemoteArticle*)art)->isRead() );
  else return false;  
}



bool KNHdrViewItem::firstColBold()
{
	if(art->type()==KNMimeBase::ATremote)
		return ( static_cast<KNRemoteArticle*>(art)->isNew() );
	else
		return false;
}

