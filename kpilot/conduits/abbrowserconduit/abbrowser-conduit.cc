// knotes-conduit.cc
//
// Copyright (C) 2000 by Dan Pilone, Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
//
//
// The KNotes conduit copies memos from the Pilot's memo pad to KNotes
// and vice-versa. It complements or replaces the builtin memo conduit
// in KPilot.
//
//



#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++, 
// then Qt, then KDE, then local includes.
//
//
#include <unistd.h>
#include <assert.h>
#include <stream.h>
#include <qdir.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kdebug.h>
#include <krun.h>

// kpilot includes
#include "abbrowser-conduit.h"
#include "conduitApp.h"
//#include "kpilotlink.h"
#include "setupDialog.h"
#include "abbrowserConduitConfig.h"
#include "kpilotConfig.h"
#include "pi-appinfo.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *id=
	"$Id$";


// This is a generic main() function, all
// conduits look basically the same,
// except for the name of the conduit.
//
//
int main(int argc, char* argv[])
    {
    ConduitApp a(argc,argv,"abbrowser",
		 I18N_NOOP("Abbrowser Conduit"),
		 "0.1");
    
    a.addAuthor("Gregory Stern",
		"Abbrowser Conduit author",
		"stern@enews.nrl.navy.mil");
    
    AbbrowserConduit conduit(a.getMode(), a.getDBSource());
    a.setConduit(&conduit);
    cout << "AbbrowserConduit about to call exec" << endl;
    return a.exec(true /* with DCOP support */, false);
    }

AbbrowserConduit::AbbrowserConduit(BaseConduit::eConduitMode mode,
				   BaseConduit::DatabaseSource source)
      : BaseConduit(mode, source),
	fDcop(NULL),
	fAddressAppInfo(), fSmartMerge(true), fConflictResolution(eUserChoose),
	fPilotOtherMap(), fPilotStreetHome(true), fPilotFaxHome(false),
	fCloseAbIfOpen(false), fBackupDone(false)
    {
    FUNCTIONSETUP;
	
    if (source == Local)
	kdDebug() << fname << "  conduit use local" << endl;
    else if (source == ConduitSocket)
	kdDebug() << fname << " conduit socket" << endl;
    else
	kdDebug() << fname << " undefined source" << endl;
      
    }

AbbrowserConduit::~AbbrowserConduit()
    {
    }

// aboutAndSetup is pretty much the same
// on all conduits as well.
//
//
QWidget* AbbrowserConduit::aboutAndSetup()
    {
    FUNCTIONSETUP;
    
    return new AbbrowserConduitOptions(0L);
    }

const char * AbbrowserConduit::dbInfo()
    {
    return "AddressDB";
    }

bool AbbrowserConduit::_startAbbrowser()
    {
    FUNCTIONSETUP;

    bool alreadyRunning = true;
    QByteArray sendData;
    QByteArray replyData;
    QCString replyTypeStr;
    if (!fDcop->call("abbrowser", "AbBrowserIface", "interfaces()",
		       sendData, replyTypeStr, replyData))
	{
	// abbrowser not running, start it
	KURL::List noargs;
	KRun::run("abbrowser", noargs);
	
	kdDebug() << fname << "Waiting to run abbrowser" << endl;
	alreadyRunning = false;
	sleep(5000);
	}

    if (!fDcop->call("abbrowser", "AbBrowserIface", "interfaces()",
		       sendData, replyTypeStr, replyData))
	{
	kdDebug() << fname << " unable to connect to abbrowser through dcop; autostart failed" << endl;
	KApplication::kApplication()->exit(BaseConduit::PeerApplicationMissing);
	}
    return alreadyRunning;
    }

void AbbrowserConduit::_stopAbbrowser(bool abAlreadyRunning)
    {
    FUNCTIONSETUP;

    kdDebug() << fname << " about to stop" << endl;
    
    if (fCloseAbIfOpen && !abAlreadyRunning)
	{
	QByteArray sendData;
	QByteArray replyData;
	QCString replyTypeStr;
	
	if (!fDcop->call("abbrowser", "AbBrowserIface",
			 "exit()",
			 sendData, replyTypeStr, replyData))
	    {
	    kdWarning() << "Unable to exit abbrowser" << endl;
	    }
	}
    }

void AbbrowserConduit::
_mapContactsToPilot(const QDict<ContactEntry> &contacts,
		    QMap<recordid_t, QString> &idContactMap,
		    QList<ContactEntry> &newContacts) const
    {
    FUNCTIONSETUP;
    
    idContactMap.clear();
    newContacts.clear();
    for (QDictIterator<ContactEntry> contactIter(contacts);
	 contactIter.current();++contactIter)
	{
	ContactEntry *aContact = contactIter.current();
	if (aContact->isNew())
	    newContacts.append(aContact);
	else
	    {
	    QString idStr = aContact->getCustomField("KPILOT_ID");
	    if (idStr != QString::null)
		{
		recordid_t id = idStr.toULong();
		idContactMap.insert(id, contactIter.currentKey());
		}
	    else
		{
		kdDebug() << fname << " contact is new but KPILOT_ID is not found in abbrowser contact; BUG!" << endl;
		newContacts.append(aContact);
		}
	    }
	}
    }

bool AbbrowserConduit::_getAbbrowserContacts(QDict<ContactEntry> &contacts)
    {
    QDict<ContactEntry> entryDict;

    QByteArray noParamData;
    QByteArray replyDictData;
    QCString replyTypeStr;
    if (!fDcop->call("abbrowser", "AbBrowserIface", "getEntryDict()",
		       noParamData, replyTypeStr, replyDictData))
	{
	kdWarning() << "AbBrowserIface::Unable to call abbrowser getEntryDict()" << endl;
	return false;
	}
    assert(replyTypeStr == "QDict<ContactEntry>");

    QDataStream dictStream(replyDictData, IO_ReadOnly);
    dictStream >> contacts;
    return true;
    }

