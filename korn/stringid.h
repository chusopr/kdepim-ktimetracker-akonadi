#ifndef STRINGID_H
#define STRINGID_H

/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * This class provides a identification with string,
 * because in KIO, I don't know if emails return in the same order.
 * Author Mart Kelder
 */

#include<qstring.h>
#include"mailid.h"

class KornStringId : public KornMailId
{
private:
	QString _id;
public:
	KornStringId( const QString & id );
	KornStringId( const KornStringId & src );
	~KornStringId() {}

	QString getId() const { return _id; }
	virtual QString toString() const { return _id; }

	virtual KornMailId *clone() const;
};

#endif //STRINGID_H
