/* abbrowser-conduit.cc                           KPilot
**
** Copyright (C) 2000,2001 by Dan Pilone
** Copyright (C) 2002-2003 by Reinhold Kainhofer
**
** The abbrowser conduit copies addresses from the Pilot's address book to
** the KDE addressbook maintained via the kabc library.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/



#include "options.h"
#include "abbrowser-conduit.moc"

#include <unistd.h>

#include <qtimer.h>
#include <qvbuttongroup.h>
#include <qcheckbox.h>
#include <qtextcodec.h>
#include <time.h>


#include <kglobal.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/resourcefile.h>

#include <pilotUser.h>
#include <pilotSerialDatabase.h>

#include "abbrowser-factory.h"
#include "resolutionDialog.h"
#include "resolutionTable.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
const char *abbrowser_conduit_id="$Id$";

using namespace KABC;

const QString AbbrowserConduit::appString=CSL1("KPILOT");
const QString AbbrowserConduit::flagString=CSL1("Flag");
const QString AbbrowserConduit::idString=CSL1("RecordID");

bool AbbrowserConduit::fPilotStreetHome=true;
bool AbbrowserConduit::fPilotFaxHome=true;
bool AbbrowserConduit::fArchive=true;
enum AbbrowserConduit::ePilotOtherEnum AbbrowserConduit::ePilotOther=AbbrowserConduit::eOtherPhone;
AddressBook*AbbrowserConduit::aBook=0L;
KCrash::HandlerType AbbrowserConduit::oldCleanupOnCrash=0L;

enum AbbrowserConduit::eCustomEnum AbbrowserConduit::eCustom[4] = {
	AbbrowserConduit::eCustomField,
	AbbrowserConduit::eCustomField,
	AbbrowserConduit::eCustomField,
	AbbrowserConduit::eCustomField
	} ;
QString AbbrowserConduit::fCustomFmt=QString::null;

/// This macro just sets the phone number of type "type" to "phone"
/// Use a macro, because that saves two lines for each call, but does not
/// have the overhead of a function call
#define _setPhoneNumber(abEntry, type, nr) \
	{ PhoneNumber phone = abEntry.phoneNumber(type); \
	phone.setNumber(nr); \
	abEntry.insertPhoneNumber(phone); }


void AbbrowserConduit::cleanupOnCrash(int sig)
{
	kdWarning()<<"Crash handler, cleaning up addressbook"<<endl;
	if (aBook) aBook->cleanUp();
	KCrash::defaultCrashHandler(sig);
}


/*********************************************************************
                        C O N S T R U C T O R
 *********************************************************************/





AbbrowserConduit::AbbrowserConduit(KPilotDeviceLink * o, const char *n, const QStringList & a):
		ConduitAction(o, n, a),
		addresseeMap(),
		syncedIds(),
		abiter(),
		ticket(0L)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT<<abbrowser_conduit_id<<endl;
#endif
	fConduitName=i18n("Addressbook");
	// Set crash handler, store old handler
	oldCleanupOnCrash=KCrash::crashHandler();
	KCrash::setCrashHandler(AbbrowserConduit::cleanupOnCrash);
}



AbbrowserConduit::~AbbrowserConduit()
{
	FUNCTIONSETUP;
	if (oldCleanupOnCrash) KCrash::setCrashHandler(oldCleanupOnCrash);
}



/*********************************************************************
                L O A D I N G   T H E   D A T A
 *********************************************************************/



/* Builds the map which links record ids to uid's of Addressee
*/
void AbbrowserConduit::_mapContactsToPilot(QMap < recordid_t, QString > &idContactMap) const
{
	FUNCTIONSETUP;

	idContactMap.clear();

	for(AddressBook::Iterator contactIter = aBook->begin();
		contactIter != aBook->end(); ++contactIter)
	{
		Addressee aContact = *contactIter;
		QString recid = aContact.custom(appString, idString);
		if(!recid.isEmpty())
		{
			recordid_t id = recid.toULong();
			idContactMap.insert(id, aContact.uid());
		}
	}
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Loaded " << idContactMap.size() <<
	    " addresses from the addressbook. " << endl;
#endif
}



bool AbbrowserConduit::_prepare()
{
	FUNCTIONSETUP;

	readConfig();
	syncedIds.clear();

	return true;
}



void AbbrowserConduit::readConfig()
{
	FUNCTIONSETUP;

	KConfigGroupSaver g(fConfig, AbbrowserConduitFactory::group());

	// General page
	fAbookType = (eAbookTypeEnum)fConfig->readNumEntry(
		AbbrowserConduitFactory::fAbookType, 0);
	fAbookFile = fConfig->readEntry(
		AbbrowserConduitFactory::fAbookFile);
	fArchive=fConfig->readBoolEntry(
		AbbrowserConduitFactory::fArchive, true);

	// Conflict page
	SyncAction::eConflictResolution res=(SyncAction::eConflictResolution)fConfig->readNumEntry(
		AbbrowserConduitFactory::fResolution, SyncAction::eUseGlobalSetting);
	if (res!=SyncAction::eUseGlobalSetting) fConflictResolution=res;

	// Fields page
	fPilotStreetHome=!fConfig->readBoolEntry(
		AbbrowserConduitFactory::fStreetType, true);
	fPilotFaxHome=!fConfig->readBoolEntry(
		AbbrowserConduitFactory::fFaxType, true);
	ePilotOther=(ePilotOtherEnum)(fConfig->readNumEntry(
		AbbrowserConduitFactory::fOtherField, eOtherPhone));

	// Custom fields page
	for (int i=0; i<4; i++)
	{
		eCustom[i]=(eCustomEnum)(fConfig->readNumEntry(
			AbbrowserConduitFactory::custom(i), eCustomField) );
	}
	fCustomFmt=fConfig->readEntry(AbbrowserConduitFactory::fCustomFmt, QString::null);

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Settings "
		<< " fConflictResolution=" << fConflictResolution
		<< " fPilotStreetHome=" << fPilotStreetHome
		<< " fPilotFaxHome=" << fPilotFaxHome
		<< " fArchive=" << fArchive
		<< " eCustom[0]=" << eCustom[0]
		<< " eCustom[1]=" << eCustom[1]
		<< " eCustom[2]=" << eCustom[2]
		<< " eCustom[3]=" << eCustom[3]
		<< " fFirstTime=" << isFirstSync()
		<< endl;
#endif
}



bool AbbrowserConduit::isDeleted(const PilotAddress*addr)
{
	if (!addr) return true;
	if (addr->isDeleted() && !addr->isArchived()) return true;
	if (addr->isArchived()) return !fArchive;
	return false;
}
bool AbbrowserConduit::isArchived(const PilotAddress*addr)
{
	if (addr && addr->isArchived()) return fArchive;
	else return false;
}
bool AbbrowserConduit::isArchived(const Addressee &addr)
{
	return addr.custom(appString, flagString) == QString::number(SYNCDEL);
}
bool AbbrowserConduit::makeArchived(Addressee &addr)
{
	FUNCTIONSETUP;
	addr.insertCustom(appString, flagString, QString::number(SYNCDEL));
	addr.removeCustom(appString, idString);
	return true;
}



bool AbbrowserConduit::_loadAddressBook()
{
	FUNCTIONSETUP;
	KConfigGroupSaver g(fConfig, AbbrowserConduitFactory::group());
	switch (fAbookType)
	{
		case eAbookResource:
			DEBUGCONDUIT<<"Loading standard addressbook"<<endl;
			aBook = StdAddressBook::self();
			break;
		case eAbookLocal: { // initialize the abook with the given file
			DEBUGCONDUIT<<"Loading custom addressbook"<<endl;
			aBook = new AddressBook();
			if (!aBook) return false;
			KABC::Resource *res = new ResourceFile( fAbookFile, "vcard" );
			if ( !aBook->addResource( res ) ) {
				DEBUG_KPILOT << "Unable to open resource for file " << fAbookFile << endl;
				KPILOT_DELETE( aBook );
				return false;
			}
			break;}
		default: break;
	}
	// find out if this can fail for reasons other than a non-existent
	// vcf file. If so, how can I determine if the missing file was the problem
	// or something more serious:
	if ( !aBook || !aBook->load() )
	{
		// Something went wrong, so tell the user and return false to exit the conduit
		emit logError(i18n("Unable to initialize and load the addressbook for the sync.") );
		kdWarning()<<k_funcinfo<<": Unable to initialize the addressbook for the sync."<<endl;
		KPILOT_DELETE(aBook);
		return false;
	}
	abChanged = false;
	ticket=aBook->requestSaveTicket();
	if (!ticket)
	{
		kdWarning()<<k_funcinfo<<": Unable to lock addressbook for writing "<<endl;
		KPILOT_DELETE(aBook);
		return false;
	}
	// get the addresseMap which maps Pilot unique record(address) id's to
	// a Abbrowser Addressee; allows for easy lookup and comparisons
	if(aBook->begin() == aBook->end())
	{
		fFirstSync = true;
	}
	else
	{
		_mapContactsToPilot(addresseeMap);
	}
	return(aBook != 0L);
}
bool AbbrowserConduit::_saveAddressBook()
{
	FUNCTIONSETUP;
#ifdef DEBUG
			DEBUGCONDUIT<<"Addressbook not changed, freeing ticket"<<endl;
#endif

	bool res=false;

	if (ticket)
	{
		if (abChanged) {
			res=aBook->save(ticket);
			if (res) { 
				aBook->releaseSaveTicket(ticket);
				ticket=0;
			} else { aBook->cleanUp(); }
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"Addressbook not changed, freeing ticket"<<endl;
#endif
			aBook->cleanUp();
			KPILOT_DELETE(ticket);
		}
	}
	else
	{
		kdWarning()<<k_funcinfo<<": No ticket available to save the "
		<<"addressbook."<<endl;
	}
	if (fAbookType!=eAbookResource)
	{
#ifdef DEBUG
		DEBUGCONDUIT<<"Deleting addressbook"<<endl;
#endif
		KPILOT_DELETE(aBook);
	}

	return res;
}



