/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "addressbooksyncee.h"

#include "syncee.h"

#include <libkdepim/kabcresourcenull.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

using namespace KSync;

AddressBookSyncEntry::AddressBookSyncEntry( const KABC::Addressee &a ) :
    SyncEntry()
{
  mAddressee = a;
}

AddressBookSyncEntry::AddressBookSyncEntry( const AddressBookSyncEntry& entry )
  : SyncEntry( entry )
{
  mAddressee = entry.mAddressee;
  m_res = entry.m_res;
}

QString AddressBookSyncEntry::name()
{
  return mAddressee.realName();
}

QString AddressBookSyncEntry::id()
{
  return mAddressee.uid();
}

void AddressBookSyncEntry::setId(const QString& id)
{
  mAddressee.setUid( id );
}

AddressBookSyncEntry *AddressBookSyncEntry::clone()
{
  return new AddressBookSyncEntry( *this );
}

QString AddressBookSyncEntry::timestamp()
{
  return mAddressee.revision().toString();
}

QString AddressBookSyncEntry::type() const
{
  return QString::fromLatin1("AddressBookSyncEntry");
}

bool AddressBookSyncEntry::equals( SyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if ( !abEntry ) {
    kdDebug(5228) << "AddressBookSyncee::equals(): Wrong type." << endl;
    return false;
  }

  if ( mAddressee == abEntry->addressee() ) {
    kdDebug(5228) << "AddressBookSyncEntry::equals(): '" << entry->name() << "':"
              << "equal" << endl;
    return true;
  } else {
    kdDebug(5228) << "AddressBookSyncEntry::equals(): '" << entry->name() << "':"
              << "not equal" << endl;
    return false;
  }
}

QString AddressBookSyncEntry::resource() const
{
  return m_res;
}

void AddressBookSyncEntry::setResource( const QString &str )
{
  m_res = str;
}

/*
 * mergeWith hell :)
 * I hope it's worth the effort
 *
 */
    /* merge function */
typedef void (*merge)(KABC::Addressee&, const KABC::Addressee& );
typedef QMap<int, merge> MergeMap;

static MergeMap* mergeMap= 0;
static KStaticDeleter<MergeMap> mergeMapDeleter;

    /* merge functions */
static void mergeFamily    ( KABC::Addressee&, const KABC::Addressee& );
static void mergeGiven     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeAdditional( KABC::Addressee&, const KABC::Addressee& );
static void mergePrefix    ( KABC::Addressee&, const KABC::Addressee& );
static void mergeSuffix    ( KABC::Addressee&, const KABC::Addressee& );
static void mergeNick      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeBirth     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeHome      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeBus       ( KABC::Addressee&, const KABC::Addressee& );
static void mergeTime      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeGeo       ( KABC::Addressee&, const KABC::Addressee& );
static void mergeTitle     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeRole      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeOrg       ( KABC::Addressee&, const KABC::Addressee& );
static void mergeNote      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeUrl       ( KABC::Addressee&, const KABC::Addressee& );
static void mergeSecrecy   ( KABC::Addressee&, const KABC::Addressee& );
static void mergePicture   ( KABC::Addressee&, const KABC::Addressee& );
static void mergeSound     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeAgent     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeHomeTel   ( KABC::Addressee&, const KABC::Addressee& );
static void mergeOffTel    ( KABC::Addressee&, const KABC::Addressee& );
static void mergeMessenger ( KABC::Addressee&, const KABC::Addressee& );
static void mergePreferredNumber( KABC::Addressee&, const KABC::Addressee& );
static void mergeVoice     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeFax       ( KABC::Addressee&, const KABC::Addressee& );
static void mergeCell      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeVideo     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeMailbox   ( KABC::Addressee&, const KABC::Addressee& );
static void mergeModem     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeCarPhone  ( KABC::Addressee&, const KABC::Addressee& );
static void mergeISDN      ( KABC::Addressee&, const KABC::Addressee& );
static void mergePCS       ( KABC::Addressee&, const KABC::Addressee& );
static void mergePager     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeHomeFax   ( KABC::Addressee&, const KABC::Addressee& );
static void mergeWorkFax   ( KABC::Addressee&, const KABC::Addressee& );
static void mergeOtherTel  ( KABC::Addressee&, const KABC::Addressee& );
static void mergeCat       ( KABC::Addressee&, const KABC::Addressee& );
static void mergeKeys      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeCustom    ( KABC::Addressee&, const KABC::Addressee& );
static void mergeLogo      ( KABC::Addressee&, const KABC::Addressee& );
static void mergeEmail     ( KABC::Addressee&, const KABC::Addressee& );
static void mergeEmails    ( KABC::Addressee&, const KABC::Addressee& );


