/*  -*- mode: C++; c-file-style: "gnu" -*-
    gnupgprocessbase.h

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

#ifndef __KLEO_GNUPGPROCESSBASE_H__
#define __KLEO_GNUPGPROCESSBASE_H__

#include <kprocess.h>

namespace Kleo {

  /**
   * @short a base class for GPG and GPGSM processes.
   *
   * This KProcess subclass implements the status-fd handling common
   * to GPG and GPGSM.
   *
   * @author Marc Mutz <mutz@kde.org>
   */
  class GnuPGProcessBase : public KProcess {
    Q_OBJECT
  public:
    GnuPGProcessBase( QObject * parent=0, const char * name=0 );
    ~GnuPGProcessBase();

    void setUseStatusFD( bool use );

    /*! \reimp */
    bool start( RunMode runmode, Communication comm );

    bool closeStatus();

  signals:
    void receivedStatus( Kleo::GnuPGProcessBase * proc, char * buffer, int buflen );

  protected:
    /* !\reimp */
    int setupCommunication( Communication comm );
    /* !\reimp */
    int commSetupDoneP();
    /* !\reimp */
    int commSetupDoneC();

    int childStatus( int fd );
    

  private slots:
    void slotChildStatus( int fd );

  private:
    class Private;
    Private * d;
  protected:
    /*! \reimp */
    void virtual_hook( int id, void * data );
  };

}

#endif // __KLEO_GNUPGPROCESSBASE_H__