void AbbrowserConduit::_getAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	unsigned char *buffer = new unsigned char[PilotAddress::APP_BUFFER_SIZE];
	int appLen=fDatabase->readAppBlock(buffer, PilotAddress::APP_BUFFER_SIZE);

	unpack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);
	delete[]buffer;
	buffer = NULL;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " lastUniqueId" << fAddressAppInfo.category.lastUniqueID << endl;
	for(int i = 0; i < 16; i++)
	{
		DEBUGCONDUIT << fname << " cat " << i << " =" << fAddressAppInfo.category.name[i] << endl;
	}

	for(int x = 0; x < 8; x++)
	{
		DEBUGCONDUIT << fname << " phone[" << x << "] = " << fAddressAppInfo.phoneLabels[x] << endl;
	}
#endif
}
void AbbrowserConduit::_setAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	int appLen = pack_AddressAppInfo(&fAddressAppInfo, 0, 0);
	unsigned char *buffer = new unsigned char[appLen];
	pack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);
	if (fDatabase) fDatabase->writeAppBlock(buffer, appLen);
	if (fLocalDatabase) fLocalDatabase->writeAppBlock(buffer, appLen);
	delete[] buffer;
}



QString AbbrowserConduit::getCustomField(const Addressee &abEntry, const int index)
{
	FUNCTIONSETUP;

	switch (eCustom[index]) {
		case eCustomBirthdate: {
			QDateTime bdate(abEntry.birthday().date());
			if (!bdate.isValid()) return abEntry.custom(appString, CSL1("CUSTOM")+QString::number(index));
			QString tmpfmt(KGlobal::locale()->dateFormat());
			if (!fCustomFmt.isEmpty()) KGlobal::locale()->setDateFormat(fCustomFmt);
#ifdef DEBUG
			DEBUGCONDUIT<<"Birthdate: "<<KGlobal::locale()->formatDate(bdate.date())<<" (QDate: "<<bdate.toString()<<endl;
#endif
			QString ret(KGlobal::locale()->formatDate(bdate.date()));
			KGlobal::locale()->setDateFormat(tmpfmt);
			return ret;
		}
		case eCustomURL:
			return abEntry.url().url();
			break;
		case eCustomIM:
			return abEntry.custom(CSL1("KADDRESSBOOK"), CSL1("X-IMAddress"));
			break;
		case eCustomField:
		default:
			return abEntry.custom(appString, CSL1("CUSTOM")+QString::number(index));
			break;
	}
}
void AbbrowserConduit::setCustomField(Addressee &abEntry,  int index, QString cust)
{
	FUNCTIONSETUP;

	switch (eCustom[index]) {
		case eCustomBirthdate: {
			QDate bdate;
			bool ok=false;
			if (!fCustomFmt.isEmpty())
			{
				// empty format means use locale setting
				bdate=KGlobal::locale()->readDate(cust, &ok);
			}
			else
			{
				// use given format
				bdate=KGlobal::locale()->readDate(cust, fCustomFmt, &ok);
			}
#ifdef DEBUG
			DEBUGCONDUIT<<"Birthdate from "<<index<<"-th custom field: "<<bdate.toString()<<endl;
			DEBUGCONDUIT<<"Is Valid: "<<bdate.isValid()<<endl;
#endif
			if (bdate.isValid())
				return abEntry.setBirthday(bdate);
			else
				return abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("X-IMAddress"), cust);
			break; }
		case eCustomURL: {
			return abEntry.setUrl(cust);
			break;}
		case eCustomIM: {
			return abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("X-IMAddress"), cust);
			break;}
		case eCustomField:
		default: {
			return abEntry.insertCustom(appString, CSL1("CUSTOM")+QString::number(index), cust);
			break;}
	}
	return;
}



QString AbbrowserConduit::getOtherField(const Addressee & abEntry)
{
	switch(ePilotOther)
	{
		case eOtherPhone:
			return abEntry.phoneNumber(0).number();
		case eAssistant:
			return abEntry.custom(CSL1("KADDRESSBOOK"), CSL1("AssistantsName"));
		case eBusinessFax:
			return abEntry.phoneNumber(PhoneNumber::Fax | PhoneNumber::Work).number();
		case eCarPhone:
			return abEntry.phoneNumber(PhoneNumber::Car).number();
		case eEmail2:
			return abEntry.emails().first();
		case eHomeFax:
			return abEntry.phoneNumber(PhoneNumber::Fax | PhoneNumber::Home).number();
		case eTelex:
			return abEntry.phoneNumber(PhoneNumber::Bbs).number();
		case eTTYTTDPhone:
			return abEntry.phoneNumber(PhoneNumber::Pcs).number();
		default:
			return QString::null;
	}
}
void AbbrowserConduit::setOtherField(Addressee & abEntry, QString nr)
{
//	PhoneNumber phone;
	switch(ePilotOther)
	{
		case eOtherPhone:
			_setPhoneNumber(abEntry, 0, nr)
			break;
		case eAssistant:
			abEntry.insertCustom(CSL1("KADDRESSBOOK"), CSL1("AssistantsName"), nr);
			break;
		case eBusinessFax:
			_setPhoneNumber(abEntry, PhoneNumber::Fax | PhoneNumber::Work, nr)
			break;
		case eCarPhone:
			_setPhoneNumber(abEntry, PhoneNumber::Car, nr)
			break;
		case eEmail2:
			return abEntry.insertEmail(nr);
		case eHomeFax:
			_setPhoneNumber(abEntry, PhoneNumber::Fax|PhoneNumber::Home, nr)
			break;
		case eTelex:
			_setPhoneNumber(abEntry, PhoneNumber::Bbs, nr)
			break;
		case eTTYTTDPhone:
			_setPhoneNumber(abEntry, PhoneNumber::Pcs, nr)
			break;
	}
}



PhoneNumber AbbrowserConduit::getFax(const Addressee & abEntry)
{
	return abEntry.phoneNumber(PhoneNumber::Fax |
		( (fPilotFaxHome) ?(PhoneNumber::Home) :(PhoneNumber::Work)));
}
void AbbrowserConduit::setFax(Addressee & abEntry, QString fax)
{
	_setPhoneNumber(abEntry, PhoneNumber::Fax | (fPilotFaxHome ? PhoneNumber::Home : PhoneNumber::Work ), fax);
}


/** First search for a preferred  address. If we don't have one, search
 *  for home or work as specified in the config dialog. If we don't have
 *  such one, either, search for the other type. If we still have no luck,
 *  return an address with preferred + home/work flag (from config dlg). */
KABC::Address AbbrowserConduit::getAddress(const Addressee & abEntry)
{
	int type=(fPilotStreetHome)?(KABC::Address::Home):(KABC::Address::Work);
	KABC::Address ad(abEntry.address(KABC::Address::Pref));
	if (!ad.isEmpty()) return ad;
	ad=abEntry.address(type);
	if (!ad.isEmpty()) return ad;
	ad=abEntry.address((fPilotStreetHome) ?(KABC::Address::Work):(KABC::Address::Home));
	if (!ad.isEmpty()) return ad;

	return abEntry.address(type | KABC::Address::Pref);
}



