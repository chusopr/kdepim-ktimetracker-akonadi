/*
    This file is part of KitchenSync.

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kgenericfactory.h>
#include <kdebug.h>
#include <klocale.h>

#include <synchistory.h>

#include <konnectorinfo.h>

#include <calendarsyncee.h>

#include "dummykonnector.h"

using namespace KSync;
using namespace KCal;

class DummyKonnectorFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new DummyKonnector( config );
    }

    KRES::ConfigWidget *configWidget( QWidget * )
    {
      return 0;
    }
};

extern "C"
{
  void *init_libdummykonnector()
  {
    KGlobal::locale()->insertCatalogue( "konnector_dummy" );
    return new DummyKonnectorFactory();
  }
}


DummyKonnector::DummyKonnector( const KConfig *config )
    : Konnector( config )
{
  Event *event = new Event;
  event->setSummary( "An Event" );
  mCalendar.addEvent( event );

  event = new Event;
  event->setSummary( "Another Event" );
  mCalendar.addEvent( event );

  mCalSyncee = new CalendarSyncee( &mCalendar );

  /* apply sync information */
  CalendarSyncHistory syncHistory( mCalSyncee, storagePath() + "/dummy-calendar.log" );
  syncHistory.load();

  mSyncees.append( mCalSyncee );
}

DummyKonnector::~DummyKonnector()
{
}

SynceeList DummyKonnector::syncees()
{
  return mSyncees;
}

bool DummyKonnector::readSyncees()
{
  emit synceesRead( this );

  return true;
}

bool DummyKonnector::connectDevice()
{
  return true;
}

bool DummyKonnector::disconnectDevice()
{
  return true;
}

KSync::KonnectorInfo DummyKonnector::info() const
{
  return KonnectorInfo( i18n("Dummy Konnector"),
                        QIconSet(),
                        "agenda", // icon name
                        false );
}

bool DummyKonnector::writeSyncees()
{

  CalendarSyncHistory syncHistory( mCalSyncee, storagePath() + "/dummy-calendar.log" );
  syncHistory.save();


  emit synceesWritten( this );
  return true;
}


#include "dummykonnector.moc"
