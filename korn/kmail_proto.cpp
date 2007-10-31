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


#include "kmail_proto.h"

#include "account_input.h"
#include "kio.h"
#include "password.h"
#include "protocols.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include <QMap>
#include <QObject>
#include <QList>
#include <QVector>
#include <QStringList>

const char* KMail_Protocol::kmailGroupName = "Account %1";
const char* KMail_Protocol::kmailKeyType = "Type";
const char* KMail_Protocol::kmailKeyName = "Name";
const char* KMail_Protocol::kmailKeyId = "Id";
const char* KMail_Protocol::kmailKeyMBox = "Location";
const char* KMail_Protocol::kmailKeyQMail = "Location";
const int KMail_Protocol::kmailFirstGroup = 1;

class KMailDrop;

KMail_Protocol::KMail_Protocol()
{
}

KMail_Protocol::~KMail_Protocol()
{
}

const Protocol* KMail_Protocol::getProtocol( AccountSettings* settings ) const
{
	KConfig kmailconfig( "kmailrc", KConfig::NoGlobals );
	int id;
	QString type;

	QString grpname;
	if( settings->readEntries().contains( "kmailname" ) )
		type = getTypeAndConfig( settings->readEntries()[ "kmailname" ], kmailconfig, id, grpname );
	else
		type = getTypeAndConfig( "", kmailconfig, id,grpname );

	if( type == "imap" )
		return Protocols::getProto( "imap" );
	if( type == "cachedimap" )
		return Protocols::getProto( "imap" );
	if( type == "pop" )
		return Protocols::getProto( "pop3" );
	if( type == "local" )
		return Protocols::getProto( "mbox" );
	if( type == "maildir" )
		return Protocols::getProto( "qmail" );

	//Type not recognized, or does not exist in KOrn
	kWarning() <<"KMail configuration not found";
	return 0;
}

KMailDrop* KMail_Protocol::createMaildrop( AccountSettings *settings ) const
{
	int id;
	QString type;
	
	KConfig kmailconfig( "kmailrc", KConfig::NoGlobals );
	QString grpname;
	if( settings->readEntries().contains( "kmailname" ) )
		type = getTypeAndConfig( settings->readEntries()[ "kmailname" ], kmailconfig, id, grpname );
	else
		type = getTypeAndConfig( "", kmailconfig, id, grpname );

	if( type == "imap" || type == "cachedimap" || type == "pop" || type == "local" || type == "maildir" )
		return new KKioDrop();
	
	kWarning() <<"KMail configuration not found";
	return 0;
}

