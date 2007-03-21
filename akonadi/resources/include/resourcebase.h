/*
    This file is part of akonadiresources.

    Copyright (c) 2006 Till Adam <adam@kde.org>
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef AKONADI_RESOURCEBASE_H
#define AKONADI_RESOURCEBASE_H

#include <QtCore/QObject>
#include <QtCore/QSettings>
#include <QtCore/QString>

#include <kdepim_export.h>

#include <libakonadi/collection.h>
#include <libakonadi/job.h>

#include "resource.h"

class KJob;

namespace Akonadi {

class Item;
class Job;
class Session;

/**
 * This class should be used as subclass by all resource agents
 * since it encapsulates large parts of the protocol between
 * resource agent, agent manager and storage.
 *
 * It provides many convenience methods to make implementing a
 * new akonadi resource agent as simple as possible.
 *
 * <h4>Synchronizing Scheme</h4>
 *
 * The following process is started by calling synchronize().
 *
 * 1) Synchronize collection tree:
 *   - retrieveCollections() is called, retrieve collection tree from
 *     backend if necessary.
 *   - convert collection tree from backend specific format into
 *     Collection objects. Make sure to add a remote id to identify
 *     collection on the backend as well as the remote identifier of
 *     the parent collection.
 *   - call collectionsRetrieved() and provide the list of collections
 *
 * 2) Synchronize a single collection
 *   - TODO
 */
class AKONADI_RESOURCES_EXPORT ResourceBase : public Resource
{
  Q_OBJECT

  public:
    /**
     * This enum describes the different states the
     * resource can be in.
     */
    enum Status
    {
      Ready = 0,
      Syncing,
      Error
    };

    /**
     * Use this method in the main function of your resource
     * application to initialize your resource subclass.
     *
     * \code
     *
     *   class MyResource : public ResourceBase
     *   {
     *     ...
     *   };
     *
     *   int main( int argc, char **argv )
     *   {
     *     QCoreApplication app;
     *
     *     ResourceBase::init<MyResource>( argc, argv );
     *
     *     return app.exec();
     *   }
     *
     * \endcode
     */
    template <typename T>
    static void init( int argc = 0, char **argv = 0 )
    {
      QString id = parseArguments( argc, argv );
      new T( id );
    }

    /**
     * This method returns the current status code of the resource.
     *
     * The following return values are possible:
     *
     *  0 - Ready
     *  1 - Syncing
     *  2 - Error
     */
    virtual int status() const;

    /**
     * This method returns an i18n'ed description of the current status code.
     */
    virtual QString statusMessage() const;

    /**
     * This method returns the current progress of the resource in percentage.
     */
    virtual uint progress() const;

    /**
     * This method returns an i18n'ed description of the current progress.
     */
    virtual QString progressMessage() const;

    /**
     * This method is called whenever the resource shall show its configuration dialog
     * to the user. It will be automatically called when the resource is started for
     * the first time.
     */
    virtual void configure();

    /**
     * This method is called whenever the resource should start synchronization.
     */
    virtual void synchronize();

    /**
     * This method is used to set the name of the resource.
     */
    virtual void setName( const QString &name );

    /**
     * Returns the name of the resource.
     */
    virtual QString name() const;

    /**
     * Returns the instance identifier of this resource.
     */
    QString identifier() const;

    /**
     * This method is called when the resource is removed from
     * the system, so it can do some cleanup stuff.
     */
    virtual void cleanup() const;

    /**
     * This method is called from the crash handler, don't call
     * it manually.
     */
    void crashHandler( int signal );

    virtual bool requestItemDelivery( int uid, const QString &remoteId, int type, const QDBusMessage &msg );
    virtual bool isOnline() const;
    virtual void setOnline( bool state );

  public Q_SLOTS:
    /**
     * This method is called to quit the resource.
     *
     * Before the application is terminated @see aboutToQuit() is called,
     * which can be reimplemented to do some session cleanup (e.g. disconnecting
     * from groupware server).
     */
    void quit();

    /**
      Enables change recording. When change recording is enabled all changes are
      stored internally and replayed as soon as change recording is disabled.
      @param enable True to enable change recoding, false to disable change recording.
    */
    void enableChangeRecording( bool enable );

