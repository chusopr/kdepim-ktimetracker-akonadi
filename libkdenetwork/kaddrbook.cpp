// -*- mode: C++; c-file-style: "gnu" -*-
// kaddrbook.cpp
// Author: Stefan Taferner <taferner@kde.org>
// This code is under GPL

#include <config.h>
#include <unistd.h>

#include "kaddrbook.h"

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kabc/stdaddressbook.h>
#include <kabc/distributionlist.h>
#include <kabc/vcardconverter.h>
#include <dcopref.h>

#include <qregexp.h>

//-----------------------------------------------------------------------------
void KAddrBookExternal::openEmail( const QString &email, const QString &addr, QWidget *) {
  //QString email = KMMessage::getEmailAddr(addr);
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
}

//-----------------------------------------------------------------------------
void KAddrBookExternal::addEmail( const QString& addr, QWidget *parent) {
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

    if ( !KAddrBookExternal::addAddressee( a ) ) {
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

void KAddrBookExternal::openAddressBook(QWidget *) {
  kapp->startServiceByDesktopName( "kaddressbook" );
}

void KAddrBookExternal::addNewAddressee( QWidget* )
{
  kapp->startServiceByDesktopName("kaddressbook");
  sleep(2);
  DCOPRef call("kaddressbook", "KAddressBookIface");
  call.send("newContact()");
}

bool KAddrBookExternal::addVCard( const KABC::Addressee& addressee, QWidget *parent )
{
  KABC::AddressBook *ab = KABC::StdAddressBook::self();
  bool inserted = false;

  KABC::Addressee::List addressees =
      ab->findByEmail( addressee.preferredEmail() );

  if ( addressees.isEmpty() ) {
    if ( !KAddrBookExternal::addAddressee( addressee ) ) {
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

bool KAddrBookExternal::addAddressee( const KABC::Addressee& addressee )
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

QString KAddrBookExternal::expandDistributionList( const QString& listName )
{
  if ( listName.isEmpty() )
    return QString::null;

  const QString lowerListName = listName.lower();
  KABC::AddressBook *addressBook = KABC::StdAddressBook::self();
  KABC::DistributionListManager manager( addressBook );
  manager.load();
  const QStringList listNames = manager.listNames();

  for ( QStringList::ConstIterator it = listNames.begin();
        it != listNames.end(); ++it) {
    if ( (*it).lower() == lowerListName ) {
      const QStringList addressList = manager.list( *it )->emails();
      return addressList.join( ", " );
    }
  }
  return QString::null;
}