MergeMap* mergeMappi() {
    if (!mergeMap ) {
        mergeMapDeleter.setObject( mergeMap,  new MergeMap );
        mergeMap->insert(AddressBookSyncee::FamilyName, mergeFamily );
        mergeMap->insert(AddressBookSyncee::GivenName,  mergeGiven );
        mergeMap->insert(AddressBookSyncee::AdditionalName, mergeAdditional );
        mergeMap->insert(AddressBookSyncee::Prefix, mergePrefix );
        mergeMap->insert(AddressBookSyncee::Suffix, mergeSuffix );
        mergeMap->insert(AddressBookSyncee::NickName, mergeNick );
        mergeMap->insert(AddressBookSyncee::Birthday, mergeBirth );
        mergeMap->insert(AddressBookSyncee::HomeAddress, mergeHome );
        mergeMap->insert(AddressBookSyncee::BusinessAddress, mergeBus );
        mergeMap->insert(AddressBookSyncee::TimeZone, mergeTime );
        mergeMap->insert(AddressBookSyncee::Geo, mergeGeo );
        mergeMap->insert(AddressBookSyncee::Title, mergeTitle );
        mergeMap->insert(AddressBookSyncee::Role, mergeRole );
        mergeMap->insert(AddressBookSyncee::Organization, mergeOrg );
        mergeMap->insert(AddressBookSyncee::Note, mergeNote );
        mergeMap->insert(AddressBookSyncee::Url, mergeUrl );
        mergeMap->insert(AddressBookSyncee::Secrecy, mergeSecrecy );
        mergeMap->insert(AddressBookSyncee::Picture, mergePicture );
        mergeMap->insert(AddressBookSyncee::Sound, mergeSound );
        mergeMap->insert(AddressBookSyncee::Agent, mergeAgent );
        mergeMap->insert(AddressBookSyncee::HomeNumbers, mergeHomeTel );
        mergeMap->insert(AddressBookSyncee::OfficeNumbers, mergeOffTel );
        mergeMap->insert(AddressBookSyncee::Messenger, mergeMessenger );
        mergeMap->insert(AddressBookSyncee::PreferredNumber, mergePreferredNumber );
        mergeMap->insert(AddressBookSyncee::Voice, mergeVoice );
        mergeMap->insert(AddressBookSyncee::Fax, mergeFax );
        mergeMap->insert(AddressBookSyncee::Cell, mergeCell );
        mergeMap->insert(AddressBookSyncee::Video, mergeVideo );
        mergeMap->insert(AddressBookSyncee::Mailbox, mergeMailbox );
        mergeMap->insert(AddressBookSyncee::Modem, mergeModem );
        mergeMap->insert(AddressBookSyncee::CarPhone, mergeCarPhone );
        mergeMap->insert(AddressBookSyncee::ISDN, mergeISDN );
        mergeMap->insert(AddressBookSyncee::PCS, mergePCS );
        mergeMap->insert(AddressBookSyncee::Pager, mergePager );
        mergeMap->insert(AddressBookSyncee::HomeFax, mergeHomeFax );
        mergeMap->insert(AddressBookSyncee::WorkFax, mergeWorkFax );
        mergeMap->insert(AddressBookSyncee::OtherTel, mergeOtherTel );
        mergeMap->insert(AddressBookSyncee::Category, mergeCat );
        mergeMap->insert(AddressBookSyncee::Custom, mergeCustom );
        mergeMap->insert(AddressBookSyncee::Keys, mergeKeys );
        mergeMap->insert(AddressBookSyncee::Logo, mergeLogo );
        mergeMap->insert(AddressBookSyncee::Email, mergeEmails );
        /* now fill it with functions.... */
    }
    return mergeMap;
}