/**
 * _getCat returns the id of the category from the given categories list.
 * If the address has no categories on the PC, QString::null is returned.
 * If the current category exists in the list of cats, it is returned
 * Otherwise the first cat in the list that exists on the HH is returned
 * If none of the categories exists on the palm, QString::null is returned
 */
QString AbbrowserConduit::_getCatForHH(const QStringList cats, const QString curr) const
{
	FUNCTIONSETUP;
	int j;
	if (cats.size()<1) return QString::null;
	if (cats.contains(curr)) return curr;
	for(QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it)
	{
		for(j = 0; j <= 15; j++)
		{
			QString catName = PilotAppCategory::codec()->
				toUnicode(fAddressAppInfo.category.name[j]);
			if(!(*it).isEmpty() && !_compare(*it, catName))
			{
				return catName;
			}
		}
	}
	// If we have a free label, return the first possible cat
	QString lastCat(fAddressAppInfo.category.name[15]);
	if (lastCat.isEmpty()) return cats.first();
	return QString::null;
}
void AbbrowserConduit::_setCategory(Addressee & abEntry, QString cat)
{
	if ( (!cat.isEmpty()))
	// &&  (cat!=QString(fAddressAppInfo.category.name[0])) )
		abEntry.insertCategory(cat);
}



/*********************************************************************
                     D E B U G   O U T P U T
 *********************************************************************/



#ifdef DEBUG
void AbbrowserConduit::showAddressee(const Addressee & abAddress)
{
	FUNCTIONSETUP;
	DEBUGCONDUIT << "\tAbbrowser Contact Entry" << endl;
	if (abAddress.isEmpty()) {
		DEBUGCONDUIT<< "\t\tEMPTY"<<endl;
		return;
	}
	DEBUGCONDUIT << "\t\tLast name = " << abAddress.familyName() << endl;
	DEBUGCONDUIT << "\t\tFirst name = " << abAddress.givenName() << endl;
	DEBUGCONDUIT << "\t\tCompany = " << abAddress.organization() << endl;
	DEBUGCONDUIT << "\t\tJob Title = " << abAddress.title() << endl;
	DEBUGCONDUIT << "\t\tNote = " << abAddress.note() << endl;
	DEBUGCONDUIT << "\t\tHome phone = " << abAddress.phoneNumber(PhoneNumber::Home).number() << endl;
	DEBUGCONDUIT << "\t\tWork phone = " << abAddress.phoneNumber(PhoneNumber::Work).number() << endl;
	DEBUGCONDUIT << "\t\tMobile phone = " << abAddress.phoneNumber(PhoneNumber::Cell).number() << endl;
	DEBUGCONDUIT << "\t\tEmail = " << abAddress.preferredEmail() << endl;
	DEBUGCONDUIT << "\t\tFax = " << getFax(abAddress).number() << endl;
	DEBUGCONDUIT << "\t\tPager = " << abAddress.phoneNumber(PhoneNumber::Pager).number() << endl;
	DEBUGCONDUIT << "\t\tCategory = " << abAddress.categories().first() << endl;
}



void AbbrowserConduit::showPilotAddress(PilotAddress *pilotAddress)
{
	FUNCTIONSETUP;
	DEBUGCONDUIT << "\tPilot Address" << endl;
	if (!pilotAddress) {
		DEBUGCONDUIT<< "\t\tEMPTY"<<endl;
		return;
	}
	DEBUGCONDUIT << "\t\tLast name = " << pilotAddress->getField(entryLastname) << endl;
	DEBUGCONDUIT << "\t\tFirst name = " << pilotAddress->getField(entryFirstname) << endl;
	DEBUGCONDUIT << "\t\tCompany = " << pilotAddress->getField(entryCompany) << endl;
	DEBUGCONDUIT << "\t\tJob Title = " << pilotAddress->getField(entryTitle) << endl;
	DEBUGCONDUIT << "\t\tNote = " << pilotAddress->getField(entryNote) << endl;
	DEBUGCONDUIT << "\t\tHome phone = " << pilotAddress->getPhoneField(PilotAddress::eHome, false) << endl;
	DEBUGCONDUIT << "\t\tWork phone = " << pilotAddress->getPhoneField(PilotAddress::eWork, false) << endl;
	DEBUGCONDUIT << "\t\tMobile phone = " << pilotAddress->getPhoneField(PilotAddress::eMobile, false) << endl;
	DEBUGCONDUIT << "\t\tEmail = " << pilotAddress->getPhoneField(PilotAddress::eEmail, false) << endl;
	DEBUGCONDUIT << "\t\tFax = " << pilotAddress->getPhoneField(PilotAddress::eFax, false) << endl;
	DEBUGCONDUIT << "\t\tPager = " << pilotAddress->getPhoneField(PilotAddress::ePager, false) << endl;
	DEBUGCONDUIT << "\t\tOther = " << pilotAddress->getPhoneField(PilotAddress::eOther, false) << endl;
	DEBUGCONDUIT << "\t\tCategory = " << pilotAddress->getCategoryLabel() << endl;
}
#endif


void AbbrowserConduit::showAdresses(Addressee &pcAddr, PilotAddress *backupAddr,
	PilotAddress *palmAddr)
{
#ifdef DEBUG
	DEBUGCONDUIT << "abEntry:" << endl;
	showAddressee(pcAddr);
	DEBUGCONDUIT << "pilotAddress:" << endl;
	showPilotAddress(palmAddr);
	DEBUGCONDUIT << "backupAddress:" << endl;
	showPilotAddress(backupAddr);
	DEBUGCONDUIT << "------------------------------------------------" << endl;
#endif
}



/*********************************************************************
                S Y N C   S T R U C T U R E
 *********************************************************************/



/* virtual */ bool AbbrowserConduit::exec()
{
	FUNCTIONSETUP;
	DEBUGCONDUIT<<abbrowser_conduit_id<<endl;

	if(!fConfig)
	{
		kdWarning() << k_funcinfo << ": No config file was set!" << endl;
		emit logError(i18n("Unable to load configuration of the addressbook conduit."));
		return false;
	}

	_prepare();

	fFirstSync = false;
	// Database names probably in latin1.
	if(!openDatabases(QString::fromLatin1("AddressDB"), &fFirstSync))
	{
		emit logError(i18n("Unable to open the addressbook databases on the handheld."));
		return false;
	}
	_getAppInfo();
	if(!_loadAddressBook())
	{
		emit logError(i18n("Unable to open the addressbook."));
		return false;
	}
	fFirstSync = fFirstSync || (aBook->begin() == aBook->end());

	// perform syncing from palm to abbrowser
	// iterate through all records in palm pilot
	pilotindex = 0;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": fullsync=" << isFullSync() << ", firstSync=" <<    isFirstSync() << endl;
	DEBUGCONDUIT << fname << ": "
		<< "syncDirection=" << fSyncDirection << ", "
		<< "archive = " << fArchive << endl;
	DEBUGCONDUIT << fname << ": conflictRes="<< fConflictResolution << endl;
	DEBUGCONDUIT << fname << ": PilotStreetHome=" << fPilotStreetHome << ", PilotFaxHOme" << fPilotFaxHome << endl;
#endif

	if (!isFirstSync())
		allIds=fDatabase->idList();

	/* Note:
	   if eCopyPCToHH or eCopyHHToPC, first sync everything, then lookup
	   those entries on the receiving side that are not yet syncced and delete
	   them. Use slotDeleteUnsyncedPCRecords and slotDeleteUnsyncedHHRecords
	   for this, and no longer purge the whole addressbook before the sync to
	   prevent data loss in case of connection loss. */

	QTimer::singleShot(0, this, SLOT(slotPalmRecToPC()));

	return true;
}



void AbbrowserConduit::slotPalmRecToPC()
{
	FUNCTIONSETUP;
	PilotRecord *palmRec = 0L, *backupRec = 0L;

	if (fSyncDirection==SyncAction::eCopyPCToHH)
	{
		abiter = aBook->begin();
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}

	if(isFullSync())
		palmRec = fDatabase->readRecordByIndex(pilotindex++);
	else
		palmRec = dynamic_cast <PilotSerialDatabase * >(fDatabase)->readNextModifiedRec();

	if(!palmRec)
	{
		abiter = aBook->begin();
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}

	// already synced, so skip:
	if(syncedIds.contains(palmRec->getID()))
	{
		KPILOT_DELETE(palmRec);
		QTimer::singleShot(0, this, SLOT(slotPalmRecToPC()));
		return;
	}

	backupRec = fLocalDatabase->readRecordById(palmRec->getID());
	PilotRecord*compareRec=(backupRec)?(backupRec):(palmRec);
	Addressee e = _findMatch(PilotAddress(fAddressAppInfo, compareRec));

	PilotAddress*backupAddr=0L;
	if (backupRec) backupAddr=new PilotAddress(fAddressAppInfo, backupRec);
	PilotAddress*palmAddr=0L;
	if (palmRec) palmAddr=new PilotAddress(fAddressAppInfo, palmRec);

	syncAddressee(e, backupAddr, palmAddr);

	syncedIds.append(palmRec->getID());
	KPILOT_DELETE(palmAddr);
	KPILOT_DELETE(backupAddr);
	KPILOT_DELETE(palmRec);
	KPILOT_DELETE(backupRec);

	QTimer::singleShot(0, this, SLOT(slotPalmRecToPC()));
}



