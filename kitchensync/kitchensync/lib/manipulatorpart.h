/*
� � This file is part of the OPIE Project
� � Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
� �                2002 Maximilian Rei� <harlekin@handhelds.org>



 �             =.
� � � � � � �.=l.
� � � � � �.>+-=
�_;:, � � .> � �:=|.         This library is free software; you can
.> <`_, � > �. � <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- � :           the terms of the GNU Library General Public
.="- .-=="i, � � .._         License as published by the Free Software
�- . � .-<_> � � .<>         Foundation; either version 2 of the License,
� � �._= =} � � � :          or (at your option) any later version.
� � .%`+i> � � � _;_.
� � .i_,=:_. � � �-<s.       This library is distributed in the hope that
� � �+ �. �-:. � � � =       it will be useful,  but WITHOUT ANY WARRANTY;
� � : .. � �.:, � � . . .    without even the implied warranty of
� � =_ � � � �+ � � =;=|`    MERCHANTABILITY or FITNESS FOR A
� _.=:. � � � : � �:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= � � � = � � � ;      Library General Public License for more
++= � -. � � .` � � .:       details.
�: � � = �...= . :.=-
�-. � .:....=;==+<;          You should have received a copy of the GNU
� -_. . . � )=. �=           Library General Public License along with
� � -- � � � �:-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/



#ifndef KSYNC_MANIPULATOR_H
#define KSYNC_MANIPULATOR_H

#include <qpixmap.h>
#include <qstring.h>
#include <kparts/part.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include <kdebug.h>

#include <syncer.h>
#include <error.h>
#include <progress.h>

//#include "ksync_mainwindow.h"
#include "ksync_profile.h"

namespace KSync {

class KSyncMainWindow;

enum SyncStatus { SYNC_START=0, SYNC_PROGRESS=1,  SYNC_DONE=2,  SYNC_FAIL };

/**
 * the ManipulatorPart is loaded into the KitchenSync
 * Shell. Every ManipulatorPart can provide a QWidget
 * and a config dialog.
 * It can access the MainWindow through core()
 * and will be asked to sync!
 * It needs to emit done() when ready with syncing
 */
class ManipulatorPart : public KParts::Part
{
    Q_OBJECT
  public:
    /**
     * The simple constructor
     * @param parent The parent
     * @param name The name of the manipulator part
     */
    ManipulatorPart(QObject *parent = 0, const char *name  = 0 );
    virtual ~ManipulatorPart();

    /**
     * @return the type of this part
     * for example like "Addressbook"
     */
    virtual QString type() const = 0;

    /**
     * @return the progress made 0-100
     */
    virtual int syncProgress() const;

    /**
     * the sync status
     */
    virtual int syncStatus() const;

    /**
     * @return a user translatable string
     */
    virtual QString name() const = 0;

    /**
     * @return a short description
     */
    virtual QString description() const = 0;

    /**
     * @return a pixmap for this part
     */
    virtual QPixmap *pixmap() = 0;

    /**
     * return a iconName
     */
    virtual QString iconName() const = 0;

    /**
     * if the partIsVisible
     */
    virtual bool partIsVisible() const;

    /**
     * if the config part is visible
     */
    virtual bool configIsVisible() const;

    /**
     * @return if the part canSync data :)
     */
    virtual bool canSync() const;

    /**
     * @return a config widget. Always create a new one
     * the ownership will be transferred
     */
    virtual QWidget *configWidget();

    /**
     * if you want to sync implement that method
     * After successfully syncing you need to call done()
     * which will emit a signal
     * @param in The Syncee List coming from a KonnectorPlugin
     * @param out The Syncee List which will be written to the Konnector
     */
    virtual void sync( const Syncee::PtrList& in, Syncee::PtrList& out );

  protected:

    /**
     * See if the user wants to be asked before writing
     * the Syncees back.
     */
    bool confirmBeforeWriting() const;

    /**
     * @return access to the shell
     */
    KSyncMainWindow *core();
    KSyncMainWindow *core() const;

    /**
     * call this whenever you make progress
     */
    void progress( int );

  protected slots:
    void progress( const Progress& );
    void error( const Error& );
    void done();

  signals:
    // 0 - 100
    void sig_progress( ManipulatorPart*, int );
    void sig_progress( ManipulatorPart*, const Progress& );
    void sig_error( ManipulatorPart*, const Error& );
    // SYNC_START SYNC_SYNC SYNC_STOP
    void sig_syncStatus( ManipulatorPart*, int );

  protected:
    /**
     * Connect to the PartChange signal
     * @see MainWindow for the slot signature
     */
    /* ManipulatorPart* old,ManipulatorPart* ne */
    void connectPartChange( const char* slot);

    /* ManipulatorPart* part,const Progress& */
    void connectPartProgress( const char* slot );

    /* ManipulatorPart* part, const Error& */
    void connectPartError( const char* slot );

    /* const QString& udi,const Progress& */
    void connectKonnectorProgress(const char* slot );

    /* const QString& udi, const Error& */
    void connectKonnectorError( const char* slot );

    /* ManipulatorPart*,int status,int prog */
    void connectSyncProgress( const char* slot );

    /* const Profile& */
    void connectProfileChanged( const char* slot );

    /* const UDI& */
    void connectKonnectorChanged( const char* slot );

    /* const UDI&,Syncee::PtrList */
    void connectKonnectorDownloaded( const char* slot );

    /* connectStartSync */
    void connectStartSync(const char* slot);

    /* connectDoneSync */
    void connectDoneSync(const char* slot);

  public slots:
    virtual void slotConfigOk();

  private:
    KSyncMainWindow *m_window;
    int m_prog;
    int m_stat;
};

}

#endif
