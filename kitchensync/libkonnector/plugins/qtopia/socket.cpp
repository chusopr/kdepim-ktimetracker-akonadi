#include <qsocket.h>
#include <qfile.h>
#include <qdir.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kfileitem.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include <addressbooksyncee.h>
#include <eventsyncee.h>
#include <todosyncee.h>

#include <idhelper.h>

#include "categoryedit.h"
#include "opiecategories.h"
#include "desktop.h"
#include "datebook.h"
#include "todo.h"
#include "addressbook.h"

#include "socket.h"

using namespace KSync;

class QtopiaSocket::Private {
public:
    enum CallIt {
        NotStarted = 0,
        Handshake  = 0,
        ABook,
        Todo,
        Calendar,
        Transactions,
        Files,
        Desktops
    };
    enum Status {
        Start = 0,
        User,
        Pass,
        Call,
        Noop,
        Done,
        Connected
    };
    Private(){}

    QString pass;
    QString user;
    bool connected    : 1;
    bool startSync    : 1;
    bool isSyncing    : 1;
    bool isConnecting : 1;
    bool first        : 1;
    bool meta         : 1;
    QString src;
    QString dest;
    QSocket* socket;
    QTimer* timer;
    QString path;
    int mode;
    int getMode;
    Syncee::PtrList m_sync;

    QValueList<OpieCategories> categories;
    QString partnerId;
    QStringList files;
    QString tz;
    OpieHelper::CategoryEdit* edit;
    KonnectorUIDHelper* helper;

};
namespace {
    void parseTZ( const QString& fileName,  QString& tz );
};
/**
 * QtopiaSocket is somehow a state machine
 * during authentication
 * then it takes care of fethcing and converting
 */
QtopiaSocket::QtopiaSocket( QObject* obj, const char* name )
    : QObject( obj, name )
{
    d = new Private;
    d->socket = 0;
    d->timer  = 0;
    d->connected    = false;
    d->startSync    = false;
    d->isSyncing    = false;
    d->isConnecting = false;
    d->meta         = true ;
    d->helper  = 0;
    d->edit    = 0;
    d->first   = false;
}
QtopiaSocket::~QtopiaSocket() {
    delete d;
}
void QtopiaSocket::setUser( const QString& user ) {
    d->user = user;
}
void QtopiaSocket::setPassword( const QString& pass ) {
    d->pass = pass;
}
void QtopiaSocket::setSrcIP( const QString& src) {
    d->src = src;
}
void QtopiaSocket::setDestIP( const QString& dest) {
    d->dest = dest;
}
void QtopiaSocket::startUp() {
    delete d->socket;
    d->socket = new QSocket(this, "Qtopia Socket" );

    /* now connect to some slots */
    connect(d->socket, SIGNAL(error(int) ),
            this, SLOT(slotError(int) ) );
    connect(d->socket, SIGNAL(connected() ),
            this, SLOT(slotConnected() ) );
    connect(d->socket, SIGNAL(connectionClosed() ),
            this, SLOT(slotClosed() ) );
    connect(d->socket, SIGNAL(readyRead() ),
            this, SLOT(process() ) );

    d->connected    = false;
    d->startSync    = false;
    d->isConnecting = true;

    d->categories.clear();
    d->isSyncing = false;
    d->socket->connectToHost(d->dest, 4243 );
}
bool QtopiaSocket::startSync() {
    if ( d->isSyncing )
        return false;
    d->isSyncing = true;
    d->getMode   = d->NotStarted;

    if (d->isConnecting ) {
        d->startSync = true;
        return true;
    }
    if (!isConnected() ) {
        startUp();
        d->startSync = true;
        return true;
    }
    slotStartSync();

    return true;
}
/*
 * check if we're connected
 */