bool AddressBookSyncEntry::mergeWith( SyncEntry* ent) 
{
    kdDebug(5228) << "mergeWith was called " << endl;
    if ( ent->name() != name() || !ent->syncee() || !syncee() )
        return false;
    kdDebug(5228) << "Passed the test " << endl;

    AddressBookSyncEntry* entry = static_cast<AddressBookSyncEntry*> (ent);
    QBitArray hier = syncee()->bitArray();
    QBitArray da   = entry->syncee()->bitArray();
    kdDebug(5228) << "Hier count " << hier.size() << " Da count "<< da.size() << endl;
    MergeMap::Iterator it;
    MergeMap* ma = mergeMappi();
    for (uint i = 0; i < da.size() && i < hier.size(); i++ ) {
        /*
         * If da supports [i] and this entry does
         * not -> merge
         */
        if ( da[i] && !hier[i] ) {
            kdDebug(5228) << " da supports and hier not " << i << endl;
            it = ma->find( i );
            if (it!= ma->end() )
                (*it.data())(mAddressee,entry->mAddressee);
        }
    }
    mergeCustom( mAddressee, entry->mAddressee ); // need to call it under all circumstances...

    return true;
}

AddressBookSyncee::AddressBookSyncee()
  : Syncee( AddressBookSyncee::Emails + 1 ) // set the support size
{
  mAddressBook = new KABC::AddressBook;
  mAddressBook->addResource( new KABC::ResourceNull() );
  mOwnAddressBook = true;

  mEntries.setAutoDelete( true );
}

AddressBookSyncee::AddressBookSyncee( KABC::AddressBook *ab )
  : Syncee( AddressBookSyncee::Emails + 1 ) // set the support size
{
  mAddressBook = ab;
  mOwnAddressBook = false;

  mEntries.setAutoDelete( true );
  
  KABC::AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    createEntry( *it );
  }
}

AddressBookSyncee::~AddressBookSyncee()
{
  if ( mOwnAddressBook ) delete mAddressBook;
}

void AddressBookSyncee::reset()
{
  mEntries.clear();
}

QString AddressBookSyncee::identifier()
{
  return mAddressBook->identifier();
}

AddressBookSyncEntry *AddressBookSyncee::firstEntry()
{
  return mEntries.first();
}

AddressBookSyncEntry *AddressBookSyncee::nextEntry()
{
  return mEntries.next();
}

#if 0 // fix me later - zecke
AddressBookSyncEntry *AddressBookSyncee::findEntry(const QString &id)
{
  Event *event = mEntries.find(id);
  return createEntry(event);
}
#endif

void AddressBookSyncee::addEntry( SyncEntry *entry )
{
//  kdDebug() << "AddressBookSyncee::addEntry()" << endl;

  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>( entry );
  abEntry = abEntry->clone();
  if ( !abEntry ) {
    kdDebug(5228) << "AddressBookSyncee::addEntry(): SyncEntry has wrong type."
                  << endl;
  } else {
    abEntry->setSyncee( this ); // set the parent
    if( abEntry->state() == SyncEntry::Undefined ) { // lets find out the state
      if( hasChanged( abEntry ) ) {
        abEntry->setState( SyncEntry::Modified );
      }
    }
    mEntries.append( abEntry );

    KABC::Addressee a = abEntry->addressee();
    a.setResource( 0 );
    mAddressBook->insertAddressee( a );
  }
}

void AddressBookSyncee::removeEntry( SyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if ( !abEntry ) {
    kdDebug(5228) << "AddressBookSyncee::removeEntry(): SyncEntry has wrong type."
                  << endl;
  } else {
    mAddressBook->removeAddressee( abEntry->addressee() );
    mEntries.remove( abEntry );
  }
}

AddressBookSyncEntry *AddressBookSyncee::createEntry( const KABC::Addressee &a )
{
  if ( !a.isEmpty() ) {
    AddressBookSyncEntry *entry = new AddressBookSyncEntry( a );
    entry->setSyncee( this );
    mEntries.append( entry );
    return entry;
  } else {
    return 0;
  }
}

#if 0
/**
 * clone it now - could be inside the Syncee but then we would have to cast
 * -zecke
 *
 */
Syncee* AddressBookSyncee::clone() 
{
    AddressBookSyncEntry* entry;
    SyncEntry* cloneE;
    AddressBookSyncee* clone = new AddressBookSyncee();
    clone->setSyncMode( syncMode() );
    clone->setFirstSync( firstSync() );
    clone->setSupports( bitArray() );
    clone->setSource( source() );
    for ( entry = mEntries.first(); entry != 0; entry = mEntries.next() ) {
        cloneE = entry->clone();
        clone->addEntry( cloneE ); // mSyncee gets updatet
    }
    return clone;
}
#endif

