#ifndef KSYNC_GENERICSYNCEE_H
#define KSYNC_GENERICSYNCEE_H

#include <qstring.h>
#include <qstringlist.h>

#include "syncer.h"

/**
 * this is my first template ever
 */
namespace KSync {
    template <class Entry= SyncEntry>
    class SyncTemplate : public Syncee {
    public:
        typedef QPtrList<Entry> PtrList;
        SyncTemplate() : Syncee()  { };
        ~SyncTemplate() { };
/*        QString type() const { return QString::fromLatin1(typeName); }*/
        /**
         * basic clone implementation
         */
        Syncee* clone() {
            SyncTemplate* temp = new SyncTemplate();
            temp->setSyncMode( syncMode() );
            Entry* entry;
            for ( entry = mList.first(); entry != 0; entry = mList.next() ) {
                temp->addEntry( entry->clone() );
            }
            return temp;
        };
        bool read() { return true;}
        bool write() { return true; }
        SyncEntry* firstEntry() {
            return mList.first();
        }
        SyncEntry* nextEntry() {
            return mList.next();
        }
        SyncEntry::PtrList added() {
            return find( SyncEntry::Added );
        }
        SyncEntry::PtrList modified() {
            return find( SyncEntry::Modified );
        }
        SyncEntry::PtrList removed() {
            return find(SyncEntry::Removed );
        }
        void addEntry( SyncEntry* entry ) {
            Entry* tempEntry = dynamic_cast<Entry*> ( entry );
            if ( tempEntry == 0l )
                return;
            tempEntry->setSyncee( this );
            if ( tempEntry->state() != SyncEntry::Undefined ) {
                if (hasChanged( tempEntry ) )
                    tempEntry->setState( SyncEntry::Modified );
            }
            mList.append( tempEntry );
        }
        void removeEntry( SyncEntry* entry ) {
            mList.remove( entry );
        }

    protected:
        SyncEntry::PtrList find( int state ) {
            SyncEntry::PtrList found;
            Entry* entry;
            for (entry = mList.first(); entry != 0; entry = mList.next() ) {
                if ( entry->state() == state )
                    found.append( entry );
            }
            return found;
        }
        PtrList mList;

    };
}


#endif