void AbbrowserConduit::slotPCRecToPalm()
{
	FUNCTIONSETUP;

	if ( (fSyncDirection==SyncAction::eCopyHHToPC) ||
		abiter == aBook->end() || (*abiter).isEmpty() )
	{
		pilotindex = 0;
		QTimer::singleShot(0, this, SLOT(slotDeletedRecord()));
		return;
	}

	PilotRecord *palmRec=0L, *backupRec=0L;
	Addressee ad = *abiter;

	abiter++;

	// If marked as archived, don't sync!
	if (isArchived(ad))
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": address with id " << ad.uid() <<
			" marked archived, so don't sync." << endl;
#endif
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}


	QString recID(ad.custom(appString, idString));
	bool ok;
	recordid_t rid = recID.toLong(&ok);
	if (recID.isEmpty() || !ok || !rid)
	{
		// it's a new item(no record ID and not inserted by the Palm -> PC sync), so add it
		syncAddressee(ad, 0L, 0L);
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}

	// look into the list of already synced record ids to see if the addressee hasn't already been synced
	if (syncedIds.contains(rid))
	{
#ifdef DEBUG
		DEBUGCONDUIT << ": address with id " << rid << " already synced." << endl;
#endif
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}


	backupRec = fLocalDatabase->readRecordById(rid);
	// only update if no backup record or the backup record is not equal to the addressee

	PilotAddress*backupAddr=0L;
	if (backupRec) backupAddr=new PilotAddress(fAddressAppInfo, backupRec);
	if(!backupRec || isFirstSync() || !_equal(backupAddr, ad)  )
	{
		palmRec = fDatabase->readRecordById(rid);
		PilotAddress*palmAddr=0L;
		if (palmRec) palmAddr= new PilotAddress(fAddressAppInfo, palmRec);
		syncAddressee(ad, backupAddr, palmAddr);
		// update the id just in case it changed
		if (palmRec) rid=palmRec->getID();
		KPILOT_DELETE(palmRec);
		KPILOT_DELETE(palmAddr);
	}
	KPILOT_DELETE(backupAddr);
	KPILOT_DELETE(backupRec);
	syncedIds.append(rid);
	// done with the sync process, go on with the next one:
	QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
}



void AbbrowserConduit::slotDeletedRecord()
{
	FUNCTIONSETUP;

	PilotRecord *backupRec = fLocalDatabase->readRecordByIndex(pilotindex++);
	if(!backupRec || isFirstSync() )
	{
		KPILOT_DELETE(backupRec);
		QTimer::singleShot(0, this, SLOT(slotDeleteUnsyncedPCRecords()));
		return;
	}

	// already synced, so skip this record:
	if(syncedIds.contains(backupRec->getID()))
	{
		KPILOT_DELETE(backupRec);
		QTimer::singleShot(0, this, SLOT(slotDeletedRecord()));
		return;
	}

	QString uid = addresseeMap[backupRec->getID()];
	Addressee e = aBook->findByUid(uid);
	PilotRecord*palmRec=fDatabase->readRecordById(backupRec->getID());
	PilotAddress*backupAddr=0L;
	if (backupRec) backupAddr=new PilotAddress(fAddressAppInfo, backupRec);
	PilotAddress*palmAddr=0L;
	if (palmRec) palmAddr=new PilotAddress(fAddressAppInfo, palmRec);

	syncedIds.append(backupRec->getID());
	syncAddressee(e, backupAddr, palmAddr);

	KPILOT_DELETE(palmAddr);
	KPILOT_DELETE(backupAddr);
	KPILOT_DELETE(palmRec);
	KPILOT_DELETE(backupRec);
	QTimer::singleShot(0, this, SLOT(slotDeletedRecord()));
}



void AbbrowserConduit::slotDeleteUnsyncedPCRecords()
{
	FUNCTIONSETUP;
	if (fSyncDirection==SyncAction::eCopyHHToPC)
	{
		QStringList uids;
		RecordIDList::iterator it;
		QString uid;
		for ( it = syncedIds.begin(); it != syncedIds.end(); ++it)
		{
			uid=addresseeMap[*it];
			if (!uid.isEmpty()) uids.append(uid);
		}
		// TODO: Does this speed up anything?
		// qHeapSort( uids );
		AddressBook::Iterator abit;
		for (abit = aBook->begin(); abit != aBook->end(); ++abit)
		{
			if (!uids.contains((*abit).uid()))
			{
#ifdef DEBUG
				DEBUGCONDUIT<<"Deleting addressee "<<(*abit).realName()<<" from PC (is not on HH, and syncing with HH->PC direction)"<<endl;
#endif
				abChanged = true;
				// TODO: Can I really remove the current iterator???
				aBook->removeAddressee(*abit);
			}
		}
	}
	QTimer::singleShot(0, this, SLOT(slotDeleteUnsyncedHHRecords()));
}



void AbbrowserConduit::slotDeleteUnsyncedHHRecords()
{
	FUNCTIONSETUP;
	if (fSyncDirection==SyncAction::eCopyPCToHH)
	{
		RecordIDList ids=fDatabase->idList();
		RecordIDList::iterator it;
		for ( it = ids.begin(); it != ids.end(); ++it )
		{
			if (!syncedIds.contains(*it))
			{
#ifdef DEBUG
				DEBUGCONDUIT<<"Deleting record with ID "<<*it<<" from handheld (is not on PC, and syncing with PC->HH direction)"<<endl;
#endif
				fDatabase->deleteRecord(*it);
				fLocalDatabase->deleteRecord(*it);
			}
		}
	}
	QTimer::singleShot(0, this, SLOT(slotCleanup()));
}


void AbbrowserConduit::slotCleanup()
{
	FUNCTIONSETUP;

	// Set the appInfoBlock, just in case the category labels changed
	_setAppInfo();
	if(fDatabase)
	{
		fDatabase->resetSyncFlags();
		fDatabase->cleanup();
	}
	if(fLocalDatabase)
	{
		fLocalDatabase->resetSyncFlags();
		fLocalDatabase->cleanup();
	}
	KPILOT_DELETE(fDatabase);
	KPILOT_DELETE(fLocalDatabase);
	_saveAddressBook();
	emit syncDone(this);
}



/*********************************************************************
              G E N E R A L   S Y N C   F U N C T I O N
         These functions modify the Handheld and the addressbook
 *********************************************************************/



