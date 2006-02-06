/*
    This file is part of kdepim.

    Copyright (c) 2005 Will Stephenson <lists@stevello.free-online.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "networkstatus.h"

#include <qtimer.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>

#include "clientifaceimpl.h"
#include "serviceifaceimpl.h"
#include "network.h"
#include <kdepimmacros.h>

extern "C" {
	KDE_EXPORT KDEDModule* create_networkstatus( const DCOPCString& obj )
	{
		return new NetworkStatusModule( obj );
	}
}

// INTERNALLY USED STRUCTS AND TYPEDEFS

//typedef QDict< Network > NetworkList;
typedef QList< Network * > NetworkList;

class NetworkStatusModule::Private
{
public:
	NetworkList networks;
/*	ClientIface * clientIface;
	ServiceIface * serviceIface;*/
};

// CTORS/DTORS

NetworkStatusModule::NetworkStatusModule( const DCOPCString & obj ) : KDEDModule( obj )
{
	d = new Private;
/*	d->clientIface = new ClientIfaceImpl( this );
	d->serviceIface = new ServiceIfaceImpl( this );*/
	connect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QByteArray& ) ) , this, SLOT( unregisteredFromDCOP( const QByteArray& ) ) );
	connect( kapp->dcopClient(), SIGNAL( applicationRegistered( const QByteArray& ) ) , this, SLOT( registeredToDCOP( const QByteArray& ) ) );
}

NetworkStatusModule::~NetworkStatusModule()
{
/*	delete d->clientIface;
	delete d->serviceIface;*/
	delete d;
}

// CLIENT INTERFACE

QStringList NetworkStatusModule::networks()
{
	kDebug() << k_funcinfo << " contains " << d->networks.count() << " networks" << endl;
	QStringList networks;
	NetworkList::iterator end = d->networks.end();
	NetworkList::iterator it = d->networks.begin();
	for ( ; it != end; ++it )
		networks.append( (*it)->name() );
	return networks;
}

int NetworkStatusModule::status( const QString & host )
{
	Network * p = networkForHost( host );
	if ( !p )
	{
		kDebug() << k_funcinfo << " no networks have status for host '" << host << "'" << endl;
		return (int)NetworkStatus::NoNetworks;
	}
	else
	{	
		kDebug() << k_funcinfo << " got status for host '" << host << "' : " << (int)(p->status()) << endl;
		return (int)(p->status());
	}
}

int NetworkStatusModule::request( const QString & host, bool userInitiated )
{
	// identify most suitable network for host
	Network * p = networkForHost( host );
	if ( !p )
		return NetworkStatus::Unavailable;
	
	NetworkStatus::EnumStatus status = p->status();
	QByteArray appId = kapp->dcopClient()->senderId();
	if ( status == NetworkStatus::Online )
	{
		p->registerUsage( appId, host );
		return NetworkStatus::Connected;
	}
	// if online
	//   register usage
	//   return Available
	else if ( status == NetworkStatus::Establishing )
	{
		p->registerUsage( appId, host );
		return NetworkStatus::RequestAccepted;
	}
	// if establishing
	//   register usage
	//   return Accepted
	else if ( status == NetworkStatus::Offline || status == NetworkStatus::ShuttingDown )
	{
		// TODO: check on demand policy
		
		p->registerUsage( appId, host );
		return NetworkStatus::RequestAccepted;
	}
	// if offline or ShuttingDown
	//   check ODP::
	//   always or Permanent: register, return accepted
	//   user: check userInitiated, register, return Accepted or UserRefused
	//   never: return UserRefused
	else if ( status == NetworkStatus::OfflineFailed )
	{
		// TODO: check user's preference for dealing with failed networks
		p->registerUsage( appId, host );
		return NetworkStatus::RequestAccepted;
	}
	// if OfflineFailed
	//   check user's preference
	else if ( status == NetworkStatus::OfflineDisconnected )
	{
		return NetworkStatus::Unavailable;
	}
	else
		return NetworkStatus::Unavailable;
	// if OfflineDisconnected or NoNetworks
	//   return Unavailable
}

