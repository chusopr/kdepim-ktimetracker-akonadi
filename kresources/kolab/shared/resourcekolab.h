 /*
    This file is part of the kolab resource.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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

#ifndef RESOURCEKOLAB_H
#define RESOURCEKOLAB_H

#include <qstring.h>
#include <qmap.h>

class QCString;
class QStringList;
class KURL;

namespace ResourceKolab {

class KMailConnection;

/**
  This class provides the kmail connectivity for IMAP resources.
*/
class ResourceKolabBase {
public:
  ResourceKolabBase( const QCString& objId );
  virtual ~ResourceKolabBase();

  // TODO: Update these methods
  // These are the methods called by KMail when the resource changes
  virtual bool addIncidence( const QString& type, const QString& resource,
                             const QString& ical ) = 0;
  virtual void deleteIncidence( const QString& type, const QString& resource,
                                const QString& uid ) = 0;
  virtual void slotRefresh( const QString& type,
                            const QString& resource ) = 0;
  virtual void subresourceAdded( const QString& type,
                                 const QString& resource ) = 0;
  virtual void subresourceDeleted( const QString& type,
                                   const QString& resource ) = 0;

protected:
  /// Do the connection to KMail.
  bool connectToKMail() const;

  // These are the KMail dcop function connections. The docs here say
  // "Get", which here means that the first argument is the return arg

  /// List all folders with a certain annotation. Returns a QMap with
  /// resourcename/writable pairs
  bool kmailSubresources( QMap<QString, bool>& lst,
                          const QString& annotation ) const;

  /// Get the mimetype attachments from this folder. Returns a
  /// QMap with serialNumber/attachment pairs.
  bool kmailIncidences( QMap<QString, QString>& lst, const QString& mimetype,
                        const QString& resource ) const;

  /// Get an attachment from a mail. Returns a URL to it. This can
  /// be called by the resource after obtaining the incidence.
  /// The resource must delete the temp file.
  bool kmailGetAttachment( KURL& url, const QString& resource,
                           const QString& sernum,
                           const QString& filename ) const;

  /// Add a new incidence. The list of attachments are URLs.
  bool kmailAddIncidence( const QString& resource, const QString& xml,
                          const QStringList& attachments );

  /// Delete an incidence.
  bool kmailDeleteIncidence( const QString& resource, const QString& sernum );

  /// Update an incidence. The list of attachments are URLs.
  bool kmailUpdate( const QString& resource, const QString& sernum,
                    const QString& xml, const QStringList& attachments,
                    const QStringList& deletedAttachments );

  /// Get the full path of the config file.
  QString configFile( const QString& type ) const;

  /// If only one of these is writable, return that. Otherwise return null.
  QString findWritableResource( const QMap<QString, bool>& resources,
                                const QString& type );
  /// If only one of these is writable, return that. Otherwise return null.
  QString findWritableResource( const QStringList& resources,
                                const QString& type );

  bool mSilent;

private:
  mutable KMailConnection* mConnection;
};

}

#endif // RESOURCEKOLAB_H
