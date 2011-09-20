/*
 * kmail: KDE mail client
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef FILTERMANAGER_H
#define FILTERMANAGER_H

#include <akonadi/collection.h>
#include <akonadi/item.h>

namespace MailCommon {
class MailFilter;
}

class FilterManager: public QObject
{
  Q_OBJECT

  public:
    /**
     * Describes the list of filters.
     */
    enum FilterSet
    {
      NoSet = 0x0,
      Inbound = 0x1,
      Outbound = 0x2,
      Explicit = 0x4,
      BeforeOutbound = 0x8,
      All = Inbound|BeforeOutbound|Outbound|Explicit
    };

    /**
     * Creates a new filter manager.
     *
     * @param parent The parent object.
     */
    FilterManager( QObject *parent = 0 );

    /**
     * Destroys the filter manager.
     */
    virtual ~FilterManager();

    /**
     * Clears the list of filters and deletes them.
     */
    void clear();

    /**
     * Reloads the filter rules from config file.
     */
    void readConfig();

    /**
     * Checks for existing filters with the @p name and extend the
     * "name" to "name (i)" until no match is found for i=1..n
     */
    QString createUniqueName( const QString &name ) const;

    /**
     * Process given message item by applying the filter rules one by
     * one. You can select which set of filters (incoming or outgoing)
     * should be used.
     *
     *  @param item The message item to process.
     *  @param set Select the filter set to use.
     *  @param account @c true if an account id is specified else @c false
     *  @param accountId The id of the KMAccount that the message was retrieved from
     *
     *  @return 2 if a critical error occurred (eg out of disk space)
     *          1 if the caller is still owner of the message and
     *          0 otherwise. If the caller does not any longer own the message
     *                       he *must* not delete the message or do similar stupid things. ;-)
     */
    int process( const Akonadi::Item &item, FilterSet set = Inbound,
                 bool account = false, const QString &accountId = QString() );

    /**
     * For ad-hoc filters.
     * 
     * Applies @p filter to message @p item.
     * Return codes are as with the above method.
     */
    int process( const Akonadi::Item &item, const MailCommon::MailFilter *filter );

    void filter( qlonglong itemId, FilterSet set, const QString &accountId );
    void filter( qlonglong itemId, const QString &filterId );

    /**
     * Applies the filters on the given @p messages.
     */
    void applyFilters( const QList<Akonadi::Item> &messages, FilterSet set = Explicit );

    /**
     * Returns whether the configured filters need the full mail content.
     */
    bool requiresFullMailBody() const;

#ifndef NDEBUG
    /**
     * Outputs all filter rules to console. Used for debugging.
     */
    void dump() const;
#endif

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the filter list has been updated.
     */
    void filterListUpdated();

    /**
     * This signal is emitted to notify that @p item has not been moved.
     */
    void itemNotMoved( const Akonadi::Item &item );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void itemsFetchJobForFilterDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void itemFetchJobForFilterDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void moveJobResult( KJob* ) )
    Q_PRIVATE_SLOT( d, void modifyJobResult( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotItemsFetchedForFilter( const Akonadi::Item::List& ) )
    //@endcond
};

#endif