bool AbbrowserConduit::syncAddressee(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr)
{
	FUNCTIONSETUP;
showAdresses(pcAddr, backupAddr, palmAddr);

	if (fSyncDirection==SyncAction::eCopyPCToHH)
	{
		if (pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"0a "<<endl;
#endif
			return _deleteAddressee(pcAddr, backupAddr, palmAddr);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"0b "<<endl;
#endif
			return _copyToHH(pcAddr, backupAddr, palmAddr);
		}
	}

	if (fSyncDirection==SyncAction::eCopyHHToPC)
	{
#ifdef DEBUG
			DEBUGCONDUIT<<"0c "<<endl;
#endif
		if (!palmAddr)
			return _deleteAddressee(pcAddr, backupAddr, palmAddr);
		else
			return _copyToPC(pcAddr, backupAddr, palmAddr);
	}

	if ( !backupAddr || isFirstSync() )
	{
#ifdef DEBUG
			DEBUGCONDUIT<<"1"<<endl;
#endif
		/*
		Resolution matrix (0..does not exist, E..exists, D..deleted flag set, A..archived):
		  HH    PC  | Resolution
		  ------------------------------------------------------------
		   0     A  |  -
		   0     E  |  PC -> HH, reset ID if not set correctly
		   D     0  |  delete (error, should never occur!!!)
		   D     E  |  CR (ERROR)
		   E/A   0  |  HH -> PC
		   E/A   E/A|  merge/CR
		 */
		if  (!palmAddr && isArchived(pcAddr) )
		{
			return true;
		}
		else if (!palmAddr && !pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1a"<<endl;
#endif
			// PC->HH
			bool res=_copyToHH(pcAddr, 0L, 0L);
			return res;
		}
		else if (!palmAddr && pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1b"<<endl;
#endif
			// everything's empty -> ERROR
			return false;
		}
		else if ( (isDeleted(palmAddr) || isArchived(palmAddr)) && pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1c"<<endl;
#endif
			if (isArchived(palmAddr))
				return _copyToPC(pcAddr, 0L, palmAddr);
			else
				// this happens if you add a record on the handheld and delete it again before you do the next sync
				return _deleteAddressee(pcAddr, 0L, palmAddr);
		}
		else if ((isDeleted(palmAddr)||isArchived(palmAddr)) && !pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1d"<<endl;
#endif
			// CR (ERROR)
			return _smartMergeAddressee(pcAddr, 0L, palmAddr);
		}
		else if (pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1e"<<endl;
#endif
			// HH->PC
			return _copyToPC(pcAddr, 0L, palmAddr);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"1f"<<endl;
#endif
			// Conflict Resolution
			return _smartMergeAddressee(pcAddr, 0L, palmAddr);
		}
	} // !backupAddr
	else
	{
#ifdef DEBUG
			DEBUGCONDUIT<<"2"<<endl;
#endif
		/*
		Resolution matrix:
		  1) if HH.(empty| (deleted &! archived) ) -> { if (PC==B) -> delete, else -> CR }
		     if HH.archied -> {if (PC==B) -> copyToPC, else -> CR }
		     if PC.empty -> { if (HH==B) -> delete, else -> CR }
		     if PC.archived -> {if (HH==B) -> delete on HH, else CR }
		  2) if PC==HH -> { update B, update ID of PC if needed }
		  3) if PC==B -> { HH!=PC, thus HH modified, so copy HH->PC }
		     if HH==B -> { PC!=HH, thus PC modified, so copy PC->HH }
		  4) else: all three addressees are different -> CR
		*/

		if (!palmAddr || isDeleted(palmAddr) )
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"2a"<<endl;
#endif
			if (_equal(backupAddr, pcAddr) || pcAddr.isEmpty())
			{
				return _deleteAddressee(pcAddr, backupAddr, 0L);
			}
			else
			{
				return _smartMergeAddressee(pcAddr, backupAddr, 0L);
			}
		}
		else if (pcAddr.isEmpty())
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"2b"<<endl;
#endif
			if (*palmAddr == *backupAddr)
			{
				return _deleteAddressee(pcAddr, backupAddr, palmAddr);
			}
			else
			{
				return _smartMergeAddressee(pcAddr, backupAddr, palmAddr);
			}
		}
		else if (_equal(palmAddr, pcAddr))
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"2c"<<endl;
#endif
			// update Backup, update ID of PC if neededd
			return _writeBackup(palmAddr);
		}
		else if (_equal(backupAddr, pcAddr))
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"2d"<<endl;
			DEBUGCONDUIT<<"Flags: "<<palmAddr->getAttrib()<<", isDeleted="<<
				isDeleted(palmAddr)<<", isArchived="<<isArchived(palmAddr)<<endl;
#endif
			if (isDeleted(palmAddr))
				return _deleteAddressee(pcAddr, backupAddr, palmAddr);
			else
				return _copyToPC(pcAddr, backupAddr, palmAddr);
		}
		else if (*palmAddr == *backupAddr)
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"2e"<<endl;
#endif
			return _copyToHH(pcAddr, backupAddr, palmAddr);
		}
		else
		{
#ifdef DEBUG
			DEBUGCONDUIT<<"2f"<<endl;
#endif
			// CR, since all are different
			return _smartMergeAddressee(pcAddr, backupAddr, palmAddr);
		}
	} // backupAddr
	return false;
}



bool AbbrowserConduit::_copyToHH(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr)
{
	FUNCTIONSETUP;

	if (pcAddr.isEmpty()) return false;
	PilotAddress*paddr=palmAddr;
	bool paddrcreated=false;
	if (!paddr)
	{
		paddr=new PilotAddress(fAddressAppInfo);
		paddrcreated=true;
	}
	_copy(paddr, pcAddr);
#ifdef DEBUG
	DEBUGCONDUIT<<"palmAddr->id="<<paddr->getID()<<", pcAddr.ID="<<
		pcAddr.custom(appString, idString)<<endl;
#endif

	if(_savePalmAddr(paddr, pcAddr))
	{
#ifdef DEBUG
		DEBUGCONDUIT<<"Vor _saveAbEntry, palmAddr->id="<<
		paddr->getID()<<", pcAddr.ID="<<pcAddr.custom(appString, idString)<<endl;
#endif
		_savePCAddr(pcAddr, backupAddr, paddr);
	}
	if (paddrcreated) KPILOT_DELETE(paddr);
	return true;
}



bool AbbrowserConduit::_copyToPC(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr)
{
	FUNCTIONSETUP;
	if (!palmAddr)
	{
		return false;
	}
#ifdef DEBUG
	showPilotAddress(palmAddr);
#endif
	_copy(pcAddr, palmAddr);
	_savePCAddr(pcAddr, backupAddr, palmAddr);
	_writeBackup(palmAddr);
	return true;
}



bool AbbrowserConduit::_writeBackup(PilotAddress *backup)
{
	FUNCTIONSETUP;
	if (!backup) return false;


#ifdef DEBUG
	showPilotAddress(backup);
#endif
	PilotRecord *pilotRec = backup->pack();
	fLocalDatabase->writeRecord(pilotRec);
	KPILOT_DELETE(pilotRec);
	return true;
}



bool AbbrowserConduit::_deleteAddressee(Addressee &pcAddr, PilotAddress*backupAddr,
		PilotAddress*palmAddr)
{
	FUNCTIONSETUP;

	if (palmAddr)
	{
		if (!syncedIds.contains(palmAddr->getID())) syncedIds.append(palmAddr->getID());
		palmAddr->makeDeleted();
		PilotRecord *pilotRec = palmAddr->pack();
		pilotRec->makeDeleted();
		pilotindex--;
		fDatabase->writeRecord(pilotRec);
		fLocalDatabase->writeRecord(pilotRec);
		syncedIds.append(pilotRec->getID());
		KPILOT_DELETE(pilotRec);
	}
	else if (backupAddr)
	{
		if (!syncedIds.contains(backupAddr->getID())) syncedIds.append(backupAddr->getID());
		backupAddr->makeDeleted();
		PilotRecord *pilotRec = backupAddr->pack();
		pilotRec->makeDeleted();
		pilotindex--;
		fLocalDatabase->writeRecord(pilotRec);
		syncedIds.append(pilotRec->getID());
		KPILOT_DELETE(pilotRec);
	}
	if (!pcAddr.isEmpty())
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " removing " << pcAddr.formattedName() << endl;
#endif
		abChanged = true;
		aBook->removeAddressee(pcAddr);
	}
	return true;
}



/*********************************************************************
                 l o w - l e v e l   f u n c t i o n s   f o r
                   adding / removing palm/pc records
 *********************************************************************/



bool AbbrowserConduit::_savePalmAddr(PilotAddress *palmAddr, Addressee &pcAddr)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << "Saving to pilot " << palmAddr->id()
		<< " " << palmAddr->getField(entryFirstname)
		<< " " << palmAddr->getField(entryLastname)<< endl;
#endif

	PilotRecord *pilotRec = palmAddr->pack();
	recordid_t pilotId = fDatabase->writeRecord(pilotRec);
#ifdef DEBUG
	DEBUGCONDUIT<<"PilotRec nach writeRecord ("<<pilotId<<": ID="<<pilotRec->getID()<<endl;
#endif
	fLocalDatabase->writeRecord(pilotRec);
	KPILOT_DELETE(pilotRec);

	// pilotId == 0 if using local db, so don't overwrite the valid id
	if(pilotId != 0)
	{
		palmAddr->setID(pilotId);
		if (!syncedIds.contains(pilotId)) syncedIds.append(pilotId);
	}

	recordid_t abId = 0;
	abId = pcAddr.custom(appString, idString).toUInt();
	if(abId != pilotId)
	{
		pcAddr.insertCustom(appString, idString, QString::number(pilotId));
		return true;
	}

	return false;
}



bool AbbrowserConduit::_savePCAddr(Addressee &pcAddr, PilotAddress*,
	PilotAddress*)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT<<"Before _savePCAddr, pcAddr.custom="<<pcAddr.custom(appString, idString)<<endl;
#endif
	if(!pcAddr.custom(appString, idString).isEmpty())
	{
		addresseeMap.insert(pcAddr.custom(appString, idString).toLong(), pcAddr.uid());
	}

	aBook->insertAddressee(pcAddr);

	abChanged = true;
	return true;
}



/*********************************************************************
                   C O P Y   R E C O R D S
 *********************************************************************/