bool QtopiaSocket::isConnected() {
    if ( d->connected || d->mode == d->Call || d->mode  == d->Noop || d->mode == d->Connected )
        return true;
    else
        return false;
}
QByteArray QtopiaSocket::retrFile( const QString& fileName ) {
    QByteArray array;

    if( isConnected() ) {
        QString file;
        KURL ur = url( fileName );
        if (  KIO::NetAccess::download( ur,  file ) ) {
            QFile file2( file );
            if ( file2.open( IO_ReadOnly ) )
                array = file2.readAll();
        }
        KIO::NetAccess::removeTempFile( file );
    }
    return array;
}
Syncee* QtopiaSocket::retrEntry( const QString&) {
    return 0l;
}
bool QtopiaSocket::insertFile( const QString& file) {
    if (!d->connected )
        return false;

    d->files.append( file );
    return true;
}
void QtopiaSocket::write( const QString& str, const QByteArray& array) {
    KTempFile temp(locateLocal("tmp", "opie-konn-tmp"),  "konn" );
    QFile* file = temp.file();

    if ( file!=0 ) {
        file->writeBlock( array  );
        temp.close();

        KURL ur = url( str );
        KIO::NetAccess::upload(temp.name(), ur );
    }
    temp.unlink();
}
void QtopiaSocket::write( Syncee::PtrList list) {
    Syncee *syncee;

    /*
     * For all Syncees we see if it
     * is one of the Opie built in functionality
     */
    for ( syncee = list.first(); syncee != 0l; syncee = list.next() ) {
        if ( syncee->type() == QString::fromLatin1("AddressBookSyncee") ) {
            AddressBookSyncee* abSyncee = dynamic_cast<AddressBookSyncee*>(syncee );
            writeAddressbook( abSyncee );
        }else if ( syncee->type() == QString::fromLatin1("EventSyncee") ) {
            EventSyncee* evSyncee = dynamic_cast<EventSyncee*>(syncee);
            writeDatebook( evSyncee );
        }else if ( syncee->type() == QString::fromLatin1("TodoSyncee") ) {
            TodoSyncee* toSyncee = dynamic_cast<TodoSyncee*>(syncee);
            writeTodoList( toSyncee );
        }
    }
    /*
     * so that a clear frees the Syncees
     */
    list.setAutoDelete( true );
    list.clear();

    /*
     * write new category informations
     */
    writeCategory();
    d->helper->save();

    /*
     * tell Opie/Qtopia that we're ready
     */
    QTextStream stream( d->socket );
    stream << "call QPE/System stopSync()" << endl;
    d->isSyncing = false;
}
/*
 * write back some Operations later ;)
 */
void QtopiaSocket::write( KOperations::ValueList ) {

}
QString QtopiaSocket::metaId()const {
    return d->partnerId;
};
void QtopiaSocket::slotError(int error) {
    d->isSyncing = false;
    d->isConnecting = false;

    emit stateChanged( false );
    emit errorKonnector( error, i18n("Connection error") );
}
void QtopiaSocket::slotConnected() {
    d->connected = true;
    delete d->timer;
    d->mode = d->Start;
}
void QtopiaSocket::slotClosed() {
    d->connected    = false;
    d->isConnecting = false;
    d->isSyncing    = false;
    emit stateChanged( false );
}
void QtopiaSocket::slotNOOP() {
    QTextStream stream( d->socket );
    stream << "NOOP" << endl;
}
void QtopiaSocket::process() {
    while ( d->socket->canReadLine() ) {
        QTextStream stream( d->socket );
        QString line = d->socket->readLine();
        kdDebug() << line << endl;
        kdDebug() << d->mode << endl;
        switch( d->mode ) {
        case d->Start:
            start(line);
            break;
        case d->User:
            user(line);
            break;
        case d->Pass:
            pass(line);
            break;
        case d->Call:
            call(line);
            break;
        case d->Noop:
            noop(line);
            break;
        }
    }
}
void QtopiaSocket::slotStartSync() {
    d->startSync = false;
    QTextStream stream( d->socket );
    stream << "call QPE/System sendHandshakeInfo()" << endl;
    d->getMode = d->Handshake;
    d->mode = d->Call;
}
KURL QtopiaSocket::url( Type  t) {
    QString uri;
    uri = d->path + "/Applications/";
    switch( t ) {
    case AddressBook:
        uri += "addressbook/addressbook.xml";
        break;
    case TodoList:
        uri += "todolist/todolist.xml";
        break;
    case DateBook:
        uri += "datebook/datebook.xml";
        break;
    }
    return url( uri );
}
KURL QtopiaSocket::url( const QString& path ) {
    KURL url;
    url.setProtocol("ftp" );
    url.setUser( d->user );
    url.setPass( d->pass );
    url.setHost( d->dest );
    url.setPort( 4242 );
    url.setPath( path );

    return url;
}
/*
 * write the categories file
 */
void QtopiaSocket::writeCategory() {
    QString fileName = QDir::homeDirPath() + "/.kitchensync/meta/" +d->partnerId;
    d->edit->save( fileName );
    KURL uri = url(  d->path + "/Settings/Categories.xml" );
    KIO::NetAccess::upload( fileName + "/categories.xml",  uri );
}
void QtopiaSocket::writeAddressbook( AddressBookSyncee* ) {

}
void QtopiaSocket::writeDatebook( EventSyncee* ) {

}
void QtopiaSocket::writeTodoList( TodoSyncee* ) {

}
void QtopiaSocket::readAddressbook() {

}
void QtopiaSocket::readDatebook() {

}
void QtopiaSocket::readTodoList() {

}
void QtopiaSocket::readPartner() {

}

