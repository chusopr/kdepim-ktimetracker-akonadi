/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "resourcekolab.h"

#include <knotes/resourcemanager.h>

#include <libkcal/icalformat.h>

#include <kdebug.h>
#include <kglobal.h>

using namespace Kolab;


class KolabFactory : public KRES::PluginFactoryBase
{
public:
  KRES::Resource *resource( const KConfig *config )
  {
    return new ResourceKolab( config );
  }

  KRES::ConfigWidget *configWidget( QWidget* )
  {
    return 0;
  }
};

extern "C"
{
  void *init_knotes_kolab()
  {
    return ( new KolabFactory() );
  }
}


ResourceKolab::ResourceKolab( const KConfig *config )
  : ResourceNotes( config ), ResourceKolabBase( "ResourceIMAP-KNotes" )
{
}

ResourceKolab::~ResourceKolab()
{
}

bool ResourceKolab::doOpen()
{
  KConfig config( configFile() );

  // Read the calendar entries
  QMap<QString, bool> resources;
  if ( !kmailSubresources( resources, "Kolab/Note" ) )
    return false;
  config.setGroup( "Note" );
  QMap<QString, bool>::ConstIterator it;
  mResources.clear();
  for ( it = resources.begin(); it != resources.end(); ++it ) {
    const QString resource = it.key();
    const bool active = config.readBoolEntry( it.key(), true );
    const bool writable = it.data();
    mResources[ resource ] = Kolab::SubResource( active, writable, resource );
  }

  return true;
}

void ResourceKolab::doClose()
{
  KConfig config( configFile() );
  config.setGroup( "Note" );
  Kolab::ResourceMap::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it )
    config.writeEntry( it.key(), it.data().active() );
}

bool ResourceKolab::loadResource( const QString& resource )
{
  // Get the list of journals
  QMap<Q_UINT32, QString> lst;
  if( !kmailIncidences( lst, "Kolab/Note", resource ) ) {
    kdError(5500) << "Communication problem in "
                  << "ResourceKolab::getIncidenceList()\n";
    return false;
  }

  // Populate the calendar with the new entries
  const bool silent = mSilent;
  mSilent = true;
  QMap<Q_UINT32, QString>::Iterator it;
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    KCal::Journal* journal = parseJournal( *it );
    if( journal )
      addNote( journal, resource );
  }
  mSilent = silent;

  return true;
}

bool ResourceKolab::load()
{
  // We get a fresh list of events, so clean out the old ones
  mCalendar.deleteAllEvents();
  mUidmap.clear();

  bool rc = true;
  Kolab::ResourceMap::ConstIterator itR;
  for ( itR = mResources.begin(); itR != mResources.end(); ++itR ) {
    if ( !itR.data().active() )
      // This resource is disabled
      continue;

    rc &= loadResource( itR.key() );
  }

  return rc;
}

bool ResourceKolab::save()
{
  return true;
}

bool ResourceKolab::addNote( KCal::Journal* journal )
{
  return addNote( journal, QString::null );
}

bool ResourceKolab::addNote( KCal::Journal* journal,
                             const QString& subresource )
{
  kdDebug(5500) << "ResourceKolab::addNote( KCal::Journal* )\n";

  mCalendar.addJournal( journal );
  manager()->registerNote( this, journal );
  journal->registerObserver( this );

  QString resource = subresource;
  if ( subresource.isEmpty() )
    resource = findWritableResource( mResources );
  mUidmap[ journal->uid() ] = resource;

  if ( mSilent ) return true;

#if 0
  // TODO: Enable saving
  KCal::ICalFormat format;
  QString note = format.toICalString( journal );
  if( !kmailAddIncidence( "Kolab/Note", resource, journal->uid(), note ) ) {
    kdError(5500) << "Communication problem in ResourceKolab::addNote()\n";
    return false;
  }
#endif

  return true;
}

bool ResourceKolab::deleteNote( KCal::Journal* journal )
{
#if 0
  const QString uid = journal->uid();
  kmailDeleteIncidence( "Kolab/Note", mUidmap[ uid ] );
  mUidmap.remove( uid );
  mCalendar.deleteJournal( journal );
#endif
  return true;
}