void AbbrowserConduit::showContactEntry(const ContactEntry &abAddress)
     {
     qDebug("\tAbbrowser Contact Entry");
     qDebug("\t\tLast name = %s", abAddress.getLastName().latin1());
     qDebug("\t\tFirst name = %s", abAddress.getFirstName().latin1());
     qDebug("\t\tCompany = %s", abAddress.getCompany().latin1());
     qDebug("\t\tJob Title = %s", abAddress.getJobTitle().latin1());
     qDebug("\t\tNote = %s", abAddress.getNote().latin1());
     qDebug("\t\tHome phone = %s", abAddress.getHomePhone().latin1());
     qDebug("\t\tWork phone = %s", abAddress.getBusinessPhone().latin1());
     qDebug("\t\tMobile phone = %s", abAddress.getMobilePhone().latin1());
     qDebug("\t\tEmail = %s", abAddress.getEmail().latin1());
     qDebug("\t\tFax = %s", abAddress.getBusinessFax().latin1());
     qDebug("\t\tPager = %s", abAddress.getPager().latin1());
     qDebug("\t\tCategory = %s", abAddress.getFolder().latin1());
     
     }


void AbbrowserConduit::showPilotAddress(const PilotAddress &pilotAddress) 
     {
     qDebug("\tPilot Address");
     qDebug("\t\tLast name = %s", pilotAddress.getField(entryLastname));
     qDebug("\t\tFirst name = %s", pilotAddress.getField(entryFirstname));
     qDebug("\t\tCompany = %s", pilotAddress.getField(entryCompany));
     qDebug("\t\tJob Title = %s", pilotAddress.getField(entryTitle));
     qDebug("\t\tNote = %s", pilotAddress.getField(entryNote));
     qDebug("\t\tHome phone = %s",
	    pilotAddress.getPhoneField(PilotAddress::eHome));
     qDebug("\t\tWork phone = %s",
	    pilotAddress.getPhoneField(PilotAddress::eWork));
     qDebug("\t\tMobile phone = %s",
	    pilotAddress.getPhoneField(PilotAddress::eMobile));
     qDebug("\t\tEmail = %s",
	    pilotAddress.getPhoneField(PilotAddress::eEmail));
     qDebug("\t\tFax = %s",
	    pilotAddress.getPhoneField(PilotAddress::eFax));
     qDebug("\t\tPager = %s",
	    pilotAddress.getPhoneField(PilotAddress::ePager));
     qDebug("\t\tCategory = %s", pilotAddress.getCategoryLabel());

     }

void AbbrowserConduit::_copy(PilotAddress &toPilotAddr,
			     ContactEntry &fromAbEntry)
    {
    // don't do a reset since this could wipe out non copied info 
    //toPilotAddr.reset();
    toPilotAddr.setField(entryLastname, fromAbEntry.getLastName().latin1());
    QString firstAndMiddle = fromAbEntry.getFirstName();
    if (!fromAbEntry.getMiddleName().isEmpty())
	firstAndMiddle += " " + fromAbEntry.getMiddleName();
    toPilotAddr.setField(entryFirstname, firstAndMiddle.latin1());
    toPilotAddr.setField(entryCompany, fromAbEntry.getCompany().latin1());
    toPilotAddr.setField(entryTitle, fromAbEntry.getJobTitle().latin1());
    toPilotAddr.setField(entryNote, fromAbEntry.getNote().latin1());
    
    // do email first, to ensure its gets stored
    toPilotAddr.setPhoneField(PilotAddress::eEmail,
			 fromAbEntry.getEmail().latin1());
    toPilotAddr.setPhoneField(PilotAddress::eWork,
			 fromAbEntry.getBusinessPhone().latin1());
    toPilotAddr.setPhoneField(PilotAddress::eHome,
			 fromAbEntry.getHomePhone().latin1());
    toPilotAddr.setPhoneField(PilotAddress::eMobile,
			 fromAbEntry.getMobilePhone().latin1());
    if (isPilotFaxHome())
	toPilotAddr.setPhoneField(PilotAddress::eFax,
				  fromAbEntry.getHomeFax().latin1());
    else
	toPilotAddr.setPhoneField(PilotAddress::eFax,
				  fromAbEntry.getBusinessFax().latin1());
    
    toPilotAddr.setPhoneField(PilotAddress::ePager,
			 fromAbEntry.getPager().latin1());
    toPilotAddr.setShownPhone(PilotAddress::eMobile);
    QString otherMapType = getPilotOtherMap();
    if (!otherMapType.isEmpty())
	toPilotAddr.setPhoneField(PilotAddress::eOther,
				  fromAbEntry.findRef(otherMapType).latin1());
    // in future, may want prefs that will map from abbrowser entries
    // to the pilot phone entries so they can do the above assignment and
    // assign the Other entry which is currenty unused
    ContactEntry::Address *homeAddress = fromAbEntry.getHomeAddress();
    if (!homeAddress->isEmpty())
	_setPilotAddress(toPilotAddr, *homeAddress);
    else
	{
	// no home address, try work address
	ContactEntry::Address *workAddress = fromAbEntry.getBusinessAddress();
	if (!workAddress->isEmpty())
	    _setPilotAddress(toPilotAddr, *workAddress);
	delete workAddress;
	workAddress = NULL;
	}
    delete homeAddress;
    homeAddress = NULL;
    }

