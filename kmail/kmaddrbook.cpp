// -*- mode: C++; c-file-style: "gnu" -*-
// kmaddrbook.cpp
// Author: Stefan Taferner <taferner@kde.org>
// This code is under GPL

#include <config.h>
#include <unistd.h>

#include "kmaddrbook.h"
#include "kcursorsaver.h"
#include "kmmessage.h"
#include "kmkernel.h" // for KabcBridge

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kabc/stdaddressbook.h>
#include <kabc/distributionlist.h>
#include <kabc/vcardconverter.h>
#include <dcopref.h>

#include <qregexp.h>

void KabcBridge::addresses(QStringList& result) // includes lists
{
  KCursorSaver busy(KBusyPtr::busy()); // loading might take a while

  KABC::AddressBook *addressBook = KABC::StdAddressBook::self();
  KABC::AddressBook::Iterator it;
  for( it = addressBook->begin(); it != addressBook->end(); ++it ) {
    QStringList emails = (*it).emails();
    QString n = (*it).prefix() + " " +
		(*it).givenName() + " " +
		(*it).additionalName() + " " +
	        (*it).familyName() + " " +
		(*it).suffix();
    n = n.simplifyWhiteSpace();

    QRegExp needQuotes("[^ 0-9A-Za-z\\x0080-\\xFFFF]");
    QString endQuote = "\" ";
    QStringList::ConstIterator mit;
    QString addr, email;

    for ( mit = emails.begin(); mit != emails.end(); ++mit ) {
      email = *mit;
      if (!email.isEmpty()) {
	if (n.isEmpty() || (email.find( '<' ) != -1))
	  addr = QString::null;
	else { // do we really need quotes around this name ?
          if (n.find(needQuotes) != -1)
	    addr = '"' + n + endQuote;
	  else
	    addr = n + ' ';
	}

	if (!addr.isEmpty() && (email.find( '<' ) == -1)
	    && (email.find( '>' ) == -1)
	    && (email.find( ',' ) == -1))
	  addr += '<' + email + '>';
	else
	  addr += email;
	addr = addr.stripWhiteSpace();
	result.append( addr );
      }
    }
  }
  KABC::DistributionListManager manager( addressBook );
  manager.load();

  QStringList names = manager.listNames();
  QStringList::Iterator jt;
  for ( jt = names.begin(); jt != names.end(); ++jt)
    result.append( *jt );
  result.sort();
}

QStringList KabcBridge::addresses()
{
    QStringList entries;
    KABC::AddressBook::ConstIterator it;

    KABC::AddressBook *addressBook = KABC::StdAddressBook::self();
    for( it = addressBook->begin(); it != addressBook->end(); ++it ) {
        entries += (*it).fullEmail();
    }
    return entries;
}

//-----------------------------------------------------------------------------
QString KabcBridge::expandNickName( const QString& nickName )
{
  if ( nickName.isEmpty() )
    return QString();

  QString lowerNickName = nickName.lower();
  KABC::AddressBook *addressBook = KABC::StdAddressBook::self();
  for( KABC::AddressBook::ConstIterator it = addressBook->begin();
       it != addressBook->end(); ++it ) {
    if ( (*it).nickName().lower() == lowerNickName )
      return (*it).fullEmail();
  }
  return QString();
}


//-----------------------------------------------------------------------------
QString KabcBridge::expandDistributionList( const QString& listName )
{
  if ( listName.isEmpty() )
    return QString();

  QString lowerListName = listName.lower();
  KABC::AddressBook *addressBook = KABC::StdAddressBook::self();
  KABC::DistributionListManager manager( addressBook );
  manager.load();
  QStringList listNames = manager.listNames();

  for ( QStringList::Iterator it = listNames.begin();
        it != listNames.end(); ++it) {
    if ( (*it).lower() == lowerListName ) {
      QStringList addressList = manager.list( *it )->emails();
      return addressList.join( ", " );
    }
  }
  return QString();
}

QStringList KabcBridge::categories()
{
  KABC::AddressBook *addressBook = KABC::StdAddressBook::self();
  KABC::Addressee::List addresses = addressBook->allAddressees();
  QStringList allcategories, aux;

  for ( KABC::Addressee::List::Iterator it = addresses.begin();
        it != addresses.end(); ++it ) {
    aux = ( *it ).categories();
    for ( QStringList::ConstIterator itAux = aux.begin();
          itAux != aux.end(); ++itAux ) {
      // don't have duplicates in allcategories
      if ( allcategories.find( *itAux ) == allcategories.end() )
        allcategories += *itAux;
    }
  }
  return allcategories;
}
