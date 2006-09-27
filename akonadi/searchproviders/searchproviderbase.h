/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_SEARCHPROVIDER_BASE_H
#define AKONADI_SEARCHPROVIDER_BASE_H

#include <searchprovider.h>

namespace Akonadi {

class SearchProviderThread;
class SearchProviderBasePrivate;

/**
  This is a convenience base class for search providers which provides
  threading support.
*/
class SearchProviderBase : public SearchProvider
{
  Q_OBJECT

  public:
    SearchProviderBase();
    virtual ~SearchProviderBase();

    virtual int search( const QString &targetCollection, const QString &searchQuery );
    virtual int fetchResponse( const QList<int> uids, const QString &field );

    /**
      Returns a worker thread. You need to reimplement this method and return an
      object derived from SearchProviderThread.
    */
    virtual SearchProviderThread* workerThread( int ticket ) = 0;

    /**
     * Use this method in the main function of your search provider
     * application to initialize your search provider subclass.
     *
     * \code
     *
     *   class MySearchProvider : public SearchProviderBase
     *   {
     *     ...
     *   };
     *
     *   int main( int argc, char **argv )
     *   {
     *     QCoreApplication app;
     *     ResourceBase::init<MySearchProvider>( argc, argv );
     *     return app.exec();
     *   }
     *
     * \endcode
     */
    template <typename T> static void init( int argc, char **argv )
    {
      new T();
    }


  private slots:
    void slotThreadFinished( SearchProviderThread* thread );

  private:
    SearchProviderBasePrivate* const d;
};

}

#endif