int AbbrowserConduit::_compare(const QString & str1, const QString & str2) const
{
//	FUNCTIONSETUP;
	if(str1.isEmpty() && str2.isEmpty()) return 0;
	else return str1.compare(str2);
}


bool AbbrowserConduit::_equal(const PilotAddress *piAddress, const Addressee &abEntry,
	enum eqFlagsType flags) const
{
	FUNCTIONSETUP;
	// empty records are never equal!
	if (!piAddress) return false;
	if (abEntry.isEmpty()) return false;
	//  Archived records match anything so they won't be copied to the HH again
	if (flags & eqFlagsFlags)
		if (isArchived(piAddress) && isArchived(abEntry) ) return true;

	if (flags & eqFlagsName)
	{
		if(_compare(abEntry.familyName(), piAddress->getField(entryLastname)))
			return false;
		if(_compare(abEntry.givenName(), piAddress->getField(entryFirstname)))
			return false;
		if(_compare(abEntry.title(), piAddress->getField(entryTitle)))
			return false;
		if(_compare(abEntry.organization(), piAddress->getField(entryCompany)))
			return false;
	}
	if (flags & eqFlagsNote)
		if(_compare(abEntry.note(), piAddress->getField(entryNote)))
			return false;

	if (flags & eqFlagsNote)
	{
		QString cat = _getCatForHH(abEntry.categories(), piAddress->getCategoryLabel());
		if(_compare(cat, piAddress->getCategoryLabel())) return false;
	}

	if (flags & eqFlagsPhones)
	{
		if(_compare(abEntry.phoneNumber(PhoneNumber::Work).number(),
			piAddress->getPhoneField(PilotAddress::eWork, false))) return false;
		if(_compare(abEntry.phoneNumber(PhoneNumber::Home).number(),
			piAddress->getPhoneField(PilotAddress::eHome, false))) return false;
		if(_compare(getOtherField(abEntry),
			piAddress->getPhoneField(PilotAddress::eOther, false))) return false;
		if(_compare(abEntry.preferredEmail(),
			piAddress->getPhoneField(PilotAddress::eEmail, false))) return false;
		if(_compare(getFax(abEntry).number(),
			piAddress->getPhoneField(PilotAddress::eFax, false))) return false;
		if(_compare(abEntry.phoneNumber(PhoneNumber::Cell).number(),
			piAddress->getPhoneField(PilotAddress::eMobile, false))) return false;
	}

	if (flags & eqFlagsAdress)
	{
		KABC::Address address = getAddress(abEntry);
		if(_compare(address.street(), piAddress->getField(entryAddress)))
			return false;
		if(_compare(address.locality(), piAddress->getField(entryCity)))
			return false;
		if(_compare(address.region(), piAddress->getField(entryState)))
			return false;
		if(_compare(address.postalCode(), piAddress->getField(entryZip)))
			return false;
		if(_compare(address.country(), piAddress->getField(entryCountry)))
			return false;
	}

	if (flags & eqFlagsCustom)
	{
		if(_compare(getCustomField(abEntry, 0),
			piAddress->getField(entryCustom1))) return false;
		if(_compare(getCustomField(abEntry, 1),
			piAddress->getField(entryCustom2))) return false;
		if(_compare(getCustomField(abEntry, 2),
			piAddress->getField(entryCustom3))) return false;
		if(_compare(getCustomField(abEntry, 3),
			piAddress->getField(entryCustom4))) return false;
	}

	// if any side is marked archived, but the other is not, the two
	// are not equal.
	if (flags & eqFlagsFlags)
		if (isArchived(piAddress) || isArchived(abEntry) ) return false;

	return true;
}



void AbbrowserConduit::_copy(PilotAddress *toPilotAddr, Addressee &fromAbEntry)
{
	FUNCTIONSETUP;
	if (!toPilotAddr) return;

	toPilotAddr->setAttrib(toPilotAddr->getAttrib() & ~(dlpRecAttrDeleted));

	// don't do a reset since this could wipe out non copied info
	//toPilotAddr->reset();
	toPilotAddr->setField(entryLastname, fromAbEntry.familyName());
	QString firstAndMiddle = fromAbEntry.givenName();
	if(!fromAbEntry.additionalName().isEmpty()) firstAndMiddle += CSL1(" ") + fromAbEntry.additionalName();
	toPilotAddr->setField(entryFirstname, firstAndMiddle);
	toPilotAddr->setField(entryCompany, fromAbEntry.organization());
	toPilotAddr->setField(entryTitle, fromAbEntry.title());
	toPilotAddr->setField(entryNote, fromAbEntry.note());

	// do email first, to ensure its gets stored
	toPilotAddr->setPhoneField(PilotAddress::eEmail, fromAbEntry.preferredEmail(), false);
	toPilotAddr->setPhoneField(PilotAddress::eWork,
		fromAbEntry.phoneNumber(PhoneNumber::Work).number(), false);
	toPilotAddr->setPhoneField(PilotAddress::eHome,
		fromAbEntry.phoneNumber(PhoneNumber::Home).number(), false);
	toPilotAddr->setPhoneField(PilotAddress::eMobile,
		fromAbEntry.phoneNumber(PhoneNumber::Cell).number(), false);
	toPilotAddr->setPhoneField(PilotAddress::eFax, getFax(fromAbEntry).number(), false);
	toPilotAddr->setPhoneField(PilotAddress::ePager,
		fromAbEntry.phoneNumber(PhoneNumber::Pager).number(), false);
	toPilotAddr->setPhoneField(PilotAddress::eOther, getOtherField(fromAbEntry), false);
	toPilotAddr->setShownPhone(PilotAddress::eMobile);

	KABC::Address homeAddress = getAddress(fromAbEntry);
	_setPilotAddress(toPilotAddr, homeAddress);

	// Process the additional entries from the Palm(the palm database app block tells us the name of the fields)
	toPilotAddr->setField(entryCustom1, getCustomField(fromAbEntry, 0));
	toPilotAddr->setField(entryCustom2, getCustomField(fromAbEntry, 1));
	toPilotAddr->setField(entryCustom3, getCustomField(fromAbEntry, 2));
	toPilotAddr->setField(entryCustom4, getCustomField(fromAbEntry, 3));

	toPilotAddr->setCategory(_getCatForHH(fromAbEntry.categories(), toPilotAddr->getCategoryLabel()));

	if (isArchived(fromAbEntry))
		toPilotAddr->makeArchived();
	else
		toPilotAddr->setAttrib(toPilotAddr->getAttrib() & ~(dlpRecAttrArchived));
}



void AbbrowserConduit::_setPilotAddress(PilotAddress *toPilotAddr, const KABC::Address & abAddress)
{
	toPilotAddr->setField(entryAddress, abAddress.street());
	toPilotAddr->setField(entryCity, abAddress.locality());
	toPilotAddr->setField(entryState, abAddress.region());
	toPilotAddr->setField(entryZip, abAddress.postalCode());
	toPilotAddr->setField(entryCountry, abAddress.country());
}



void AbbrowserConduit::_copyPhone(Addressee &toAbEntry,
			      PhoneNumber phone, QString palmphone)
{
	if(!palmphone.isEmpty())
	{
		phone.setNumber(palmphone);
		toAbEntry.insertPhoneNumber(phone);
	}
	else
	{
		toAbEntry.removePhoneNumber(phone);
	}
}



