#include <addressbooksyncee.h>
#include <todosyncee.h>
#include <eventsyncee.h>

#include "device.h"

using KSync::EventSyncee;
using KSync::AddressBookSyncee;
using KSync::TodoSyncee;

using namespace OpieHelper;

Device::Device() {
    m_model = Opie;
}
Device::~Device() {
}
int Device::distribution()const {
    return m_model;
}
void Device::setDistribution( int dist ) {
    m_model = dist;
}
QBitArray Device::supports( enum PIM pim) {
    QBitArray ar;
    switch( pim ) {
    case Calendar:
        ar = opieCal();
        break;
    case Addressbook:
        ar = opieAddr();
        break;
    case Todolist:
        ar = opieTo();
        break;
    }
    return ar;
}
QBitArray Device::opieCal() {
    QBitArray ar( EventSyncee::DtEnd+1 );
    ar[EventSyncee::Organizer] = false;
    ar[EventSyncee::ReadOnly ] = false; // we do not support the read only attribute
    ar[EventSyncee::DtStart  ] = true;
    ar[EventSyncee::Duration ] = true;
    ar[EventSyncee::Float    ] = true;
    ar[EventSyncee::Attendee ] = false;
    ar[EventSyncee::CreatedDate ] = false;
    ar[EventSyncee::Revision ] = false;
    ar[EventSyncee::Description ] = true;
    ar[EventSyncee::Summary] = ( m_model == Opie ); // if we're in opie mode we do support the summary!
    ar[EventSyncee::Category ] = true;
    ar[EventSyncee::Relations ] = false;
    ar[EventSyncee::ExDates ] = false; // currently we do not support the Exception to Recurrence
    ar[EventSyncee::Attachments ] = false;
    ar[EventSyncee::Secrecy ] = false;
    ar[EventSyncee::Resources ] = false; // we do not support resources
    ar[EventSyncee::Priority ] = false; // no priority for calendar
    ar[EventSyncee::Alarms ] = false; // Opie/Qtopia alarms are so different in nature
    ar[EventSyncee::Recurrence ] = true; // we do not support everything though...
    ar[EventSyncee::Location] = true;
    ar[EventSyncee::DtEnd ] = true;

    return ar;
}
QBitArray Device::opieAddr() {
    QBitArray ar(AddressBookSyncee::Emails +1 );

    ar[AddressBookSyncee::FamilyName] = true;
    ar[AddressBookSyncee::GivenName] = true;
    ar[AddressBookSyncee::AdditionalName] = true;
    ar[AddressBookSyncee::Prefix ] = false;
    ar[AddressBookSyncee::Suffix] = true;
    ar[AddressBookSyncee::NickName] = true;
    ar[AddressBookSyncee::Birthday] = true;
    ar[AddressBookSyncee::HomeAddress ] = true;
    ar[AddressBookSyncee::BusinessAddress]= true;
    ar[AddressBookSyncee::TimeZone] = false;
    ar[AddressBookSyncee::Geo ] = false;
    ar[AddressBookSyncee::Title ] = false;
    ar[AddressBookSyncee::Role ] = true;
    ar[AddressBookSyncee::Organization ] = true;
    ar[AddressBookSyncee::Note ] = true;
    ar[AddressBookSyncee::Url ] = false;
    ar[AddressBookSyncee::Secrecy ] = false;
    ar[AddressBookSyncee::Picture ] = false;
    ar[AddressBookSyncee::Sound ] = false;
    ar[AddressBookSyncee::Agent ] = false;
    ar[AddressBookSyncee::HomeNumbers] = true;
    ar[AddressBookSyncee::OfficeNumbers] = true;
    ar[AddressBookSyncee::Messenger ] = false;
    ar[AddressBookSyncee::PreferedNumber ] = false;
    ar[AddressBookSyncee::Voice ] = false;
    ar[AddressBookSyncee::Fax ] = false;
    ar[AddressBookSyncee::Cell ] = false;
    ar[AddressBookSyncee::Video ] = false;
    ar[AddressBookSyncee::Mailbox ] = false;
    ar[AddressBookSyncee::Modem ] = false;
    ar[AddressBookSyncee::CarPhone ] = false;
    ar[AddressBookSyncee::ISDN ] = false;
    ar[AddressBookSyncee::PCS ] = false;
    ar[AddressBookSyncee::Pager ] = false;
    ar[AddressBookSyncee::HomeFax] = true;
    ar[AddressBookSyncee::WorkFax] = true;
    ar[AddressBookSyncee::OtherTel] = false;
    ar[AddressBookSyncee::Category] = true;
    ar[AddressBookSyncee::Custom] = true;
    ar[AddressBookSyncee::Keys] = false;
    ar[AddressBookSyncee::Logo] = false;
    ar[AddressBookSyncee::Email] = true;
    ar[AddressBookSyncee::Emails] = true;
    return ar;
}
QBitArray Device::opieTo() {

}