void NetworkStatusModule::relinquish( const QString & host )
{
	QByteArray appId = kapp->dcopClient()->senderId();
	// find network currently used by app for host...
	NetworkList::iterator end = d->networks.end();
	NetworkList::iterator it = d->networks.begin();
	for ( ; it != end; ++it )
	{
		Network * net = *it;
		NetworkUsageList usage = net->usage();
		NetworkUsageList::iterator end2 = usage.end();
		for ( NetworkUsageList::iterator usageIt = usage.begin(); usageIt != end2; ++usageIt )
		{
			if ( (*usageIt).appId == appId && (*usageIt).host == host )
			{
				// remove host usage record
				usage.erase( usageIt );
				// if requested shutdown flagged for network
				//  check if all hosts have relinquished
				//   call confirmShutDown on Service
				//checkShutdownOk();
			}
		}
	}
}

bool NetworkStatusModule::reportFailure( const QString & host )
{
	// find network for host
	// check IP record.  remove IP usage record.  if other IP exists, return true.
	Q_UNUSED( host );
	kDebug() << k_funcinfo << "NOT IMPLEMENTED" << endl;
	return false;
}

// PROTECTED UTILITY FUNCTIONS
/*
 * Determine the network to use for the supplied host
 */
Network * NetworkStatusModule::networkForHost( const QString & host )
{
	// return a null pointer if no networks are registered
	if ( d->networks.isEmpty() )
		return 0;
	
	NetworkList::iterator it = d->networks.begin();
	Network * bestNetwork = *(it++);
	NetworkList::iterator end = d->networks.end();
 	for ( ; it != end; ++it )
	{
		if ( (*it)->reachabilityFor( host ) > bestNetwork->reachabilityFor( host ) )
		{
			bestNetwork = (*it);
		}
	}
	return bestNetwork;
}


void NetworkStatusModule::registeredToDCOP( const QByteArray & appId )
{
}

void NetworkStatusModule::unregisteredFromDCOP( const QByteArray & appId )
{
	// unregister any networks owned by a service that has just unregistered
	NetworkList::iterator it = d->networks.begin();
	NetworkList::iterator end = d->networks.end();
	for ( ; it != end; ++it )
	{
		if ( (*it)->service() == appId)
		{
			kDebug() << k_funcinfo << "removing '" << (*it)->name() << "', registered by " << appId << endl;
			d->networks.erase( it );
			break;
		}
	}
}

// SERVICE INTERFACE //
void NetworkStatusModule::setNetworkStatus( const QString & networkName, int st )
{
	kDebug() << k_funcinfo << endl;
	NetworkStatus::EnumStatus status = (NetworkStatus::EnumStatus)st;
	Network * net = 0;
	NetworkList::iterator it = d->networks.begin();
	NetworkList::iterator end = d->networks.end();
	for ( ; it != end; ++it )
	{
		if ( (*it)->name() == networkName )
		{
			net = (*it);
			break;
		}
	}
	if ( net )
	{
		if ( net->status() == status )
			return;

		// update the status of the network
		net->setStatus( status );

		// notify for each host in use on that network
		NetworkUsageList usage = net->usage();
		NetworkUsageList::iterator end = usage.end();
		QStringList notified;
		for ( NetworkUsageList::iterator it = usage.begin(); it != end; ++it )
		{
			// only notify once per host
			if ( !notified.contains( (*it).host ) )
			{
				kDebug() << "notifying statusChange of " << networkName << " to " << (int)status << 
						" because " << (*it).appId << " is using " << (*it).host << endl;
				/*d->clientIface->*/statusChange( (*it).host, (int)status );
				notified.append( (*it).host );
			}
		}

		// if we are now anything but Establishing or Online, reset the usage records for that network
		if ( !( net->status() == NetworkStatus::Establishing || net->status() == NetworkStatus::Establishing ) )
			net->removeAllUsage();
	}
	else
		kDebug() << k_funcinfo << "No network found by this name" << endl;
}

void NetworkStatusModule::registerNetwork( const QString & networkName, const NetworkStatus::Properties properties )
{
	kDebug() << k_funcinfo << "registering '" << networkName << "', with status " << properties.status << endl;
	// TODO: check for re-registration, checking appid matches
	
	d->networks.append( new Network( networkName, properties ) );
}

void NetworkStatusModule::unregisterNetwork( const QString & networkName )
{
	// TODO: check appid
	//d->networks.remove( networkName );
}

void NetworkStatusModule::requestShutdown( const QString & networkName )
{
	Q_UNUSED( networkName );
	kDebug() << k_funcinfo << "NOT IMPLEMENTED" << endl;
}

#include "networkstatus.moc"