void AbbrowserConduit::_setPilotAddress(PilotAddress &toPilotAddr,
					const ContactEntry::Address &abAddress)
    {
    toPilotAddr.setField(entryAddress, abAddress.getStreet().latin1());
    toPilotAddr.setField(entryCity, abAddress.getCity().latin1());
    toPilotAddr.setField(entryState, abAddress.getState().latin1());
    toPilotAddr.setField(entryZip, abAddress.getZip().latin1());
    toPilotAddr.setField(entryCountry, abAddress.getCountry().latin1());
    }

void AbbrowserConduit::_copy(ContactEntry &toAbEntry,
			     const PilotAddress &fromPiAddr)
    {
    // copy straight forward values
    toAbEntry.setLastName( fromPiAddr.getField(entryLastname) );
    toAbEntry.setFirstName( fromPiAddr.getField(entryFirstname) );
    toAbEntry.setCompany( fromPiAddr.getField(entryCompany) );
    toAbEntry.setJobTitle( fromPiAddr.getField(entryTitle) );
    toAbEntry.setNote( fromPiAddr.getField(entryNote) );
    toAbEntry.setName();

    // copy the phone stuff
    toAbEntry.setEmail( fromPiAddr.getPhoneField(PilotAddress::eEmail));
    toAbEntry.setHomePhone( fromPiAddr.getPhoneField(PilotAddress::eHome));
    toAbEntry.setBusinessPhone( fromPiAddr.getPhoneField(PilotAddress::eWork));
    toAbEntry.setMobilePhone( fromPiAddr.getPhoneField(PilotAddress::eMobile));
    if (isPilotFaxHome())
	toAbEntry.setHomeFax( fromPiAddr.getPhoneField(PilotAddress::eFax));
    else
	toAbEntry.setBusinessFax(fromPiAddr.getPhoneField(PilotAddress::eFax));
    toAbEntry.setPager( fromPiAddr.getPhoneField(PilotAddress::ePager));
    if (!getPilotOtherMap().isEmpty())
	toAbEntry.replaceValue( getPilotOtherMap(),
			       fromPiAddr.getPhoneField(PilotAddress::eOther));
	
    //in future, probably the address assigning to work or home should
    // be a prefs option
    // for now, just assign to home since that's what I'm using it for
    ContactEntry::Address *homeAddress = toAbEntry.getHomeAddress();
    homeAddress->setStreet(fromPiAddr.getField(entryAddress));
    homeAddress->setCity(fromPiAddr.getField(entryCity));
    homeAddress->setState(fromPiAddr.getField(entryState));
    homeAddress->setZip(fromPiAddr.getField(entryZip));
    homeAddress->setCountry(fromPiAddr.getField(entryCountry));
    delete homeAddress;
    homeAddress = NULL;
    
    // copy the fromPiAddr pilot id to the custom field KPilot_Id;
    // pilot id may be zero (since it could be new) but couldn't hurt
    // to even assign it to zero; let's us know what state the
    // toAbEntry is in
    toAbEntry.setCustomField("KPILOT_ID", QString::number(fromPiAddr.getID()));
    }


void AbbrowserConduit::_addToAbbrowser(const PilotAddress &address)
    {
    ContactEntry entry;
    _copy(entry, address);
    _saveAbEntry(entry, QString::null);
    }

void AbbrowserConduit::_addToPalm(ContactEntry &entry)
    {
    PilotAddress pilotAddress(fAddressAppInfo);
    _copy(pilotAddress, entry);
    if (_savePilotAddress(pilotAddress, entry))
	_saveAbEntry(entry, QString::null);
    }

bool AbbrowserConduit::_conflict(const QString &str1,
				 const QString &str2,
				 bool &mergeNeeded, QString &mergedStr) const
    {
    mergeNeeded = false;
    if (str1.isEmpty() && str2.isEmpty())
	return false;
    if (str1.isEmpty() || str2.isEmpty())
	{
	mergeNeeded = true;
	if (str1 == QString::null)
	    mergedStr = str2;
	else
	    mergedStr = str1;
	return false;
	}
    if (str1 != str2)
	return true;
    return false;
    }

