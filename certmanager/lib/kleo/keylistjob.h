/*
    keylistjob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifndef __KLEO_KEYLISTJOB_H__
#define __KLEO_KEYLISTJOB_H__

#include "job.h"

namespace GpgME {
  class Error;
  class Key;
  class KeyListResult;
}

class QStringList;

namespace Kleo {

  /**
     @short An abstract base class for asynchronous key listers

     To use a KeyListJob, first obtain an instance from the
     CryptoBackend implementation, connect the nextKey(), progress()
     and result() signals to suitable slots and then start the key
     listing with a call to start(). This call might fail, in which
     case the KeylistJob instance will have schedules it's own
     destruction with a call to QObject::deleteLater().

     During keylisting, you will receive new key objects through the
     nextKey() signal as they arrive. After result() is emitted, the
     KeyListJob will schedule it's own destruction by calling
     QObject::deleteLater().
  */
  class KeyListJob : public Job {
    Q_OBJECT
  protected:
    KeyListJob( QObject * parent, const char * name );
    ~KeyListJob();

  public:
    /**
       Starts the keylist operation. \a pattern is a list of patterns
       used to restrict the list of keys returned. Empty patterns are
       ignored. If \a pattern is empty or contains only empty strings,
       all keys are returned (however, the backend is free to truncate
       the result and should do so; when this happens, it will be
       reported by the reult object).

       If \a secretOnly is true, only keys for which the secret key is
       also available are returned. Use this if you need to select a
       key for signing.
    */
    virtual GpgME::Error start( const QStringList & patterns, bool secretOnly=false ) = 0;

  signals:
    void nextKey( const GpgME::Key & key );
    void result( const GpgME::KeyListResult & result );
  };

}

#endif // __KLEO_KEYLISTJOB_H__
