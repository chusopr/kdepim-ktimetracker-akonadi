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
  return (fetchResult == 0);
}

void AkonadiStorage::itemsReceived(const Akonadi::Item::List& items)
{
  Calendar::Ptr cal = calendar();
  for (QList<Akonadi::Item>::const_iterator item = items.begin(); item != items.end(); item++)
  {
    if (item->hasPayload<KCalCore::Todo::Ptr>())
      cal->addTodo(item->payload<KCalCore::Todo::Ptr>());
    else if (item->hasPayload<KCalCore::Journal::Ptr>())
      cal->addJournal(item->payload<KCalCore::Journal::Ptr>());
    else if (item->hasPayload<KCalCore::Event::Ptr>())
      cal->addEvent(item->payload<KCalCore::Event::Ptr>());
  }
}

void AkonadiStorage::fetchJobResult(KJob* job)
{
  fetchResult = job->error();
}

/*  QString productId;
  // First try the supplied format. Otherwise fall through to iCalendar, then
  // to vCalendar
  success = saveFormat() && saveFormat()->load( calendar(), d->mFileName );
  if ( success ) {
    productId = saveFormat()->loadedProductId();
  } else {
    ICalFormat iCal;

    success = iCal.load( calendar(), d->mFileName );

    if ( success ) {
      productId = iCal.loadedProductId();
    } else {
      if ( iCal.exception() ) {
        if ( iCal.exception()->code() == Exception::CalVersion1 ) {
          // Expected non vCalendar file, but detected vCalendar
          kDebug() << "Fallback to VCalFormat";
          VCalFormat vCal;
          success = vCal.load( calendar(), d->mFileName );
          productId = vCal.loadedProductId();
        } else {
          return false;
        }
      } else {
        kWarning() << "There should be an exception set.";
        return false;
      }
    }
  }

  calendar()->setProductId( productId );
  calendar()->setModified( false );

  return true;
}*/

bool AkonadiStorage::save()
{
  /*
  kDebug();
  if ( d->mFileName.isEmpty() ) {
    return false;
  }

  CalFormat *format = d->mSaveFormat ? d->mSaveFormat : new ICalFormat;

  bool success = format->save( calendar(), d->mFileName );

  if ( success ) {
    calendar()->setModified( false );
  } else {
    if ( !format->exception() ) {
      kDebug() << "Error. There should be an expection set.";
    } else {
      kDebug() << int( format->exception()->code() );
    }
  }

  if ( !d->mSaveFormat ) {
    delete format;
  }

  return success;*/
  return true;
}

bool AkonadiStorage::close()
{
  return true;
}
