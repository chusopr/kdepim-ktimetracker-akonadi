/*
    This file is part of libkcal.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_FOLDERLISTER_H
#define KCAL_FOLDERLISTER_H

#include <kurl.h>

#include <qvaluelist.h>
#include <qstring.h>
#include <qobject.h>

namespace KIO {
class DavJob;
class Job;
}

class KConfig;

namespace KCal {

class FolderLister : public QObject
{
    Q_OBJECT
  public:
    class Entry
    {
      public:
        Entry() : active( false ) {}
      
        typedef QValueList<Entry> List;
      
        QString id;
        QString name;
        bool active;
    };

    enum Type { AddressBook, Calendar };
  
    FolderLister( Type );
  
    void retrieveFolders( const KURL & );
  
    void setFolders( const Entry::List & );
    Entry::List folders() const { return mFolders; }

    QStringList activeFolderIds() const;
    bool isActive( const QString &id ) const;

    void setWriteDestinationId( const QString & );
    QString writeDestinationId() const { return mWriteDestinationId; }
        
    void readConfig( const KConfig * );
    void writeConfig( KConfig * );
    
  signals:
    void foldersRead();

  protected slots:
    void slotListJobResult( KIO::Job * );

  private:
    Type mType;
    KURL mUrl;
    Entry::List mFolders;
    QString mWriteDestinationId;
    KIO::DavJob *mListEventsJob;
};

}

#endif
