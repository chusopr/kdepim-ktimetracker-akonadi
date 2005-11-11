/*
 * Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "qmail_proto.h"
//Added by qt3to4:
#include <Q3PtrList>

#include <kdebug.h>
#include <kurlrequester.h>

#include <qlayout.h>
#include <qvector.h>
#include <qstringlist.h>

#include "account_input.h"

void QMail_Protocol::configFillGroupBoxes( QStringList* groupBoxes ) const
{
	groupBoxes->append( "Maildir" );
}

void QMail_Protocol::configFields( QVector< QWidget* >* vector, const QObject*, Q3PtrList< AccountInput > *result ) const
{
	result->append( new URLInput( vector->at( 0 ), i18n( "Path:" ), "", "mailbox" ) );
	dynamic_cast<KURLRequester*>(result->last()->rightWidget())->setMode( KFile::Directory );
}

void QMail_Protocol::readEntries( QMap< QString, QString >*, QMap< QString, QString >* ) const
{
}

void QMail_Protocol::writeEntries( QMap< QString, QString >* map ) const
{
	clearFields( map, (KIO_Protocol::Fields)( server | port | username | password | save_password | metadata ) );
}
