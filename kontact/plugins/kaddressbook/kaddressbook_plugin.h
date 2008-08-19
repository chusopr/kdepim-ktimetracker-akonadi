/*
  This file is part of Kontact.

  Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KADDRESSBOOK_PLUGIN_H
#define KADDRESSBOOK_PLUGIN_H

#include <kontactinterfaces/plugin.h>
#include <kontactinterfaces/uniqueapphandler.h>

#include <kparts/part.h>

class QDropEvent;
class OrgKdeKAddressbookCoreInterface;

class KABUniqueAppHandler : public Kontact::UniqueAppHandler
{
  public:
    KABUniqueAppHandler( Kontact::Plugin *plugin ) : Kontact::UniqueAppHandler( plugin ) {}
    virtual void loadCommandLineOptions();
    virtual int newInstance();
};

class KAddressbookPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    KAddressbookPlugin( Kontact::Core *core, const QVariantList & );
    ~KAddressbookPlugin();

    virtual bool createDBUSInterface( const QString &serviceType );
    virtual bool isRunningStandalone();
    virtual QString tipFile() const;
    int weight() const { return 300; }

    bool canDecodeMimeData( const QMimeData * );
    void processDropEvent( QDropEvent * );

    virtual QStringList configModules() const;

    virtual QStringList invisibleToolbarActions() const;

    virtual void configUpdated();

    OrgKdeKAddressbookCoreInterface *interface();

    //override
    void loadProfile( const QString& directory );

    //override
    void saveToProfile( const QString& directory ) const;

  protected:
    KParts::ReadOnlyPart *createPart();

  private slots:
    void slotNewContact();
    void slotNewDistributionList();
    void slotSyncContacts();

  private:
    OrgKdeKAddressbookCoreInterface *m_interface;
    Kontact::UniqueAppWatcher *mUniqueAppWatcher;
};

#endif