void ResourceKolab::incidenceUpdated( KCal::IncidenceBase* i )
{
#if 0
  KCal::ICalFormat format;
  QString note = format.toICalString( static_cast<KCal::Journal*>( i ) );
  if( !kmailUpdate( "Kolab/Note", mUidmap[ i->uid() ], i->uid(), note ) )
    kdError(5500) << "Communication problem in ResourceKolab::addNote()\n";
#endif
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool ResourceKolab::fromKMailAddIncidence( const QString& type,
                                           const QString& resource,
                                           const QString& note )
{
  // Check if this is a note
  if( type != "Kolab/Note" ) return false;

  // kdDebug(5500) << "ResourceKolab::addIncidence( " << type << ", "
  //               << /*ical*/"..." << " )" << endl;
  KCal::Journal* journal = parseJournal( note );
  if ( !journal ) return false;

  const bool silent = mSilent;
  mSilent = true;
  addNote( journal, resource );
  mSilent = silent;

  return true;
}

void ResourceKolab::fromKMailDelIncidence( const QString& type,
                                           const QString& /*resource*/,
                                           const QString& note )
{
#if 0
  // Check if this is a note
  if( type != "Kolab/Note" ) return;

  // kdDebug(5500) << "ResourceKolab::deleteIncidence( " << type << ", " << uid
  //               << " )" << endl;

  const bool silent = mSilent;
  mSilent = true;
  KCal::Journal* j = mCalendar.journal( uid );
  if( j ) deleteNote( j );
  mSilent = silent;
#endif
}

void ResourceKolab::slotRefresh( const QString& type,
                                 const QString& /*resource*/ )
{
  if ( type == "Kolab/Note" )
    load();
}

void ResourceKolab::fromKMailAddSubresource( const QString& type,
                                             const QString& resource,
                                             bool writable )
{
  if ( type != "Kolab/Note" )
    // Not ours
    return;

  if ( mResources.contains( resource ) )
    // Already registered
    return;

  KConfig config( configFile() );
  config.setGroup( "Note" );

  bool active = config.readBoolEntry( resource, true );
  mResources[ resource ] = Kolab::SubResource( active, writable, resource );
  loadResource( resource );
  emit signalSubresourceAdded( this, type, resource );
}

void ResourceKolab::fromKMailDelSubresource( const QString& type,
                                             const QString& resource )
{
  if ( type != "Kolab/Note" )
    // Not ours
    return;

  if ( !mResources.contains( resource ) )
    // Not registered
    return;

  // Ok, it's our job, and we have it here
  mResources.erase( resource );

  KConfig config( configFile() );
  config.setGroup( "Kolab/Note" );
  config.deleteEntry( resource );
  config.sync();

  // Make a list of all uids to remove
  QMap<QString, QString>::ConstIterator mapIt;
  QStringList uids;
  for ( mapIt = mUidmap.begin(); mapIt != mUidmap.end(); ++mapIt )
    if ( mapIt.data() == resource )
      // We have a match
      uids << mapIt.key();

  // Finally delete all the incidences
  if ( !uids.isEmpty() ) {
    QStringList::ConstIterator it;
    for ( it = uids.begin(); it != uids.end(); ++it ) {
      KCal::Journal* j = mCalendar.journal( *it );
      if( j ) deleteNote( j );
      mUidmap.remove( *it );
    }
  }

  emit signalSubresourceRemoved( this, type, resource );
}

QStringList ResourceKolab::subresources() const
{
  return mResources.keys();
}

bool ResourceKolab::subresourceActive( const QString& res ) const
{
  if ( mResources.contains( res ) ) {
    return mResources[ res ].active();
  }

  // Safe default bet:
  kdDebug(5650) << "subresourceActive( " << res << " ): Safe bet\n";

  return true;
}


KCal::Journal* ResourceKolab::parseJournal( const QString& str )
{
  KCal::ICalFormat format;
  KCal::Incidence* i = format.fromString( str );
  if ( i ) {
    if ( i->type() == "Journal" )
      return static_cast<KCal::Journal*>( i );
    else {
      kdDebug(5500) << "Unknown incidence type " << i->type() << endl;
      delete i;
    }
  } else
    kdDebug(5500) << "Parse error\n";

  return 0;
}


#include "resourcekolab.moc"