void AbbrowserConduit::_copy(Addressee &toAbEntry, PilotAddress *fromPiAddr)
{
	FUNCTIONSETUP;
	if (!fromPiAddr) return;
	// copy straight forward values
	toAbEntry.setFamilyName(fromPiAddr->getField(entryLastname));
	toAbEntry.setGivenName(fromPiAddr->getField(entryFirstname));
	toAbEntry.setOrganization(fromPiAddr->getField(entryCompany));
	toAbEntry.setTitle(fromPiAddr->getField(entryTitle));
	toAbEntry.setNote(fromPiAddr->getField(entryNote));

	// copy the phone stuff
	toAbEntry.removeEmail(toAbEntry.preferredEmail());
	toAbEntry.insertEmail(fromPiAddr->getPhoneField(PilotAddress::eEmail, false), true);

	_copyPhone(toAbEntry,
		toAbEntry.phoneNumber(PhoneNumber::Home),
		fromPiAddr->getPhoneField(PilotAddress::eHome, false));
	_copyPhone(toAbEntry,
		toAbEntry.phoneNumber(PhoneNumber::Work),
		fromPiAddr->getPhoneField(PilotAddress::eWork, false));
	_copyPhone(toAbEntry,
		toAbEntry.phoneNumber(PhoneNumber::Cell),
		fromPiAddr->getPhoneField(PilotAddress::eMobile, false));
	_copyPhone(toAbEntry,
		getFax(toAbEntry),
		fromPiAddr->getPhoneField(PilotAddress::eFax, false));
	_copyPhone(toAbEntry,
		toAbEntry.phoneNumber(PhoneNumber::Pager),
		fromPiAddr->getPhoneField(PilotAddress::ePager, false));
	setOtherField(toAbEntry, fromPiAddr->getPhoneField(PilotAddress::eOther, false));

	KABC::Address homeAddress = getAddress(toAbEntry);
	homeAddress.setStreet(fromPiAddr->getField(entryAddress));
	homeAddress.setLocality(fromPiAddr->getField(entryCity));
	homeAddress.setRegion(fromPiAddr->getField(entryState));
	homeAddress.setPostalCode(fromPiAddr->getField(entryZip));
	homeAddress.setCountry(fromPiAddr->getField(entryCountry));
	toAbEntry.insertAddress(homeAddress);

	setCustomField(toAbEntry, 0, fromPiAddr->getField(entryCustom1));
	setCustomField(toAbEntry, 1, fromPiAddr->getField(entryCustom2));
	setCustomField(toAbEntry, 2, fromPiAddr->getField(entryCustom3));
	setCustomField(toAbEntry, 3, fromPiAddr->getField(entryCustom4));

	// copy the fromPiAddr pilot id to the custom field KPilot_Id;
	// pilot id may be zero(since it could be new) but couldn't hurt
	// to even assign it to zero; let's us know what state the
	// toAbEntry is in
	toAbEntry.insertCustom(appString, idString, QString::number(fromPiAddr->getID()));


	int cat = fromPiAddr->getCat();
	QString category;
	if (0 < cat && cat <= 15) category = fAddressAppInfo.category.name[cat];
	_setCategory(toAbEntry, category);
#ifdef DEBUG
	showAddressee(toAbEntry);
#endif
	if (isArchived(fromPiAddr))
		makeArchived(toAbEntry);
}



/*********************************************************************
 C O N F L I C T   R E S O L U T I O N   a n d   M E R G I N G
 *********************************************************************/



/** smartly merge the given field for the given entry. use the backup record to determine which record has been modified
	@pc, @backup, @palm ... entries of the according databases
	@returns string of the merged entries.
*/
QString AbbrowserConduit::_smartMergeString(const QString &pc, const QString & backup,
	const QString & palm, eConflictResolution confRes)
{
	FUNCTIONSETUP;

	// if both entries are already the same, no need to do anything
	if(pc == palm) return pc;

	// If this is a first sync, we don't have a backup record, so
	if(isFirstSync() || backup.isEmpty()) {
		if (pc.isEmpty() && palm.isEmpty() ) return QString::null;
		if(pc.isEmpty()) return palm;
		if(palm.isEmpty()) return pc;
	} else {
		// only one side modified, so return that string, no conflict
		if(palm == backup) return pc;
		if(pc == backup) return palm;
	}

#ifdef DEBUG
	DEBUGCONDUIT<<"pc="<<pc<<", backup="<<backup<<", palm="<<
		palm<<", ConfRes="<<confRes<<endl;
	DEBUGCONDUIT<<"Use conflict resolution :"<<confRes<<
		", PC="<<SyncAction::ePCOverrides<<endl;
#endif
	switch(confRes) {
		case SyncAction::ePCOverrides: return pc; break;
		case SyncAction::eHHOverrides: return palm; break;
		case SyncAction::ePreviousSyncOverrides: return backup; break;
		default: break;
	}
	return QString::null;
}



bool AbbrowserConduit::_buildResolutionTable(ResolutionTable*tab, const Addressee &pcAddr,
	PilotAddress *backupAddr, PilotAddress *palmAddr)
{
	FUNCTIONSETUP;
	if (!tab) return false;
	tab->setAutoDelete( TRUE );
	tab->labels[0]=i18n("Item on PC");
	tab->labels[1]=i18n("Handheld");
	tab->labels[2]=i18n("Last sync");
	if (!pcAddr.isEmpty())
		tab->fExistItems=(eExistItems)(tab->fExistItems|eExistsPC);
	if (backupAddr)
		tab->fExistItems=(eExistItems)(tab->fExistItems|eExistsBackup);
	if (palmAddr)
		tab->fExistItems=(eExistItems)(tab->fExistItems|eExistsPalm);

#define appendGen(desc, abfield, palmfield) \
	tab->append(new ResolutionItem(desc, tab->fExistItems, \
		(!pcAddr.isEmpty())?(abfield):(QString::null), \
		(palmAddr)?(palmAddr->palmfield):(QString::null), \
		(backupAddr)?(backupAddr->palmfield):(QString::null) ))
#define appendAddr(desc, abfield, palmfield) \
	appendGen(desc, abfield, getField(palmfield))
#define appendGenPhone(desc, abfield, palmfield) \
	appendGen(desc, abfield, getPhoneField(PilotAddress::palmfield, false))
#define appendPhone(desc, abfield, palmfield) \
	appendGenPhone(desc, pcAddr.phoneNumber(PhoneNumber::abfield).number(), palmfield)


	appendAddr(i18n("Last name"), pcAddr.familyName(), entryLastname);
	appendAddr(i18n("First name"), pcAddr.givenName(), entryFirstname);
	appendAddr(i18n("Organization"), pcAddr.organization(), entryCompany);
	appendAddr(i18n("Title"), pcAddr.title(), entryTitle);
	appendAddr(i18n("Note"), pcAddr.note(), entryNote);
	appendAddr(i18n("Custom 1"), getCustomField(pcAddr, 0), entryCustom1);
	appendAddr(i18n("Custom 2"), getCustomField(pcAddr, 1), entryCustom2);
	appendAddr(i18n("Custom 3"), getCustomField(pcAddr, 2), entryCustom3);
	appendAddr(i18n("Custom 4"), getCustomField(pcAddr, 3), entryCustom4);
	appendPhone(i18n("Work Phone"), Work, eWork);
	appendPhone(i18n("Home Phone"), Home, eHome);
	appendPhone(i18n("Mobile Phone"), Cell, eMobile);
	appendGenPhone(i18n("Fax"), getFax(pcAddr).number(), eFax);
	appendPhone(i18n("Pager"), Pager, ePager);
	appendGenPhone(i18n("Other"), getOtherField(pcAddr), eOther);
	appendGenPhone(i18n("Email"), pcAddr.preferredEmail(), eEmail);

	KABC::Address abAddress = getAddress(pcAddr);
	appendAddr(i18n("Address"), abAddress.street(), entryAddress);
	appendAddr(i18n("City"), abAddress.locality(), entryCity);
	appendAddr(i18n("Region"), abAddress.region(), entryState);
	appendAddr(i18n("Postal code"), abAddress.postalCode(), entryZip);
	appendAddr(i18n("Country"), abAddress.country(), entryCountry);

	appendGen(i18n("Category"),
		_getCatForHH(pcAddr.categories(), (palmAddr)?(palmAddr->getCategoryLabel()):(QString::null)),
		getCategoryLabel());

#undef appendGen
#undef appendAddr
#undef appendGenPhone
#undef appendPhone

	return true;
}