SyncEntry::PtrList AddressBookSyncee::added()
{
    return find( SyncEntry::Added );
}

SyncEntry::PtrList AddressBookSyncee::modified()
{
    return find( SyncEntry::Modified );
}

SyncEntry::PtrList AddressBookSyncee::removed()
{
    return find( SyncEntry::Removed );
}

SyncEntry::PtrList AddressBookSyncee::find( int state )
{
    QPtrList<SyncEntry> found;
    AddressBookSyncEntry* entry;
    for ( entry = mEntries.first(); entry != 0; entry = mEntries.next() ) {
        if ( entry->state() == state )
            found.append( entry );
    }

    return found;
}

QString AddressBookSyncee::type() const
{
    return QString::fromLatin1("AddressBookSyncee");
}

QString AddressBookSyncee::newId() const
{
    return KApplication::randomString( 10 );
}

static QStringList mergeList( const QStringList& entry, const QStringList& other ) {
    QStringList list = entry;

    QStringList::ConstIterator it;
    for (it = other.begin(); it != other.end(); ++it ) {
        if (!list.contains( (*it) ) )
            list << (*it);
    }

    return list;
}
/* merge functions */
static void mergeFamily    ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setFamilyName( other.familyName() );
}
static void mergeGiven     ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setGivenName( other.givenName() );
}
static void mergeAdditional( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setAdditionalName( other.additionalName() );
}
static void mergePrefix    ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setPrefix( other.prefix() );
}
static void mergeSuffix    ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setSuffix( other.suffix() );
}
static void mergeNick      ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setNickName( other.nickName() );
}
static void mergeBirth     ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setBirthday( other.birthday() );
}
static void mergeHome      ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertAddress( other.address( KABC::Address::Home ) );
}
static void mergeBus       ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertAddress( other.address( KABC::Address::Work ) );
}
static void mergeTime      ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setTimeZone( other.timeZone() );
}
static void mergeGeo       ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setGeo( other.geo() );
}
static void mergeTitle     ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setTitle( other.title() );
}
static void mergeRole      ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setRole( other.role() );
}
static void mergeOrg       ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setOrganization( other.organization() );
}
static void mergeNote      ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setNote( other.note() );
}
static void mergeUrl       ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setUrl( other.url() );
}
static void mergeSecrecy   ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setSecrecy( other.secrecy() );
}
static void mergePicture   ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setPhoto( other.photo() );
}
static void mergeSound     ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setSound( other.sound() );
}
static void mergeAgent     ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setAgent( other.agent() );
}
static void mergeHomeTel   ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Home ) );
}
static void mergeOffTel    ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Work ) );
}
static void mergeMessenger ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Msg ) );
}
static void mergePreferredNumber( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Pref ) );
}
static void mergeVoice     ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Voice ) );
}
static void mergeFax       ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Fax ) );
}
static void mergeCell      ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Cell ) );
}
static void mergeVideo     ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Video ) );
}
static void mergeMailbox   ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Bbs ) );
}
static void mergeModem     ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Modem ) );
}
static void mergeCarPhone  ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Car ) );
}
static void mergeISDN      ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Isdn ) );
}
static void mergePCS       ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Pcs ) );
}
static void mergePager     ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Pager ) );
}
static void mergeHomeFax   ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax ) );
}
static void mergeWorkFax   ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertPhoneNumber( other.phoneNumber( KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax ) );
}
static void mergeOtherTel ( KABC::Addressee&, const KABC::Addressee& ) {

}
static void mergeCat       ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setCategories( other.categories() );
}
static void mergeKeys      ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setKeys( other.keys() );
}
static void mergeCustom    ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setCustoms( mergeList( entry.customs(), other.customs() ) );
}
static void mergeLogo      ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.setLogo( other.logo() );
}
static void mergeEmail     ( KABC::Addressee& entry, const KABC::Addressee& other) {
    entry.insertEmail( other.preferredEmail(), true );
}
static void mergeEmails    ( KABC::Addressee& entry, const KABC::Addressee& other) {
    QString pref = entry.preferredEmail();
    entry.setEmails( other.emails() );
    entry.insertEmail( pref, true );
}