void QtopiaSocket::start(const QString& line ) {
    QTextStream stream( d->socket );
    if ( line.left(3) != QString::fromLatin1("220") ) {
        // something went wrong
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    }else{
        /*
         * parse the uuid
         * here
         */
        QStringList list = QStringList::split(";", line );
        qWarning(list[1] );
        QString uid = list[1];
        uid = uid.mid(11, uid.length()-12 );
        d->partnerId = uid;
        stream << "USER " << d->user << endl;
        d->mode = d->User;
    }
}

void QtopiaSocket::user( const QString& line) {
    QTextStream stream( d->socket );
    if ( line.left(3) != QString::fromLatin1("331") ) {
        // wrong user name
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    }else{
        stream << "PASS " << d->pass << endl;
        d->mode = d->Pass;
    }
}
void QtopiaSocket::pass( const QString& line) {
    if ( line.left(3) != QString::fromLatin1("230") ) {
        // wrong password
        d->socket->close();
        d->mode = d->Done;
        d->connected    = false;
        d->isConnecting = false;
    }else {
        kdDebug() << "Konnected" << endl;
        d->mode = d->Noop;
        QTimer::singleShot(10000, this, SLOT(slotNOOP() ) );
        emit stateChanged( true );
    }
}
void QtopiaSocket::call( const QString& line) {
    if ( line.contains("220 Command okay" ) &&
         ( d->getMode == d->Handshake || d->getMode == d->ABook ) )
        return;

    if ( line.startsWith("CALL QPE/Desktop docLinks(QString)") ) {
        OpieHelper::Desktop desk( d->edit );
        Syncee* sync = desk.toSyncee( line );
        if ( sync )
            d->m_sync.append( sync );
    }
    switch( d->getMode ) {
    case d->Handshake:
        handshake(line);
        break;
    case d->ABook:
        download();
        break;
    case d->Desktops:
        initSync( line );
        break;
    }
}
void QtopiaSocket::noop( const QString&) {
    d->isConnecting = false;
    if (!d->startSync ) {
        d->mode = d->Noop;
        QTimer::singleShot(10000, this, SLOT(slotNOOP() ) );
    }else
        slotStartSync();
}
void QtopiaSocket::handshake( const QString& line) {
    QTextStream stream( d->socket );
    QStringList list = QStringList::split( QString::fromLatin1(" "), line );
    d->path = list[3];
    d->getMode = d->Desktops;
    stream << "call QPE/System startSync(QString) KitchenSync" << endl;
}
void QtopiaSocket::download() {
    readAddressbook();
    readDatebook();
    readTodoList();

    /*
     * we're all set now
     * start sync
     * and clear our list
     */
    emit sync( d->m_sync );
    d->mode = d->Noop;
    d->m_sync.clear();
}
void QtopiaSocket::initSync( const QString& ) {
    QString tmpFileName;
    downloadFile( "/Settings/Categories.xml", tmpFileName );

    /* Category Edit */
    delete d->edit;
    d->edit = new OpieHelper::CategoryEdit( tmpFileName );
    KIO::NetAccess::removeTempFile( tmpFileName );

    /* KonnectorUIDHelper */
    delete d->helper;
    d->helper = new KonnectorUIDHelper( partnerIdPath() );

    /* TimeZones */
    readTimeZones();

    /*
     * now we can progress during sync
     */
    QTextStream stream( d->socket );
    stream << "call QPE/System getAllDocLinks()" << endl;
    d->getMode  = d->ABook;
}
void QtopiaSocket::initFiles() {
    QDir di( QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId );
    /*
     * if our meta path exists do not recreate it
     */
    if (di.exists()  )
        return;

    QDir dir;
    dir.mkdir(QDir::homeDirPath() + "/.kitchensync");
    dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta");
    dir.mkdir(QDir::homeDirPath() + "/.kitchensync/meta/" + d->partnerId );
}
QString QtopiaSocket::partnerIdPath()const {
    QString str = QDir::homeDirPath();
    str += "/.kitchensync/meta/";
    str += d->partnerId;

    return str;
};
void QtopiaSocket::readTimeZones() {
    QString tmpFileName;
    /* read the time zone from file */
    if ( downloadFile("/Settings/locale.conf", tmpFileName ) ) {
        QFile file( tmpFileName );
        if ( file.open( IO_ReadOnly ) ) {
            QTextStream stream ( &file );
            QString line;
            while ( !stream.atEnd() ) {
                line = stream.readLine();
                if ( line.startsWith("Timezone = ") )
                    d->tz = line.mid(11);
            }
        }
    }else
        d->tz = "America/New_York";
}
bool QtopiaSocket::downloadFile( const QString& str, QString& dest ) {
    KURL uri = url( d->path + str );
    return KIO::NetAccess::download( uri, dest );
}
#include "socket.moc"