bool AbbrowserConduit::_smartMerge(PilotAddress &outPilotAddress,
				   ContactEntry &outAbEntry)
    {
    PilotAddress pilotAddress(outPilotAddress);
    ContactEntry abEntry(outAbEntry);
    
    bool mergeNeeded = false;
    QString mergedStr;

    if (_conflict(pilotAddress.getCategoryLabel(),
		  abEntry.getFolder(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setCategory(mergedStr.latin1());
	abEntry.setFolder(mergedStr);
	}
    
    if (_conflict(pilotAddress.getField(entryLastname),
		  abEntry.getLastName(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setField(entryLastname, mergedStr.latin1());
	abEntry.setLastName(mergedStr);
	}

    if (_conflict(pilotAddress.getField(entryFirstname),
		  abEntry.getFirstName(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setField(entryFirstname, mergedStr.latin1());
	abEntry.setFirstName(mergedStr);
	}

    if (_conflict(pilotAddress.getField(entryCompany),
		  abEntry.getCompany(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setField(entryCompany, mergedStr.latin1());
	abEntry.setCompany(mergedStr);
	}

    if (_conflict(pilotAddress.getField(entryTitle),
		  abEntry.getJobTitle(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setField(entryTitle, mergedStr.latin1());
	abEntry.setJobTitle(mergedStr);
	}

    if (_conflict(pilotAddress.getField(entryNote),
		  abEntry.getNote(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setField(entryNote, mergedStr.latin1());
	abEntry.setNote(mergedStr);
	}

    if (_conflict(pilotAddress.getPhoneField(PilotAddress::eWork),
		  abEntry.getBusinessPhone(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setPhoneField(PilotAddress::eWork, mergedStr.latin1());
	abEntry.setBusinessPhone(mergedStr);
	}

    if (_conflict(pilotAddress.getPhoneField(PilotAddress::eHome),
		  abEntry.getHomePhone(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setPhoneField(PilotAddress::eHome, mergedStr.latin1());
	abEntry.setHomePhone(mergedStr);
	}

    if (_conflict(pilotAddress.getPhoneField(PilotAddress::eEmail),
		  abEntry.getEmail(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setPhoneField(PilotAddress::eEmail, mergedStr.latin1());
	abEntry.setEmail(mergedStr);
	}

    if (_conflict(pilotAddress.getPhoneField(PilotAddress::eMobile),
		  abEntry.getMobilePhone(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setPhoneField(PilotAddress::eMobile, mergedStr.latin1());
	abEntry.setMobilePhone(mergedStr);
	}

    if (isPilotFaxHome())
	if (_conflict(pilotAddress.getPhoneField(PilotAddress::eFax),
		      abEntry.getHomeFax(), mergeNeeded, mergedStr))
	    return false;
    else
	if (_conflict(pilotAddress.getPhoneField(PilotAddress::eFax),
		      abEntry.getBusinessFax(), mergeNeeded, mergedStr))
	    return false;
    if (mergeNeeded)
	{
	pilotAddress.setPhoneField(PilotAddress::eFax, mergedStr.latin1());
	if (isPilotFaxHome())
	    abEntry.setHomeFax(mergedStr);
	else
	    abEntry.setBusinessFax(mergedStr);
	}

    if (_conflict(pilotAddress.getPhoneField(PilotAddress::ePager),
		  abEntry.getPager(), mergeNeeded, mergedStr))
	return false;
    if (mergeNeeded)
	{
	pilotAddress.setPhoneField(PilotAddress::ePager, mergedStr.latin1());
	abEntry.setPager(mergedStr);
	}

    ContactEntry::Address *abAddress = abEntry.getHomeAddress();
    if (_conflict(pilotAddress.getField(entryAddress) ,abAddress->getStreet(),
					mergeNeeded, mergedStr))
	{
	delete abAddress;
	abAddress = abEntry.getBusinessAddress();

	if (_conflict(pilotAddress.getField(entryAddress) ,
		      abAddress->getStreet(),mergeNeeded, mergedStr))
	    {
	    delete abAddress;
	    return false;
	    }
	}
    if (mergeNeeded)
	{
	pilotAddress.setField(entryAddress, mergedStr.latin1());
	abAddress->setStreet(mergedStr);
	}
    
    if (_conflict(pilotAddress.getField(entryCity), abAddress->getCity(),
					mergeNeeded, mergedStr))
	{
	delete abAddress;
	return false;
	}
    if (mergeNeeded)
	{
	pilotAddress.setField(entryCity, mergedStr.latin1());
	abAddress->setCity(mergedStr);
	}

    if (_conflict(pilotAddress.getField(entryState), abAddress->getState(),
					mergeNeeded, mergedStr))
	{
	delete abAddress;
	return false;
	}
    if (mergeNeeded)
	{
	pilotAddress.setField(entryState, mergedStr.latin1());
	abAddress->setState(mergedStr);
	}

    if (_conflict(pilotAddress.getField(entryZip), abAddress->getZip(),
					mergeNeeded, mergedStr))
	{
	delete abAddress;
	return false;
	}
    if (mergeNeeded)
	{
	pilotAddress.setField(entryZip, mergedStr.latin1());
	abAddress->setZip(mergedStr);
	}

    if (_conflict(pilotAddress.getField(entryCountry), abAddress->getCountry(),
					mergeNeeded, mergedStr))
	{
	delete abAddress;
	return false;
	}
    if (mergeNeeded)
	{
	pilotAddress.setField(entryCountry, mergedStr.latin1());
	abAddress->setCountry(mergedStr);
	}

    delete abAddress;

    abEntry.setCustomField("KPILOT_ID", QString::number(pilotAddress.id()));
    abEntry.setName();
    
    outPilotAddress = pilotAddress;
    outAbEntry = abEntry;
    
    return true;
    }

/** There was a conflict between the two fields; either could be null,
 *  or both have been modified
 */
void AbbrowserConduit::_handleConflict(PilotAddress *pilotAddress,
				       ContactEntry *abEntry,
				       const QString &abKey)
    {
    FUNCTIONSETUP;
    
    if (pilotAddress && abEntry)
	{
	if (doSmartMerge() && _smartMerge(*pilotAddress, *abEntry))
	    {
	    
	    kdDebug() << fname << " Both records exist but both were changed => MERGE done" << endl;
	    //showPilotAddress(*pilotAddress);
	    if (getMode() != BaseConduit::Backup)
		_savePilotAddress(*pilotAddress, *abEntry);
	    _saveAbEntry(*abEntry, abKey);
	    }
	else
	    { 
	    kdDebug() << fname << "AbbrowserConduit::_handleConflict => Both records exist but both were changed => conflict, unable to merge; keeping both"
		      << endl;
	    showPilotAddress(*pilotAddress);
	    showContactEntry(*abEntry);
	    switch (getResolveConflictOption())
		{
		case eUserChoose :
		    // implement later with conflict dialog...
		case eKeepBothInAbbrowser :
		    _addToAbbrowser(*pilotAddress);
		    break;
	        case ePilotOverides :
		    _addToAbbrowser(*pilotAddress);
		    _removeAbEntry(abKey);
		    break;
		case eAbbrowserOverides :		    
		    // in future, should see what the config wants to do for
		    // deconfliction; for now will just keep both
		    if (getMode() != BaseConduit::Backup)
			{
			_addToPalm(*abEntry);
			_removePilotAddress(*pilotAddress);
			}
		    break;
		case eDoNotResolve :
		default :
		    // do nothing
		    break;
		}
	    }
	}
    else if (pilotAddress)
	{
	kdDebug() << fname << " ContactEntry was deleted but pilotAddress was modified" << endl;
	showPilotAddress(*pilotAddress);
	}
    else
	{
	kdDebug() << fname << " PilotAddress was deleted but ConactEntry was modified" << endl;
	showContactEntry(*abEntry);
	}
    }

void AbbrowserConduit::_removePilotAddress(PilotAddress &address)
    {
    FUNCTIONSETUP;
    
    kdDebug() << fname << endl;
    showPilotAddress(address);
    }

void AbbrowserConduit::_removeAbEntry(const QString & ) // key
    {
    FUNCTIONSETUP;
    
    kdDebug() << fname << endl;
    //showContactEntry(abEntry);
    }

void AbbrowserConduit::_saveAbEntry(ContactEntry &abEntry,
				    const QString &key)
    {
    FUNCTIONSETUP;
    
    // this marks that this field has been synced
    abEntry.setModified(false);

    kdDebug() << fname << " mod = "
	      << abEntry.isModified() << endl;
	
    //showContactEntry(abEntry);
    
    // save over kdcop to abbrowser
    QByteArray sendData;
    QByteArray replyData;
    QCString replyTypeStr;
    QDataStream out(sendData, IO_WriteOnly);
    if (key == QString::null)
	{
	// new entry; just send the contact entry
	out << abEntry;
	if (!fDcop->call("abbrowser", "AbBrowserIface",
			 "addEntry(ContactEntry)",
			 sendData, replyTypeStr, replyData))
	    {
	    kdWarning() << "Unable to call abbrowser addEntry" << endl;
	    qApp->exit(1);
	    }
	}
    else
	{
	// change entry, send contact and key
	out << key;
	out << abEntry;
	if (!fDcop->call("abbrowser", "AbBrowserIface",
			 "changeEntry(QString, ContactEntry)",
			 sendData, replyTypeStr, replyData))
	    {
	    kdWarning() << "Unable to call abbrowser changeEntry" << endl;
	    qApp->exit(1);
	    }
	}
    }

void AbbrowserConduit::_saveAbChanges()
    {
    QByteArray sendData;
    QByteArray replyData;
    QCString replyTypeStr;

    if (!fDcop->call("abbrowser", "AbBrowserIface",
		     "save()",
		     sendData, replyTypeStr, replyData))
	{
	kdWarning() << "Unable to save abbrowser" << endl;
	qApp->exit(1);
	}
    }

bool AbbrowserConduit::_savePilotAddress(PilotAddress &address,
					 ContactEntry &abEntry)
    {
    FUNCTIONSETUP;
    
    kdDebug() << fname <<
	" id = " << address.id() << endl;

    PilotRecord *pilotRec = address.pack();
    recordid_t pilotId = writeRecord(pilotRec);
    delete pilotRec;
    // pilotId == 0 if using local db, so don't overwrite the valid id
    if (pilotId != 0)
	address.setID(pilotId);
    
    recordid_t abId = 0;
    if (abEntry.getCustomField("KPILOT_ID") != QString::null)
	abId = abEntry.getCustomField("KPILOT_ID").toUInt();
    if (abId != address.id())
	{
	QString abIdStr = QString::number(address.id());
	abEntry.setCustomField("KPILOT_ID", abIdStr);
	return true;
	}
    
    return false;
    }

void AbbrowserConduit::_setAppInfo()
    {
    FUNCTIONSETUP;
    // get the address application header information
    unsigned char * buffer = new unsigned char [PilotAddress::APP_BUFFER_SIZE];
    int appLen = readAppInfo(buffer);
    unpack_AddressAppInfo(&fAddressAppInfo, buffer, appLen);
    delete []buffer;
    buffer = NULL;

    kdDebug() << fname << " lastUniqueId"
	      << fAddressAppInfo.category.lastUniqueID << endl;
    for (int i=0;i < 16;i++)
	kdDebug() << fname << " cat " << i << " =" <<
	    fAddressAppInfo.category.name[i] << endl;
    }

bool AbbrowserConduit::_equal(const PilotAddress &piAddress,
			      ContactEntry &abEntry) const
    {
    bool mergeNeeded=false; // not needed here, but in merge func
    QString mergedStr; // not needed here, but in merge func
    if (_conflict(piAddress.getField(entryLastname),
		  abEntry.getLastName(), mergeNeeded, mergedStr))
	return false;
    if (_conflict(piAddress.getField(entryFirstname), abEntry.getFirstName(),
		  mergeNeeded, mergedStr))
	return false;
    if (_conflict(piAddress.getField(entryTitle), abEntry.getJobTitle(),
		  mergeNeeded, mergedStr))
	return false;
    if (_conflict(piAddress.getField(entryCompany), abEntry.getCompany(),
		  mergeNeeded, mergedStr))
	return false;
    if (_conflict(piAddress.getField(entryNote), abEntry.getNote(),
		  mergeNeeded, mergedStr))
	return false;
    if (_conflict(piAddress.getCategoryLabel(), abEntry.getFolder(),
		  mergeNeeded, mergedStr))
	return false;
    if (_conflict(piAddress.getPhoneField(PilotAddress::eWork),
		  abEntry.getBusinessPhone(), mergeNeeded, mergedStr))
	return false;
    if (_conflict(piAddress.getPhoneField(PilotAddress::eHome),
		  abEntry.getHomePhone(), mergeNeeded, mergedStr))
	return false;
    if (_conflict(piAddress.getPhoneField(PilotAddress::eEmail),
		  abEntry.getEmail(), mergeNeeded, mergedStr))
	return false;
    if (isPilotFaxHome())
	if (_conflict(piAddress.getPhoneField(PilotAddress::eFax),
		      abEntry.getHomeFax(), mergeNeeded, mergedStr))
	    return false;
    else
	if (_conflict(piAddress.getPhoneField(PilotAddress::eFax),
		      abEntry.getBusinessFax(), mergeNeeded, mergedStr))
	    return false;
    if (_conflict(piAddress.getPhoneField(PilotAddress::eMobile),
	abEntry.getMobilePhone(), mergeNeeded, mergedStr))
	return false;
    ContactEntry::Address *address = abEntry.getHomeAddress();
    if (_conflict(piAddress.getField(entryAddress), address->getStreet(),
		  mergeNeeded, mergedStr))
	{
	delete address;
	address = abEntry.getBusinessAddress();
	if (_conflict(piAddress.getField(entryAddress), address->getStreet(),
		      mergeNeeded, mergedStr))
	    {
	    delete address;
	    return false;
	    }
	}
    if (_conflict(piAddress.getField(entryCity), address->getCity(),
		  mergeNeeded, mergedStr))
	{
	delete address;
	return false;
	}
    if (_conflict(piAddress.getField(entryState), address->getState(),
		  mergeNeeded, mergedStr))
	{
	delete address;
	return false;
	}
    if (_conflict(piAddress.getField(entryZip), address->getZip(),
		  mergeNeeded, mergedStr))
	{
	delete address;
	return false;
	}
    if (_conflict(piAddress.getField(entryCountry), address->getCountry(),
		  mergeNeeded, mergedStr))
	{
	delete address;
	return false;
	}
    
    delete address;
    return true;
    }

ContactEntry *
AbbrowserConduit::_findMatch(const QDict<ContactEntry> &entries,
			     const PilotAddress &pilotAddress,
			     QString &contactKey) const
    {
    bool piFirstEmpty = (pilotAddress.getField(entryFirstname) == 0L);
    bool piLastEmpty = (pilotAddress.getField(entryLastname) == 0L);
    bool piCompanyEmpty = (pilotAddress.getField(entryCompany) == 0L);
    // return not found if not matching keys
    if (piFirstEmpty && piLastEmpty && piCompanyEmpty)  
	return 0L;

    contactKey = QString::null;
    // for now just loop throug all entries; in future, probably better
    // to create a map for first and last name, then just do O(1) calls
    for (QDictIterator<ContactEntry> iter(entries); iter.current();
	 ++iter)
	{
	ContactEntry *abEntry = iter.current();

	// do quick empty check's
	if (piFirstEmpty != abEntry->getFirstName().isEmpty() ||
	    piLastEmpty != abEntry->getLastName().isEmpty() ||
	    piCompanyEmpty != abEntry->getCompany().isEmpty())
	    continue;
	
	if (piFirstEmpty && piLastEmpty)
	    {
	    if (abEntry->getCompany() == pilotAddress.getField(entryCompany))
		{
		contactKey = iter.currentKey();
		return abEntry;
		}
	    }
	else
	    // the first and last name must be equal; they are equal
	    // if they are both empty or the strings match
	    if (((piLastEmpty && abEntry->getLastName().isEmpty()) ||
		 (abEntry->getLastName()==pilotAddress.getField(entryLastname)))
		&&
		((piFirstEmpty && abEntry->getFirstName().isEmpty()) ||
		 (abEntry->getFirstName()==pilotAddress.getField(entryFirstname) )))
		{
		contactKey = iter.currentKey();
		return abEntry;
		}
		 
	}
    return 0L;
    }

ContactEntry *AbbrowserConduit::_syncPilotEntry(PilotAddress &pilotAddress,
		     const QDict<ContactEntry> &abbrowserContacts)
    {
    FUNCTIONSETUP;
    
    QString abKey;
    // look for the possible match of names
    ContactEntry *abEntry =
	_findMatch(abbrowserContacts, pilotAddress, abKey);
    if (abEntry)
	{
	// if already found in abbrowser and kpilot, just assign
	// the kpilot id and save
	if (_equal(pilotAddress, *abEntry))
	    {
	    kdDebug() << fname << " both records already exist and are equal, just assigning the KPILOT_ID to the abbrowser entry" << endl;
	    abEntry->setCustomField("KPILOT_ID", QString::number(pilotAddress.getID()));
	    _saveAbEntry(*abEntry, abKey);
	    }
	else
	    {
	    kdDebug() << fname <<
		" both records exist (no id match) but conflict" << endl;
	    _handleConflict(&pilotAddress, abEntry, abKey);
	    }
	}
    else  // if not found in the abbrowser contacts, add it
	{
	kdDebug() << fname <<
	    " adding new pilot record to abbrowser => " << endl;
	showPilotAddress(pilotAddress);
	_addToAbbrowser(pilotAddress);
	}
    return abEntry;
    }

bool AbbrowserConduit::_prepare(QDict<ContactEntry> &abbrowserContacts,
				QMap<recordid_t, QString> &idContactMap,
				QList<ContactEntry> &newContacts,
				bool &abAlreadyRunning)
    {
    FUNCTIONSETUP;

    kdDebug() << fname << endl;
    
    readConfig();
    
    if (!fDcop)
	fDcop = KApplication::kApplication()->dcopClient();
    if (!fDcop)
	{
	kdDebug() << fname << " unable to connect to dcop" << endl;
	return false;
	}
    abAlreadyRunning = _startAbbrowser();
    _setAppInfo();
    
    
    // get the contacts from abbrowser
    if (!_getAbbrowserContacts(abbrowserContacts))
	{
	kdDebug() << fname <<
	    "AbbrowserConduit::_prepare unable to get contacts" << endl;
	return false;
	}

    // get the idContactMap and the newContacts
    // - the idContactMap maps Pilot unique record (address) id's to
    //   a Abbrowser ContactEntry; allows for easy lookup and comparisons
    // - created from the list of Abbrowser Contacts
    _mapContactsToPilot(abbrowserContacts, idContactMap, newContacts);

    kdDebug() << fname << " mapped pilot contacts, ready to sync" << endl;
    
    return true;
    }

void AbbrowserConduit::_backupDone()
    {
    FUNCTIONSETUP;
    
    if (!fBackupDone)
	{
	kdDebug() << fname << " setting FIRST_TIME_SYNCING to false" << endl;
	fBackupDone = true;
	KConfig& c = KPilotConfig::getConfig(AbbrowserConduitOptions::Group);
	c.writeEntry(AbbrowserConduitConfig::FIRST_TIME_SYNCING, !fBackupDone);
	setFirstTime(c, false);
	}

    kdDebug() << fname << " stop" << endl;
    }

void AbbrowserConduit::doBackup()
    {
    FUNCTIONSETUP;

    kdDebug() << fname << " start" << endl;
    
    QDict<ContactEntry> abbrowserContacts;
    QMap<recordid_t, QString> idContactMap;
    QList<ContactEntry> newContacts;
    bool abAlreadyRunning;
    if (!_prepare(abbrowserContacts, idContactMap, newContacts,
		  abAlreadyRunning))
	return ;

     PilotRecord *record = 0L;
    
    // iterate through all records in palm pilot
    for (int catIndex = 0; catIndex < 16;catIndex++)
	for (record = readNextRecordInCategory(_getCatId(catIndex));
	     record != NULL;
	     record = readNextRecordInCategory(_getCatId(catIndex)))
	    {
	    PilotAddress pilotAddress(fAddressAppInfo, record);
	    QString abKey = QString::null;
	
	    // if already stored in the abbrowser
	    if (idContactMap.contains( pilotAddress.id() ))
		{
		abKey = idContactMap[pilotAddress.id()];
		ContactEntry *abEntry = abbrowserContacts[abKey];
		assert(abEntry != NULL);
	    
		// if equal, do nothing since it is already there
		if (!_equal(pilotAddress, *abEntry))
		    {
		    kdDebug() << fname << " id = " << pilotAddress.id()  
			      << " match but not equal; pilot = '"
			      << pilotAddress.getField(entryFirstname) << "' '"
			      << pilotAddress.getField(entryLastname)
			      << "' abEntry = '"
			      << abEntry->findRef("fn").latin1()
			      << "'" << endl;
		
		    // if not equal, let the user choose what to do
		    _handleConflict( &pilotAddress, abEntry, abKey);
		    }
		}
	    else
		_syncPilotEntry( pilotAddress, abbrowserContacts );
	    }
    
    _saveAbChanges();
    _stopAbbrowser(abAlreadyRunning);
    _backupDone();
    }

int AbbrowserConduit::_getCatId(int catIndex) const
    {
    return fAddressAppInfo.category.ID[catIndex];
    }

void AbbrowserConduit::doSync()
    {
    FUNCTIONSETUP;

    kdDebug() << fname << " start" << endl;
	
    QDict<ContactEntry> abbrowserContacts;
    QMap<recordid_t, QString> idContactMap;
    QList<ContactEntry> newContacts;
    bool abAlreadyRunning;
    if (!_prepare(abbrowserContacts, idContactMap, newContacts,
		  abAlreadyRunning))
	return ;

    // perform syncing from palm to abbrowser
    PilotRecord *record = 0L;
    
    // iterate through all records in palm pilot
    for (int catIndex = 0; catIndex < 16;catIndex++)
	for (record = readNextRecordInCategory(_getCatId(catIndex));
	     record != NULL;
	     record = readNextRecordInCategory(_getCatId(catIndex)))
	    {
	    PilotAddress pilotAddress(fAddressAppInfo, record);
	    QString abKey = QString::null;
	    
	    //   if record not in abbrowser
	    if (!idContactMap.contains( pilotAddress.id() ))
		{
		//  ** I would like to use the the below algorithm, but there
		//  ** is no way currenty to know if a address has been deleted
		//  ** in abbrowser (unless the trash can functionality is
		//  ** done)
		//if record has been deleted in abbrowser
		//        then query user what to do
		//     else // not been deleted, so must be new
		//        add record to abbrowser

		// if pilotAddress id == 0, then probably syncing locally
		// and the pilotAddress has already been added
		if (pilotAddress.id() != 0)
		    {
		    // assume that it was deleted from the abbrowser since a
		    // backup should have been done before the first sync;
		    // if the pilot address was modified, then query the
		    // user what to do?
		    if (fBackupDone)
			{
			if (pilotAddress.isModified())
			    _handleConflict(&pilotAddress, NULL, abKey);
			else
			    _removePilotAddress(pilotAddress);
			}
		    else
			{
			ContactEntry *syncedContact = 
			    _syncPilotEntry(pilotAddress, abbrowserContacts);
			kdDebug() << fname << " SYNCED THE PILOT TO ABBROWSER"
				  << endl;
			if (syncedContact)
			    newContacts.remove(syncedContact);
			}
		    }
		}
	    else 
		{
		abKey = idContactMap[pilotAddress.id()];
		ContactEntry *abEntry = abbrowserContacts[abKey];
		assert(abEntry != NULL);
		// the record exists in the abbrowser and the palm
		if (pilotAddress.isModified() && abEntry->isModified())
		    {
		    // query the user what to do...
		    _handleConflict(&pilotAddress, abEntry, abKey);
		    }
		else  // record is either modified in the abbrowser or the palm
		    // or not modified at all
		    if (pilotAddress.isModified())
			{
			if (pilotAddress.isDeleted())
			    _removeAbEntry(abKey);
			else
			    {
			    // update abbrowser
			    _copy(*abEntry, pilotAddress);
			    _saveAbEntry( *abEntry, abKey );
			    }
			}
		    else if (abEntry->isModified())
			{
			kdDebug() << fname <<
			    "abEntry is modified but pilot wasn't; abEntry => "
				  << endl;
			showContactEntry(*abEntry);
			// update pilot
			_copy(pilotAddress, *abEntry);
			_savePilotAddress( pilotAddress, *abEntry );
			// set modified to false and save abEntry
			abEntry->setModified(false);
			_saveAbEntry(*abEntry, abKey);
			}
		// else not modified at either end, leave alone
		}
	    } // end pilot record loop

    // add all new entries from abbrowser to the palm pilot
    for (QListIterator<ContactEntry> newAbIter(newContacts);
	 newAbIter.current();++newAbIter)
	_addToPalm(*newAbIter.current());
    
    // add everything from pilot to abbrowser that is modified (or new)
    // add everything from abbrowser to pilot that is modified (or new) 
    // delete anything in pilot that is not in abbrowser
    //   (this assumes a backup has been done prior to the sync)
    _saveAbChanges();
    _backupDone();
    }

void AbbrowserConduit::doTest()
    {
    FUNCTIONSETUP;

    kdDebug() << fname << " start" << endl;
    
    QDict<ContactEntry> abbrowserContacts;
    QMap<recordid_t, QString> idContactMap;
    QList<ContactEntry> newContacts;
    bool abAlreadyRunning;
    if (!_prepare(abbrowserContacts, idContactMap, newContacts,
		  abAlreadyRunning))
	kdDebug() << fname
		  << " Test failed" << endl;
    else
	{
	kdDebug() << fname << " Test passed!" << endl;
	_stopAbbrowser(abAlreadyRunning);
	}
    }

const char *AbbrowserConduit::_getKabFieldForOther(const QString &desc) const
    {
    if (desc == "Assistant")
	return "X-AssistantsPhone";
    if (desc == "Other Phone")
	return "X-OtherPhone";
    if (desc == "Business Phone 2")
	return "X-BusinessPhone2";
    if (desc == "Business Fax")
	return "X-BusinessFax";
    if (desc == "Car Phone")
	return "X-CarPhone";
    if (desc == "Email 2")
	return "X-E-mail2";
    if (desc == "Home Fax")
	return "X-HomeFax";
    if (desc == "Home Phone 2")
	return "X-HomePhone2";
    if (desc == "Telex")
	return "X-Telex";
    if (desc == "TTY/TDD Phone")
	return "X-TtyTddPhone";
    return "X-OtherPhone";
    }

void AbbrowserConduit::readConfig()
    {
    FUNCTIONSETUP;

    KConfig& c = KPilotConfig::getConfig(AbbrowserConduitOptions::Group);
    getDebugLevel(c);
    fSmartMerge = c.readBoolEntry(AbbrowserConduitConfig::SMART_MERGE_ENTRY,
				  true);
    fConflictResolution = (EConflictResolution)
	c.readNumEntry(AbbrowserConduitConfig::CONFLICT_RESOLV_ENTRY,
		       eDoNotResolve);
    QString other = c.readEntry(AbbrowserConduitConfig::PILOT_OTHER_MAP_ENTRY,
				"Other Phone");
    fPilotOtherMap = _getKabFieldForOther(other);
    
    QString prefsStr =
	c.readEntry(AbbrowserConduitConfig::PILOT_STREET_TYPE_ENTRY,
		    "Home Street");
    fPilotStreetHome = true;
    prefsStr = prefsStr.left( prefsStr.find(' ') );
    if (prefsStr != "Home")
	fPilotStreetHome = false;

    prefsStr = c.readEntry(AbbrowserConduitConfig::PILOT_FAX_TYPE_ENTRY,
			   "Home Fax");
    fPilotFaxHome = true;
    prefsStr = prefsStr.left( prefsStr.find(' ') );
    if (prefsStr != "Home")
	fPilotFaxHome = false;

    //fBackupDone = !c.readBoolEntry(AbbrowserConduitConfig::FIRST_TIME_SYNCING,
    //			   true);
    fBackupDone = !getFirstTime(c);
#ifdef DEBUG
    if (debug_level & SYNC_MINOR)
	{
	kdDebug() << fname
		  << ": Settings "
		  << "fSmartMerge=" << fSmartMerge
		  << " fConflictResolution=" << fConflictResolution
		  << " fPilotOtherMap=" << fPilotOtherMap
		  << " fPilotStreetHome=" << fPilotStreetHome
		  << " fPilotFaxHome=" << fPilotFaxHome
		  << endl;
	}
#endif
    }
