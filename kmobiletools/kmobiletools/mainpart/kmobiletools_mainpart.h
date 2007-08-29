/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>
   by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef _KMOBILETOOLSMAINPART_H_
#define _KMOBILETOOLSMAINPART_H_

#include <KParts/Part>
#include <KParts/StatusBarExtension>
#include <KParts/MainWindow>

#include <QStringList>
#include <QList>
#include <QHash>
#include <QMutex>

#include "deviceslist.h"

class QStackedWidget;
class QString;
class QTreeWidgetItem;
class QTreeWidget;
class QTreeView;
class QModelIndex;
class DeviceItem;
class ServiceItem;
class DeviceManager;
class KAboutData;
class KSystemTrayIcon;
class ErrorLogDialog;
class DeviceHome;
class ServiceModel;
class QProgressDialog;

namespace KMobileTools {
    class CoreService;
    class homepagePart;
}

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Marco Gulino <marco.gulino@gmail.com>
 * @author Matthias Lechner <matthias@lmme.de>
 * @version 0.5.0
 */
typedef QList<QWidget*> QWidgetList;

class kmobiletoolsMainPart : public KParts::ReadOnlyPart/*, virtual public MainIFace*/
{
    Q_OBJECT
    public:
        /**
         * Constructs a new part (@see KParts::GenericFactory)
         */
        kmobiletoolsMainPart(QWidget *parentWidget, QObject *parent, const QStringList &args=QStringList() );

        /**
         * Destructs the part
         */
        virtual ~kmobiletoolsMainPart();

        /**
         * This method is needed for @see KParts::GenericFactory
         *
         * @return about data of our part
         */
        static KAboutData *createAboutData();

        /**
         * @see KParts::ReadOnlyPart
         */
        bool openFile() { return false; }

        QTreeView *listview() { return m_treeView; }
        KSystemTrayIcon * sysTray() { return p_sysTray; }
        KParts::StatusBarExtension *statusBarExtension() { return p_statusBarExtension;}

    public Q_SLOTS:
        void loadDevicePart(const QString &deviceName, bool setActive=false);
        void configSlot( const QString& command );
        void deleteDevicePart(const QString &deviceName);
        void slotConfigNotify();

    private Q_SLOTS:
        void goHome();
        void nextPart();
        void prevPart();

        void deviceUnloaded( const QString& deviceName );
        void removeServiceWidget( const QString&, KMobileTools::CoreService* );
        void shutDownSucceeded();

        void slotQuit();
        void slotFinallyQuit();
        void slotAutoLoadDevices();

        void treeItemClicked( const QModelIndex& index );
        void treeViewContextMenu( const QPoint& position );

    private:
        void setupGUI( QWidget* parent );
        void setupActions();
        void setupDialogs();

        bool checkConfigVersion();

        /**
         * Displays the home page of the given @p deviceItem
         *
         * @param deviceItem the device item
         */
        void handleDeviceItem( DeviceItem* deviceItem );

        /**
         * Activates the service associated with the given @p serviceItem
         *
         * @param serviceItem the service item
         */
        void handleServiceItem( ServiceItem* serviceItem );

        QProgressDialog* m_shutDownDialog;

        ServiceModel* m_serviceModel;
        QMultiHash<QString,QWidget*> m_loadedWidgets;

        QStackedWidget *m_widget;
        QTreeView *m_treeView;
        KSystemTrayIcon * p_sysTray;

        KParts::StatusBarExtension *p_statusBarExtension;

        ErrorLogDialog* m_errorLogDialog;
        DeviceManager* m_deviceManager;

        QMutex m_mutex;

    signals:
        void showServiceToolBar( bool );
        void showDeviceToolBar( bool );

        void devicesUpdated();
        void deviceChanged(const QString &);
};

#endif // _KMOBILETOOLSPART_H_
