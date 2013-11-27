/*
 *     Copyright (C) 2012 by Sérgio Martins <iamsergio@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the
 *      Free Software Foundation, Inc.
 *      51 Franklin Street, Fifth Floor
 *      Boston, MA  02110-1301  USA.
 *
 */

#include "kttcalendar.h"

#include "akonadistorage.h"
#include <KCalCore/MemoryCalendar>
#include <KCalCore/ICalFormat>

#include <KDateTime>
#include <KDirWatch>
#include <KDebug>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemFetchJob>
#include <QEventLoop>

using namespace KCalCore;
using namespace KTimeTracker;

class KTTCalendar::Private {
public:
  Private()
  {
  }
  Akonadi::Collection m_collection;
  QWeakPointer<KTTCalendar> m_weakPtr;
  KCalCore::AkonadiStorage::Ptr m_akonadiStorage;
};

// TODO pass akonadi collection
//KTTCalendar::KTTCalendar( const QString &filename,
//                          bool monitorFile ) : KCalCore::MemoryCalendar( KDateTime::LocalZone )
//                                             , d( new Private( filename ) )
KTTCalendar::KTTCalendar() : KCalCore::MemoryCalendar( KDateTime::LocalZone )
                                             , d( new Private() )
{
  // TODO
  // if ( monitorFile ) {
  //   connect( KDirWatch::self(), SIGNAL(dirty(QString)), SIGNAL(calendarChanged()) );
  // }
}

KTTCalendar::~KTTCalendar()
{
  delete d;
}

bool KTTCalendar::reload()
{
  deleteAllTodos();

  // TODO: For now, we are just picking the first Todo collection
  // this should be replaced by a configuration option
  Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive, this);
  job->fetchScope().setContentMimeTypes(QStringList() << "application/x-vnd.akonadi.calendar.todo");

  fetchResult = false;
  connect(job, SIGNAL(collectionsReceived(const Akonadi::Collection::List&)),
         this, SLOT(collectionsReceived(const Akonadi::Collection::List&)));
  connect(job, SIGNAL(result(KJob*)), this, SLOT(fetchJobResult(KJob*)));
  QEventLoop loop;
  connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
  loop.exec();
  disconnect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
  return fetchResult;
}

void KTTCalendar::collectionsReceived(const Akonadi::Collection::List &collections)
{
  d->m_collection = collections[0];

  KTTCalendar::Ptr calendar = weakPointer().toStrongRef();
  KCalCore::AkonadiStorage::Ptr akonadiStorage = AkonadiStorage::Ptr( new AkonadiStorage( calendar, d->m_collection ) );

  fetchResult = akonadiStorage->load();
}

void KTTCalendar::fetchJobResult(KJob* job)
{
  fetchResult = job->error();
}

QWeakPointer<KTTCalendar> KTTCalendar::weakPointer() const
{
  return d->m_weakPtr;
}

void KTTCalendar::setWeakPointer(const QWeakPointer<KTTCalendar> &ptr )
{
  d->m_weakPtr = ptr;
}

/** static */
// TODO pass akonadi collection
KTTCalendar::Ptr KTTCalendar::createInstance()
{
  KTTCalendar::Ptr calendar( new KTTCalendar() );
  calendar->setWeakPointer( calendar.toWeakRef() );
  return calendar;
}

/** static */
bool KTTCalendar::save()
{
  KTTCalendar::Ptr calendar = weakPointer().toStrongRef();
  AkonadiStorage::Ptr akonadiStorage = AkonadiStorage::Ptr( new AkonadiStorage( calendar, d->m_collection ) );

  const bool result = akonadiStorage->save();
  if ( !result )
    kError() << "KTTCalendar::save: problem saving calendar";
  return result;
}
