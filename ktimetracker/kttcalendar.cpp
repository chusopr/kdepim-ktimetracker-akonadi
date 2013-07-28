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

#include <KCalCore/FileStorage>
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
  Private( const QString &filename ) : m_filename( filename )
  {
  }
  QString m_filename;
  QWeakPointer<KTTCalendar> m_weakPtr;
  KCalCore::FileStorage::Ptr m_fileStorage;
};

KTTCalendar::KTTCalendar( const QString &filename,
                          bool monitorFile ) : KCalCore::MemoryCalendar( KDateTime::LocalZone )
                                             , d( new Private( filename ) )
{
  // TODO
  if ( monitorFile ) {
    connect( KDirWatch::self(), SIGNAL(dirty(QString)), SIGNAL(calendarChanged()) );
    if ( !KDirWatch::self()->contains( filename ) ) {
      KDirWatch::self()->addFile( filename );
    }
  }
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

  connect(job, SIGNAL(collectionsReceived(const Akonadi::Collection::List&)),
         this, SLOT(collectionsReceived(const Akonadi::Collection::List&)));
  connect(job, SIGNAL(result(KJob*)), this, SLOT(fetchJobResult(KJob*)));
  QEventLoop loop;
  connect(this, SIGNAL(itemsAttached()), &loop, SLOT(quit()));
  loop.exec();
  disconnect(this, SIGNAL(itemsAttached()), &loop, SLOT(quit()));
  return (fetchResult == 0);
}

void KTTCalendar::collectionsReceived(const Akonadi::Collection::List &collections)
{
  collection = collections[0];
  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(collection, this);
  job->fetchScope().fetchFullPayload(true);
  connect(job, SIGNAL(itemsReceived(const Akonadi::Item::List&)), this, SLOT(itemsReceived(const Akonadi::Item::List&)));
  connect(job, SIGNAL(result(KJob*)), this, SLOT(fetchJobResult(KJob*)));
}

void KTTCalendar::fetchJobResult(KJob* job)
{
  fetchResult = job->error();
  if (fetchResult != 0)
  {
    emit itemsAttached();
    return;
  }
}

void KTTCalendar::itemsReceived(const Akonadi::Item::List& items)
{
  for (QList<Akonadi::Item>::const_iterator todo = items.begin(); todo != items.end(); todo++)
    this->addTodo(todo->payload<KCalCore::Todo::Ptr>());
  emit itemsAttached();
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
KTTCalendar::Ptr KTTCalendar::createInstance( const QString &filename, bool monitorFile )
{
  KTTCalendar::Ptr calendar( new KTTCalendar( filename, monitorFile ) );
  calendar->setWeakPointer( calendar.toWeakRef() );
  return calendar;
}

/** static */
bool KTTCalendar::save()
{
  KTTCalendar::Ptr calendar = weakPointer().toStrongRef();
  FileStorage::Ptr fileStorage = FileStorage::Ptr( new FileStorage( calendar,
                                                                    d->m_filename,
                                                                    new ICalFormat() ) );

  const bool result = fileStorage->save();
  if ( !result )
    kError() << "KTTCalendar::save: problem saving calendar";
  return result;
}
