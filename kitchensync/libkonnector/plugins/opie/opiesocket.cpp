#include <qhostaddress.h>
#include <qtimer.h>
#include <qdom.h>
#include <qfile.h>

#include <opiedesktopsyncentry.h>
#include <koperations.h>
#include <kgenericfactory.h>
#include <qsocket.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <kurl.h>
#include <qregexp.h>

#include "opiesocket.h"
#include "opiecategories.h"
#include "opiehelper.h"

class OpieSocket::OpieSocketPrivate{
public:
    OpieSocketPrivate(){}
    QString pass;
    QString user;
    bool connected:1;
    bool startSync:1;
    bool isSyncing:1;
    bool isConnecting:1;
    QHostAddress src;
    QHostAddress dest;
    QSocket *socket;
    QTimer *timer;
    QString path;
    int mode;
    int getMode;
    enum Call{NOTSTARTED=0, HANDSHAKE=0, ABOOK, TODO, CALENDAR, TRANSACTIONS, FILES, DESKTOPS};
    enum Status {START=0, USER=1, PASS, CALL, NOOP, DONE , CONNECTED};
    QPtrList<KSyncEntry> m_sync;
    QValueList<OpieCategories> m_categories;
};

OpieSocket::OpieSocket(QObject *obj, const char *name )
    : QObject( obj, name )
{
    d = new OpieSocketPrivate;
    d->socket = 0;
    d->timer = 0;
    d->connected = false;
    d->startSync = false;
    d->isSyncing = false;
    d->isConnecting = false;
}
void OpieSocket::setUser(const QString &user )
{
    d->user = user;
}
void OpieSocket::setPassword(const QString &pass )
{
    d->pass = pass;
}
void OpieSocket::setSrcIP(const QHostAddress &src )
{
    d->src = src;
}

