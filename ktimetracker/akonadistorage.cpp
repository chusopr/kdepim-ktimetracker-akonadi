/*
  This file is part of the kcalcore library.

  Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the AkonadiStorage class.

  @brief
  This class provides a calendar storage as an AkonadiCollection.

  @author Jesús Pérez (Chuso) \<kde@chuso.net\>
*/
#include "akonadistorage.h"
#include "kcalcore/exceptions.h"
#include "kcalcore/memorycalendar.h"
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemModifyJob>
#include <QEventLoop>

#include <KDebug>

using namespace KCalCore;

/*
  Private class that helps to provide binary compatibility between releases.
*/
//@cond PRIVATE
class KCalCore::AkonadiStorage::Private
{
  public:
    Private( const Akonadi::Collection &collection )
      : mCollection( collection )
    {}

    Akonadi::Collection mCollection;
};
//@endcond

AkonadiStorage::AkonadiStorage( const Calendar::Ptr &cal, const Akonadi::Collection &collection )
  : CalStorage( cal ),
    d( new Private( collection ) )
{
}

AkonadiStorage::~AkonadiStorage()
{
  delete d;
}

void AkonadiStorage::setCollection( const Akonadi::Collection &collection )
{
  d->mCollection = collection;
}

Akonadi::Collection AkonadiStorage::collection() const
{
  return d->mCollection;
}

bool AkonadiStorage::open()
{
  return true;
}

bool AkonadiStorage::load()
{
  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(d->mCollection, this);
  job->fetchScope().fetchFullPayload(true);
  connect(job, SIGNAL(itemsReceived(const Akonadi::Item::List&)), this, SLOT(itemsReceived(const Akonadi::Item::List&)));
  connect(job, SIGNAL(result(KJob*)), this, SLOT(fetchJobResult(KJob*)));
  QEventLoop loop;
  connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
  loop.exec();
  disconnect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
  calendar()->setProductId(d->mCollection.remoteId());
  calendar()->setModified( false );

  return (fetchResult == 0);
}

void AkonadiStorage::itemsReceived(const Akonadi::Item::List& items)
{
  for (QList<Akonadi::Item>::const_iterator item = items.begin(); item != items.end(); item++)
  {
    // TODO: same code repeated three times?
    if (item->hasPayload<KCalCore::Todo::Ptr>())
    {
      item->payload<KCalCore::Todo::Ptr>()->setUid(item->remoteId());
      // after changing uid, it has been set as dirty, reset it
      if (calendar()->addTodo(item->payload<KCalCore::Todo::Ptr>()))
        calendar()->rawTodos().last()->resetDirtyFields();
      // TODO: else
    }
    else if (item->hasPayload<KCalCore::Journal::Ptr>())
    {
      item->payload<KCalCore::Journal::Ptr>()->setUid(item->remoteId());
      // after changing uid, it has been set as dirty, reset it
      if (calendar()->addJournal(item->payload<KCalCore::Journal::Ptr>()))
        calendar()->rawJournals().last()->resetDirtyFields();
      // TODO: else
    }
    else if (item->hasPayload<KCalCore::Event::Ptr>())
    {
      item->payload<KCalCore::Event::Ptr>()->setUid(item->remoteId());
      // after changing uid, it has been set as dirty, reset it
      if (calendar()->addEvent(item->payload<KCalCore::Event::Ptr>()))
        calendar()->rawEvents().last()->resetDirtyFields();
      // TODO: else
    }
  }
  // FIXME: UIDs becoming dirty because of previous setUid()
  KCalCore::Todo::List todos = calendar()->rawTodos();
  for (KCalCore::Todo::List::iterator todo = todos.begin();
      todo != todos.end();
      todo++)
    (*todo)->resetDirtyFields();
}

void AkonadiStorage::fetchJobResult(KJob* job)
{
  fetchResult = job->error();
}

void AkonadiStorage::saveJobResult(KJob* job)
{
  saveResult = job->error();
}

bool AkonadiStorage::save()
{
  kDebug();

  if (!d->mCollection.isValid())
    return false;

  Todo::List rawTodos = calendar()->rawTodos();
  for (Todo::List::iterator todo = rawTodos.begin();
       todo != rawTodos.end();
       todo++)
  {
    // New items have an UID dirty field
    if ((*todo)->dirtyFields().count() > 0)
    {
      Akonadi::Item item;
      item.setMimeType("application/x-vnd.akonadi.calendar.todo");
      item.setPayload<Todo::Ptr>((*todo));
      item.setRemoteId((*todo)->uid());
      if ((*todo)->dirtyFields().contains(Todo::FieldUid))
      {
        Akonadi::ItemCreateJob *job;
        job = new Akonadi::ItemCreateJob(item, collection());
        connect(job, SIGNAL(result(KJob*)), this, SLOT(saveJobResult(KJob*)));
        QEventLoop loop;
        connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
        loop.exec();
        disconnect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
        disconnect(job, SIGNAL(result(KJob*)), this, SLOT(saveJobResult(KJob*)));
      }
      else
      {
        Akonadi::ItemModifyJob *job;
        job = new Akonadi::ItemModifyJob(item);
        connect(job, SIGNAL(result(KJob*)), this, SLOT(saveJobResult(KJob*)));
        QEventLoop loop;
        connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
        loop.exec();
        disconnect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
        disconnect(job, SIGNAL(result(KJob*)), this, SLOT(saveJobResult(KJob*)));
      }
      if (saveResult == false) return false;
    }
  }

  return true;
}

bool AkonadiStorage::close()
{
  return true;
}