bool AbbrowserConduit::_applyResolutionTable(ResolutionTable*tab, Addressee &pcAddr,
	PilotAddress *backupAddr, PilotAddress *palmAddr)
{
	FUNCTIONSETUP;
	if (!tab) return false;
	if (!palmAddr) {
#ifdef DEBUG
		DEBUGCONDUIT<<"Empty palmAddr after conf res. ERROR!!!!"<<endl;
#endif
		kdWarning()<<"Empty palmAddr after conf res. ERROR!!!!"<<endl;
		return false;
	}

	ResolutionItem*item=tab->first();
#define SETGENFIELD(abfield, palmfield) \
	if (item) {\
		abfield; \
		palmAddr->setField(palmfield, item->fResolved); \
	}\
	item=tab->next();
#define SETFIELD(abfield, palmfield) \
	SETGENFIELD(pcAddr.set##abfield(item->fResolved), palmfield)
#define SETCUSTOMFIELD(abfield, palmfield) \
	SETGENFIELD(setCustomField(pcAddr, abfield, item->fResolved), palmfield)
#define SETGENPHONE(abfield, palmfield) \
	if (item) { \
		abfield; \
		palmAddr->setPhoneField(PilotAddress::palmfield, item->fResolved, false); \
	}\
	item=tab->next();
#define SETPHONEFIELD(abfield, palmfield) \
	SETGENPHONE(_setPhoneNumber(pcAddr, PhoneNumber::abfield, item->fResolved), palmfield)
#define SETADDRESSFIELD(abfield, palmfield) \
	SETGENFIELD(abAddress.abfield(item->fResolved), palmfield)

	SETFIELD(FamilyName, entryLastname);
	SETFIELD(GivenName, entryFirstname);
	SETFIELD(Organization, entryCompany);
	SETFIELD(Title, entryTitle);
	SETFIELD(Note, entryNote);
	SETCUSTOMFIELD(0, entryCustom1);
	SETCUSTOMFIELD(1, entryCustom2);
	SETCUSTOMFIELD(2, entryCustom3);
	SETCUSTOMFIELD(3, entryCustom4);
	SETPHONEFIELD(Work, eWork);
	SETPHONEFIELD(Home, eHome);
	SETPHONEFIELD(Cell, eMobile);
	SETGENPHONE(setFax(pcAddr, item->fResolved), eFax);
	SETPHONEFIELD(Pager, ePager);
	SETGENPHONE(setOtherField(pcAddr, item->fResolved), eOther);

	// TODO: fix email
	if (item) {
		palmAddr->setPhoneField(PilotAddress::eEmail, item->fResolved, false);
		if (backupAddr)
			pcAddr.removeEmail(backupAddr->getPhoneField(PilotAddress::eEmail, false));
		pcAddr.removeEmail(palmAddr->getPhoneField(PilotAddress::eEmail, false));
		pcAddr.insertEmail(item->fResolved, true);
	}
	item=tab->next();

	KABC::Address abAddress = getAddress(pcAddr);
	SETADDRESSFIELD(setStreet, entryAddress);
	SETADDRESSFIELD(setLocality, entryCity);
	SETADDRESSFIELD(setRegion, entryState);
	SETADDRESSFIELD(setPostalCode, entryZip);
	SETADDRESSFIELD(setCountry, entryCountry);
	pcAddr.insertAddress(abAddress);

	// TODO: Is this correct?
	if (item) {
		palmAddr->setCategory(item->fResolved);
		_setCategory(pcAddr, item->fResolved);
	}


#undef SETGENFIELD
#undef SETFIELD
#undef SETCUSTOMFIELD
#undef SETGENPHONE
#undef SETPHONEFIELD
#undef SETADDRESSFIELD

	return true;
}



bool AbbrowserConduit::_smartMergeTable(ResolutionTable*tab)
{
	FUNCTIONSETUP;
	if (!tab) return false;
	bool noconflict=true;
	ResolutionItem*item;
	for ( item = tab->first(); item; item = tab->next() )
	{
		// try to merge the three strings
		item->fResolved=_smartMergeString(item->fEntries[0],
			item->fEntries[2], item->fEntries[1], fConflictResolution);
		// if a conflict occurred, set the default to something sensitive:
		if (item->fResolved.isNull() && !(item->fEntries[0].isEmpty() &&
			item->fEntries[1].isEmpty() && item->fEntries[2].isEmpty() ) )
		{
			item->fResolved=item->fEntries[0];
			noconflict=false;
		}
		if (item->fResolved.isNull()) item->fResolved=item->fEntries[1];
		if (item->fResolved.isNull()) item->fResolved=item->fEntries[2];
	}
	return  noconflict;
}



/** Merge the palm and the pc entries with the additional information of
 *  the backup.
 *  return value: no meaning yet
 */
bool AbbrowserConduit::_smartMergeAddressee(Addressee &pcAddr,
	PilotAddress *backupAddr, PilotAddress *palmAddr)
{
	FUNCTIONSETUP;

	// Merge them, then look which records have to be written to device or abook
	int res = SyncAction::eAskUser;
 	bool result=true;
	ResolutionTable tab;

	result &= _buildResolutionTable(&tab, pcAddr, backupAddr, palmAddr);
	// Now attempt a smart merge. If that fails, let conflict resolution do the job
	bool mergeOk=_smartMergeTable(&tab);

	if (!mergeOk)
	{
		QString dlgText;
		if (!palmAddr)
		{
			dlgText=i18n("The following address entry was changed, but does no longer exist on the handheld. Please resolve this conflict:");
		}
		else if (pcAddr.isEmpty())
		{
			dlgText=i18n("The following address entry was changed, but does no longer exist on the PC. Please resolve this conflict:");
		}
		else
		{
			dlgText=i18n("The following address entry was changed on the handheld as well as on the PC side. The changes could not be merged automatically, so please resolve the conflict yourself:");
		}
		ResolutionDlg*resdlg=new ResolutionDlg(0L, fHandle, i18n("Address conflict"), dlgText, &tab);
		resdlg->exec();
		KPILOT_DELETE(resdlg);
	}
	res=tab.fResolution;

	// Disallow some resolution under certain conditions, fix wrong values:
	switch (res) {
		case SyncAction::eHHOverrides:
			if (!palmAddr) res=SyncAction::eDelete;
			break;
		case SyncAction::ePCOverrides:
			if (pcAddr.isEmpty()) res=SyncAction::eDelete;
			break;
		case SyncAction::ePreviousSyncOverrides:
			if (!backupAddr) res=SyncAction::eDoNothing;
			break;
	}

	PilotAddress*pAddr=palmAddr;
	bool pAddrCreated=false;
	// Now that we have done a possible conflict resolution, apply the changes
	switch (res) {
		case SyncAction::eDuplicate:
			// Set the Palm ID to 0 so we don't overwrite the existing record.
			pcAddr.removeCustom(appString, idString);
			result &= _copyToHH(pcAddr, 0L, 0L);
			{
			Addressee pcadr;
			result &= _copyToPC(pcadr, backupAddr, palmAddr);
			}
			break;
		case SyncAction::eDoNothing:
			break;
		case SyncAction::eHHOverrides:
				result &= _copyToPC(pcAddr, backupAddr, palmAddr);
				break;
		case SyncAction::ePCOverrides:
			result &= _copyToHH(pcAddr, backupAddr, pAddr);
			break;
		case SyncAction::ePreviousSyncOverrides:
			_copy(pcAddr, backupAddr);
			if (palmAddr && backupAddr) *palmAddr=*backupAddr;
			result &= _savePalmAddr(backupAddr, pcAddr);
			result &= _savePCAddr(pcAddr, backupAddr, backupAddr);
			break;
		case SyncAction::eDelete:
			result &= _deleteAddressee(pcAddr, backupAddr, palmAddr);
			break;
		case SyncAction::eAskUser:
		default:
			if (!pAddr)
			{
				pAddr=new PilotAddress(fAddressAppInfo);
				pAddrCreated=true;
			}
			result &= _applyResolutionTable(&tab, pcAddr, backupAddr, pAddr);
showAdresses(pcAddr, backupAddr, pAddr);
			// savePalmAddr sets the RecordID custom field already
			result &= _savePalmAddr(pAddr, pcAddr);
			result &= _savePCAddr(pcAddr, backupAddr, pAddr);
			if (pAddrCreated) KPILOT_DELETE(pAddr);
			break;
	}

	return result;
}



// TODO: right now entries are equal if both first/last name and organization are
//  equal. This rules out two entries for the same person(e.g. real home and weekend home)
//  or two persons with the same name where you don't know the organization.!!!
Addressee AbbrowserConduit::_findMatch(const PilotAddress & pilotAddress) const
{
	FUNCTIONSETUP;
	// TODO: also search with the pilotID
	// first, use the pilotID to UID map to find the appropriate record
	if( !isFirstSync() && (pilotAddress.id() > 0) )
	{
		QString id(addresseeMap[pilotAddress.id()]);
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": PilotRecord has id " << pilotAddress.id() << ", mapped to " << id << endl;
#endif
		if(!id.isEmpty())
		{
			Addressee res(aBook->findByUid(id));
			if(!res.isEmpty()) return res;
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": PilotRecord has id " << pilotAddress.id() << ", but could not be found in the addressbook" << endl;
#endif
		}
	}

	for(AddressBook::Iterator iter = aBook->begin(); iter != aBook->end(); ++iter)
	{
		Addressee abEntry = *iter;
		QString recID(abEntry.custom(appString, idString));
		bool ok;
		if (!recID.isEmpty() )
		{
			recordid_t rid = recID.toLong(&ok);
			if (ok && rid)
			{
				if (rid==pilotAddress.id()) return abEntry;// yes, we found it
				// skip this addressee, as it can an other corresponding address on the handheld
				if (allIds.contains(rid)) continue;
			}
		}

		if (_equal(&pilotAddress, abEntry, eqFlagsAlmostAll))
		{
			return abEntry;
		}
	}
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Could not find any addressbook enty matching " << pilotAddress.getField(entryLastname) << endl;
#endif
	return Addressee();
}