QMap< QString, QString > * KMail_Protocol::createConfig( AccountSettings* settings ) const
{
	QMap< QString, QString > *result = new QMap<QString, QString>;
	int id;
	QString type;
	
	KConfig kmailconfig( "kmailrc", KConfig::NoGlobals );
	QString grpname;
	//First: find the account in the configuration and get the type and id out of it.
	if( settings->readEntries().contains( "kmailname" ) )
		type = getTypeAndConfig( settings->readEntries()[ "kmailname" ], kmailconfig, id, grpname );
	else
		type = getTypeAndConfig( "", kmailconfig, id, grpname );
	QString metadata;

	KConfigGroup grp(&kmailconfig, grpname);
	if( type == "imap" || type == "cachedimap" )
	{
		//Construct metadata
		if( grp.hasKey( "auth" ) )
			metadata += QString( "auth=%1," ).arg( grp.readEntry( "auth" ) );
		if( !grp.hasKey( "use-tls" ) )
			metadata += "tls=auto";
		else
		{
			if( grp.readEntry( "use-tls", false ) )
				metadata += "tls=on";
			else
				metadata += "tls=off";
		}
		//Add the fields into the mapping.
		result->insert( "name", settings->accountName() );
		result->insert( "server", grp.readEntry( "host", "localhost" ) );
		result->insert( "port", grp.readEntry( "port", "143" ) );
		result->insert( "ssl", grp.readEntry( "use-ssl", "false" ) );
		result->insert( "metadata", metadata );
		result->insert( "username", grp.readEntry( "login", "" ) );
		result->insert( "mailbox", "INBOX" ); //Didn't find a good way to get this out of the configuration yet.
		result->insert( "password", readPassword( grp.readEntry( "store-passwd", false ), kmailconfig, id ) );
		result->insert( "savepassword", grp.readEntry( "store-passwd", "false" ) );
	}
	if( type == "pop" )
	{
		//Constructing metadata
		if( grp.hasKey( "auth" ) )
			metadata += QString( "auth=%1," ).arg( grp.readEntry( "auth" ) );
		if( !grp.hasKey( "use-tls" ) )
			metadata += "tls=auto";
		else
		{
			if( grp.readEntry( "use-tls", false ) )
				metadata += "tls=on";
			else
				metadata += "tls=off";
		}
		result->insert( "name", settings->accountName() );
		result->insert( "server", grp.readEntry( "host", "localhost" ) );
		result->insert( "port", grp.readEntry( "port", "110" ) );
		result->insert( "ssl", grp.readEntry( "use-ssl", "false" ) );
		result->insert( "metadata", metadata );
		result->insert( "username", grp.readEntry( "login", "" ) );
		result->insert( "mailbox", "" );
		result->insert( "password", readPassword( grp.readEntry( "store-passwd", false ), kmailconfig, id ) );
		result->insert( "savepassword", grp.readEntry( "store-password", "false" ) );
	}
	if( type == "local" ) //mbox
	{
		result->insert( "name", settings->accountName() );
		result->insert( "server", "" );
		result->insert( "port", "0" );
		result->insert( "ssl", "false" );
		result->insert( "metadata", "" );
		result->insert( "username", "" );
		result->insert( "mailbox", grp.readPathEntry( kmailKeyMBox, "" ) );
		result->insert( "password", "" );
		result->insert( "savepassword", "false" );
	}
	if( type == "maildir" )
	{
		result->insert( "name", settings->accountName() );
		result->insert( "server", "" );
		result->insert( "port", "0" );
		result->insert( "ssl", "false" );
		result->insert( "metadata", "" );
		result->insert( "username", "" );
		result->insert( "mailbox", grp.readPathEntry( kmailKeyQMail, "" ) );
		result->insert( "password", "" );
		result->insert( "savepassword", "false" );
	}

	return result;
}

void KMail_Protocol::configFillGroupBoxes( QStringList* lijst ) const
{
	lijst->append( "KMail" );
}

void KMail_Protocol::configFields( QVector< QWidget* >* vector, const QObject*, QList< AccountInput* >* result ) const
{
	QMap< QString, QString > accountList;
	QString type;
	QString name;
	int nummer = kmailFirstGroup - 1;
	
	KConfig kmailconfig( "kmailrc", KConfig::NoGlobals );
	while( kmailconfig.hasGroup( QString( kmailGroupName ).arg( ++nummer ) ) )
	{
		
		KConfigGroup group = kmailconfig.group( QString( kmailGroupName ).arg( nummer ) );
		type = group.readEntry( kmailKeyType, QString() );
		name = group.readEntry( kmailKeyName, QString() );
		if( type == "imap" || type == "cachedimap" || type == "pop" || type == "local" )
		{
			accountList.insert( name, name );
		}
	}

	result->append( new ComboInput( vector->at( 0 ), i18n( "KMail name" ), accountList, *accountList.begin(), "kmailname" ) );
}

void KMail_Protocol::readEntries( QMap< QString, QString >* ) const
{
	//The configuartion is read out on the right way
}

void KMail_Protocol::writeEntries( QMap< QString, QString >* ) const
{
	//The configuartion is read out on the right way
}

QString KMail_Protocol::readPassword( bool store, const KConfigBase& config, int id ) const
{
	if( !store )
		return "";

	return KOrnPassword::readKMailPassword( id, config );
}

QString KMail_Protocol::getTypeAndConfig( const QString& kmailname, KConfig &kmailconfig, int &id, QString& groupname ) const
{
	int nummer = kmailFirstGroup - 1;
	bool found = false;

	id = -1;
	
	while( kmailconfig.hasGroup( QString( kmailGroupName ).arg( ++nummer ) ) )
	{
		KConfigGroup group = kmailconfig.group( QString( kmailGroupName ).arg( nummer ) );
		if( group.readEntry( kmailKeyName, QString() ) == kmailname )
		{
			id = group.readEntry( kmailKeyId, 0 );
			groupname = group.name();
			found = true;
			break;
		}
	}
	if( !found )
	{
		nummer = -1;
		return QString();
	}

	//The correct group is found
	return kmailconfig.group(groupname).readEntry( kmailKeyType, QString() );
}