void OpieSocket::setDestIP(const QHostAddress &dest )
{
    d->dest = dest;
}
void OpieSocket::startUp() // start the connection
// and find out the the Homedir Path ;)
// start connecting
{
    kdDebug() << "OPieQCOPSocket" << endl;
    delete d->socket;
    d->socket = new QSocket(this, "OpieQCOPSocket" );
    connect( d->socket, SIGNAL(error(int) ), this,
	     SLOT(slotError(int) ) );
    connect( d->socket, SIGNAL(connected() ), this,
	     SLOT(slotConnected() ) );
    connect(d->socket, SIGNAL(connectionClosed() ), this,
	    SLOT(slotClosed() ) );
    connect(d->socket, SIGNAL(readyRead() ), this,
	    SLOT(process() ) );
    //connect(d->timer, SIGNAL(), this,
    //    SLOT(slotNOOP() ) );
    d->connected = false;
    d->startSync = false;
    d->isConnecting = true;
    d->m_categories.clear();
    d->socket->connectToHost(d->dest.toString(), 4243 );
}
bool OpieSocket::startSync()
{
    kdDebug() << "startSync " << endl;
    if( d->isSyncing ){
	kdDebug() << "isSyncing" << endl;
	return false;
    }
    d->isSyncing = true;
    d->getMode = d->NOTSTARTED;
    if(d->isConnecting ){
	kdDebug() << "is connecting tell it to startSync if in NOOP" << endl;
	d->startSync = true;
	return true;
    }
    if(!isConnected()  ){
	kdDebug() << "Not connected starting connection" << endl;
	startUp();
	d->startSync = true;
	return true;
    }
    kdDebug("start sync the normal way" );
    slotStartSync();
    return true;
}
bool OpieSocket::isConnected()
{
    if( d->mode == d->CALL || d->mode == d->NOOP || d->mode == d->CONNECTED ){
	return true;
    }else{
	return false;
    }
}
QByteArray OpieSocket::retrFile(const QString &path )
{
    if( d->mode == d->NOOP ){
    //stop the timer 

    }
}
bool OpieSocket::insertFile( const QString &fileName )
{
// files
}
void OpieSocket::write(const QString &, const QByteArray & )
{

}
void OpieSocket::write(QPtrList<KSyncEntry> )
{

}
void OpieSocket::write(QValueList<KOperations> )
{

}
void OpieSocket::slotError(int error )
{
    kdDebug() << "error" << endl;
}
void OpieSocket::slotConnected()
{
    d->connected = true;
    delete d->timer;
    d->mode = d->START;
}
void OpieSocket::slotClosed()
{

}
void OpieSocket::process()
{
    while(d->socket->canReadLine() ){
	QTextStream stream( d->socket );
	QString line = d->socket->readLine();
	kdDebug() << line.stripWhiteSpace() << endl;
	switch( d->mode ){
	    case d->START: {
		if( line.left(3) != QString::fromLatin1("220") ){
		    // error
		    d->socket->close();
		    d->mode = d->DONE;
		    d->connected = false;
		    d->isConnecting = false;
		}else {
		    stream << "USER " << d->user << "\r\n";
		    d->mode = d->USER;
		}
		break;
	    }
	    case d->USER:{
		if(line.left(3) != QString::fromLatin1("331" ) ){
                // wrong user name
		    d->socket->close();
		    d->mode = d->DONE;
		    d->connected = false;
		    d->isConnecting = false;
		}else{
		    stream << "PASS " << d->pass << "\r\n";
		    d->mode = d->PASS;
		}
		break;
	    }
	    case d->PASS:{
		if(line.left(3) != QString::fromLatin1("230" ) ){
                // error
		    d->socket->close();
		    d->mode = d->DONE;
		    d->connected = false;
		    d->isConnecting = false;
		}else{
                // ok start with the NOOP
                // start the Timer
		    kdDebug() << "start the NOOP" << endl;
		    d->mode = d->NOOP;
		    d->timer = new QTimer(this );
		    connect(d->timer, SIGNAL(timeout() ), this, SLOT(slotNOOP() ) );
		    d->timer->start(10000 );
		}
		break;
	    }
	    case d->CALL:
		kdDebug() << "CALL :)" << endl;
		manageCall( line );
		break;
	    case d->NOOP:
		//stream << "NOOP\r\n";
		d->isConnecting = false;
		if(!d->startSync ){
		    kdDebug() << "start NOOP" << endl;
		    d->mode = d->NOOP;
		    d->timer = new QTimer(this );
		    connect(d->timer, SIGNAL(timeout() ), this, SLOT(slotNOOP() ) );
		    d->timer->start(10000 );
		} else {
		    kdDebug () << "slotStartSync now" << endl;
		    slotStartSync();
		}
		break;
	};
    }
}
void OpieSocket::slotNOOP()
{
  QTextStream stream( d->socket );
  stream << "NOOP\r\n";
  kdDebug() << "NOOP" << endl;
  delete d->timer;
  d->timer = 0;
}

