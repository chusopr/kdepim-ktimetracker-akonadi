 /*
    This file is part of kdepim.

    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>


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
#ifndef KABC_EXCHANGEADDRESSBOOKADAPTOR_H
#define KABC_EXCHANGEADDRESSBOOKADAPTOR_H

#include "exchangeglobals.h"

#include <groupwareresourcejob.h>
#include <davaddressbookadaptor.h>

#include <folderlister.h>

#include <kabc/addressee.h>
#include <kurl.h>
#include <qdom.h>

namespace KABC {

class ExchangeAddressBookUploadItem : public KPIM::GroupwareUploadItem
{
  public:
    ExchangeAddressBookUploadItem( AddressBookAdaptor *adaptor, KABC::Addressee addr, UploadType type );
    virtual ~ExchangeAddressBookUploadItem() {}
    virtual KIO::TransferJob *createUploadJob(
            KPIM::GroupwareDataAdaptor *adaptor, const KURL &url );

  protected:
    ExchangeAddressBookUploadItem( UploadType type ) : KPIM::GroupwareUploadItem( type ) {}
    QDomDocument mDavData;
};

class ExchangeAddressBookAdaptor : public DavAddressBookAdaptor
{
  public:
    ExchangeAddressBookAdaptor();

    void adaptDownloadUrl( KURL &url );
    void adaptUploadUrl( KURL &url );
    QString mimeType() const { return "message/rfc822"; }
    QCString identifier() const { return "KABCResourceExchange"; }
    QString defaultNewItemName( KPIM::GroupwareUploadItem *item );


    
    QString extractFingerprint( KIO::TransferJob *job, const QString &rawText )
        { return ExchangeGlobals::extractFingerprint( job, rawText ); }

    
    
    // Creating Jobs
    KIO::Job *createListFoldersJob( const KURL &url )
        { return ExchangeGlobals::createListFoldersJob( url ); }
    KIO::TransferJob *createListItemsJob( const KURL &url )
        { return ExchangeGlobals::createListItemsJob( url ); }
    KIO::TransferJob *createDownloadItemJob( const KURL &url, KPIM::GroupwareJob::ContentType ctype );
    KIO::Job *createRemoveItemsJob( const KURL &uploadurl, KPIM::GroupwareUploadItem::List deletedItems )
        { return ExchangeGlobals::createRemoveItemsJob( uploadurl, deletedItems ); }

        
    
    // Interpreting Jobs
    bool interpretListItemsJob( KIO::Job *job, QStringList &currentlyOnServer,
            QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload )
       { return ExchangeGlobals::interpretListItemsJob( this, job, currentlyOnServer, itemsForDownload ); }
    KABC::Addressee::List interpretDownloadItemJob( KIO::TransferJob *job, const QString &rawText );
    

    
    KPIM::GroupwareUploadItem *newUploadItem( KABC::Addressee addr,
            KPIM::GroupwareUploadItem::UploadType type );




    bool getFolderHasSubs( const QDomNode &folderNode )
        { return ExchangeGlobals::getFolderHasSubs( folderNode ); }
    KPIM::FolderLister::FolderType getFolderType( const QDomNode &folderNode )
        { return ExchangeGlobals::getFolderType( folderNode ); }
};

}

#endif
