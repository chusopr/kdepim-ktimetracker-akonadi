/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KSYNC_KONNECTOR_H
#define KSYNC_KONNECTOR_H

#include <qcstring.h>
#include <qiconset.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qptrlist.h>

#include <kresources/resource.h>

#include <syncee.h>
#include <synceelist.h>

#include "stderror.h"
#include "stdprogress.h"

namespace KSync {

class KonnectorInfo;
class Kapabilities;
class ConfigWidget;

/**
  This class provides the interface for a Konnector. A Konnector is a class
  responsible for communication with a certain kind of PIM data source. It does
  the actual transport of the data and conversion of data is needed. It provides
  the PIM data in a generic way using the KSyncee class which is suitable for
  further processing within KDE, e.g. for syncing or being accessed by a
  kioslave.
*/
class Konnector : public KRES::Resource
{
    Q_OBJECT
  public:
    /**
      Construct Konnector from information in config file.
    */
    Konnector( const KConfig *config );

    /**
      Write configuration to config file.
    */
    void writeConfig( KConfig *config );

    /**
      Destruct Konnector object.
    */
    virtual ~Konnector();

    /**
      Request list of Syncee objects containing PIM data from connected entity.
      The response is sent with the signal synceesRead(). If an error occurs
      during read the signal synceeReadError() is emitted.

      @return true, if request could successfully be started, false otherwise.
    */
    virtual bool readSyncees() = 0;

    /**
      Request to write back data contained in Syncee objects delivered by
      synceesAvailable() to connected entity. The result of the write is sent
      with the signal synceesWritten(). If an error occurs during write the
      signal synceeWriteError() is emitted.

      @return true, if request could successfully be started, false otherwise.
    */
    virtual bool writeSyncees() = 0;

  signals:
    /**
      Emitted when Syncee list becomes available as response to
      requestSyncees().
    */
    void synceesRead( Konnector *, const SynceeList & );

    /**
      Emitted when an error occurs during read.
    */
    void synceeReadError( Konnector * );

    /**
      Emitted when Syncee list was successfully written back to connected
      entity.
    */
    void synceesWritten( Konnector * );

    /**
      Emitted when an error occurs during write.
    */
    void synceeWriteError( Konnector * );

  public:
    /**
      Return capabilities of the Konnector.
    */
    virtual Kapabilities capabilities() = 0;
    /**
      Set capabilities.
      TODO: Shouldn't this be internally done by the implementation?
    */
    virtual void setCapabilities( const Kapabilities &kaps ) = 0;

    /**
      Start sync. By calling this function a synchronisation process is
      initiated. As response to this function a list of Syncees is delivered by
      the sync() signal.

      @return False, if error occurred while trying to start syncing, otherwise
              true.
    */
//    virtual bool startSync() = 0;
    /**
      Start restore.
    */
    virtual bool startRestore( const QString &path ) = 0;
    /**
      Start backup.
    */
    virtual bool startBackup( const QString &path ) = 0;

    /**
      Connect device. Return true, if device could be connected.
    */
    virtual bool connectDevice() = 0;
    /**
      Disconnect device.
    */
    virtual bool disconnectDevice() = 0;

    /**
      Return meta information about this Konnector.
    */
    virtual KonnectorInfo info() const = 0;

    /**
      Return configuration widget of this Konnector.
    */
    virtual ConfigWidget *configWidget( const Kapabilities &, QWidget *parent,
                                        const char *name = 0 );
    /**
      Return configuration widget of this Konnector.
    */
    virtual ConfigWidget *configWidget( QWidget *parent, const char *name = 0 );

    virtual void add( const QString &res );
    virtual void remove( const QString &res );
    virtual QStringList resources() const;

    /**
     * can be a file, a resource, a Syncee...
     */
    virtual void download( const QString &resource ) = 0;

    /**
     * the Syncees that are supported builtIn
     */
    virtual QStringList builtIn() const;
    bool isConnected() const;

  protected:
    void progress( const Progress & );
    void error( const Error & );

  protected:
    /**
      Write data contained in list of Syncee object back to device.
    */
//    virtual void write( Syncee::PtrList ) = 0;

  signals:
    /**
      This signal is emitted to provide the results to a startSync() call.
    */
//    void sync( Konnector *, Syncee::PtrList );

    void sig_progress( Konnector *, const Progress & );
    void sig_error( Konnector *, const Error & );
    void sig_downloaded( Konnector *, const SynceeList & );

  private:
    QStringList m_resources;
    bool m_isCon : 1;
};

}

#endif