  protected:
    /**
      Retrieve the collection tree from the remote server and supply it via
      collectionsRetrieved().
    */
    virtual void retrieveCollections() = 0;

    /**
      Synchronize the given collection with the backend.
      @param collection The collection to sync.
    */
    virtual void synchronizeCollection( const Collection &collection ) = 0;

    /**
     * Creates a base resource.
     *
     * @param id The instance id of the resource.
     */
    ResourceBase( const QString & id );

    /**
     * Destroys the base resource.
     */
    ~ResourceBase();

    /**
     * This method shall be used to report warnings.
     */
    void warning( const QString& message );

    /**
     * This method shall be used to report errors.
     */
    void error( const QString& message );

    /**
     * This method shall be used to signal a state change.
     *
     * @param status The new status of the resource.
     * @param message An i18n'ed description of the status. If message
     *                is empty, the default description for the status is used.
     */
    void changeStatus( Status status, const QString &message = QString() );

    /**
     * This method shall be used to signal a progress change.
     *
     * @param progress The new progress of the resource in percentage.
     * @param message An i18n'ed description of the progress.
     */
    void changeProgress( uint progress, const QString &message = QString() );

    /**
     * This method is called whenever the application is about to
     * quit.
     *
     * Reimplement this method to do session cleanup (e.g. disconnecting
     * from groupware server).
     */
    virtual void aboutToQuit();

    /**
     * This method returns the settings object which has to be used by the
     * resource to store its configuration data.
     *
     * Don't delete this object!
     */
    QSettings* settings();

    /**
     * Returns a session for communicating with the storage backend. It should
     * be used for all jobs.
     */
    Session* session();

    /**
     * This method is called whenever an external query for putting data in the
     * storage is received. Must be reimplemented in any resource.
     *
     * @param ref The DataReference of this item.
     * @param remoteId The remote identifier of the item that is requested.
     * @param type The type of the data that shall be put, either a full object or
     *             just a lightweight version.
     * @param msg QDBusMessage to pass along for delayed reply.
     */
    virtual bool requestItemDelivery( const DataReference &ref, int type, const QDBusMessage &msg ) = 0;

    /**
      Call this method from in requestItemDelivery(). It will generate an appropriate
      D-Bus reply as soon as the given job has finished.
      @param job The job which actually delivers the item.
      @param msg The D-Bus message requesting the delivery.
    */
    bool deliverItem( Akonadi::Job* job, const QDBusMessage &msg );

    /**
      Reimplement to handle adding of new items.
      @param item The newly added item.
    */
    virtual void itemAdded( const Item &item ) { Q_UNUSED( item ); }

    /**
      Reimplement to handle changes to existing items.
      @param item The changed item.
    */
    virtual void itemChanged( const Item &item ) { Q_UNUSED( item ); }

    /**
      Reimplement to handle deletion of items.
      @param ref DataReference to the deleted item.
    */
    virtual void itemRemoved( const DataReference &ref ) { Q_UNUSED( ref ); }

    /**
      Resets the dirty flag of the given item and updates the remote id.
      Call whenever you have successfully written changes back to the server.
      @param ref DataReference of the item.
    */
    void changesCommitted( const DataReference &ref );

    /**
      Call this to supply the folder tree retrieved from the remote server.
      @param collections A list of collections.
    */
    void collectionsRetrieved( const Collection::List &collections );

  private:
    static QString parseArguments( int, char** );

  private Q_SLOTS:
    void slotDeliveryDone( KJob* job );

    void slotItemAdded( const Akonadi::Item &item );
    void slotItemChanged( const Akonadi::Item &item );
    void slotItemRemoved( const Akonadi::DataReference &reference );

    void slotReplayNextItem();
    void slotReplayItemAdded( KJob *job );
    void slotReplayItemChanged( KJob *job );

    void slotCollectionSyncDone( KJob *job );
    void slotLocalListDone( KJob *job );
    void slotSyncNextCollection();

  private:
    class Private;
    Private* const d;
};

}

#endif
