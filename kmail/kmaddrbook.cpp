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
#if !KDE_IS_VERSION( 3, 1, 92 )
#include <kstandarddirs.h>
#include <kprocess.h>
#include <krun.h>
#endif

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

//-----------------------------------------------------------------------------
void KMAddrBookExternal::openEmail( const QString &addr, QWidget *) {
#if KDE_IS_VERSION( 3, 1, 92 )
  QString email = KMMessage::getEmailAddr(addr);
  KABC::AddressBook *addressBook = KABC::StdAddressBook::self();
  KABC::Addressee::List addresseeList = addressBook->findByEmail(email);
  kapp->startServiceByDesktopName( "kaddressbook" );
  DCOPRef call( "kaddressbook", "KAddressBookIface" );
  if( !addresseeList.isEmpty() ) {
    call.send( "showContactEditor(QString)", addresseeList.first().uid() );
  }
  else {
    call.send( "addEmail(QString)", addr );
  }
#else
  if ( checkForAddressBook() ) {
    KRun::runCommand( "kaddressbook -a " + KProcess::quote(addr) );
  }
#endif
}

//-----------------------------------------------------------------------------
void KMAddrBookExternal::addEmail( const QString& addr, QWidget *parent) {
  QString email;
  QString name;

  KABC::Addressee::parseEmailAddress( addr, name, email );

  KABC::AddressBook *ab = KABC::StdAddressBook::self();

  // force a reload of the address book file so that changes that were made
  // by other programs are loaded
  ab->load();

  KABC::Addressee::List addressees = ab->findByEmail( email );

  if ( addressees.isEmpty() ) {
    KABC::Addressee a;
    a.setNameFromString( name );
    a.insertEmail( email, true );

    if ( !KMAddrBookExternal::addAddressee( a ) ) {
      KMessageBox::error( parent, i18n("Cannot save to addressbook.") );
    } else {
      QString text = i18n("<qt>The email address <b>%1</b> was added to your "
                          "addressbook; you can add more information to this "
                          "entry by opening the addressbook.</qt>").arg( addr );
      KMessageBox::information( parent, text, QString::null, "addedtokabc" );
    }
  } else {
    QString text = i18n("<qt>The email address <b>%1</b> is already in your "
                        "addressbook.</qt>").arg( addr );
    KMessageBox::information( parent, text );
  }
}

void KMAddrBookExternal::openAddressBook(QWidget *) {
#if KDE_IS_VERSION( 3, 1, 92 )
  kapp->startServiceByDesktopName( "kaddressbook" );
#else
  if ( checkForAddressBook() ) {
    KRun::runCommand( "kaddressbook" );
  }
#endif
}

#if !KDE_IS_VERSION( 3, 1, 92 )
bool KMAddrBookExternal::checkForAddressBook()
{
  if ( KStandardDirs::findExe( "kaddressbook" ).isEmpty() ) {
    KMessageBox::information( 0,
        i18n("No external address book application found. You might want to "
             "install KAddressBook from the kdepim module.") );
    return false;
  } else {
    return true;
  }
}
#endif

void KMAddrBookExternal::addNewAddressee( QWidget* )
{
#if KDE_IS_VERSION( 3, 1, 92 )
  kapp->startServiceByDesktopName("kaddressbook");
  sleep(2);
  DCOPRef call("kaddressbook", "KAddressBookIface");
  call.send("newContact()");
#else
  if ( checkForAddressBook() ) {
    KRun::runCommand( "kaddressbook --editor-only --new-contact" );
  }
#endif
}

bool KMAddrBookExternal::addVCard( const KABC::Addressee& addressee, QWidget *parent )
{
  KABC::AddressBook *ab = KABC::StdAddressBook::self();
  bool inserted = false;

  KABC::Addressee::List addressees =
      ab->findByEmail( addressee.preferredEmail() );

  if ( addressees.isEmpty() ) {
    if ( !KMAddrBookExternal::addAddressee( addressee ) ) {
      KMessageBox::error( parent, i18n("Cannot save to addressbook.") );
      inserted = false;
    } else {
      QString text = i18n("The VCard was added to your addressbook; "
                          "you can add more information to this "
                          "entry by opening the addressbook.");
      KMessageBox::information( parent, text, QString::null, "addedtokabc" );
      inserted = true;
    }
  } else {
    QString text = i18n("The VCard's primary email address is already in "
                        "your addressbook; however, you may save the VCard "
                        "into a file and import it into the addressbook "
                        "manually.");
    KMessageBox::information( parent, text );
    inserted = true;
  }

  return inserted;
}

bool KMAddrBookExternal::addAddressee( const KABC::Addressee& addressee )
{
  KABC::AddressBook *ab = KABC::StdAddressBook::self();
  KABC::Ticket *t = ab->requestSaveTicket();
  bool saved = false;
  if ( t ) {
    ab->insertAddressee( addressee );
    saved = ab->save( t );
    if ( !saved )
      ab->releaseSaveTicket( t );
  }
  return saved;
}
