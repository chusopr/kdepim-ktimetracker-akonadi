#ifndef QTOPIA_OPIE_SOCKET_H
#define QTOPIA_OPIE_SOCKET_H

#include <qobject.h>

#include <stderror.h>
#include <stdprogress.h>
#include "konnector.h"

class KURL;
namespace KSync {

    class AddressBookSyncee;
    class EventSyncee;
    class TodoSyncee;
    class QtopiaSocket : public QObject{
        Q_OBJECT
    public:
        QtopiaSocket( QObject* obj, const char* name );
        ~QtopiaSocket();

        void setUser( const QString& user );
        void setPassword( const QString& pass );
        void setSrcIP( const QString& );
        void setDestIP( const QString& );
	void setModel( const QString& model, const QString& name );

        void startUp();
        void hangUP();

        bool startSync();
        bool isConnected();

        void write( SynceeList );
        void download( const QString& res );
        void setResources( const QStringList& );
        QString metaId()const;


    signals:
        void sync( SynceeList );
        void error( const Error& );
        void prog( const Progress& );

   private slots:
        void slotError(int);
        void slotConnected();
        void slotClosed();
        void slotNOOP();
        void process();
        void slotStartSync();

    private:
        class Private;
        Private *d;

    private:
        enum Type {
            AddressBook,
            TodoList,
            DateBook
        };
        KURL url( Type );
        KURL url( const QString& path );
        void writeCategory();
        void writeAddressbook( AddressBookSyncee*);
        void writeDatebook( EventSyncee* );
        void writeTodoList( TodoSyncee* );
        void readAddressbook();
        void readDatebook();
        void readTodoList();

        /* for processing the connection and authentication */
        void start(const QString& );
        void user( const QString& );
        void pass( const QString& );
        void call( const QString& );
	void flush( const QString& );
        void noop( const QString& );


        void handshake( const QString& );
        void download();
        void initSync(const QString& );

        void initFiles();
        QString partnerIdPath()const;
        void readTimeZones();

        /* download relative from the home dir */
        bool downloadFile( const QString& str,  QString& newDest);
	int m_flushedApps;

    };
};


#endif