void OpieSocket::slotStartSync()
{
  kdDebug() << "slotStartSync()" << endl;
  delete d->timer;
  kdDebug() << "after deleting" << endl;
  d->startSync = false;
  QTextStream stream( d->socket );
  stream << "call QPE/System sendHandshakeInfo()\r\n";
  d->getMode = d->HANDSHAKE;
  d->mode = d->CALL; 
}
void OpieSocket::manageCall(const QString &line )
{
    QTextStream stream( d->socket );
    if( line.contains("200 Command okay" ) && d->getMode == d->HANDSHAKE ){
	return;
    }
    if( line.startsWith("CALL QPE/Desktop docLinks(QString)" ) ){
	kdDebug( ) << "CALL docLinks desktop entry" << endl;
	OpieHelper::self()->toOpieDesktopEntry( line, &d->m_sync, d->m_categories  );   
	return;
    }
    switch( d->getMode ){
	case d->HANDSHAKE: {
	    QStringList list = QStringList::split(QString::fromLatin1(" "), line );
	    kdDebug() << list[3] << endl;
	    d->path = list[3];
	    d->getMode = d->DESKTOPS;
	    stream << "call QPE/System startSync(QString) KitchenSync"; 
	    break;
	}
	case d->ABOOK:{
	    // start with the files use KIO::NetAcces for it simpleness first
	    kdDebug() << "Starting fetching" << endl;
	    QString tmpFileName;
	    KURL url;
	    url.setProtocol("ftp" );
	    url.setUser( d->user );
	    url.setPass( d->pass );
	    url.setHost( d->dest.toString() );
	    url.setPort( 4242 );
	    url.setPath(d->path + "/Settings/Categories.xml " );
	    //tmpFileName = QString::fromLatin1("/home/ich/categories.xml") ;
	    KIO::NetAccess::download( url, tmpFileName );
	    parseCategory(tmpFileName);
	    KIO::NetAccess::removeTempFile( tmpFileName );
	    url.setPath(d->path + "/Applications/addressbook/addressbook.xml" );
	    tmpFileName = QString::null;
	    //tmpFileName = "/home/ich/addressbook.xml";
	    KIO::NetAccess::download( url, tmpFileName );
	    OpieHelper::self()->toAddressbook( tmpFileName, &d->m_sync, d->m_categories  );
	    KIO::NetAccess::removeTempFile( tmpFileName );
	    QString todo;
	    url.setPath(d->path + "/Applications/todolist/todolist.xml" );
	    KIO::NetAccess::download(url, todo );
	    url.setPath(d->path + "/Applications/datebook/datebook.xml" );
	    KIO::NetAccess::download(url, tmpFileName );
	    OpieHelper::self()->toCalendar(todo, tmpFileName, &d->m_sync, d->m_categories );
	    KIO::NetAccess::removeTempFile( tmpFileName );
	    KIO::NetAccess::removeTempFile( todo );
	    // done with fetching
	    break;
	}
	case d->DESKTOPS:{
	    kdDebug() << "desktops" << endl;
	    stream << "call QPE/System getAllDocLinks()\r\n";
	    d->getMode = d->ABOOK;
	    break;
	}
    }
}
void OpieSocket::parseCategory(const QString &tempFile )
{
    kdDebug() << "parsing the categories" << endl;
    QDomDocument doc( "mydocument" );
    QFile f( tempFile );
    if ( !f.open( IO_ReadOnly ) ){
	kdDebug() << "can not open " <<tempFile << endl;
	return;
    }
    if ( !doc.setContent( &f ) ) {
	kdDebug() << "can not setContent" << endl;
	f.close();
	return;
    }
    f.close();
    // print out the element names of all elements that are a direct child
    // of the outermost element.
    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    kdDebug() << "NodeName: " << docElem.nodeName() << endl;
    if( docElem.nodeName() == QString::fromLatin1("Categories") ){
	kdDebug() << "Category" << endl;
	while( !n.isNull() ) {
	    QDomElement e = n.toElement(); // try to convert the node to an element.
	    if( !e.isNull() ) { // the node was really an element.
		kdDebug() << "tag name" << e.tagName() << endl;
              
		QString id = e.attribute("id" );
		QString app = e.attribute("app" );
		QString name = e.attribute("name");
		OpieCategories category( id, name, app );
		kdDebug() << "Cat " << id << " " << app << " " << name << endl;
		d->m_categories.append( category );
	    }
	    n = n.nextSibling();
	}
    }

}


QString OpieSocket::categoryById(const QString &id, const QString &app )
{
    QValueList<OpieCategories>::Iterator it;
    QString category;
    for( it = d->m_categories.begin(); it != d->m_categories.end(); ++it ){
	kdDebug() << "it :" << (*it).id() << "id:" << id << "ende"<<endl; 
	if( id.stripWhiteSpace() == (*it).id().stripWhiteSpace() ){
	    //if( app == (*it).app() ){
	    kdDebug() << "found category" << endl;
	    category = (*it).name();
	    break;
		//}
	}else {
	    kdDebug() << "not equal " << endl;
	}
    }
    kdDebug() << "CategoryById: " << category << endl;
    return category;
}




