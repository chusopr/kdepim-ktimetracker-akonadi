
#include <kdebug.h>
#include <qregexp.h>

#include "opiehelper.h"

namespace {
QString categoryById(const QString &id, const QString &app, QValueList<OpieCategories> cate )
{
    QValueList<OpieCategories>::Iterator it;
    QString category;
    for( it = cate.begin(); it != cate.end(); ++it ){
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
};

};

void OpieHelper::toOpieDesktopEntry( const QString &str, QPtrList<KSyncEntry> *entry, const QValueList<OpieCategories> &cat )
{
    QString string ( str );
    string.remove(0, 35 );
    string.replace(QRegExp("&amp;"), "&" );
    string.replace(QRegExp("&0x20;"), " ");
    string.replace(QRegExp("&0x0d;"), "\n");
    string.replace(QRegExp("&0x0a;"), "\r");
    string.replace(QRegExp("\r\n"), "\n" ); // hell we're on unix
    kdDebug() << string << endl;
    if(!string.contains("[Desktop Entry]")  )
	return;
    QStringList list = QStringList::split('\n', string );
    QString name, category, type, fileName, size;
    QStringList::Iterator it;
    it = list.begin(); // desktopentry;
 
    list.remove( it );
    // yuyuyi
    OpieDesktopSyncEntry *entr;
    for( it = list.begin(); it != list.end(); ++it ){
	QString con( (*it) );
	kdDebug() << con << "ende" << endl;
	con = con.stripWhiteSpace();
	if( con.startsWith("Categories = " ) ){
	    con = con.remove(0, 13 );
	    con = con.remove( con.length() -1, 1 );
	    category = categoryById( con, "Document View", cat );
	}else if(con.startsWith("Name =" ) ){

	}
	if( (*it).stripWhiteSpace() == "[Desktop Entry]" ){ // ok next entry starts
	    entr = new OpieDesktopSyncEntry(category, fileName, name, type, size );
	    entry->append( entr );
	}
    }
//ok here we got one more entry I guess I'm wrong here
    entr = new OpieDesktopSyncEntry(category, fileName, name, type, size );
    entry->append( entr );
}

 void OpieHelper::toCalendar(const QString &fileName, QPtrList<KSyncEntry> *list,const QValueList<OpieCategories> & )
{
    QPtrList<KAlendarSyncEntry> entry;
    //return entry;
}
void OpieHelper::toAddressbook(const QString &fileName, QPtrList<KSyncEntry> *list,const QValueList<OpieCategories> &)
{
    QPtrList<KAddressbookSyncEntry> entry;
    //return entry;
}

