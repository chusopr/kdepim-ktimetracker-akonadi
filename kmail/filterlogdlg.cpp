/*
    This file is part of KMail.
    Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "filterlogdlg.h"
#include "filterlog.h"

#include <kdebug.h>
#include <klocale.h>
#include <ktextedit.h>

#include <qstringlist.h>


using namespace KMail;


FilterLogDialog::FilterLogDialog( QWidget * parent )
: KDialogBase( parent, "FilterLogDlg", false, i18n( "KMail Filter Log Viewer" ),
              User1|Close, Close, true, i18n("C&lear") )
{
  textEdit = new KTextEdit( this );
  setMainWidget( textEdit );
  textEdit->setReadOnly( true );

  QStringList logEntries = FilterLog::instance()->getLogEntries();
  for ( QStringList::Iterator it = logEntries.begin(); 
        it != logEntries.end(); ++it ) 
  {
    textEdit->append( *it );
  }
  
  connect(FilterLog::instance(), SIGNAL(logEntryAdded(QString)), 
          this, SLOT(slotLogEntryAdded(QString)));
  
  setInitialSize( QSize( 500, 300 ) );
}


void FilterLogDialog::slotLogEntryAdded( QString logEntry )
{
  textEdit->append( logEntry );
}


void FilterLogDialog::slotUser1()
{
  FilterLog::instance()->clear();
  textEdit->clear();
}


#include "filterlogdlg.moc"
